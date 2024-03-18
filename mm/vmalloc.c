#include <linux/mm.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>

#include <asm/uaccess.h>
#include <asm/tlbflush.h>

/*VMALOC映射区中vmalloc区域保护锁*/
DEFINE_RWLOCK(vmlist_lock);
/*保存VMALLOC映射区中第一个子vmalloc区域的地址*/
struct vm_struct *vmlist;

static void *__vmalloc_node(unsigned long size, gfp_t gfp_mask, pgprot_t prot, int node);

/**/
static void vunmap_pte_range(pmd_t *pmd, unsigned long addr, unsigned long end)
{
	pte_t *pte;

	pte = pte_offset_kernel(pmd, addr);
	do {
		pte_t ptent = ptep_get_and_clear(&init_mm, addr, pte);
		WARN_ON(!pte_none(ptent) && !pte_present(ptent));
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

/**/
static inline void vunmap_pmd_range(pud_t *pud, unsigned long addr,	unsigned long end)
{
	pmd_t *pmd;
	unsigned long next;

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (pmd_none_or_clear_bad(pmd))
			continue;
		vunmap_pte_range(pmd, addr, next);
	} while (pmd++, addr = next, addr != end);
}

/**/
static inline void vunmap_pud_range(pgd_t *pgd, unsigned long addr,	unsigned long end)
{
	pud_t *pud;
	unsigned long next;

	pud = pud_offset(pgd, addr);
	do {
		next = pud_addr_end(addr, end);
		if (pud_none_or_clear_bad(pud))
			continue;
		vunmap_pmd_range(pud, addr, next);
	} while (pud++, addr = next, addr != end);
}

/**/
void unmap_kernel_range(unsigned long addr, unsigned long size)
{
	pgd_t *pgd;
	unsigned long next;
	/*获取待删除虚拟地址空间的起始地址*/
	unsigned long start = addr;
	/*获取待删除虚拟地址空间的结束地址*/
	unsigned long end = addr + size;
	/*待删除区域起止地址必须是顺序递增的*/
	BUG_ON(addr >= end);
	/*获取内核进程pgd*/
	pgd = pgd_offset_k(addr);
	flush_cache_vunmap(addr, end);
	do
	{
		/**/
		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		vunmap_pud_range(pgd, addr, next);
	} while (pgd++, addr = next, addr != end);
	flush_tlb_kernel_range(start, end);
}

/*删除vmalloc映射区中指定的子区域*/
static void unmap_vm_area(struct vm_struct *area)
{
	/**/
	unmap_kernel_range((unsigned long)area->addr, area->size);
}

/**/
static int vmap_pte_range(pmd_t *pmd, unsigned long addr, unsigned long end, pgprot_t prot,
							struct page ***pages)
{
	pte_t *pte;

	pte = pte_alloc_kernel(pmd, addr);
	if (!pte)
		return -ENOMEM;
	do {
		struct page *page = **pages;
		WARN_ON(!pte_none(*pte));
		if (!page)
			return -ENOMEM;
		set_pte_at(&init_mm, addr, pte, mk_pte(page, prot));
		(*pages)++;
	} while (pte++, addr += PAGE_SIZE, addr != end);
	return 0;
}

/**/
static inline int vmap_pmd_range(pud_t *pud, unsigned long addr, unsigned long end, pgprot_t prot,
									struct page ***pages)
{
	pmd_t *pmd;
	unsigned long next;

	pmd = pmd_alloc(&init_mm, pud, addr);
	if (!pmd)
		return -ENOMEM;
	do {
		next = pmd_addr_end(addr, end);
		if (vmap_pte_range(pmd, addr, next, prot, pages))
			return -ENOMEM;
	} while (pmd++, addr = next, addr != end);
	return 0;
}

/**/
static inline int vmap_pud_range(pgd_t *pgd, unsigned long addr,	unsigned long end, pgprot_t prot,
				struct page ***pages)
{
	pud_t *pud;
	unsigned long next;

	pud = pud_alloc(&init_mm, pgd, addr);
	if (!pud)
		return -ENOMEM;
	do {
		next = pud_addr_end(addr, end);
		if (vmap_pmd_range(pud, addr, next, prot, pages))
			return -ENOMEM;
	} while (pud++, addr = next, addr != end);
	return 0;
}

/**/
int map_vm_area(struct vm_struct *area, pgprot_t prot, struct page ***pages)
{
	pgd_t *pgd;
	unsigned long next;
	unsigned long addr = (unsigned long) area->addr;
	unsigned long end = addr + area->size - PAGE_SIZE;
	int err;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);
	do {
		next = pgd_addr_end(addr, end);
		err = vmap_pud_range(pgd, addr, next, prot, pages);
		if (err)
			break;
	} while (pgd++, addr = next, addr != end);
	flush_cache_vmap((unsigned long) area->addr, end);
	return err;
}
EXPORT_SYMBOL_GPL(map_vm_area);

/*在指定node结点的VMALLOC映射区中[start, end]起止区域获取特定分配标识指定连续长度的
vmalloc子区域，分配物理内存页时指定分配标识是gfp_mask*/
static struct vm_struct *__get_vm_area_node(unsigned long size, unsigned long flags,
					    unsigned long start, unsigned long end,   int node, gfp_t gfp_mask)
{
	/*需要一个二级指针保存遍历过程中的当前结点的地址*/
	struct vm_struct **p;
	/*保存遍历过程中的当前结点*/
	struct vm_struct *tmp;
	/*保存虚拟地址中申请的struct vm_struct实例地址*/
	struct vm_struct *area;
	/*保存地址对齐要求，默认是按字节对齐*/
	unsigned long align = 1;
	unsigned long addr;

	/*不能处于软中断或硬中断中*/
	BUG_ON(in_interrupt());

	if (flags & VM_IOREMAP)
	{
		int bit = fls(size);

		if (bit > IOREMAP_MAX_ORDER)
			bit = IOREMAP_MAX_ORDER;
		else if (bit < PAGE_SHIFT)
			bit = PAGE_SHIFT;

		align = 1ul << bit;
	}
	/*对齐起始地址，通常是在VMALLOC映射区的起始处开始搜索*/
	addr = ALIGN(start, align);
	/*保存页对齐后的长度*/
	size = PAGE_ALIGN(size);
	/*无效长度0直接返回NULL*/
	if (unlikely(!size))
		return NULL;
	/*在指定结点上申请struct vm_struct实例*/
	area = kmalloc_node(sizeof(*area), gfp_mask & GFP_RECLAIM_MASK, node);
	/*申请失败，返回NULL*/
	if (unlikely(!area))
		return NULL;

	/*经常需要在虚拟地址空间中多分配一个放置结尾的保护页*/
	size += PAGE_SIZE;
	/**/
	write_lock(&vmlist_lock);
	/*从VMALLOC映射区中子区域链表头开始搜索，查找*/
	for (p = &vmlist; (tmp = *p) != NULL ;p = &tmp->next)
	{
		/*如果当前vmalloc子区域的起始地址小于指定地址，重新查找下一个子区域*/
		if ((unsigned long)tmp->addr < addr)
		{
			/*如果当前vmalloc子区域的结束地址大于指定地址，则重新设置指定地址为该子区
			域结束地址对齐后的地址*/
			if((unsigned long)tmp->addr + tmp->size >= addr)
				addr = ALIGN(tmp->size + (unsigned long)tmp->addr, align);
			/*继续下一个子区域查找*/
			continue;
		}
		/*当前子区域的起始地址不小于指定地址，但是申请长度与地址之和溢出，截断后回绕*/
		if ((size + addr) < addr)
			goto out;
		/*当前vmalloc子区域和前一个子区域之间的空闲区能满足申请长度，则找到的空闲区
		可以创建该子区域，跳转到找到时处理分支*/
		if (size + addr <= (unsigned long)tmp->addr)
			goto found;
		/*指定地址在当前vmalloc子区域内，因此，需要重置指定地址为该子区区域的结束地址*/
		addr = ALIGN(tmp->size + (unsigned long)tmp->addr, align);
		/*遍历到VMALLOC映射区的结尾页没有找到合适的分配区，查找失败，搜索结束*/
		if (addr > end - size)
			goto out;
	}

found:
	/*通过二级指针保存当前查找子区域描述符的地址（也即当前子区域前一子区域的next域
	地址），以下两语句就将待申请子区域按起始地址顺序添加到vmlist链表中*/
	/*设置待插入子区域的后一个区域是当前区域*/
	area->next = *p;
	/*设置当前区域前移区域的后一个区域是待插入区域。*/
	*p = area;
	/*设置该子vmalloc区域的的VM_*标识*/
	area->flags = flags;
	/*设置该子区域的起始地址，按结构体中的定义转换为通用指针类型*/
	area->addr = (void *)addr;
	/*设置该子区域的长度*/
	area->size = size;
	/*设置该子区域对应的物理内存页二级指针为空，即当前在虚拟地址VMALLOC映射区中已分
	配虚拟内存，但还没有分配对应的物理内存并建立映射，使用时因为没有建立映射，产生缺
	页中断，然后由内核分配物理内存，并建立该子区域中虚拟页与物理页的映射，才能使用*/
	area->pages = NULL;
	/*申请的虚拟内存占物理内存的数目，设置为0，目前没有分配物理内存*/
	area->nr_pages = 0;
	area->phys_addr = 0;
	write_unlock(&vmlist_lock);
	/*申请子区域成功时返回子区域的描述符*/
	return area;

/*申请VMALLOC子区域虚拟内存空间失败跳转此处*/
out:
	write_unlock(&vmlist_lock);
	/*释放已申请的内存*/
	kfree(area);
	if (printk_ratelimit())
		printk(KERN_WARNING "allocation failed: out of vmalloc space - use vmalloc=<size> to increase size.\n");
	return NULL;
}

/*从VMALLOC映射区域指定子区域内查询并获取一个特定标识（VM_ALLOC或者VM_IOREMAP）的指
定长度的连续内核虚拟区域，成功时返回子区域标识符，否则返回NULL*/
struct vm_struct *__get_vm_area(unsigned long size, unsigned long flags, unsigned long start,
					unsigned long end)
{
	return __get_vm_area_node(size, flags, start, end, -1, GFP_KERNEL);
}
EXPORT_SYMBOL_GPL(__get_vm_area);

/*从VMALLOC映射区域内查询并获取一个特定标识（VM_ALLOC或者VM_IOREMAP）的指定长度的连
续内核虚拟区域，成功时返回子区域标识符，否则返回NULL*/
struct vm_struct *get_vm_area(unsigned long size, unsigned long flags)
{
	return __get_vm_area(size, flags, VMALLOC_START, VMALLOC_END);
}

/*在VMALLOC映射区的空闲子区域中申请特定分配标识指定长度的虚拟内存*/
struct vm_struct *get_vm_area_node(unsigned long size, unsigned long flags, int node,
			gfp_t gfp_mask)
{
	return __get_vm_area_node(size, flags, VMALLOC_START, VMALLOC_END, node, gfp_mask);
}

/*获取指定地址为起始地址的vmalloc子区域。只查询区域起始地址是否为指定区域，区域内是
否包含指定区域不关心。主调函数必须持有vmlist_lock锁*/
static struct vm_struct *__find_vm_area(void *addr)
{
	struct vm_struct *tmp;
	/*从vmalooc区域的单链表头开始遍历所有子区域信息，查找指定地址*/
	for (tmp = vmlist; tmp != NULL; tmp = tmp->next)
	{
		/*如果子区域的起始地址为指定地址，则返回该子区域*/
		 if (tmp->addr == addr)
			break;
	}

	return tmp;
}

/*释放指定地址为起始地址的vmalloc子区域。主调函数必须持有vmlist_lock锁*/
static struct vm_struct *__remove_vm_area(void *addr)
{
	/*安全遍历单链表时需要一个二级指针保存当前结点的地址*/
	struct vm_struct **p;
	/*保存当前遍历结点*/
	struct vm_struct *tmp;
	/*从vmalloc单链表头开始，查询指定地址为起始地址的vmalloc子区域*/
	for (p = &vmlist ; (tmp = *p) != NULL ;p = &tmp->next)
	{
		/*找到指定地址为起始地址的vmalloc子区域，跳转*/
		 if (tmp->addr == addr)
			 goto found;
	}
	/*vmalloc映射区中没有指定地址为起始地址的子区域，则返回NULL*/
	return NULL;

found:
	/**/
	unmap_vm_area(tmp);
	/*将待删除区域的下一个子区域设置为其前一区域的下一个子区域，也就是释放待删除区域*/
	*p = tmp->next;

	/*删除保护页	 */
	tmp->size -= PAGE_SIZE;
	return tmp;
}

/**
 *	remove_vm_area  -  find and remove a continuous kernel virtual area
 *	@addr:		base address
 *
 *	Search for the kernel VM area starting at @addr, and remove it.
 *	This function returns the found VM area, but using it is NOT safe
 *	on SMP machines, except for its size or flags.
 */
/*在vmlist_lock保护下释放指定地址为起始地址的vmalloc子区域*/
struct vm_struct *remove_vm_area(void *addr)
{
	struct vm_struct *v;
	write_lock(&vmlist_lock);
	v = __remove_vm_area(addr);
	write_unlock(&vmlist_lock);
	return v;
}

/**/
static void __vunmap(void *addr, int deallocate_pages)
{
	struct vm_struct *area;

	/*地址为空直接返回*/
	if (!addr)
		return;

	/*地址不是页对齐的，打印警告信息后退出*/
	if ((PAGE_SIZE-1) & (unsigned long)addr)
	{
		printk(KERN_ERR "Trying to vfree() bad address (%p)\n", addr);
		WARN_ON(1);
		return;
	}

	/*释放指定地址为起始地址的vmalloc子区域*/
	area = remove_vm_area(addr);
	/*指定的地址不是子区域的首地址*/
	if (unlikely(!area))
	{
		printk(KERN_ERR "Trying to vfree() nonexistent vm area (%p)\n",	addr);
		WARN_ON(1);
		return;
	}

	debug_check_no_locks_freed(addr, area->size);

	if (deallocate_pages) {
		int i;

		for (i = 0; i < area->nr_pages; i++) {
			BUG_ON(!area->pages[i]);
			__free_page(area->pages[i]);
		}

		if (area->flags & VM_VPAGES)
			vfree(area->pages);
		else
			kfree(area->pages);
	}

	kfree(area);
	return;
}

/**
 *	vfree  -  release memory allocated by vmalloc()
 *	@addr:		memory base address
 *
 *	Free the virtually continuous memory area starting at @addr, as
 *	obtained from vmalloc(), vmalloc_32() or __vmalloc(). If @addr is
 *	NULL, no operation is performed.
 *
 *	Must not be called in interrupt context.
 */
/**/
void vfree(void *addr)
{
	BUG_ON(in_interrupt());
	__vunmap(addr, 1);
}
EXPORT_SYMBOL(vfree);

/**
 *	vunmap  -  release virtual mapping obtained by vmap()
 *	@addr:		memory base address
 *
 *	Free the virtually contiguous memory area starting at @addr,
 *	which was created from the page array passed to vmap().
 *
 *	Must not be called in interrupt context.
 */
/**/
void vunmap(void *addr)
{
	BUG_ON(in_interrupt());
	__vunmap(addr, 0);
}
EXPORT_SYMBOL(vunmap);

/**
 *	vmap  -  map an array of pages into virtually contiguous space
 *	@pages:		array of page pointers
 *	@count:		number of pages to map
 *	@flags:		vm_area->flags
 *	@prot:		page protection for the mapping
 *
 *	Maps @count pages from @pages into contiguous kernel virtual
 *	space.
 */
/**/
void *vmap(struct page **pages, unsigned int count,	unsigned long flags, pgprot_t prot)
{
	struct vm_struct *area;

	if (count > num_physpages)
		return NULL;

	area = get_vm_area((count << PAGE_SHIFT), flags);
	if (!area)
		return NULL;
	if (map_vm_area(area, prot, &pages)) {
		vunmap(area->addr);
		return NULL;
	}

	return area->addr;
}
EXPORT_SYMBOL(vmap);

/**/
void *__vmalloc_area_node(struct vm_struct *area, gfp_t gfp_mask,
				pgprot_t prot, int node)
{
	struct page **pages;
	unsigned int nr_pages, array_size, i;

	nr_pages = (area->size - PAGE_SIZE) >> PAGE_SHIFT;
	array_size = (nr_pages * sizeof(struct page *));

	area->nr_pages = nr_pages;
	/* Please note that the recursion is strictly bounded. */
	if (array_size > PAGE_SIZE) {
		pages = __vmalloc_node(array_size, gfp_mask | __GFP_ZERO,
					PAGE_KERNEL, node);
		area->flags |= VM_VPAGES;
	} else {
		pages = kmalloc_node(array_size,
				(gfp_mask & GFP_RECLAIM_MASK) | __GFP_ZERO,
				node);
	}
	area->pages = pages;
	if (!area->pages) {
		remove_vm_area(area->addr);
		kfree(area);
		return NULL;
	}

	for (i = 0; i < area->nr_pages; i++) {
		if (node < 0)
			area->pages[i] = alloc_page(gfp_mask);
		else
			area->pages[i] = alloc_pages_node(node, gfp_mask, 0);
		if (unlikely(!area->pages[i])) {
			/* Successfully allocated i pages, free them in __vunmap() */
			area->nr_pages = i;
			goto fail;
		}
	}

	if (map_vm_area(area, prot, &pages))
		goto fail;
	return area->addr;

fail:
	vfree(area->addr);
	return NULL;
}

/**/
void *__vmalloc_area(struct vm_struct *area, gfp_t gfp_mask, pgprot_t prot)
{
	return __vmalloc_area_node(area, gfp_mask, prot, -1);
}

/**
 *	__vmalloc_node  -  allocate virtually contiguous memory
 *	@size:		allocation size
 *	@gfp_mask:	flags for the page level allocator
 *	@prot:		protection mask for the allocated pages
 *	@node:		node to use for allocation or -1
 *
 *	Allocate enough pages to cover @size from the page level
 *	allocator with @gfp_mask flags.  Map them into contiguous
 *	kernel virtual space, using a pagetable protection of @prot.
 */
/**/
static void *__vmalloc_node(unsigned long size, gfp_t gfp_mask, pgprot_t prot, int node)
{
	struct vm_struct *area;
	/*长度页长度对齐*/
	size = PAGE_ALIGN(size);
	/*长度为零或超过整个物理内存长度时，长度无效，返回NULL*/
	if (!size || (size >> PAGE_SHIFT) > num_physpages)
		return NULL;
	/**/
	area = get_vm_area_node(size, VM_ALLOC, node, gfp_mask);
	if (!area)
		return NULL;

	return __vmalloc_area_node(area, gfp_mask, prot, node);
}

/**/
void *__vmalloc(unsigned long size, gfp_t gfp_mask, pgprot_t prot)
{
	return __vmalloc_node(size, gfp_mask, prot, -1);
}
EXPORT_SYMBOL(__vmalloc);

/**
 *	vmalloc  -  allocate virtually contiguous memory
 *	@size:		allocation size
 *	Allocate enough pages to cover @size from the page level
 *	allocator and map them into contiguous kernel virtual space.
 *
 *	For tight control over page level allocator and protection flags
 *	use __vmalloc() instead.
 */
/**/
void *vmalloc(unsigned long size)
{
	return __vmalloc(size, GFP_KERNEL | __GFP_HIGHMEM, PAGE_KERNEL);
}
EXPORT_SYMBOL(vmalloc);

/**
 * vmalloc_user - allocate zeroed virtually contiguous memory for userspace
 * @size: allocation size
 *
 * The resulting memory area is zeroed so it can be mapped to userspace
 * without leaking data.
 */
/**/
void *vmalloc_user(unsigned long size)
{
	struct vm_struct *area;
	void *ret;

	/**/
	ret = __vmalloc(size, GFP_KERNEL | __GFP_HIGHMEM | __GFP_ZERO, PAGE_KERNEL);
	if (ret)
	{
		write_lock(&vmlist_lock);
		/**/
		area = __find_vm_area(ret);
		/**/
		area->flags |= VM_USERMAP;
		write_unlock(&vmlist_lock);
	}
	return ret;
}
EXPORT_SYMBOL(vmalloc_user);

/**
 *	vmalloc_node  -  allocate memory on a specific node
 *	@size:		allocation size
 *	@node:		numa node
 *
 *	Allocate enough pages to cover @size from the page level
 *	allocator and map them into contiguous kernel virtual space.
 *
 *	For tight control over page level allocator and protection flags
 *	use __vmalloc() instead.
 */
/**/
void *vmalloc_node(unsigned long size, int node)
{
	return __vmalloc_node(size, GFP_KERNEL | __GFP_HIGHMEM, PAGE_KERNEL, node);
}
EXPORT_SYMBOL(vmalloc_node);

#ifndef PAGE_KERNEL_EXEC
#define PAGE_KERNEL_EXEC PAGE_KERNEL
#endif

/**
 *	vmalloc_exec  -  allocate virtually contiguous, executable memory
 *	@size:		allocation size
 *
 *	Kernel-internal function to allocate enough pages to cover @size
 *	the page level allocator and map them into contiguous and
 *	executable kernel virtual space.
 *
 *	For tight control over page level allocator and protection flags
 *	use __vmalloc() instead.
 */

/**/
void *vmalloc_exec(unsigned long size)
{
	return __vmalloc(size, GFP_KERNEL | __GFP_HIGHMEM, PAGE_KERNEL_EXEC);
}

#if defined(CONFIG_64BIT) && defined(CONFIG_ZONE_DMA32)
#define GFP_VMALLOC32 GFP_DMA32 | GFP_KERNEL
#elif defined(CONFIG_64BIT) && defined(CONFIG_ZONE_DMA)
#define GFP_VMALLOC32 GFP_DMA | GFP_KERNEL
#else
#define GFP_VMALLOC32 GFP_KERNEL
#endif

/**
 *	vmalloc_32  -  allocate virtually contiguous memory (32bit addressable)
 *	@size:		allocation size
 *
 *	Allocate enough 32bit PA addressable pages to cover @size from the
 *	page level allocator and map them into contiguous kernel virtual space.
 */
/**/
void *vmalloc_32(unsigned long size)
{
	return __vmalloc(size, GFP_VMALLOC32, PAGE_KERNEL);
}
EXPORT_SYMBOL(vmalloc_32);

/**
 * vmalloc_32_user - allocate zeroed virtually contiguous 32bit memory
 *	@size:		allocation size
 *
 * The resulting memory area is 32bit addressable and zeroed so it can be
 * mapped to userspace without leaking data.
 */
/**/
void *vmalloc_32_user(unsigned long size)
{
	struct vm_struct *area;
	void *ret;

	/**/
	ret = __vmalloc(size, GFP_VMALLOC32 | __GFP_ZERO, PAGE_KERNEL);
	if (ret)
	{
		write_lock(&vmlist_lock);
		/**/
		area = __find_vm_area(ret);
		/**/
		area->flags |= VM_USERMAP;
		write_unlock(&vmlist_lock);
	}
	return ret;
}
EXPORT_SYMBOL(vmalloc_32_user);

/**/
long vread(char *buf, char *addr, unsigned long count)
{
	struct vm_struct *tmp;
	char *vaddr, *buf_start = buf;
	unsigned long n;

	/* Don't allow overflow */
	if ((unsigned long) addr + count < count)
		count = -(unsigned long) addr;

	read_lock(&vmlist_lock);
	for (tmp = vmlist; tmp; tmp = tmp->next) {
		vaddr = (char *) tmp->addr;
		if (addr >= vaddr + tmp->size - PAGE_SIZE)
			continue;
		while (addr < vaddr) {
			if (count == 0)
				goto finished;
			*buf = '\0';
			buf++;
			addr++;
			count--;
		}
		n = vaddr + tmp->size - PAGE_SIZE - addr;
		do {
			if (count == 0)
				goto finished;
			*buf = *addr;
			buf++;
			addr++;
			count--;
		} while (--n > 0);
	}
finished:
	read_unlock(&vmlist_lock);
	return buf - buf_start;
}

/**/
long vwrite(char *buf, char *addr, unsigned long count)
{
	struct vm_struct *tmp;
	char *vaddr, *buf_start = buf;
	unsigned long n;

	/*不允许上溢*/
	if ((unsigned long) addr + count < count)
		count = -(unsigned long) addr;

	read_lock(&vmlist_lock);
	for (tmp = vmlist; tmp; tmp = tmp->next)
	{
		vaddr = (char *) tmp->addr;
		if (addr >= vaddr + tmp->size - PAGE_SIZE)
			continue;
		while (addr < vaddr)
		{
			if (count == 0)
				goto finished;
			buf++;
			addr++;
			count--;
		}
		n = vaddr + tmp->size - PAGE_SIZE - addr;
		do {
			if (count == 0)
				goto finished;
			*addr = *buf;
			buf++;
			addr++;
			count--;
		} while (--n > 0);
	}
finished:
	read_unlock(&vmlist_lock);
	return buf - buf_start;
}

/**
 *	remap_vmalloc_range  -  map vmalloc pages to userspace
 *	@vma:		vma to cover (map full range of vma)
 *	@addr:		vmalloc memory
 *	@pgoff:		number of pages into addr before first page to map
 *	@returns:	0 for success, -Exxx on failure
 *
 *	This function checks that addr is a valid vmalloc'ed area, and
 *	that it is big enough to cover the vma. Will return failure if
 *	that criteria isn't met.
 *
 *	Similar to remap_pfn_range() (see mm/memory.c)
 */
/**/
int remap_vmalloc_range(struct vm_area_struct *vma, void *addr,
						unsigned long pgoff)
{
	struct vm_struct *area;
	unsigned long uaddr = vma->vm_start;
	unsigned long usize = vma->vm_end - vma->vm_start;
	int ret;

	if ((PAGE_SIZE-1) & (unsigned long)addr)
		return -EINVAL;

	read_lock(&vmlist_lock);
	area = __find_vm_area(addr);
	if (!area)
		goto out_einval_locked;

	if (!(area->flags & VM_USERMAP))
		goto out_einval_locked;

	if (usize + (pgoff << PAGE_SHIFT) > area->size - PAGE_SIZE)
		goto out_einval_locked;
	read_unlock(&vmlist_lock);

	addr += pgoff << PAGE_SHIFT;
	do {
		struct page *page = vmalloc_to_page(addr);
		ret = vm_insert_page(vma, uaddr, page);
		if (ret)
			return ret;

		uaddr += PAGE_SIZE;
		addr += PAGE_SIZE;
		usize -= PAGE_SIZE;
	} while (usize > 0);

	/* Prevent "things" like memory migration? VM_flags need a cleanup... */
	vma->vm_flags |= VM_RESERVED;

	return ret;

out_einval_locked:
	read_unlock(&vmlist_lock);
	return -EINVAL;
}
EXPORT_SYMBOL(remap_vmalloc_range);

/*
 * Implement a stub for vmalloc_sync_all() if the architecture chose not to
 * have one.
 */
void  __attribute__((weak)) vmalloc_sync_all(void)
{
}


static int f(pte_t *pte, struct page *pmd_page, unsigned long addr, void *data)
{
	/* apply_to_page_range() does all the hard work. */
	return 0;
}

/**
 *	alloc_vm_area - allocate a range of kernel address space
 *	@size:		size of the area
 *	@returns:	NULL on failure, vm_struct on success
 *
 *	This function reserves a range of kernel address space, and
 *	allocates pagetables to map that range.  No actual mappings
 *	are created.  If the kernel address space is not shared
 *	between processes, it syncs the pagetable across all
 *	processes.
 */
/**/
struct vm_struct *alloc_vm_area(size_t size)
{
	struct vm_struct *area;

	area = get_vm_area(size, VM_IOREMAP);
	if (area == NULL)
		return NULL;

	/*
	 * This ensures that page tables are constructed for this region
	 * of kernel virtual address space and mapped into init_mm.
	 */
	if (apply_to_page_range(&init_mm, (unsigned long)area->addr,
				area->size, f, NULL)) {
		free_vm_area(area);
		return NULL;
	}

	/* Make sure the pagetables are constructed in process kernel
	   mappings */
	vmalloc_sync_all();

	return area;
}
EXPORT_SYMBOL_GPL(alloc_vm_area);

/*删除VMALLOC映射区中指定地址为起始地址的vmalloc子区域*/
void free_vm_area(struct vm_struct *area)
{
	struct vm_struct *ret;
	/*删除指定地址为起始地址的vmalloc子区域*/
	ret = remove_vm_area(area->addr);
	/**/
	BUG_ON(ret != area);
	/*释放子区域内存*/
	kfree(area);
}
EXPORT_SYMBOL_GPL(free_vm_area);
