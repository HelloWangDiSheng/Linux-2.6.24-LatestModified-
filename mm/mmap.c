#include <linux/slab.h>
#include <linux/backing-dev.h>
#include <linux/mm.h>
#include <linux/shm.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/swap.h>
#include <linux/syscalls.h>
#include <linux/capability.h>
#include <linux/init.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/personality.h>
#include <linux/security.h>
#include <linux/hugetlb.h>
#include <linux/profile.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/mempolicy.h>
#include <linux/rmap.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <asm/tlb.h>
#include <asm/mmu_context.h>

#ifndef arch_mmap_check
#define arch_mmap_check(addr, len, flags)	(0)
#endif

static void unmap_region(struct mm_struct *mm,	struct vm_area_struct *vma,
							struct vm_area_struct *prev, unsigned long start, unsigned long end);

/*警告！调试将使用递归算法，因此永远不要启用，除非你知道自己在干啥！*/
#undef DEBUG_MM_RB

/* description of effects of mapping type and prot in current implementation.
 * this is due to the limited x86 page protection hardware.  The expected
 * behavior is in parens:
 *
 * map_type	prot
 *		PROT_NONE	PROT_READ	PROT_WRITE	PROT_EXEC
 * MAP_SHARED	r: (no) no	r: (yes) yes	r: (no) yes	r: (no) yes
 *		w: (no) no	w: (no) no	w: (yes) yes	w: (no) no
 *		x: (no) no	x: (no) yes	x: (no) yes	x: (yes) yes
 *
 * MAP_PRIVATE	r: (no) no	r: (yes) yes	r: (no) yes	r: (no) yes
 *		w: (no) no	w: (no) no	w: (copy) copy	w: (no) no
 *		x: (no) no	x: (no) yes	x: (no) yes	x: (yes) yes
 *
 */
pgprot_t protection_map[16] =
{
	__P000, __P001, __P010, __P011, __P100, __P101, __P110, __P111,
	__S000, __S001, __S010, __S011, __S100, __S101, __S110, __S111
};

pgprot_t vm_get_page_prot(unsigned long vm_flags)
{
	return protection_map[vm_flags &	(VM_READ|VM_WRITE|VM_EXEC|VM_SHARED)];
}
EXPORT_SYMBOL(vm_get_page_prot);

int sysctl_overcommit_memory = OVERCOMMIT_GUESS;  /* heuristic overcommit */
int sysctl_overcommit_ratio = 50;	/* default is 50% */
int sysctl_max_map_count __read_mostly = DEFAULT_MAX_MAP_COUNT;
atomic_t vm_committed_space = ATOMIC_INIT(0);

/*
 * Check that a process has enough memory to allocate a new virtual
 * mapping. 0 means there is enough memory for the allocation to
 * succeed and -ENOMEM implies there is not.
 *
 * We currently support three overcommit policies, which are set via the
 * vm.overcommit_memory sysctl.  See Documentation/vm/overcommit-accounting
 *
 * Strict overcommit modes added 2002 Feb 26 by Alan Cox.
 * Additional code 2002 Jul 20 by Robert Love.
 *
 * cap_sys_admin is 1 if the process has admin privileges, 0 otherwise.
 *
 * Note this is a helper function intended to be used by LSMs which
 * wish to use this logic.
 */
int __vm_enough_memory(struct mm_struct *mm, long pages, int cap_sys_admin)
{
	unsigned long free, allowed;

	vm_acct_memory(pages);

	/*OVERCOMMIT_ALWAYS是最为激进的overcommit策略，无论进程申请多大的虚拟内存，只要不
	超过整个进程虚拟内存空间的大小，内核总会痛快的答应。但是这种策略下，虚拟内存的申请
	虽然容易了，但是当进程遇到缺页，内核为其分配物理内存的时候，会非常容易造成OOM*/
	if (sysctl_overcommit_memory == OVERCOMMIT_ALWAYS)
		return 0;
	/*是内核的默认overcommit策略。在这种模式下，特别激进的，过量的虚拟内存申请将会被拒
	绝，内核会对虚拟内存能够过量申请多少做出一定的限制，这种策略既不激进也不保守，比较
	中庸*/
	if (sysctl_overcommit_memory == OVERCOMMIT_GUESS)
	{
		unsigned long n;
		/*获取非线性映射页数目*/
		free = global_page_state(NR_FILE_PAGES);
		/*累计交换区中页数目*/
		free += nr_swap_pages;

		/*
		 * Any slabs which are created with the
		 * SLAB_RECLAIM_ACCOUNT flag claim to have contents
		 * which are reclaimable, under pressure.  The dentry
		 * cache and most inode caches should fall into this
		 */
		free += global_page_state(NR_SLAB_RECLAIMABLE);

		/*root用户保留3%	 */
		if (!cap_sys_admin)
			free -= free / 32;
		/*如果空闲页数目大于申请的数目，尅申请成功*/
		if (free > pages)
			return 0;

		/*获取系统中的空闲页数目。该函数在大型系统上和费资源，仅在预期失败时调用*/
		n = nr_free_pages();

		/*忽略预留页，此种页不是用作匿名页*/
		if (n <= totalreserve_pages)
			goto error;
		else
			n -= totalreserve_pages;

		/*给root预留3%空闲页		 */
		if (!cap_sys_admin)
			n -= n / 32;
		free += n;

		if (free > pages)
			return 0;

		goto error;
	}
	/**/
	allowed = (totalram_pages - hugetlb_total_pages()) * sysctl_overcommit_ratio / 100;
	/*
	 * Leave the last 3% for root
	 */
	if (!cap_sys_admin)
		allowed -= allowed / 32;
	allowed += total_swap_pages;

	/* Don't let a single process grow too big:
	   leave 3% of the size of this process for other processes */
	allowed -= mm->total_vm / 32;

	/*
	 * cast `allowed' as a signed long because vm_committed_space
	 * sometimes has a negative value
	 */
	if (atomic_read(&vm_committed_space) < (long)allowed)
		return 0;
error:
	vm_unacct_memory(pages);

	return -ENOMEM;
}

/*需要inode->i_mapping->i_mmap_lock锁保护*/
static void __remove_shared_vm_struct(struct vm_area_struct *vma, struct file *file,
											struct address_space *mapping)
{
	/*如果该虚拟内存域禁止写入，则*/
	if (vma->vm_flags & VM_DENYWRITE)
		atomic_inc(&file->f_path.dentry->d_inode->i_writecount);
	/*共享映射内存域*/
	if (vma->vm_flags & VM_SHARED)
		mapping->i_mmap_writable--;
	/**/
	flush_dcache_mmap_lock(mapping);
	/*该vma是非线性映射vma*/
	if (unlikely(vma->vm_flags & VM_NONLINEAR))
		list_del_init(&vma->shared.vm_set.list);
	else
		vma_prio_tree_remove(vma, &mapping->i_mmap);
	flush_dcache_mmap_unlock(mapping);
}

/*
 * Unlink a file-based vm structure from its prio_tree, to hide
 * vma from rmap and vmtruncate before freeing its page tables.
 */
/*从优先树中删除基于文件的vma，在释放其页表前将其从逆向映射和vmtruncate中隐藏*/
void unlink_file_vma(struct vm_area_struct *vma)
{
	/*获取虚拟内存域中的文件句柄，结果为空说明该映射是匿名映射，否则是文件映射*/
	struct file *file = vma->vm_file;
	/*如果是文件映射，则删除*/
	if (file)
	{
		/*获取文件映射的地址空间*/
		struct address_space *mapping = file->f_mapping;
		/*获取地址空间操作保护锁*/
		spin_lock(&mapping->i_mmap_lock);
		/**/
		__remove_shared_vm_struct(vma, file, mapping);
		spin_unlock(&mapping->i_mmap_lock);
	}
}

/*从虚拟地址空间中删除指定的vma，返回下一个vma*/
static struct vm_area_struct *remove_vma(struct vm_area_struct *vma)
{
	/*获取下一个vma*/
	struct vm_area_struct *next = vma->vm_next;
	/*删除vma操作可能休眠*/
	might_sleep();
	/*如果vma指定了删除vma映射区域的close（一般为NULL）函数，则执行close函数*/
	if (vma->vm_ops && vma->vm_ops->close)
		vma->vm_ops->close(vma);
	/*如果vma存在映射文件，则取消对映射文件的引用*/
	if (vma->vm_file)
		fput(vma->vm_file);
	/**/
	mpol_free(vma_policy(vma));
	/*释放vma对应的slab缓存*/
	kmem_cache_free(vm_area_cachep, vma);
	return next;
}

/**/
asmlinkage unsigned long sys_brk(unsigned long brk)
{
	unsigned long rlim, retval;
	unsigned long newbrk, oldbrk;
	/*获取当前进程的虚拟地址空间*/
	struct mm_struct *mm = current->mm;
	/*申请mmap写保护锁*/
	down_write(&mm->mmap_sem);
	/*指定堆地址小于代码段的结束地址，无效参数，继续用原来的堆*/
	if (brk < mm->end_code)
		goto out;

	/*
	 * Check against rlimit here. If this check is done later after the test
	 * of oldbrk with newbrk then it can escape the test and let the data
	 * segment grow beyond its set limit the in case where the limit is
	 * not page aligned -Ram Gupta
	 */
	/*此处检查资源限制。如果*/
	/*获取当前进程数据段的长度*/
	rlim = current->signal->rlim[RLIMIT_DATA].rlim_cur;
	/*如果当前进程数据段长度不是无限制的，并且进程堆的起始地址和数据段起始地址之间的
	间隔长度大于数据段的长度，则继续使用之前的堆*/
	if (rlim < RLIM_INFINITY && brk - mm->start_data > rlim)
		goto out;
	/*获取对齐页长度之后的新堆结束地址*/
	newbrk = PAGE_ALIGN(brk);
	/*获取当前进程堆对齐页长度之后的结束地址*/
	oldbrk = PAGE_ALIGN(mm->brk);
	/*如果新申请堆地址与当前进程堆地址在同一个页地址内，则重置进程堆地址为指定的堆地址*/
	if (oldbrk == newbrk)
		goto set_brk;
	/*新堆结束地址小于当前堆结束地址，缩小堆。经常允许缩小堆*/
	if (brk <= mm->brk)
	{
		/*将堆中[newbrk, oldbrk)区域映射解除*/
		if (!do_munmap(mm, newbrk, oldbrk-newbrk))
			goto set_brk;
		goto out;
	}

	/* Check against existing mmap mappings. */
	/*brk > mm->brk扩大堆，如果新旧堆结束地址与现有映射重叠，则继续使用之前的堆*/
	if (find_vma_intersection(mm, oldbrk, newbrk+PAGE_SIZE))
		goto out;

	/*堆扩大的部分与现存映射都不重叠，就在原堆结束地址brk处在扩大newbrk-oldbrk长度*/
	if (do_brk(oldbrk, newbrk-oldbrk) != oldbrk)
		goto out;
set_brk:
	/*设置新堆结束地址为指定的结束地址*/
	mm->brk = brk;
out:
	retval = mm->brk;
	up_write(&mm->mmap_sem);
	return retval;
}

#ifdef DEBUG_MM_RB
/*遍历红黑树获取虚拟地址空间中vma的数目。红黑树中每个vma的起止地址及地址内部都是按由
小到大顺序排序的。程序中顺序逆序两次遍历红黑树中结点数目，遍历期间打印提示信息，返回
有效结点数目，如果前后两次遍历显示结点数目不一致，则将树中结点数目清零*/
static int browse_rb(struct rb_root *root)
{
	int i = 0, j;
	struct rb_node *nd, *pn = NULL;
	unsigned long prev = 0, pend = 0;
	/*从红黑树最左侧左子树根结点开始遍历*/
	for (nd = rb_first(root); nd; nd = rb_next(nd))
	{
		struct vm_area_struct *vma;
		/*获取红黑树结点对应的vma实例*/
		vma = rb_entry(nd, struct vm_area_struct, vm_rb);
		/*如果当前vma的起始地址小于前一个vma区域的起始地址，则打印提示信息，逗号表达式
		说明该vma无效（当前vma和前一个vma重叠，系统在操作mmap期间如果发现前后mmap可以合
		并时，会自动合并）*/
		if (vma->vm_start < prev)
			printk("vm_start %lx prev %lx\n", vma->vm_start, prev), i = -1;
		/*当前vma区域的起始地址小于前一个vma区域的结束地址下一个字节的地址*/
		if (vma->vm_start < pend)
			printk("vm_start %lx pend %lx\n", vma->vm_start, pend);
		/*当前vma区域起始地址大于当前vma区域的结束地址*/
		if (vma->vm_start > vma->vm_end)
			printk("vm_end %lx < vm_start %lx\n", vma->vm_end, vma->vm_start);
		/*遍历结点数目*/
		i++;
		/*保存当前vma区域*/
		pn = nd;
		/*保存当前vma区域的起始地址*/
		prev = vma->vm_start;
		/*保存当前vma区域的结束地址的下一个字节地址*/
		pend = vma->vm_end;
	}
	j = 0;
	/*逆序遍历红黑树中结点（从红黑树最右子树向前遍历前一结点，直到遍历完红黑树的最左
	子树结点为止）*/
	for (nd = pn; nd; nd = rb_prev(nd)) {
		j++;
	}
	/*顺序和逆序遍历红黑树的结点数目不一致，这种情况显示红黑树中结点信息错误，要打印
	提示信息，并将红黑树中结点数目重置为零*/
	if (i != j)
		printk("backwards %d, forwards %d\n", j, i), i = 0;
	return i;
}

void validate_mm(struct mm_struct *mm)
{
	int bug = 0;
	int i = 0;
	/*遍历mm->mmap获取虚拟地址空间中vma的数目*/
	struct vm_area_struct *tmp = mm->mmap;
	while (tmp)
	{
		tmp = tmp->vm_next;
		i++;
	}
	/*虚拟地址空间中实际的vma数目和保存的不一致*/
	if (i != mm->map_count)
		printk("map_count %d vm_next %d\n", mm->map_count, i), bug = 1;
	/*遍历红黑树获取虚拟地址空间中vma个数*/
	i = browse_rb(&mm->mm_rb);
	if (i != mm->map_count)
		printk("map_count %d rb %d\n", mm->map_count, i), bug = 1;
	BUG_ON(bug);
}
#else
#define validate_mm(mm) do { } while (0)
#endif

/**/
static struct vm_area_struct *find_vma_prepare(struct mm_struct *mm, unsigned long addr,
								struct vm_area_struct **pprev, struct rb_node ***rb_link,
								struct rb_node ** rb_parent)
{
	struct vm_area_struct * vma;
	struct rb_node ** __rb_link, * __rb_parent, * rb_prev;

	/*获取进程虚拟地址空间红黑树根节点的地址*/
	__rb_link = &mm->mm_rb.rb_node;
	rb_prev = __rb_parent = NULL;
	vma = NULL;
	/*根结点非空进入while-do循环*/
	while (*__rb_link)
	{
		struct vm_area_struct *vma_tmp;
		/*保存当前结点*/
		__rb_parent = *__rb_link;
		/*获取当前结点对应的vma实例*/
		vma_tmp = rb_entry(__rb_parent, struct vm_area_struct, vm_rb);
		/*如果指定地址在当前vma实例的起止区域中，则返回该vma。否则，根据vma起始地址和
		终止地址与指定地址关系来决定查询当前结点的左子树还是右子树*/
		if (vma_tmp->vm_end > addr)
		{
			vma = vma_tmp;
			/*如果指定地址在当前vma实例的起止区域中，则返回当前vma实例地址*/
			if (vma_tmp->vm_start <= addr)
				return vma;
			/*指定区域在当前vma区域的前面，则保存并查询该vma的左子树*/
			__rb_link = &__rb_parent->rb_left;
		}
		else
		{
			/*指定地址在当前vma实例起止区域的后面，则保存当前查找结点*/
			rb_prev = __rb_parent;
			/*保存并查询该结点的右子树*/
			__rb_link = &__rb_parent->rb_right;
		}
	}

	*pprev = NULL;
	/*如果之前查找过程中，有查找过任一查找结点的右子树（前面代码可以看出只有查询结点
	右子树时，才保存上次查找结点信息），则获取最近查找的右子树的父结点对应的vma*/
	if (rb_prev)
		*pprev = rb_entry(rb_prev, struct vm_area_struct, vm_rb);
	/*保存当前查找结点*/
	*rb_link = __rb_link;
	/*保存当前查找结点的父结点*/
	*rb_parent = __rb_parent;
	return vma;
}

static inline void
__vma_link_list(struct mm_struct *mm, struct vm_area_struct *vma,
		struct vm_area_struct *prev, struct rb_node *rb_parent)
{
	if (prev) {
		vma->vm_next = prev->vm_next;
		prev->vm_next = vma;
	} else {
		mm->mmap = vma;
		if (rb_parent)
			vma->vm_next = rb_entry(rb_parent,
					struct vm_area_struct, vm_rb);
		else
			vma->vm_next = NULL;
	}
}

void __vma_link_rb(struct mm_struct *mm, struct vm_area_struct *vma,
		struct rb_node **rb_link, struct rb_node *rb_parent)
{
	rb_link_node(&vma->vm_rb, rb_parent, rb_link);
	rb_insert_color(&vma->vm_rb, &mm->mm_rb);
}

static inline void __vma_link_file(struct vm_area_struct *vma)
{
	struct file * file;

	file = vma->vm_file;
	if (file) {
		struct address_space *mapping = file->f_mapping;

		if (vma->vm_flags & VM_DENYWRITE)
			atomic_dec(&file->f_path.dentry->d_inode->i_writecount);
		if (vma->vm_flags & VM_SHARED)
			mapping->i_mmap_writable++;

		flush_dcache_mmap_lock(mapping);
		if (unlikely(vma->vm_flags & VM_NONLINEAR))
			vma_nonlinear_insert(vma, &mapping->i_mmap_nonlinear);
		else
			vma_prio_tree_insert(vma, &mapping->i_mmap);
		flush_dcache_mmap_unlock(mapping);
	}
}

static void
__vma_link(struct mm_struct *mm, struct vm_area_struct *vma,
	struct vm_area_struct *prev, struct rb_node **rb_link,
	struct rb_node *rb_parent)
{
	__vma_link_list(mm, vma, prev, rb_parent);
	__vma_link_rb(mm, vma, rb_link, rb_parent);
	__anon_vma_link(vma);
}

static void vma_link(struct mm_struct *mm, struct vm_area_struct *vma,
			struct vm_area_struct *prev, struct rb_node **rb_link,
			struct rb_node *rb_parent)
{
	struct address_space *mapping = NULL;

	if (vma->vm_file)
		mapping = vma->vm_file->f_mapping;

	if (mapping) {
		spin_lock(&mapping->i_mmap_lock);
		vma->vm_truncate_count = mapping->truncate_count;
	}
	anon_vma_lock(vma);

	__vma_link(mm, vma, prev, rb_link, rb_parent);
	__vma_link_file(vma);

	anon_vma_unlock(vma);
	if (mapping)
		spin_unlock(&mapping->i_mmap_lock);

	mm->map_count++;
	validate_mm(mm);
}

/*
 * Helper for vma_adjust in the split_vma insert case:
 * insert vm structure into list and rbtree and anon_vma,
 * but it has already been inserted into prio_tree earlier.
 */
static void
__insert_vm_struct(struct mm_struct * mm, struct vm_area_struct * vma)
{
	struct vm_area_struct * __vma, * prev;
	struct rb_node ** rb_link, * rb_parent;

	__vma = find_vma_prepare(mm, vma->vm_start,&prev, &rb_link, &rb_parent);
	BUG_ON(__vma && __vma->vm_start < vma->vm_end);
	__vma_link(mm, vma, prev, rb_link, rb_parent);
	mm->map_count++;
}

static inline void
__vma_unlink(struct mm_struct *mm, struct vm_area_struct *vma,
		struct vm_area_struct *prev)
{
	prev->vm_next = vma->vm_next;
	rb_erase(&vma->vm_rb, &mm->mm_rb);
	if (mm->mmap_cache == vma)
		mm->mmap_cache = prev;
}

/*
 * We cannot adjust vm_start, vm_end, vm_pgoff fields of a vma that
 * is already present in an i_mmap tree without adjusting the tree.
 * The following helper function should be used when such adjustments
 * are necessary.  The "insert" vma (if any) is to be inserted
 * before we drop the necessary locks.
 */
void vma_adjust(struct vm_area_struct *vma, unsigned long start,
	unsigned long end, pgoff_t pgoff, struct vm_area_struct *insert)
{
	struct mm_struct *mm = vma->vm_mm;
	struct vm_area_struct *next = vma->vm_next;
	struct vm_area_struct *importer = NULL;
	struct address_space *mapping = NULL;
	struct prio_tree_root *root = NULL;
	struct file *file = vma->vm_file;
	struct anon_vma *anon_vma = NULL;
	long adjust_next = 0;
	int remove_next = 0;

	if (next && !insert) {
		if (end >= next->vm_end) {
			/*
			 * vma expands, overlapping all the next, and
			 * perhaps the one after too (mprotect case 6).
			 */
again:			remove_next = 1 + (end > next->vm_end);
			end = next->vm_end;
			anon_vma = next->anon_vma;
			importer = vma;
		} else if (end > next->vm_start) {
			/*
			 * vma expands, overlapping part of the next:
			 * mprotect case 5 shifting the boundary up.
			 */
			adjust_next = (end - next->vm_start) >> PAGE_SHIFT;
			anon_vma = next->anon_vma;
			importer = vma;
		} else if (end < vma->vm_end) {
			/*
			 * vma shrinks, and !insert tells it's not
			 * split_vma inserting another: so it must be
			 * mprotect case 4 shifting the boundary down.
			 */
			adjust_next = - ((vma->vm_end - end) >> PAGE_SHIFT);
			anon_vma = next->anon_vma;
			importer = next;
		}
	}

	if (file) {
		mapping = file->f_mapping;
		if (!(vma->vm_flags & VM_NONLINEAR))
			root = &mapping->i_mmap;
		spin_lock(&mapping->i_mmap_lock);
		if (importer &&
		    vma->vm_truncate_count != next->vm_truncate_count) {
			/*
			 * unmap_mapping_range might be in progress:
			 * ensure that the expanding vma is rescanned.
			 */
			importer->vm_truncate_count = 0;
		}
		if (insert) {
			insert->vm_truncate_count = vma->vm_truncate_count;
			/*
			 * Put into prio_tree now, so instantiated pages
			 * are visible to arm/parisc __flush_dcache_page
			 * throughout; but we cannot insert into address
			 * space until vma start or end is updated.
			 */
			__vma_link_file(insert);
		}
	}

	/*
	 * When changing only vma->vm_end, we don't really need
	 * anon_vma lock: but is that case worth optimizing out?
	 */
	if (vma->anon_vma)
		anon_vma = vma->anon_vma;
	if (anon_vma) {
		spin_lock(&anon_vma->lock);
		/*
		 * Easily overlooked: when mprotect shifts the boundary,
		 * make sure the expanding vma has anon_vma set if the
		 * shrinking vma had, to cover any anon pages imported.
		 */
		if (importer && !importer->anon_vma) {
			importer->anon_vma = anon_vma;
			__anon_vma_link(importer);
		}
	}

	if (root) {
		flush_dcache_mmap_lock(mapping);
		vma_prio_tree_remove(vma, root);
		if (adjust_next)
			vma_prio_tree_remove(next, root);
	}

	vma->vm_start = start;
	vma->vm_end = end;
	vma->vm_pgoff = pgoff;
	if (adjust_next) {
		next->vm_start += adjust_next << PAGE_SHIFT;
		next->vm_pgoff += adjust_next;
	}

	if (root) {
		if (adjust_next)
			vma_prio_tree_insert(next, root);
		vma_prio_tree_insert(vma, root);
		flush_dcache_mmap_unlock(mapping);
	}

	if (remove_next) {
		/*
		 * vma_merge has merged next into vma, and needs
		 * us to remove next before dropping the locks.
		 */
		__vma_unlink(mm, next, vma);
		if (file)
			__remove_shared_vm_struct(next, file, mapping);
		if (next->anon_vma)
			__anon_vma_merge(vma, next);
	} else if (insert) {
		/*
		 * split_vma has split insert from vma, and needs
		 * us to insert it before dropping the locks
		 * (it may either follow vma or precede it).
		 */
		__insert_vm_struct(mm, insert);
	}

	if (anon_vma)
		spin_unlock(&anon_vma->lock);
	if (mapping)
		spin_unlock(&mapping->i_mmap_lock);

	if (remove_next) {
		if (file)
			fput(file);
		mm->map_count--;
		mpol_free(vma_policy(next));
		kmem_cache_free(vm_area_cachep, next);
		/*
		 * In mprotect's case 6 (see comments on vma_merge),
		 * we must remove another next too. It would clutter
		 * up the code too much to do both in one go.
		 */
		if (remove_next == 2) {
			next = vma->vm_next;
			goto again;
		}
	}

	validate_mm(mm);
}

/*
 * If the vma has a ->close operation then the driver probably needs to release
 * per-vma resources, so we don't attempt to merge those.
 */
#define VM_SPECIAL (VM_IO | VM_DONTEXPAND | VM_RESERVED | VM_PFNMAP)

static inline int is_mergeable_vma(struct vm_area_struct *vma,
			struct file *file, unsigned long vm_flags)
{
	if (vma->vm_flags != vm_flags)
		return 0;
	if (vma->vm_file != file)
		return 0;
	if (vma->vm_ops && vma->vm_ops->close)
		return 0;
	return 1;
}

static inline int is_mergeable_anon_vma(struct anon_vma *anon_vma1,
					struct anon_vma *anon_vma2)
{
	return !anon_vma1 || !anon_vma2 || (anon_vma1 == anon_vma2);
}

/*
 * Return true if we can merge this (vm_flags,anon_vma,file,vm_pgoff)
 * in front of (at a lower virtual address and file offset than) the vma.
 *
 * We cannot merge two vmas if they have differently assigned (non-NULL)
 * anon_vmas, nor if same anon_vma is assigned but offsets incompatible.
 *
 * We don't check here for the merged mmap wrapping around the end of pagecache
 * indices (16TB on ia32) because do_mmap_pgoff() does not permit mmap's which
 * wrap, nor mmaps which cover the final page at index -1UL.
 */
static int
can_vma_merge_before(struct vm_area_struct *vma, unsigned long vm_flags,
	struct anon_vma *anon_vma, struct file *file, pgoff_t vm_pgoff)
{
	if (is_mergeable_vma(vma, file, vm_flags) &&
	    is_mergeable_anon_vma(anon_vma, vma->anon_vma)) {
		if (vma->vm_pgoff == vm_pgoff)
			return 1;
	}
	return 0;
}

/*
 * Return true if we can merge this (vm_flags,anon_vma,file,vm_pgoff)
 * beyond (at a higher virtual address and file offset than) the vma.
 *
 * We cannot merge two vmas if they have differently assigned (non-NULL)
 * anon_vmas, nor if same anon_vma is assigned but offsets incompatible.
 */
static int
can_vma_merge_after(struct vm_area_struct *vma, unsigned long vm_flags,
	struct anon_vma *anon_vma, struct file *file, pgoff_t vm_pgoff)
{
	if (is_mergeable_vma(vma, file, vm_flags) &&
	    is_mergeable_anon_vma(anon_vma, vma->anon_vma)) {
		pgoff_t vm_pglen;
		vm_pglen = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
		if (vma->vm_pgoff + vm_pglen == vm_pgoff)
			return 1;
	}
	return 0;
}

/*
 * Given a mapping request (addr,end,vm_flags,file,pgoff), figure out
 * whether that can be merged with its predecessor or its successor.
 * Or both (it neatly fills a hole).
 *
 * In most cases - when called for mmap, brk or mremap - [addr,end) is
 * certain not to be mapped by the time vma_merge is called; but when
 * called for mprotect, it is certain to be already mapped (either at
 * an offset within prev, or at the start of next), and the flags of
 * this area are about to be changed to vm_flags - and the no-change
 * case has already been eliminated.
 *
 * The following mprotect cases have to be considered, where AAAA is
 * the area passed down from mprotect_fixup, never extending beyond one
 * vma, PPPPPP is the prev vma specified, and NNNNNN the next vma after:
 *
 *     AAAA             AAAA                AAAA          AAAA
 *    PPPPPPNNNNNN    PPPPPPNNNNNN    PPPPPPNNNNNN    PPPPNNNNXXXX
 *    cannot merge    might become    might become    might become
 *                    PPNNNNNNNNNN    PPPPPPPPPPNN    PPPPPPPPPPPP 6 or
 *    mmap, brk or    case 4 below    case 5 below    PPPPPPPPXXXX 7 or
 *    mremap move:                                    PPPPNNNNNNNN 8
 *        AAAA
 *    PPPP    NNNN    PPPPPPPPPPPP    PPPPPPPPNNNN    PPPPNNNNNNNN
 *    might become    case 1 below    case 2 below    case 3 below
 *
 * Odd one out? Case 8, because it extends NNNN but needs flags of XXXX:
 * mprotect_fixup updates vm_flags & vm_page_prot on successful return.
 */
struct vm_area_struct *vma_merge(struct mm_struct *mm,
			struct vm_area_struct *prev, unsigned long addr,
			unsigned long end, unsigned long vm_flags,
		     	struct anon_vma *anon_vma, struct file *file,
			pgoff_t pgoff, struct mempolicy *policy)
{
	pgoff_t pglen = (end - addr) >> PAGE_SHIFT;
	struct vm_area_struct *area, *next;

	/*
	 * We later require that vma->vm_flags == vm_flags,
	 * so this tests vma->vm_flags & VM_SPECIAL, too.
	 */
	if (vm_flags & VM_SPECIAL)
		return NULL;

	if (prev)
		next = prev->vm_next;
	else
		next = mm->mmap;
	area = next;
	if (next && next->vm_end == end)		/* cases 6, 7, 8 */
		next = next->vm_next;

	/*
	 * Can it merge with the predecessor?
	 */
	if (prev && prev->vm_end == addr &&
  			mpol_equal(vma_policy(prev), policy) &&
			can_vma_merge_after(prev, vm_flags,
						anon_vma, file, pgoff)) {
		/*
		 * OK, it can.  Can we now merge in the successor as well?
		 */
		if (next && end == next->vm_start &&
				mpol_equal(policy, vma_policy(next)) &&
				can_vma_merge_before(next, vm_flags,
					anon_vma, file, pgoff+pglen) &&
				is_mergeable_anon_vma(prev->anon_vma,
						      next->anon_vma)) {
							/* cases 1, 6 */
			vma_adjust(prev, prev->vm_start,
				next->vm_end, prev->vm_pgoff, NULL);
		} else					/* cases 2, 5, 7 */
			vma_adjust(prev, prev->vm_start,
				end, prev->vm_pgoff, NULL);
		return prev;
	}

	/*
	 * Can this new request be merged in front of next?
	 */
	if (next && end == next->vm_start &&
 			mpol_equal(policy, vma_policy(next)) &&
			can_vma_merge_before(next, vm_flags,
					anon_vma, file, pgoff+pglen)) {
		if (prev && addr < prev->vm_end)	/* case 4 */
			vma_adjust(prev, prev->vm_start,
				addr, prev->vm_pgoff, NULL);
		else					/* cases 3, 8 */
			vma_adjust(area, addr, next->vm_end,
				next->vm_pgoff - pglen, NULL);
		return area;
	}

	return NULL;
}

/*
 * find_mergeable_anon_vma is used by anon_vma_prepare, to check
 * neighbouring vmas for a suitable anon_vma, before it goes off
 * to allocate a new anon_vma.  It checks because a repetitive
 * sequence of mprotects and faults may otherwise lead to distinct
 * anon_vmas being allocated, preventing vma merge in subsequent
 * mprotect.
 */
struct anon_vma *find_mergeable_anon_vma(struct vm_area_struct *vma)
{
	struct vm_area_struct *near;
	unsigned long vm_flags;

	near = vma->vm_next;
	if (!near)
		goto try_prev;

	/*
	 * Since only mprotect tries to remerge vmas, match flags
	 * which might be mprotected into each other later on.
	 * Neither mlock nor madvise tries to remerge at present,
	 * so leave their flags as obstructing a merge.
	 */
	vm_flags = vma->vm_flags & ~(VM_READ|VM_WRITE|VM_EXEC);
	vm_flags |= near->vm_flags & (VM_READ|VM_WRITE|VM_EXEC);

	if (near->anon_vma && vma->vm_end == near->vm_start &&
 			mpol_equal(vma_policy(vma), vma_policy(near)) &&
			can_vma_merge_before(near, vm_flags,
				NULL, vma->vm_file, vma->vm_pgoff +
				((vma->vm_end - vma->vm_start) >> PAGE_SHIFT)))
		return near->anon_vma;
try_prev:
	/*
	 * It is potentially slow to have to call find_vma_prev here.
	 * But it's only on the first write fault on the vma, not
	 * every time, and we could devise a way to avoid it later
	 * (e.g. stash info in next's anon_vma_node when assigning
	 * an anon_vma, or when trying vma_merge).  Another time.
	 */
	BUG_ON(find_vma_prev(vma->vm_mm, vma->vm_start, &near) != vma);
	if (!near)
		goto none;

	vm_flags = vma->vm_flags & ~(VM_READ|VM_WRITE|VM_EXEC);
	vm_flags |= near->vm_flags & (VM_READ|VM_WRITE|VM_EXEC);

	if (near->anon_vma && near->vm_end == vma->vm_start &&
  			mpol_equal(vma_policy(near), vma_policy(vma)) &&
			can_vma_merge_after(near, vm_flags,
				NULL, vma->vm_file, vma->vm_pgoff))
		return near->anon_vma;
none:
	/*
	 * There's no absolute need to look only at touching neighbours:
	 * we could search further afield for "compatible" anon_vmas.
	 * But it would probably just be a waste of time searching,
	 * or lead to too many vmas hanging off the same anon_vma.
	 * We're trying to allow mprotect remerging later on,
	 * not trying to minimize memory used for anon_vmas.
	 */
	return NULL;
}

#ifdef CONFIG_PROC_FS
/**/
void vm_stat_account(struct mm_struct *mm, unsigned long flags,	struct file *file,
						long pages)
{
	/*获取栈标识*/
	const unsigned long stack_flags	= VM_STACK_FLAGS & (VM_GROWSUP|VM_GROWSDOWN);
	/*如果是文件映射，则更新文件共享内存页数目*/
	if (file)
	{
		/*更新共享页数目*/
		mm->shared_vm += pages;
		/*如果页是可执行虚拟内存页，则更新可执行虚拟内存页数目*/
		if ((flags & (VM_EXEC|VM_WRITE)) == VM_EXEC)
			mm->exec_vm += pages;
	}
	/*如果时栈虚拟内存页，则更新栈虚拟内存页数目*/
	else if (flags & stack_flags)
		mm->stack_vm += pages;
	/*如果页是保留或io虚拟内存页，则更新保留页数目*/
	if (flags & (VM_RESERVED|VM_IO))
		mm->reserved_vm += pages;
}
#endif /* CONFIG_PROC_FS */

/* 主调函数必须持有down_write(current->mm->mmap_sem)*/
unsigned long do_mmap_pgoff(struct file * file, unsigned long addr,	unsigned long len,
								unsigned long prot, unsigned long flags, unsigned long pgoff)
{
	/*获取当前进程的虚拟地址空间*/
	struct mm_struct * mm = current->mm;
	struct inode *inode;
	unsigned int vm_flags;
	int error;
	int accountable = 1;
	unsigned long reqprot = prot;

	/*
	 * Does the application expect PROT_READ to imply PROT_EXEC?
	 *
	 * (the exception is when the underlying filesystem is noexec
	 *  mounted, in which case we dont add PROT_EXEC.)
	 */
	if ((prot & PROT_READ) && (current->personality & READ_IMPLIES_EXEC))
		if (!(file && (file->f_path.mnt->mnt_flags & MNT_NOEXEC)))
			prot |= PROT_EXEC;

	/*映射长度不能为空*/
	if (!len)
		return -EINVAL;
	/*如果不是固定地址映射，则*/
	if (!(flags & MAP_FIXED))
		addr = round_hint_to_min(addr);

	error = arch_mmap_check(addr, len, flags);
	if (error)
		return error;

	/* Careful about overflows.. */
	len = PAGE_ALIGN(len);
	if (!len || len > TASK_SIZE)
		return -ENOMEM;

	/* offset overflow? */
	if ((pgoff + (len >> PAGE_SHIFT)) < pgoff)
               return -EOVERFLOW;

	/* Too many mappings? */
	if (mm->map_count > sysctl_max_map_count)
		return -ENOMEM;

	/* Obtain the address to map to. we verify (or select) it and ensure
	 * that it represents a valid section of the address space.
	 */
	addr = get_unmapped_area(file, addr, len, pgoff, flags);
	if (addr & ~PAGE_MASK)
		return addr;

	/* Do simple checking here so the lower-level routines won't have
	 * to. we assume access permissions have been handled by the open
	 * of the memory object, so we don't do any here.
	 */
	vm_flags = calc_vm_prot_bits(prot) | calc_vm_flag_bits(flags) |
			mm->def_flags | VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC;

	if (flags & MAP_LOCKED) {
		if (!can_do_mlock())
			return -EPERM;
		vm_flags |= VM_LOCKED;
	}
	/* mlock MCL_FUTURE? */
	if (vm_flags & VM_LOCKED) {
		unsigned long locked, lock_limit;
		locked = len >> PAGE_SHIFT;
		locked += mm->locked_vm;
		lock_limit = current->signal->rlim[RLIMIT_MEMLOCK].rlim_cur;
		lock_limit >>= PAGE_SHIFT;
		if (locked > lock_limit && !capable(CAP_IPC_LOCK))
			return -EAGAIN;
	}

	inode = file ? file->f_path.dentry->d_inode : NULL;

	if (file) {
		switch (flags & MAP_TYPE) {
		case MAP_SHARED:
			if ((prot&PROT_WRITE) && !(file->f_mode&FMODE_WRITE))
				return -EACCES;

			/*
			 * Make sure we don't allow writing to an append-only
			 * file..
			 */
			if (IS_APPEND(inode) && (file->f_mode & FMODE_WRITE))
				return -EACCES;

			/*
			 * Make sure there are no mandatory locks on the file.
			 */
			if (locks_verify_locked(inode))
				return -EAGAIN;

			vm_flags |= VM_SHARED | VM_MAYSHARE;
			if (!(file->f_mode & FMODE_WRITE))
				vm_flags &= ~(VM_MAYWRITE | VM_SHARED);

			/* fall through */
		case MAP_PRIVATE:
			if (!(file->f_mode & FMODE_READ))
				return -EACCES;
			if (file->f_path.mnt->mnt_flags & MNT_NOEXEC) {
				if (vm_flags & VM_EXEC)
					return -EPERM;
				vm_flags &= ~VM_MAYEXEC;
			}
			if (is_file_hugepages(file))
				accountable = 0;

			if (!file->f_op || !file->f_op->mmap)
				return -ENODEV;
			break;

		default:
			return -EINVAL;
		}
	} else {
		switch (flags & MAP_TYPE) {
		case MAP_SHARED:
			vm_flags |= VM_SHARED | VM_MAYSHARE;
			break;
		case MAP_PRIVATE:
			/*
			 * Set pgoff according to addr for anon_vma.
			 */
			pgoff = addr >> PAGE_SHIFT;
			break;
		default:
			return -EINVAL;
		}
	}

	error = security_file_mmap(file, reqprot, prot, flags, addr, 0);
	if (error)
		return error;

	return mmap_region(file, addr, len, flags, vm_flags, pgoff,
			   accountable);
}
EXPORT_SYMBOL(do_mmap_pgoff);

/*
 * Some shared mappigns will want the pages marked read-only
 * to track write events. If so, we'll downgrade vm_page_prot
 * to the private version (using protection_map[] without the
 * VM_SHARED bit).
 */
int vma_wants_writenotify(struct vm_area_struct *vma)
{
	unsigned int vm_flags = vma->vm_flags;

	/* If it was private or non-writable, the write bit is already clear */
	if ((vm_flags & (VM_WRITE|VM_SHARED)) != ((VM_WRITE|VM_SHARED)))
		return 0;

	/* The backer wishes to know when pages are first written to? */
	if (vma->vm_ops && vma->vm_ops->page_mkwrite)
		return 1;

	/* The open routine did something to the protections already? */
	if (pgprot_val(vma->vm_page_prot) !=
	    pgprot_val(vm_get_page_prot(vm_flags)))
		return 0;

	/* Specialty mapping? */
	if (vm_flags & (VM_PFNMAP|VM_INSERTPAGE))
		return 0;

	/* Can the mapping track the dirty pages? */
	return vma->vm_file && vma->vm_file->f_mapping &&
		mapping_cap_account_dirty(vma->vm_file->f_mapping);
}


unsigned long mmap_region(struct file *file, unsigned long addr,
			  unsigned long len, unsigned long flags,
			  unsigned int vm_flags, unsigned long pgoff,
			  int accountable)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma, *prev;
	int correct_wcount = 0;
	int error;
	struct rb_node **rb_link, *rb_parent;
	unsigned long charged = 0;
	struct inode *inode =  file ? file->f_path.dentry->d_inode : NULL;

	/* Clear old maps */
	error = -ENOMEM;
munmap_back:
	vma = find_vma_prepare(mm, addr, &prev, &rb_link, &rb_parent);
	if (vma && vma->vm_start < addr + len) {
		if (do_munmap(mm, addr, len))
			return -ENOMEM;
		goto munmap_back;
	}

	/* Check against address space limit. */
	if (!may_expand_vm(mm, len >> PAGE_SHIFT))
		return -ENOMEM;

	if (accountable && (!(flags & MAP_NORESERVE) ||
			    sysctl_overcommit_memory == OVERCOMMIT_NEVER)) {
		if (vm_flags & VM_SHARED) {
			/* Check memory availability in shmem_file_setup? */
			vm_flags |= VM_ACCOUNT;
		} else if (vm_flags & VM_WRITE) {
			/*
			 * Private writable mapping: check memory availability
			 */
			charged = len >> PAGE_SHIFT;
			if (security_vm_enough_memory(charged))
				return -ENOMEM;
			vm_flags |= VM_ACCOUNT;
		}
	}

	/*
	 * Can we just expand an old private anonymous mapping?
	 * The VM_SHARED test is necessary because shmem_zero_setup
	 * will create the file object for a shared anonymous map below.
	 */
	if (!file && !(vm_flags & VM_SHARED) &&
	    vma_merge(mm, prev, addr, addr + len, vm_flags,
					NULL, NULL, pgoff, NULL))
		goto out;

	/*
	 * Determine the object being mapped and call the appropriate
	 * specific mapper. the address has already been validated, but
	 * not unmapped, but the maps are removed from the list.
	 */
	vma = kmem_cache_zalloc(vm_area_cachep, GFP_KERNEL);
	if (!vma) {
		error = -ENOMEM;
		goto unacct_error;
	}

	vma->vm_mm = mm;
	vma->vm_start = addr;
	vma->vm_end = addr + len;
	vma->vm_flags = vm_flags;
	vma->vm_page_prot = vm_get_page_prot(vm_flags);
	vma->vm_pgoff = pgoff;

	if (file) {
		error = -EINVAL;
		if (vm_flags & (VM_GROWSDOWN|VM_GROWSUP))
			goto free_vma;
		if (vm_flags & VM_DENYWRITE) {
			error = deny_write_access(file);
			if (error)
				goto free_vma;
			correct_wcount = 1;
		}
		vma->vm_file = file;
		get_file(file);
		error = file->f_op->mmap(file, vma);
		if (error)
			goto unmap_and_free_vma;
	} else if (vm_flags & VM_SHARED) {
		error = shmem_zero_setup(vma);
		if (error)
			goto free_vma;
	}

	/* We set VM_ACCOUNT in a shared mapping's vm_flags, to inform
	 * shmem_zero_setup (perhaps called through /dev/zero's ->mmap)
	 * that memory reservation must be checked; but that reservation
	 * belongs to shared memory object, not to vma: so now clear it.
	 */
	if ((vm_flags & (VM_SHARED|VM_ACCOUNT)) == (VM_SHARED|VM_ACCOUNT))
		vma->vm_flags &= ~VM_ACCOUNT;

	/* Can addr have changed??
	 *
	 * Answer: Yes, several device drivers can do it in their
	 *         f_op->mmap method. -DaveM
	 */
	addr = vma->vm_start;
	pgoff = vma->vm_pgoff;
	vm_flags = vma->vm_flags;

	if (vma_wants_writenotify(vma))
		vma->vm_page_prot = vm_get_page_prot(vm_flags & ~VM_SHARED);

	if (!file || !vma_merge(mm, prev, addr, vma->vm_end,
			vma->vm_flags, NULL, file, pgoff, vma_policy(vma))) {
		file = vma->vm_file;
		vma_link(mm, vma, prev, rb_link, rb_parent);
		if (correct_wcount)
			atomic_inc(&inode->i_writecount);
	} else {
		if (file) {
			if (correct_wcount)
				atomic_inc(&inode->i_writecount);
			fput(file);
		}
		mpol_free(vma_policy(vma));
		kmem_cache_free(vm_area_cachep, vma);
	}
out:
	mm->total_vm += len >> PAGE_SHIFT;
	vm_stat_account(mm, vm_flags, file, len >> PAGE_SHIFT);
	if (vm_flags & VM_LOCKED) {
		mm->locked_vm += len >> PAGE_SHIFT;
		make_pages_present(addr, addr + len);
	}
	if ((flags & MAP_POPULATE) && !(flags & MAP_NONBLOCK))
		make_pages_present(addr, addr + len);
	return addr;

unmap_and_free_vma:
	if (correct_wcount)
		atomic_inc(&inode->i_writecount);
	vma->vm_file = NULL;
	fput(file);

	/* Undo any partial mapping done by a device driver. */
	unmap_region(mm, vma, prev, vma->vm_start, vma->vm_end);
	charged = 0;
free_vma:
	kmem_cache_free(vm_area_cachep, vma);
unacct_error:
	if (charged)
		vm_unacct_memory(charged);
	return error;
}

/* Get an address range which is currently unmapped.
 * For shmat() with addr=0.
 *
 * Ugly calling convention alert:
 * Return value with the low bits set means error value,
 * ie
 *	if (ret & ~PAGE_MASK)
 *		error = ret;
 *
 * This function "knows" that -ENOMEM has the bits set.
 */
/*获取一个当前没有映射的地址区间，也就是找新vma。在向进程虚拟地址空间插入新的vma之前，
内核必须确认虚拟地址空间中有足够的空闲空间，可用于给定长度的区域*/

#ifndef HAVE_ARCH_UNMAPPED_AREA
unsigned long arch_get_unmapped_area(struct file *filp, unsigned long addr, unsigned long len,
											unsigned long pgoff, unsigned long flags)
{
	/*获取当前进程的虚拟内存空间*/
	struct mm_struct *mm = current->mm;
	/*保存新查找的映射实例地址*/
	struct vm_area_struct *vma;
	/*设置搜索起始地址，第一次搜索不成功时，设置TASK_UNMAPPED_BASE为重新搜索的起始地址*/
	unsigned long start_addr;
	/*指定长度超出进程虚拟地址空间长度，无效参数，返回内存不足错误信息*/
	if (len > TASK_SIZE)
		return -ENOMEM;
	/*如果是固定映射，直接返回指定地址*/
	if (flags & MAP_FIXED)
		return addr;
	/*如果指定了一个特定的优先选用（与固定地址不同）地址，内核会检查该区域是否与现存
	区域重叠，如果不重叠，则将该地址作为目标地址*/
	if (addr)
	{
		/*获取页对齐后的地址*/
		addr = PAGE_ALIGN(addr);
		/*在虚拟地址空间中查找addr < vma->vm_end区域的vma*/
		vma = find_vma(mm, addr);
		/*预分配[PAGE_ALIGN(addr), addr+len]不与任何现存vma重合，并且分配该vma后没有
		超出进程虚拟地址空间，则可以分配该vma*/
		if (TASK_SIZE - len >= addr &&		    (!vma || addr + len <= vma->vm_start))
			return addr;
	}
	/*实际的遍历，或者开始于虚拟地址空间中最后一个空洞的地址，或者开始于全局的起始地址*/
	if (len > mm->cached_hole_size)
	{
		start_addr = addr = mm->free_area_cache;
	}
	else
	{
	    start_addr = addr = TASK_UNMAPPED_BASE;
		mm->cached_hole_size = 0;
	}
full_search:
	/*查询进程虚拟地址空间中所有vma，先通过红黑树查询PAGE_ALINGN(addr) < vma->vm_end
	的第一个右侧vma，如果vma不存在，则说明查询地址在现在vma的最大vm_end的右侧，如果非
	空，则说明找到该地址右侧第一个vma*/
	for (vma = find_vma(mm, addr); ; vma = vma->vm_next)
	{
		/*此时(!vma || addr < vma->vm_end)，如果到达虚拟地址空间的顶端，则设置
		TASK_UNMAPPED_BASE起始地址为重新搜索的起始地址，如果本次查找就是从该起始位置开
		始的，则说明虚拟地址空间中目前不适合该次申请，返回内存不足错误信息*/
		if (TASK_SIZE - len < addr)
		{
			/*开始一次新的搜索，以防错过某些空洞			 */
			if (start_addr != TASK_UNMAPPED_BASE)
			{
				/*设置查找地址为起始地址，然后重新从头开始查找*/
				addr = TASK_UNMAPPED_BASE;
				start_addr = addr;
				mm->cached_hole_size = 0;
				goto full_search;
			}
			/*本次查找就是从TASK_UNMAPPED_BASE开始查找的，搜索持续到用户地址空间的末端
			（TASE_SIZE），仍然没有找打合适的vma，则查找失败，返回内存不足错误信息，错
			误必须发送到用户空间，且由相关的应用程序处理，该错误表明虚拟地址空间中可用
			内存不足，无法满足应用程序的请求*/
			return -ENOMEM;
		}
		/*虚拟地址空间中有适合该次分配的空闲区域，则分配成功*/
		if (!vma || addr + len <= vma->vm_start)
		{
			/*保存本次停止查找的地址*/
			mm->free_area_cache = addr + len;
			return addr;
		}
		/*查找地址addr在vma区域中，也即[PAGE_ALIGN(addr), PAGE_ALIGN(addr) + len]与
		[vma->vm_start, vma->vm_end)区域重叠*/
		if (addr + mm->cached_hole_size < vma->vm_start)
		        mm->cached_hole_size = vma->vm_start - addr;
		addr = vma->vm_end;
	}
}
#endif

/**/
void arch_unmap_area(struct mm_struct *mm, unsigned long addr)
{
	/*最低的可能地址起始处有一个空洞*/
	if (addr >= TASK_UNMAPPED_BASE && addr < mm->free_area_cache)
	{
		mm->free_area_cache = addr;
		mm->cached_hole_size = ~0UL;
	}
}

/*mmap分配器从栈底（与栈顶相对的另一端）自上而下分配一个新区域*/
#ifndef HAVE_ARCH_UNMAPPED_AREA_TOPDOWN
unsigned long arch_get_unmapped_area_topdown(struct file *filp, const unsigned long addr0,
			  			const unsigned long len, const unsigned long pgoff, const unsigned long flags)
{
	struct vm_area_struct *vma;
	/*获取当前进程的虚拟地址空间*/
	struct mm_struct *mm = current->mm;
	unsigned long addr = addr0;

	/*请求长度大于整个进程的虚拟地址空间，参数无效，返回内存不足错误*/
	if (len > TASK_SIZE)
		return -ENOMEM;

	/*如果指定固定映射地址，则返回该地址*/
	if (flags & MAP_FIXED)
		return addr;

	/*如果指定一个特定地址，则先将该地址对齐到页长度，然后查询该地址起始的长度是否与
	现有映射重叠，无重叠时可用*/
	if (addr)
	{
		/*将地址对齐到页长度*/
		addr = PAGE_ALIGN(addr);
		/*查找地址是否存在于现存的映射中*/
		vma = find_vma(mm, addr);
		/*如果该地址指定的区域在用户虚拟空间中，并且该地址区域不与现存的任何映射重叠，
		则可以分配该虚拟地址区域*/
		if (TASK_SIZE - len >= addr &&(!vma || addr + len <= vma->vm_start))
			return addr;
	}

	/*检查free_area_cache是否可用*/
	if (len <= mm->cached_hole_size)
	{
		mm->cached_hole_size = 0;
 		mm->free_area_cache = mm->mmap_base;
 	}

	/* either no address requested or can't fit in requested address hole */
	addr = mm->free_area_cache;

	/*确认剩余地址空间中有适合本次分配的区域*/
	if (addr > len)
	{
		/**/
		vma = find_vma(mm, addr-len);
		if (!vma || addr <= vma->vm_start)
			/* remember the address as a hint for next time */
			return (mm->free_area_cache = addr-len);
	}

	if (mm->mmap_base < len)
		goto bottomup;

	addr = mm->mmap_base-len;

	do {
		/*
		 * Lookup failure means no vma is above this address,
		 * else if new region fits below vma->vm_start,
		 * return with success:
		 */
		vma = find_vma(mm, addr);
		if (!vma || addr+len <= vma->vm_start)
			/* remember the address as a hint for next time */
			return (mm->free_area_cache = addr);

 		/* remember the largest hole we saw so far */
 		if (addr + mm->cached_hole_size < vma->vm_start)
 		        mm->cached_hole_size = vma->vm_start - addr;

		/* try just below the current vma->vm_start */
		addr = vma->vm_start-len;
	} while (len < vma->vm_start);

bottomup:
	/*
	 * A failed mmap() very likely causes application failure,
	 * so fall back to the bottom-up function here. This scenario
	 * can happen with large stack limits and large mmap()
	 * allocations.
	 */
	mm->cached_hole_size = ~0UL;
  	mm->free_area_cache = TASK_UNMAPPED_BASE;
	addr = arch_get_unmapped_area(filp, addr0, len, pgoff, flags);
	/*
	 * Restore the topdown base:
	 */
	mm->free_area_cache = mm->mmap_base;
	mm->cached_hole_size = ~0UL;

	return addr;
}
#endif

void arch_unmap_area_topdown(struct mm_struct *mm, unsigned long addr)
{
	/*
	 * Is this a new hole at the highest possible address?
	 */
	if (addr > mm->free_area_cache)
		mm->free_area_cache = addr;

	/* dont allow allocations above current base */
	if (mm->free_area_cache > mm->mmap_base)
		mm->free_area_cache = mm->mmap_base;
}

unsigned long
get_unmapped_area(struct file *file, unsigned long addr, unsigned long len,
		unsigned long pgoff, unsigned long flags)
{
	unsigned long (*get_area)(struct file *, unsigned long,
				  unsigned long, unsigned long, unsigned long);

	get_area = current->mm->get_unmapped_area;
	if (file && file->f_op && file->f_op->get_unmapped_area)
		get_area = file->f_op->get_unmapped_area;
	addr = get_area(file, addr, len, pgoff, flags);
	if (IS_ERR_VALUE(addr))
		return addr;

	if (addr > TASK_SIZE - len)
		return -ENOMEM;
	if (addr & ~PAGE_MASK)
		return -EINVAL;

	return addr;
}

EXPORT_SYMBOL(get_unmapped_area);

/*查询第一个addr<vm_end的vma，否则返回NULL，如果addr大于红黑树最右侧结点，则while循环
中左子树分支都不会进去，这个时候返回NULL，如果addr小于红黑树的最左侧结点，则说明addr
没有在任何vma实例中，返回的vma是符合addr<vma->vm_end条件的，但该addr不在vma指定的区间
内部*/
struct vm_area_struct * find_vma(struct mm_struct * mm, unsigned long addr)
{
	struct vm_area_struct *vma = NULL;
	/*指定的虚拟地址空间存在时，在其vma区域中查询符合要求的vma实例*/
	if (mm)
	{
		/*（根据程序的空间局部性原理）检查上次保存vma，典型的命中率在35%左右*/
		vma = mm->mmap_cache;
		/*上次保存的vma不符合要求*/
		if (!(vma && vma->vm_end > addr && vma->vm_start <= addr))
		{
			struct rb_node * rb_node;
			/*获取该虚拟地址空间中的组织vma的红黑树根结点*/
			rb_node = mm->mm_rb.rb_node;
			/*重新初始化保存查找结果的vma实例*/
			vma = NULL;
			while (rb_node)
			{
				/*虚拟地址空间中的vma是以红黑树结点的形式并入到红黑树中，因此，查询				vma
				信息需要根据contain_of机制，根据红黑树中结点rb_node信息查询对应的vma结点*/
				struct vm_area_struct * vma_tmp;
				/*根据红黑树中的结点获取对应的vma实例*/
				vma_tmp = rb_entry(rb_node,	struct vm_area_struct, vm_rb);
				/*如果当前结点的结束地址大于指定地址*/
				if (vma_tmp->vm_end > addr)
				{
					/*如果指定地址在查找到的vma的结束地址左侧，保存当前vma实例*/
					vma = vma_tmp;
					/*如果当前结点的起始地址不大于指定地址，说明该vma实例就是要找的
					vma实例，跳出循环*/
					if (vma_tmp->vm_start <= addr)
						break;
					/*当前vma实例的起始地址大于addr，说明需要在该实例的左子树中查找*/
					rb_node = rb_node->rb_left;
				}
				else
					/*当前vma实例的结束地址小于addr，说明需要在该结点的右子树中查询*/
					rb_node = rb_node->rb_right;
			}
			/*如果查找到addr在起止区间内的vma实例，则保存该查询结果作为下次查询开始结点*/
			if (vma)
				mm->mmap_cache = vma;
		}
	}
	return vma;
}

EXPORT_SYMBOL(find_vma);

/* Same as find_vma, but also return a pointer to the previous VMA in *pprev. */
struct vm_area_struct *find_vma_prev(struct mm_struct *mm, unsigned long addr,
												struct vm_area_struct **pprev)
{
	struct vm_area_struct *vma = NULL, *prev = NULL;
	struct rb_node * rb_node;
	if (!mm)
		goto out;

	/* Guard against addr being lower than the first VMA */
	vma = mm->mmap;

	/* Go through the RB tree quickly. */
	rb_node = mm->mm_rb.rb_node;

	while (rb_node) {
		struct vm_area_struct *vma_tmp;
		vma_tmp = rb_entry(rb_node, struct vm_area_struct, vm_rb);

		if (addr < vma_tmp->vm_end) {
			rb_node = rb_node->rb_left;
		} else {
			prev = vma_tmp;
			if (!prev->vm_next || (addr < prev->vm_next->vm_end))
				break;
			rb_node = rb_node->rb_right;
		}
	}

out:
	*pprev = prev;
	return prev ? prev->vm_next : vma;
}

/*
 * Verify that the stack growth is acceptable and
 * update accounting. This is shared with both the
 * grow-up and grow-down cases.
 */
static int acct_stack_growth(struct vm_area_struct * vma, unsigned long size, unsigned long grow)
{
	struct mm_struct *mm = vma->vm_mm;
	struct rlimit *rlim = current->signal->rlim;
	unsigned long new_start;

	/* address space limit tests */
	if (!may_expand_vm(mm, grow))
		return -ENOMEM;

	/* Stack limit test */
	if (size > rlim[RLIMIT_STACK].rlim_cur)
		return -ENOMEM;

	/* mlock limit tests */
	if (vma->vm_flags & VM_LOCKED) {
		unsigned long locked;
		unsigned long limit;
		locked = mm->locked_vm + grow;
		limit = rlim[RLIMIT_MEMLOCK].rlim_cur >> PAGE_SHIFT;
		if (locked > limit && !capable(CAP_IPC_LOCK))
			return -ENOMEM;
	}

	/* Check to ensure the stack will not grow into a hugetlb-only region */
	new_start = (vma->vm_flags & VM_GROWSUP) ? vma->vm_start :
			vma->vm_end - size;
	if (is_hugepage_only_range(vma->vm_mm, new_start, size))
		return -EFAULT;

	/*
	 * Overcommit..  This must be the final test, as it will
	 * update security statistics.
	 */
	if (security_vm_enough_memory(grow))
		return -ENOMEM;

	/* Ok, everything looks good - let it rip */
	mm->total_vm += grow;
	if (vma->vm_flags & VM_LOCKED)
		mm->locked_vm += grow;
	vm_stat_account(mm, vma->vm_flags, vma->vm_file, grow);
	return 0;
}

#if defined(CONFIG_STACK_GROWSUP) || defined(CONFIG_IA64)
/*
 * PA-RISC uses this for its stack; IA64 for its Register Backing Store.
 * vma is the last one with address > vma->vm_end.  Have to extend vma.
 */
#ifndef CONFIG_IA64
static inline
#endif
int expand_upwards(struct vm_area_struct *vma, unsigned long address)
{
	int error;

	if (!(vma->vm_flags & VM_GROWSUP))
		return -EFAULT;

	/*
	 * We must make sure the anon_vma is allocated
	 * so that the anon_vma locking is not a noop.
	 */
	if (unlikely(anon_vma_prepare(vma)))
		return -ENOMEM;
	anon_vma_lock(vma);

	/*
	 * vma->vm_start/vm_end cannot change under us because the caller
	 * is required to hold the mmap_sem in read mode.  We need the
	 * anon_vma lock to serialize against concurrent expand_stacks.
	 * Also guard against wrapping around to address 0.
	 */
	if (address < PAGE_ALIGN(address+4))
		address = PAGE_ALIGN(address+4);
	else {
		anon_vma_unlock(vma);
		return -ENOMEM;
	}
	error = 0;

	/* Somebody else might have raced and expanded it already */
	if (address > vma->vm_end) {
		unsigned long size, grow;

		size = address - vma->vm_start;
		grow = (address - vma->vm_end) >> PAGE_SHIFT;

		error = acct_stack_growth(vma, size, grow);
		if (!error)
			vma->vm_end = address;
	}
	anon_vma_unlock(vma);
	return error;
}
#endif /* CONFIG_STACK_GROWSUP || CONFIG_IA64 */


/*vma是第一个address < vma->vm_start的vma，必须扩展vma*/
static inline int expand_downwards(struct vm_area_struct *vma, unsigned long address)
{
	int error;

	/*
	 * We must make sure the anon_vma is allocated
	 * so that the anon_vma locking is not a noop.
	 */
	if (unlikely(anon_vma_prepare(vma)))
		return -ENOMEM;

	address &= PAGE_MASK;
	error = security_file_mmap(0, 0, 0, 0, address, 1);
	if (error)
		return error;

	anon_vma_lock(vma);

	/*
	 * vma->vm_start/vm_end cannot change under us because the caller
	 * is required to hold the mmap_sem in read mode.  We need the
	 * anon_vma lock to serialize against concurrent expand_stacks.
	 */

	/* Somebody else might have raced and expanded it already */
	if (address < vma->vm_start) {
		unsigned long size, grow;

		size = vma->vm_end - address;
		grow = (vma->vm_start - address) >> PAGE_SHIFT;

		error = acct_stack_growth(vma, size, grow);
		if (!error) {
			vma->vm_start = address;
			vma->vm_pgoff -= grow;
		}
	}
	anon_vma_unlock(vma);
	return error;
}

int expand_stack_downwards(struct vm_area_struct *vma, unsigned long address)
{
	return expand_downwards(vma, address);
}

#ifdef CONFIG_STACK_GROWSUP
int expand_stack(struct vm_area_struct *vma, unsigned long address)
{
	return expand_upwards(vma, address);
}

struct vm_area_struct *
find_extend_vma(struct mm_struct *mm, unsigned long addr)
{
	struct vm_area_struct *vma, *prev;

	addr &= PAGE_MASK;
	vma = find_vma_prev(mm, addr, &prev);
	if (vma && (vma->vm_start <= addr))
		return vma;
	if (!prev || expand_stack(prev, addr))
		return NULL;
	if (prev->vm_flags & VM_LOCKED)
		make_pages_present(addr, prev->vm_end);
	return prev;
}
#else
/*扩展栈空间*/
int expand_stack(struct vm_area_struct *vma, unsigned long address)
{
	return expand_downwards(vma, address);
}

struct vm_area_struct *find_extend_vma(struct mm_struct * mm, unsigned long addr)
{
	struct vm_area_struct * vma;
	unsigned long start;

	addr &= PAGE_MASK;
	vma = find_vma(mm,addr);
	if (!vma)
		return NULL;
	if (vma->vm_start <= addr)
		return vma;
	if (!(vma->vm_flags & VM_GROWSDOWN))
		return NULL;
	start = vma->vm_start;
	if (expand_stack(vma, addr))
		return NULL;
	if (vma->vm_flags & VM_LOCKED)
		make_pages_present(addr, start);
	return vma;
}
#endif

/*删除虚拟地址空间中指定的vma。操作期间应申请*/
static void remove_vma_list(struct mm_struct *mm, struct vm_area_struct *vma)
{
	/*减少虚拟内存前应先更新高水印值*/
	update_hiwater_vm(mm);
	do
	{
		/*获取vma所占用的虚拟地址空间页数目*/
		long nrpages = vma_pages(vma);
		/*更新虚拟地址空间中已申请的虚拟内存页数目*/
		mm->total_vm -= nrpages;
		/*如果该vma被锁定，则要更新被锁定页的数目*/
		if (vma->vm_flags & VM_LOCKED)
			mm->locked_vm -= nrpages;
		/*更新虚拟内存域中的统计信息*/
		vm_stat_account(mm, vma->vm_flags, vma->vm_file, -nrpages);
		/**/
		vma = remove_vma(vma);
	} while (vma);
	validate_mm(mm);
}

/*
 * Get rid of page table information in the indicated region.
 *
 * Called with the mm semaphore held.
 */
static void unmap_region(struct mm_struct *mm,	struct vm_area_struct *vma,
							struct vm_area_struct *prev,	unsigned long start, unsigned long end)
{
	struct vm_area_struct *next = prev? prev->vm_next: mm->mmap;
	struct mmu_gather *tlb;
	unsigned long nr_accounted = 0;

	lru_add_drain();
	tlb = tlb_gather_mmu(mm, 0);
	update_hiwater_rss(mm);
	unmap_vmas(&tlb, vma, start, end, &nr_accounted, NULL);
	vm_unacct_memory(nr_accounted);
	free_pgtables(&tlb, vma, prev? prev->vm_end: FIRST_USER_ADDRESS,
				 next? next->vm_start: 0);
	tlb_finish_mmu(tlb, start, end);
}

/*
 * Create a list of vma's touched by the unmap, removing them from the mm's
 * vma list as we go..
 */
static void
detach_vmas_to_be_unmapped(struct mm_struct *mm, struct vm_area_struct *vma,
	struct vm_area_struct *prev, unsigned long end)
{
	struct vm_area_struct **insertion_point;
	struct vm_area_struct *tail_vma = NULL;
	unsigned long addr;

	insertion_point = (prev ? &prev->vm_next : &mm->mmap);
	do {
		rb_erase(&vma->vm_rb, &mm->mm_rb);
		mm->map_count--;
		tail_vma = vma;
		vma = vma->vm_next;
	} while (vma && vma->vm_start < end);
	*insertion_point = vma;
	tail_vma->vm_next = NULL;
	if (mm->unmap_area == arch_unmap_area)
		addr = prev ? prev->vm_end : mm->mmap_base;
	else
		addr = vma ?  vma->vm_start : mm->mmap_base;
	mm->unmap_area(mm, addr);
	mm->mmap_cache = NULL;		/* Kill the cache. */
}

/*
 * Split a vma into two pieces at address 'addr', a new vma is allocated
 * either for the first part or the tail.
 */
int split_vma(struct mm_struct * mm, struct vm_area_struct * vma,
	      unsigned long addr, int new_below)
{
	struct mempolicy *pol;
	struct vm_area_struct *new;

	if (is_vm_hugetlb_page(vma) && (addr & ~HPAGE_MASK))
		return -EINVAL;

	if (mm->map_count >= sysctl_max_map_count)
		return -ENOMEM;

	new = kmem_cache_alloc(vm_area_cachep, GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	/* most fields are the same, copy all, and then fixup */
	*new = *vma;

	if (new_below)
		new->vm_end = addr;
	else {
		new->vm_start = addr;
		new->vm_pgoff += ((addr - vma->vm_start) >> PAGE_SHIFT);
	}

	pol = mpol_copy(vma_policy(vma));
	if (IS_ERR(pol)) {
		kmem_cache_free(vm_area_cachep, new);
		return PTR_ERR(pol);
	}
	vma_set_policy(new, pol);

	if (new->vm_file)
		get_file(new->vm_file);

	if (new->vm_ops && new->vm_ops->open)
		new->vm_ops->open(new);

	if (new_below)
		vma_adjust(vma, addr, vma->vm_end, vma->vm_pgoff +
			((addr - new->vm_start) >> PAGE_SHIFT), new);
	else
		vma_adjust(vma, vma->vm_start, addr, vma->vm_pgoff, new);

	return 0;
}

/* Munmap is split into 2 main parts -- this part which finds
 * what needs doing, and the areas themselves, which do the
 * work.  This now handles partial unmappings.
 * Jeremy Fitzhardinge <jeremy@goop.org>
 */
/**/
int do_munmap(struct mm_struct *mm, unsigned long start, size_t len)
{
	unsigned long end;
	struct vm_area_struct *vma, *prev, *last;

	if ((start & ~PAGE_MASK) || start > TASK_SIZE || len > TASK_SIZE-start)
		return -EINVAL;

	if ((len = PAGE_ALIGN(len)) == 0)
		return -EINVAL;

	/* Find the first overlapping VMA */
	vma = find_vma_prev(mm, start, &prev);
	if (!vma)
		return 0;
	/* we have  start < vma->vm_end  */

	/* if it doesn't overlap, we have nothing.. */
	end = start + len;
	if (vma->vm_start >= end)
		return 0;

	/*
	 * If we need to split any vma, do it now to save pain later.
	 *
	 * Note: mremap's move_vma VM_ACCOUNT handling assumes a partially
	 * unmapped vm_area_struct will remain in use: so lower split_vma
	 * places tmp vma above, and higher split_vma places tmp vma below.
	 */
	if (start > vma->vm_start) {
		int error = split_vma(mm, vma, start, 0);
		if (error)
			return error;
		prev = vma;
	}

	/* Does it split the last one? */
	last = find_vma(mm, end);
	if (last && end > last->vm_start) {
		int error = split_vma(mm, last, end, 1);
		if (error)
			return error;
	}
	vma = prev? prev->vm_next: mm->mmap;

	/*
	 * Remove the vma's, and unmap the actual pages
	 */
	detach_vmas_to_be_unmapped(mm, vma, prev, end);
	unmap_region(mm, vma, prev, start, end);

	/* Fix up all other VM information */
	remove_vma_list(mm, vma);

	return 0;
}

EXPORT_SYMBOL(do_munmap);

asmlinkage long sys_munmap(unsigned long addr, size_t len)
{
	int ret;
	struct mm_struct *mm = current->mm;

	profile_munmap(addr);

	down_write(&mm->mmap_sem);
	ret = do_munmap(mm, addr, len);
	up_write(&mm->mmap_sem);
	return ret;
}

static inline void verify_mm_writelocked(struct mm_struct *mm)
{
#ifdef CONFIG_DEBUG_VM
	/*非阻塞式获取读信号量*/
	if (unlikely(down_read_trylock(&mm->mmap_sem)))
	{
		/*显示文件名、行号、函数名调试信息*/
		WARN_ON(1);
		/*显示调用栈信息*/
		up_read(&mm->mmap_sem);
	}
#endif
}

/*
 *  this is really a simplified "do_mmap".  it only handles
 *  anonymous maps.  eventually we may be able to do some
 *  brk-specific accounting here.
 */
/*调用malloc申请内存时，如果申请的是小块内存（低于128K）则会使用do_brk()系统调用通过
调整堆中的brk指针大小来增加或者回收堆内存.如果申请的是比较大块的内存（超过128K）时，
则会调用mmap在上图虚拟内存空间中的文件映射与匿名映射区创建出一块VMA内存区域*/
unsigned long do_brk(unsigned long addr, unsigned long len)
{
	/*获取当前进程的虚拟地址空间*/
	struct mm_struct * mm = current->mm;
	struct vm_area_struct * vma, * prev;
	unsigned long flags;
	struct rb_node ** rb_link, * rb_parent;
	/*获取页对齐后的地址偏移*/
	pgoff_t pgoff = addr >> PAGE_SHIFT;
	int error;
	/*获取页对齐后的长度*/
	len = PAGE_ALIGN(len);
	/*长度为零，不用扩展堆，直接返回原来堆指针*/
	if (!len)
		return addr;
	/*申请的地址空间结束位置超过用户进程虚拟地址空间上限。或超过无符号长整形表示范围溢
	出截断后小于起始地址，都返回无效参数错误信息*/
	if ((addr + len) > TASK_SIZE || (addr + len) < addr)
		return -EINVAL;
	/**/
	if (is_hugepage_only_range(mm, addr, len))
		return -EINVAL;
	/**/
	error = security_file_mmap(0, 0, 0, 0, addr, 1);
	if (error)
		return error;
	/*设置申请空间的页保护标识*/
	flags = VM_DATA_DEFAULT_FLAGS | VM_ACCOUNT | mm->def_flags;
	/**/
	error = arch_mmap_check(addr, len, flags);
	if (error)
		return error;

	/*
	 * mlock MCL_FUTURE?
	 */
	/*如果虚拟内存区被锁定，则判断锁定后的页数目是否超出进程允许的锁定数目，超过时进
	程是否有权限调用mlock类函数取锁定多出的页*/
	if (mm->def_flags & VM_LOCKED)
	{
		unsigned long locked, lock_limit;
		/*获取预锁定页数目*/
		locked = len >> PAGE_SHIFT;
		/*获取进程已锁定页数目*/
		locked += mm->locked_vm;
		/*获取当前进程可以锁定的最大长度*/
		lock_limit = current->signal->rlim[RLIMIT_MEMLOCK].rlim_cur;
		/*获取当前进程可以锁定的页数目*/
		lock_limit >>= PAGE_SHIFT;
		/*如果锁定的页数目大于系统允许锁定的页数目且进程没有调用mlock类函数的权限，则
		返回重试一次错误信息*/
		if (locked > lock_limit && !capable(CAP_IPC_LOCK))
			return -EAGAIN;
	}

	/*当进程睡眠时，需要申请mm->mmap_sem防止另一个进程修改映射信息*/
	verify_mm_writelocked(mm);

	/*
	 * Clear old maps.  this also does some error checking for us
	 */
	/*清除旧映射*/
 munmap_back:
	vma = find_vma_prepare(mm, addr, &prev, &rb_link, &rb_parent);
	if (vma && vma->vm_start < addr + len)
	{
		if (do_munmap(mm, addr, len))
			return -ENOMEM;
		goto munmap_back;
	}

	/* Check against address space limits *after* clearing old maps... */
	if (!may_expand_vm(mm, len >> PAGE_SHIFT))
		return -ENOMEM;

	if (mm->map_count > sysctl_max_map_count)
		return -ENOMEM;

	if (security_vm_enough_memory(len >> PAGE_SHIFT))
		return -ENOMEM;

	/* Can we just expand an old private anonymous mapping? */
	if (vma_merge(mm, prev, addr, addr + len, flags,
					NULL, NULL, pgoff, NULL))
		goto out;

	/*
	 * create a vma struct for an anonymous mapping
	 */
	vma = kmem_cache_zalloc(vm_area_cachep, GFP_KERNEL);
	if (!vma) {
		vm_unacct_memory(len >> PAGE_SHIFT);
		return -ENOMEM;
	}

	vma->vm_mm = mm;
	vma->vm_start = addr;
	vma->vm_end = addr + len;
	vma->vm_pgoff = pgoff;
	vma->vm_flags = flags;
	vma->vm_page_prot = vm_get_page_prot(flags);
	vma_link(mm, vma, prev, rb_link, rb_parent);
out:
	mm->total_vm += len >> PAGE_SHIFT;
	if (flags & VM_LOCKED) {
		mm->locked_vm += len >> PAGE_SHIFT;
		make_pages_present(addr, addr + len);
	}
	return addr;
}

EXPORT_SYMBOL(do_brk);

/* Release all mmaps. */
void exit_mmap(struct mm_struct *mm)
{
	struct mmu_gather *tlb;
	struct vm_area_struct *vma = mm->mmap;
	unsigned long nr_accounted = 0;
	unsigned long end;

	/* mm's last user has gone, and its about to be pulled down */
	arch_exit_mmap(mm);

	lru_add_drain();
	flush_cache_mm(mm);
	tlb = tlb_gather_mmu(mm, 1);
	/* Don't update_hiwater_rss(mm) here, do_exit already did */
	/* Use -1 here to ensure all VMAs in the mm are unmapped */
	end = unmap_vmas(&tlb, vma, 0, -1, &nr_accounted, NULL);
	vm_unacct_memory(nr_accounted);
	free_pgtables(&tlb, vma, FIRST_USER_ADDRESS, 0);
	tlb_finish_mmu(tlb, 0, end);

	/*
	 * Walk the list again, actually closing and freeing it,
	 * with preemption enabled, without holding any MM locks.
	 */
	while (vma)
		vma = remove_vma(vma);

	BUG_ON(mm->nr_ptes > (FIRST_USER_ADDRESS+PMD_SIZE-1)>>PMD_SHIFT);
}

/* Insert vm structure into process list sorted by address
 * and into the inode's i_mmap tree.  If vm_file is non-NULL
 * then i_mmap_lock is taken here.
 */
/*内核用于插入新区域的标准函数*/
int insert_vm_struct(struct mm_struct * mm, struct vm_area_struct * vma)
{
	struct vm_area_struct * __vma, * prev;
	struct rb_node ** rb_link, * rb_parent;

	/*
	 * The vm_pgoff of a purely anonymous vma should be irrelevant
	 * until its first write fault, when page's anon_vma and index
	 * are set.  But now set the vm_pgoff it will almost certainly
	 * end up with (unless mremap moves it elsewhere before that
	 * first wfault), so /proc/pid/maps tells a consistent story.
	 *
	 * By setting it to reflect the virtual start address of the
	 * vma, merges and splits can happen in a seamless way, just
	 * using the existing file pgoff checks and manipulations.
	 * Similarly in do_mmap_pgoff and in do_brk.
	 */
	/*该vma对应的是匿名映射区域*/
	if (!vma->vm_file)
	{
		/**/
		BUG_ON(vma->anon_vma);
		/*获取映射区域的页偏移*/
		vma->vm_pgoff = vma->vm_start >> PAGE_SHIFT;
	}
	__vma = find_vma_prepare(mm,vma->vm_start,&prev,&rb_link,&rb_parent);
	if (__vma && __vma->vm_start < vma->vm_end)
		return -ENOMEM;
	if ((vma->vm_flags & VM_ACCOUNT) &&
	     security_vm_enough_memory_mm(mm, vma_pages(vma)))
		return -ENOMEM;
	vma_link(mm, vma, prev, rb_link, rb_parent);
	return 0;
}

/*
 * Copy the vma structure to a new location in the same mm,
 * prior to moving page table entries, to effect an mremap move.
 */
struct vm_area_struct *copy_vma(struct vm_area_struct **vmap,
	unsigned long addr, unsigned long len, pgoff_t pgoff)
{
	struct vm_area_struct *vma = *vmap;
	unsigned long vma_start = vma->vm_start;
	struct mm_struct *mm = vma->vm_mm;
	struct vm_area_struct *new_vma, *prev;
	struct rb_node **rb_link, *rb_parent;
	struct mempolicy *pol;

	/*
	 * If anonymous vma has not yet been faulted, update new pgoff
	 * to match new location, to increase its chance of merging.
	 */
	if (!vma->vm_file && !vma->anon_vma)
		pgoff = addr >> PAGE_SHIFT;

	find_vma_prepare(mm, addr, &prev, &rb_link, &rb_parent);
	new_vma = vma_merge(mm, prev, addr, addr + len, vma->vm_flags,
			vma->anon_vma, vma->vm_file, pgoff, vma_policy(vma));
	if (new_vma) {
		/*
		 * Source vma may have been merged into new_vma
		 */
		if (vma_start >= new_vma->vm_start &&
		    vma_start < new_vma->vm_end)
			*vmap = new_vma;
	} else {
		new_vma = kmem_cache_alloc(vm_area_cachep, GFP_KERNEL);
		if (new_vma) {
			*new_vma = *vma;
			pol = mpol_copy(vma_policy(vma));
			if (IS_ERR(pol)) {
				kmem_cache_free(vm_area_cachep, new_vma);
				return NULL;
			}
			vma_set_policy(new_vma, pol);
			new_vma->vm_start = addr;
			new_vma->vm_end = addr + len;
			new_vma->vm_pgoff = pgoff;
			if (new_vma->vm_file)
				get_file(new_vma->vm_file);
			if (new_vma->vm_ops && new_vma->vm_ops->open)
				new_vma->vm_ops->open(new_vma);
			vma_link(mm, new_vma, prev, rb_link, rb_parent);
		}
	}
	return new_vma;
}

/*
 * Return true if the calling process may expand its vm space by the passed
 * number of pages
 */
int may_expand_vm(struct mm_struct *mm, unsigned long npages)
{
	unsigned long cur = mm->total_vm;	/* pages */
	unsigned long lim;

	lim = current->signal->rlim[RLIMIT_AS].rlim_cur >> PAGE_SHIFT;

	if (cur + npages > lim)
		return 0;
	return 1;
}


static struct page *special_mapping_nopage(struct vm_area_struct *vma,
					   unsigned long address, int *type)
{
	struct page **pages;

	BUG_ON(address < vma->vm_start || address >= vma->vm_end);

	address -= vma->vm_start;
	for (pages = vma->vm_private_data; address > 0 && *pages; ++pages)
		address -= PAGE_SIZE;

	if (*pages) {
		struct page *page = *pages;
		get_page(page);
		return page;
	}

	return NOPAGE_SIGBUS;
}

/*
 * Having a close hook prevents vma merging regardless of flags.
 */
static void special_mapping_close(struct vm_area_struct *vma)
{
}

static struct vm_operations_struct special_mapping_vmops = {
	.close = special_mapping_close,
	.nopage	= special_mapping_nopage,
};

/*
 * Called with mm->mmap_sem held for writing.
 * Insert a new vma covering the given region, with the given flags.
 * Its pages are supplied by the given array of struct page *.
 * The array can be shorter than len >> PAGE_SHIFT if it's null-terminated.
 * The region past the last page supplied will always produce SIGBUS.
 * The array pointer and the pages it points to are assumed to stay alive
 * for as long as this mapping might exist.
 */
int install_special_mapping(struct mm_struct *mm,
			    unsigned long addr, unsigned long len,
			    unsigned long vm_flags, struct page **pages)
{
	struct vm_area_struct *vma;

	vma = kmem_cache_zalloc(vm_area_cachep, GFP_KERNEL);
	if (unlikely(vma == NULL))
		return -ENOMEM;

	vma->vm_mm = mm;
	vma->vm_start = addr;
	vma->vm_end = addr + len;

	vma->vm_flags = vm_flags | mm->def_flags;
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

	vma->vm_ops = &special_mapping_vmops;
	vma->vm_private_data = pages;

	if (unlikely(insert_vm_struct(mm, vma))) {
		kmem_cache_free(vm_area_cachep, vma);
		return -ENOMEM;
	}

	mm->total_vm += len >> PAGE_SHIFT;

	return 0;
}
