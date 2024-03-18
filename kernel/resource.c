#include <linux/module.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <asm/io.h>

/*io端口和io内存时两种概念上的方法，用以支持设备驱动程序和设备之间的通信，为使得各种
不同的驱动程序彼此互不干扰，有必要事先为驱动程序分配端口和io内存范围。这确保几种设备
驱动程序不会试图访问同样的资源*/

/*初始化io端口资源根结点*/
struct resource ioport_resource =
{
	/*资源名称*/
	.name	= "PCI IO",
	/*起始地址*/
	.start	= 0,
	/*结束地址，u16位最大值*/
	.end	= IO_SPACE_LIMIT,
	/*资源标识*/
	.flags	= IORESOURCE_IO,
};
EXPORT_SYMBOL(ioport_resource);

/*初始化io内存资源根结点*/
struct resource iomem_resource =
{
	/*资源名称*/
	.name	= "PCI mem",
	/*起始地址*/
	.start	= 0,
	/*结束地址，这个是无符号字长的最大值*/
	.end	= -1,
	/*资源标识*/
	.flags	= IORESOURCE_MEM,
};
EXPORT_SYMBOL(iomem_resource);

/*定义一个保护资源操作的读写锁*/
static DEFINE_RWLOCK(resource_lock);

#ifdef CONFIG_PROC_FS
/*配置PROC_FS时可在proc虚拟文件系统中显示ioport和iomem相关信息*/

/*定义proc虚拟文件系统中io资源的最大层次*/
enum { MAX_IORES_LEVEL = 5 };

/**/
static void *r_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct resource *p = v;
	(*pos)++;
	if (p->child)
		return p->child;
	while (!p->sibling && p->parent)
		p = p->parent;
	return p->sibling;
}

/**/
static void *r_start(struct seq_file *m, loff_t *pos)
	__acquires(resource_lock)
{
	struct resource *p = m->private;
	loff_t l = 0;
	read_lock(&resource_lock);
	for (p = p->child; p && l < *pos; p = r_next(m, p, &l))
		;
	return p;
}

static void r_stop(struct seq_file *m, void *v)
	__releases(resource_lock)
{
	read_unlock(&resource_lock);
}

static int r_show(struct seq_file *m, void *v)
{
	struct resource *root = m->private;
	struct resource *r = v, *p;
	int width = root->end < 0x10000 ? 4 : 8;
	int depth;

	for (depth = 0, p = r; depth < MAX_IORES_LEVEL; depth++, p = p->parent)
		if (p->parent == root)
			break;
	seq_printf(m, "%*s%0*llx-%0*llx : %s\n",
			depth * 2, "",
			width, (unsigned long long) r->start,
			width, (unsigned long long) r->end,
			r->name ? r->name : "<BAD>");
	return 0;
}

static const struct seq_operations resource_op =
{
	.start	= r_start,
	.next	= r_next,
	.stop	= r_stop,
	.show	= r_show,
};

static int ioports_open(struct inode *inode, struct file *file)
{
	int res = seq_open(file, &resource_op);
	if (!res)
	{
		struct seq_file *m = file->private_data;
		m->private = &ioport_resource;
	}
	return res;
}

static int iomem_open(struct inode *inode, struct file *file)
{
	int res = seq_open(file, &resource_op);
	if (!res)
	{
		struct seq_file *m = file->private_data;
		m->private = &iomem_resource;
	}
	return res;
}

/*在proc虚拟文件系统中可对io端口目录执行的操作*/
static const struct file_operations proc_ioports_operations =
{
	.open		= ioports_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

/*在proc文件系统总可对iomem文件系统执行的操作*/
static const struct file_operations proc_iomem_operations =
{
	.open		= iomem_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

/*将ioport和iomem资源并入到proc虚拟文件系统中*/
static int __init ioresources_init(void)
{
	struct proc_dir_entry *entry;
	/*在proc文件系统中创建ioports根目录*/
	entry = create_proc_entry("ioports", 0, NULL);
	/*创建成功后设置可对该目录执行的操作*/
	if (entry)
		entry->proc_fops = &proc_ioports_operations;
	/*在proc文件系统中创建iomem根目录*/
	entry = create_proc_entry("iomem", 0, NULL);
	/*创建成功后设置可对该目录执行的操作*/
	if (entry)
		entry->proc_fops = &proc_iomem_operations;
	return 0;
}
__initcall(ioresources_init);

#endif /* CONFIG_PROC_FS */

/* Return the conflict entry if you can't request it */
/*遍历父节点root的子节点，中查找能满足申请结点new起止区域的空闲区，如果申请结点的起始
地址无效（起始地址大于结束地址）或申请结点的起止区间不在父节点的起止区间之内，则参数无
效，返回父节点地址，如果root子节点之间的空闲区包含申请结点的起止区间，则可以分配，并将
申请结点插入到root子节点链表中适当的位置，并返回NULL，否则，说明申请结点与root现存子节
点之间有区域重叠冲突，分配失败，返回有由冲突的子节点地址。注意！在扫描特定父节点的子节
点时，之会在一个层次上扫描兄弟结点链表，内核不会扫描更低层子节点链表*/
static struct resource * __request_resource(struct resource *root, struct resource *new)
{
	/*获取申请结点的起始地址*/
	resource_size_t start = new->start;
	/*获取申请结点的结束地址*/
	resource_size_t end = new->end;
	/*tmp指针保存当前结点，p这个二级指针指向当前结点的地址*/
	struct resource *tmp, **p;

	/*申请结点的起止地址参数无效，直接返回父节点*/
	if (end < start)
		return root;
	/*申请结点的起始地址超出父节点起始地址范围，参数无效，直接返回父节点*/
	if (start < root->start)
		return root;
	/*申请结点的结束范围超出父节点结束地址范围，参数无效，直接返回父节点*/
	if (end > root->end)
		return root;
	/*获取父节点的第一个子节点的地址*/
	p = &root->child;
	/*从父节点的第一个子节点开始查找合适的区域，如果查找结点之前的空闲区域能满足
	申请结点的需要，则分配给查找结点使用，否则，说明查找结点与申请结点之间的起始
	地址有重叠冲突，返回查找结点*/
	for (;;)
	{
		/*tmp保存查找结点，p是指向查找结点的地址*/
		tmp = *p;
		/*父节点对应的资源树为空或者申请结点所在的区域没有与任何现存区域重合，则分配。
		代码*/
		if (!tmp || tmp->start > end)
		{
			/*将申请资源的结点插入到已查到的结点之前*/
			new->sibling = tmp;
			/*p保存的tmp结点的地址，一次该语句之间建立new结点与前结点之间的关系*/
			*p = new;
			/*设置父节点*/
			new->parent = root;
			/*申请结点成功插入资源树中，返回NULL*/
			return NULL;
		}
		/*获取下一个兄弟结点的地址*/
		p = &tmp->sibling;
		/*如果查找结点的结束地址小于申请资源的结点的起始地址，则继续遍历下一个兄弟结点
		，否则，说明查找结点和申请结点起止区间冲突，直接返回查找结点*/
		if (tmp->end < start)
			continue;
		return tmp;
	}
}

/*只能释放之前已经申请的未合并资源结点，如果释放结点为已存在结点的子节点，也即释放
结点的区间是已存在结点的开头子区间，那么该释放时候问题的！*/
static int __release_resource(struct resource *old)
{
	/*tmp设置为查找结点，p设置为查找结点的地址*/
	struct resource *tmp, **p;
	/*获取待释放结点的父节点的第一个子节点的地址*/
	p = &old->parent->child;
	for (;;)
	{
		/*tmp为查找结点，p为查找结点的地址，p和tmp一前一后*/
		tmp = *p;
		/*指定的待释放结点不存在，跳出返回无效参数*/
		if (!tmp)
			break;
		/*找到待释放结点，则释放该结点*/
		if (tmp == old)
		{
			/*p赋值为当前结点的下一个兄弟结点，直接将tmp结点从单链表上删除*/
			*p = tmp->sibling;
			/*待释放结点的父节点指针设置为NULL*/
			old->parent = NULL;
			/*删除成功后返回0*/
			return 0;
		}
		/*获取查找结点的下一个兄弟结点的地址*/
		p = &tmp->sibling;
	}
	/*失败返回无效参数错误码*/
	return -EINVAL;
}

/*申请并保存I/O端口或内存资源，申请期间要持有保护资源的resource_lock锁，申请成功
时返回0，否则返回-EBUSY*/
int request_resource(struct resource *root, struct resource *new)
{
	struct resource *conflict;

	write_lock(&resource_lock);
	conflict = __request_resource(root, new);
	write_unlock(&resource_lock);
	return conflict ? -EBUSY : 0;
}

EXPORT_SYMBOL(request_resource);

/**
 * release_resource - release a previously reserved resource
 * @old: resource pointer
 */
 /*释放之前已申请的资源，操作期间应持有resource_lock锁*/
int release_resource(struct resource *old)
{
	int retval;

	write_lock(&resource_lock);
	retval = __release_resource(old);
	write_unlock(&resource_lock);
	return retval;
}

EXPORT_SYMBOL(release_resource);

#ifdef CONFIG_MEMORY_HOTPLUG
/*配置内存热插拔功能*/

/*
 * Finds the lowest memory reosurce exists within [res->start.res->end)
 * the caller must specify res->start, res->end, res->flags.
 * If found, returns 0, res is overwritten, if not found, returns -1.
 */
static int find_next_system_ram(struct resource *res)
{
	resource_size_t start, end;
	struct resource *p;

	BUG_ON(!res);

	start = res->start;
	end = res->end;
	BUG_ON(start >= end);

	read_lock(&resource_lock);
	for (p = iomem_resource.child; p ; p = p->sibling) {
		/* system ram is just marked as IORESOURCE_MEM */
		if (p->flags != res->flags)
			continue;
		if (p->start > end) {
			p = NULL;
			break;
		}
		if ((p->end >= start) && (p->start < end))
			break;
	}
	read_unlock(&resource_lock);
	if (!p)
		return -1;
	/* copy data */
	if (res->start < p->start)
		res->start = p->start;
	if (res->end > p->end)
		res->end = p->end;
	return 0;
}
int
walk_memory_resource(unsigned long start_pfn, unsigned long nr_pages, void *arg,
			int (*func)(unsigned long, unsigned long, void *))
{
	struct resource res;
	unsigned long pfn, len;
	u64 orig_end;
	int ret = -1;
	res.start = (u64) start_pfn << PAGE_SHIFT;
	res.end = ((u64)(start_pfn + nr_pages) << PAGE_SHIFT) - 1;
	res.flags = IORESOURCE_MEM | IORESOURCE_BUSY;
	orig_end = res.end;
	while ((res.start < res.end) && (find_next_system_ram(&res) >= 0)) {
		pfn = (unsigned long)(res.start >> PAGE_SHIFT);
		len = (unsigned long)((res.end + 1 - res.start) >> PAGE_SHIFT);
		ret = (*func)(pfn, len, arg);
		if (ret)
			break;
		res.start = res.end + 1;
		res.end = orig_end;
	}
	return ret;
}

#endif

/*
 * Find empty slot in the resource tree given range and alignment.
 */
static int find_resource(struct resource *root, struct resource *new,
			 resource_size_t size, resource_size_t min,
			 resource_size_t max, resource_size_t align,
			 void (*alignf)(void *, struct resource *,
					resource_size_t, resource_size_t),
			 void *alignf_data)
{
	struct resource *this = root->child;

	new->start = root->start;
	/*
	 * Skip past an allocated resource that starts at 0, since the assignment
	 * of this->start - 1 to new->end below would cause an underflow.
	 */
	if (this && this->start == 0) {
		new->start = this->end + 1;
		this = this->sibling;
	}
	for(;;) {
		if (this)
			new->end = this->start - 1;
		else
			new->end = root->end;
		if (new->start < min)
			new->start = min;
		if (new->end > max)
			new->end = max;
		new->start = ALIGN(new->start, align);
		if (alignf)
			alignf(alignf_data, new, size, align);
		if (new->start < new->end && new->end - new->start >= size - 1) {
			new->end = new->start + size - 1;
			return 0;
		}
		if (!this)
			break;
		new->start = this->end + 1;
		this = this->sibling;
	}
	return -EBUSY;
}

/**
 * allocate_resource - allocate empty slot in the resource tree given range & alignment
 * @root: root resource descriptor
 * @new: resource descriptor desired by caller
 * @size: requested resource region size
 * @min: minimum size to allocate
 * @max: maximum size to allocate
 * @align: alignment requested, in bytes
 * @alignf: alignment function, optional, called if not NULL
 * @alignf_data: arbitrary data to pass to the @alignf function
 */
int allocate_resource(struct resource *root, struct resource *new,
		      resource_size_t size, resource_size_t min,
		      resource_size_t max, resource_size_t align,
		      void (*alignf)(void *, struct resource *,
				     resource_size_t, resource_size_t),
		      void *alignf_data)
{
	int err;

	write_lock(&resource_lock);
	err = find_resource(root, new, size, min, max, align, alignf, alignf_data);
	if (err >= 0 && __request_resource(root, new))
		err = -EBUSY;
	write_unlock(&resource_lock);
	return err;
}

EXPORT_SYMBOL(allocate_resource);

/**
 * insert_resource - Inserts a resource in the resource tree
 * @parent: parent of the new resource
 * @new: new resource to insert
 *
 * Returns 0 on success, -EBUSY if the resource can't be inserted.
 *
 * This function is equivalent to request_resource when no conflict
 * happens. If a conflict happens, and the conflicting resources
 * entirely fit within the range of the new resource, then the new
 * resource is inserted and the conflicting resources become children of
 * the new resource.
 */
int insert_resource(struct resource *parent, struct resource *new)
{
	int result;
	struct resource *first, *next;

	write_lock(&resource_lock);

	for (;; parent = first) {
	 	result = 0;
		first = __request_resource(parent, new);
		if (!first)
			goto out;

		result = -EBUSY;
		if (first == parent)
			goto out;

		if ((first->start > new->start) || (first->end < new->end))
			break;
		if ((first->start == new->start) && (first->end == new->end))
			break;
	}

	for (next = first; ; next = next->sibling) {
		/* Partial overlap? Bad, and unfixable */
		if (next->start < new->start || next->end > new->end)
			goto out;
		if (!next->sibling)
			break;
		if (next->sibling->start > new->end)
			break;
	}

	result = 0;

	new->parent = parent;
	new->sibling = next->sibling;
	new->child = first;

	next->sibling = NULL;
	for (next = first; next; next = next->sibling)
		next->parent = new;

	if (parent->child == first) {
		parent->child = new;
	} else {
		next = parent->child;
		while (next->sibling != first)
			next = next->sibling;
		next->sibling = new;
	}

 out:
	write_unlock(&resource_lock);
	return result;
}

/**
 * adjust_resource - modify a resource's start and size
 * @res: resource to modify
 * @start: new start value
 * @size: new size
 *
 * Given an existing resource, change its start and size to match the
 * arguments.  Returns 0 on success, -EBUSY if it can't fit.
 * Existing children of the resource are assumed to be immutable.
 */
int adjust_resource(struct resource *res, resource_size_t start, resource_size_t size)
{
	struct resource *tmp, *parent = res->parent;
	resource_size_t end = start + size - 1;
	int result = -EBUSY;

	write_lock(&resource_lock);

	if ((start < parent->start) || (end > parent->end))
		goto out;

	for (tmp = res->child; tmp; tmp = tmp->sibling) {
		if ((tmp->start < start) || (tmp->end > end))
			goto out;
	}

	if (res->sibling && (res->sibling->start <= end))
		goto out;

	tmp = parent->child;
	if (tmp != res) {
		while (tmp->sibling != res)
			tmp = tmp->sibling;
		if (start <= tmp->end)
			goto out;
	}

	res->start = start;
	res->end = end;
	result = 0;

 out:
	write_unlock(&resource_lock);
	return result;
}

EXPORT_SYMBOL(adjust_resource);

/*
 * This is compatibility stuff for IO resources.
 *
 * Note how this, unlike the above, knows about
 * the IO flag meanings (busy etc).
 *
 * request_region creates a new busy region.
 *
 * check_region returns non-zero if the area is already busy.
 *
 * release_region releases a matching busy region.
 */

/**
 * __request_region - create a new busy resource region
 * @parent: parent resource descriptor
 * @start: resource start address
 * @n: resource region size
 * @name: reserving caller's ID string
 */
struct resource * __request_region(struct resource *parent,
				   resource_size_t start, resource_size_t n,
				   const char *name)
{
	struct resource *res = kzalloc(sizeof(*res), GFP_KERNEL);

	if (res) {
		res->name = name;
		res->start = start;
		res->end = start + n - 1;
		res->flags = IORESOURCE_BUSY;

		write_lock(&resource_lock);

		for (;;) {
			struct resource *conflict;

			conflict = __request_resource(parent, res);
			if (!conflict)
				break;
			if (conflict != parent) {
				parent = conflict;
				if (!(conflict->flags & IORESOURCE_BUSY))
					continue;
			}

			/* Uhhuh, that didn't work out.. */
			kfree(res);
			res = NULL;
			break;
		}
		write_unlock(&resource_lock);
	}
	return res;
}
EXPORT_SYMBOL(__request_region);

/**
 * __check_region - check if a resource region is busy or free
 * @parent: parent resource descriptor
 * @start: resource start address
 * @n: resource region size
 *
 * Returns 0 if the region is free at the moment it is checked,
 * returns %-EBUSY if the region is busy.
 *
 * NOTE:
 * This function is deprecated because its use is racy.
 * Even if it returns 0, a subsequent call to request_region()
 * may fail because another driver etc. just allocated the region.
 * Do NOT use it.  It will be removed from the kernel.
 */
int __check_region(struct resource *parent, resource_size_t start,
			resource_size_t n)
{
	struct resource * res;

	res = __request_region(parent, start, n, "check-region");
	if (!res)
		return -EBUSY;

	release_resource(res);
	kfree(res);
	return 0;
}
EXPORT_SYMBOL(__check_region);

/**
 * __release_region - release a previously reserved resource region
 * @parent: parent resource descriptor
 * @start: resource start address
 * @n: resource region size
 *
 * The described resource region must match a currently busy region.
 */
void __release_region(struct resource *parent, resource_size_t start,
			resource_size_t n)
{
	struct resource **p;
	resource_size_t end;

	p = &parent->child;
	end = start + n - 1;

	write_lock(&resource_lock);

	for (;;) {
		struct resource *res = *p;

		if (!res)
			break;
		if (res->start <= start && res->end >= end) {
			if (!(res->flags & IORESOURCE_BUSY)) {
				p = &res->child;
				continue;
			}
			if (res->start != start || res->end != end)
				break;
			*p = res->sibling;
			write_unlock(&resource_lock);
			kfree(res);
			return;
		}
		p = &res->sibling;
	}

	write_unlock(&resource_lock);

	printk(KERN_WARNING "Trying to free nonexistent resource "
		"<%016llx-%016llx>\n", (unsigned long long)start,
		(unsigned long long)end);
}
EXPORT_SYMBOL(__release_region);

/*
 * Managed region resource
 */
struct region_devres
{
	struct resource *parent;
	resource_size_t start;
	resource_size_t n;
};

static void devm_region_release(struct device *dev, void *res)
{
	struct region_devres *this = res;

	__release_region(this->parent, this->start, this->n);
}

static int devm_region_match(struct device *dev, void *res, void *match_data)
{
	struct region_devres *this = res, *match = match_data;

	return this->parent == match->parent &&
		this->start == match->start && this->n == match->n;
}

struct resource * __devm_request_region(struct device *dev,
				struct resource *parent, resource_size_t start,
				resource_size_t n, const char *name)
{
	struct region_devres *dr = NULL;
	struct resource *res;

	dr = devres_alloc(devm_region_release, sizeof(struct region_devres),
			  GFP_KERNEL);
	if (!dr)
		return NULL;

	dr->parent = parent;
	dr->start = start;
	dr->n = n;

	res = __request_region(parent, start, n, name);
	if (res)
		devres_add(dev, dr);
	else
		devres_free(dr);

	return res;
}
EXPORT_SYMBOL(__devm_request_region);

void __devm_release_region(struct device *dev, struct resource *parent,
			   resource_size_t start, resource_size_t n)
{
	struct region_devres match_data = { parent, start, n };

	__release_region(parent, start, n);
	WARN_ON(devres_destroy(dev, devm_region_release, devm_region_match,
			       &match_data));
}
EXPORT_SYMBOL(__devm_release_region);

/*
 * Called from init/main.c to reserve IO ports.
 */
#define MAXRESERVE 4
static int __init reserve_setup(char *str)
{
	static int reserved;
	static struct resource reserve[MAXRESERVE];

	for (;;) {
		int io_start, io_num;
		int x = reserved;

		if (get_option (&str, &io_start) != 2)
			break;
		if (get_option (&str, &io_num)   == 0)
			break;
		if (x < MAXRESERVE) {
			struct resource *res = reserve + x;
			res->name = "reserved";
			res->start = io_start;
			res->end = io_start + io_num - 1;
			res->flags = IORESOURCE_BUSY;
			res->child = NULL;
			if (request_resource(res->start >= 0x10000 ? &iomem_resource : &ioport_resource, res) == 0)
				reserved = x+1;
		}
	}
	return 1;
}

__setup("reserve=", reserve_setup);
