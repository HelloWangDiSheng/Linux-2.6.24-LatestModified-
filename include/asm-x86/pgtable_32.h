#ifndef _I386_PGTABLE_H
#define _I386_PGTABLE_H
/*
 * The Linux memory management assumes a three-level page table setup. On
 * the i386, we use that, but "fold" the mid level into the top-level page
 * table, so that we physically have the same two-level page table as the
 * i386 mmu expects.
 *
 * This file contains the functions and defines necessary to modify and use
 * the i386 page table tree.
 */
#ifndef __ASSEMBLY__
#include <asm/processor.h>
#include <asm/fixmap.h>
#include <linux/threads.h>
#include <asm/paravirt.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>

struct mm_struct;
struct vm_area_struct;

/*零页是一个经常用于已建立内存映射的页内值为零的全局共享页*/
#define ZERO_PAGE(vaddr) (virt_to_page(empty_zero_page))
extern unsigned long empty_zero_page[1024];
extern pgd_t swapper_pg_dir[1024];
extern struct kmem_cache *pmd_cache;
extern spinlock_t pgd_lock;
extern struct page *pgd_list;
void check_pgt_cache(void);

void pmd_ctor(struct kmem_cache *, void *);
void pgtable_cache_init(void);
void paging_init(void);

/*
 * The Linux x86 paging architecture is 'compile-time dual-mode', it
 * implements both the traditional 2-level x86 page tables and the
 * newer 3-level PAE-mode page tables.
 */
#ifdef CONFIG_X86_PAE
#include <asm/pgtable-3level-defs.h>
#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE-1))
#else
#include <asm/pgtable-2level-defs.h>
#endif
/*全局页目录（Page Global Directory）数目，默认是4M（1<<22）*/
#define PGDIR_SIZE					(1UL << PGDIR_SHIFT)
/*全局页目录掩码，低22位全部清零*/
#define PGDIR_MASK					(~(PGDIR_SIZE - 1))
/*用户进程可用的全局页目录数目，默认是3M*/
#define USER_PTRS_PER_PGD			(TASK_SIZE / PGDIR_SIZE)
#define FIRST_USER_ADDRESS			0
/*用户进程可用的全局页目录数目，配置3G时是768*/
#define USER_PGD_PTRS 				(PAGE_OFFSET >> PGDIR_SHIFT)
/*内核进程可用的全局页目录数目（默认是1024-768=256）*/
#define KERNEL_PGD_PTRS 			(PTRS_PER_PGD - USER_PGD_PTRS)
/*二级页表中页目录项偏移位*/
#define TWOLEVEL_PGDIR_SHIFT		22
/**/
#define BOOT_USER_PGD_PTRS			(__PAGE_OFFSET >> TWOLEVEL_PGDIR_SHIFT)
/**/
#define BOOT_KERNEL_PGD_PTRS 		(1024 - BOOT_USER_PGD_PTRS)

/* Just any arbitrary offset to the start of the vmalloc VM area: the
 * current 8MB value just means that there will be a 8MB "hole" after the
 * physical memory until the kernel virtual memory starts.  That means that
 * any out-of-bounds memory accesses will hopefully be caught.
 * The vmalloc() routines leaves a hole of 4kB between each vmalloced
 * area for the same reason. ;)
 */
/*直接映射区和和VMALLOC区域之间的8M缺口，该缺口可用作针对任何内核故障的保护措施。如
果访问越界地址（即无意地访问物理上不存在的内存区），则访问失败并生成一个异常，报告该
错误。如果VMALLOC区域紧接着直接映射取，那么访问将成功而不会注意到错误。在稳定运行的情
况下，肯定不需要这个额外的保护措施，但它对开发尚未乘数的新内核特性时有用的*/
#define VMALLOC_OFFSET	(8*1024*1024)
/*VMALLOC区域的起始地址，对齐到高端内存域high_memory之后间隔VMALLOC_OFFSET的起始地址
。虚拟内存中连续，物理内存中不连续的内存区，可以在VMALOC区域中分配。改机制通常用于用
户进程，内核会试图尽量避免非连续的内存区。内核通常会成功，因为大部分的内存块都在启动
时分配给内核，那时的内存碎片问题尚不严重，但在已经运行了很长时间的系统上，在内核需要
物理内存时，就可能出现不连续的情况。此类情况，主要出现在动态加载内核模块时*/
#define VMALLOC_START	(((unsigned long) high_memory + 2*VMALLOC_OFFSET-1) & ~(VMALLOC_OFFSET-1))
#ifdef CONFIG_HIGHMEM
/*VMALLOC区域的结束地址，有高端内存时，在持续映射区开始位置的前两个保护页起始位置结束*/
#define VMALLOC_END	(PKMAP_BASE-2*PAGE_SIZE)
#else
/*VMALLOC区域的结束地址。无高端内存时，在固定映射取起始位置的前两个保护页的起始位置结束*/
#define VMALLOC_END	(FIXADDR_START-2*PAGE_SIZE)
#endif

/*页目录项中_PAGE_PSE集仅仅意味着该页目录项直接指向一个内存中4M毗邻块（巨型页）*/
#define _PAGE_BIT_PRESENT		0
#define _PAGE_BIT_RW			1
#define _PAGE_BIT_USER			2
#define _PAGE_BIT_PWT			3
#define _PAGE_BIT_PCD			4
#define _PAGE_BIT_ACCESSED		5
#define _PAGE_BIT_DIRTY			6
#define _PAGE_BIT_PSE			7
#define _PAGE_BIT_GLOBAL		8
#define _PAGE_BIT_UNUSED1		9
#define _PAGE_BIT_UNUSED2		10
#define _PAGE_BIT_UNUSED3		11
#define _PAGE_BIT_NX			63

/*表示该PTE映射的物理内存页是否在内存中，值为1表示物理内存页在内存中驻留，值为0表示
物理内存页不在内存中，可能被swap到磁盘上了。当PTE中的P位为0时，上图中的其他权限位将
变得没有意义，这种情况下其他bit位存放物理内存页在磁盘中的地址。当物理内存页需要被
swap in的时候，内核会在这里找到物理内存页在磁盘中的位置.当我们通过上述虚拟内存寻址过
程找到其对应的PTE之后，首先会检查它的P位，如果为0直接触发缺页中断（page fault），随
后进入内核态，由内核的缺页异常处理程序负责将映射的物理页面换入到内存中*/
#define _PAGE_PRESENT			0x001
/*表示进程对该物理内存页拥有的读，写权限，值为1表示进程对该物理页拥有读写权限，值为0
表示进程对该物理页拥有只读权限，进程对只读页面进行写操作将触发page fault（写保护中断
异常），用于写时复制（Copy On Write，COW）的场景。比如，父进程通过fork系统调用创建子
进程之后，父子进程的虚拟内存空间完全是一模一样的，包括父子进程的页表内容都是一样的，
父子进程页表中的PTE均指向同一物理内存页面，此时内核会将父子进程页表中的PTE均改为只读
的，并将父子进程共同映射的这个物理页面引用计数加1。当父进程或者子进程对该页面发生写操
作的时候，我们现在假设子进程先对页面发生写操作，随后子进程发现自己页表中的PTE是只读的
，于是产生写保护中断，子进程进入内核态，在内核的缺页中断处理程序中发现，访问的这个物理
页面引用计数大于1，说明此时该物理内存页面存在多进程共享的情况，于是发生写时复制（Copy
On Write，COW），内核为子进程重新分配一个新的物理页面，然后将原来物理页中的内容拷贝到
新的页面中，最后子进程页表中的PTE指向新的物理页面并将PTE的R/W位设置为1，原来物理页面的
引用计数减1。后面父进程在对页面进行写操作的时候，同样也会发现父进程的页表中PTE是只读的
，也会产生写保护中断，但是在内核的缺页中断处理程序中，发现访问的这个物理页面引用计数为
1了，那么就只需要将父进程页表中的PTE的R/W位设置为1就可以了。*/
#define _PAGE_RW				0x002
/*值为0表示该物理内存页只有内核才可以访问，值为1表示用户空间的进程也可以访问*/
#define _PAGE_USER				0x004
/*同样也是和CPU CACHE  相关的控制位，Page Write Through的缩写，值为1表示CPU CACHE中的数
据发生修改之后，采用Write Through的方式同步回物理内存页中。值为0表示采用Write Back的方
式同步回物理内存页。当CPU修改了高速缓存中的数据之后，这些修改后的缓存内容同步回内存的方
式有两种：（1）Write Back：CPU修改后的缓存数据不会立马同步回内存，只有当cache line被替
换时，这些修改后的缓存数据才会被同步回内存中，并覆盖掉对应物理内存页中旧的数据。（2）
Write Through：CPU  修改高速缓存中的数据之后，会立刻被同步回物理内存页中*/
#define _PAGE_PWT				0x008
/*是Page Cache Disabled的缩写，表示PTE指向的这个物理内存页中的内容是否可以被缓存在CPU
CACHE中，值为1表示Disabled，值为0表示Enabled*/
#define _PAGE_PCD				0x010
/*表示PTE指向的这个物理内存页最近是否被访问过，1表示最近被访问过（读或者写访问都会设置
为1），0表示没有。该bit位被硬件MMU设置，由操作系统重置。内核会经常检查该比特位，以确定
该物理内存页的活跃程度，不经常使用的内存页，很可能就会被内核swap out出去*/
#define _PAGE_ACCESSED			0x020
/*主要针对文件页使用，当PTE指向的物理内存页是一个文件页时，进程对这个文件页写入了新的
数据，这时文件页就变成了脏页，对应的PTE中D比特位会被设置为1，表示文件页中的内容与其背
后对应磁盘中的文件内容不同步了*/
#define _PAGE_DIRTY				0x040
/* 4 MB (or 2MB) page, Pentium+, if present.. */
#define _PAGE_PSE				0x080
/*设置为1表示该PTE是全局的，该标志位表示PTE中保存的映射关系是否是全局的，一般来说进程
都有各自独立的虚拟内存空间，进程的页表也是独立的，CPU每次访问进程虚拟内存地址的时候都
需要进行地址翻译，为了加速地址翻译的速度，避免每次遍历页表，CPU会把经常被访问到的PTE
缓存在一个TLB的硬件缓存中，由于TLB中缓存的是当前进程相关的PTE，所以操作系统每次在切换
进程的时候，都会重新刷新TLB缓存。而有一些PTE是所有进程共享的，比如说内核虚拟内存空间中
的映射关系，所有进程进入内核态看到的都是一样的。所以会将这些全局共享的PTE中的G比特位置
为1，这样在每次进程切换的时候，就不会flush掉TLB缓存的那些共享的全局PTE比如内核地址的空
间中使用的PTE）,从而在很大程度上提升了性能*/
#define _PAGE_GLOBAL			0x100
/*程序员可用*/
#define _PAGE_UNUSED1			0x200
#define _PAGE_UNUSED2			0x400
#define _PAGE_UNUSED3			0x800
/*如果_PAGE_RESENT是清零状态，使用以下标识*/
/*页在内存中时是非线性映射标识，页不在内存中时则该项指向一个换出页的位置*/
#define _PAGE_FILE				0x040	/* nonlinear file mapping, saved PTE; unset:swap */
/*如果用户使用PROT_NONE映射页，调用pte_present时返回true。该比特位置位的页。相当于标
记为mmap系统调用完全不可访问。尽管这种页不需要对应到物理页帧上，内核可以以这种方式禁
止访问该页*/
#define _PAGE_PROTNONE			0x080
#ifdef CONFIG_X86_PAE
#define _PAGE_NX	(1ULL<<_PAGE_BIT_NX)
#else
/*将页标记为不可执行*/
#define _PAGE_NX	0
#endif

/**/
#define _PAGE_TABLE	(_PAGE_PRESENT | _PAGE_RW | _PAGE_USER | _PAGE_ACCESSED | _PAGE_DIRTY)
/**/
#define _KERNPG_TABLE	(_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED | _PAGE_DIRTY)
/**/
#define _PAGE_CHG_MASK	(PTE_MASK | _PAGE_ACCESSED | _PAGE_DIRTY)
/**/
#define PAGE_NONE __pgprot(_PAGE_PROTNONE | _PAGE_ACCESSED)
/*共享页标识*/
#define PAGE_SHARED __pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_USER | _PAGE_ACCESSED)
/*可执行的共享页*/
#define PAGE_SHARED_EXEC __pgprot(_PAGE_PRESENT | _PAGE_RW | _PAGE_USER | _PAGE_ACCESSED)
/*可复制的共享页标识*/
#define PAGE_COPY_NOEXEC __pgprot(_PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED | _PAGE_NX)
/*可复制执行的页标识*/
#define PAGE_COPY_EXEC __pgprot(_PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED)
#define PAGE_COPY PAGE_COPY_NOEXEC
/*只读页标识，不可写或执行*/
#define PAGE_READONLY __pgprot(_PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED | _PAGE_NX)
/**/
#define PAGE_READONLY_EXEC __pgprot(_PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED)
/*可读写的非执行的内核页标识*/
#define _PAGE_KERNEL 	(_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_NX)
/*可读写执行的内核页标识*/
#define _PAGE_KERNEL_EXEC (_PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED)

extern unsigned long long __PAGE_KERNEL, __PAGE_KERNEL_EXEC;
/*只读内核标识*/
#define __PAGE_KERNEL_RO				(__PAGE_KERNEL & ~_PAGE_RW)
/**/
#define __PAGE_KERNEL_RX				(__PAGE_KERNEL_EXEC & ~_PAGE_RW)
/**/
#define __PAGE_KERNEL_NOCACHE			(__PAGE_KERNEL | _PAGE_PCD)
/**/
#define __PAGE_KERNEL_LARGE				(__PAGE_KERNEL | _PAGE_PSE)
/**/
#define __PAGE_KERNEL_LARGE_EXEC		(__PAGE_KERNEL_EXEC | _PAGE_PSE)
/**/
#define PAGE_KERNEL						__pgprot(__PAGE_KERNEL)
/**/
#define PAGE_KERNEL_RO					__pgprot(__PAGE_KERNEL_RO)
/**/
#define PAGE_KERNEL_EXEC				__pgprot(__PAGE_KERNEL_EXEC)
/**/
#define PAGE_KERNEL_RX					__pgprot(__PAGE_KERNEL_RX)
/**/
#define PAGE_KERNEL_NOCACHE				__pgprot(__PAGE_KERNEL_NOCACHE)
/**/
#define PAGE_KERNEL_LARGE				__pgprot(__PAGE_KERNEL_LARGE)
/**/
#define PAGE_KERNEL_LARGE_EXEC			__pgprot(__PAGE_KERNEL_LARGE_EXEC)

/*
 * The i386 can't do page protection for execute, and considers that
 * the same are read. Also, write permissions imply read permissions.
 * This is the closest we can get..
 */
#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY
#define __P011	PAGE_COPY
#define __P100	PAGE_READONLY_EXEC
#define __P101	PAGE_READONLY_EXEC
#define __P110	PAGE_COPY_EXEC
#define __P111	PAGE_COPY_EXEC

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED
#define __S011	PAGE_SHARED
#define __S100	PAGE_READONLY_EXEC
#define __S101	PAGE_READONLY_EXEC
#define __S110	PAGE_SHARED_EXEC
#define __S111	PAGE_SHARED_EXEC

/*
 * Define this if things work differently on an i386 and an i486:
 * it will (on an i486) warn about kernel memory accesses that are
 * done without a 'access_ok(VERIFY_WRITE,..)'
 */
#undef TEST_ACCESS_OK

/*启动页表（所有的页表都创建为一维数组）*/
extern unsigned long pg0[];
/*检查页是否在内存中，如果页在内存中，_PAGE_PRESENT是置位的*/
#define pte_present(x)	((x).pte_low & (_PAGE_PRESENT | _PAGE_PROTNONE))

/*为避免有害竟态，PAE功能启用时应该检查pmd_none(x)低位*/
#define pmd_none(x)	(!(unsigned long)pmd_val(x))
/*检查中间页目录项PND是否在内存中*/
#define pmd_present(x)	(pmd_val(x) & _PAGE_PRESENT)
/*检查中间页目录项是否无效。如果函数从外部接收输入参数，则无法假定参数是有效的。为
保证安全，可以调用这些函数进行检查*/
#define	pmd_bad(x)	((pmd_val(x) & (~PAGE_MASK & ~_PAGE_USER)) != _KERNPG_TABLE)
#define pages_to_mb(x) ((x) >> (20-PAGE_SHIFT))

/*仅当页在内存中时（也即pte_present()为真时），以下函数才有效，否则结果未定义*/
/*页表项是否是“脏“的，被修改过，与后备存储器内容不一致*/
static inline int pte_dirty(pte_t pte)		{ return (pte).pte_low & _PAGE_DIRTY; }
/*测试页是否刚被访问过*/
static inline int pte_young(pte_t pte)		{ return (pte).pte_low & _PAGE_ACCESSED; }
/*测试页是否可写*/
static inline int pte_write(pte_t pte)		{ return (pte).pte_low & _PAGE_RW; }
/*页是否是巨型页*/
static inline int pte_huge(pte_t pte)		{ return (pte).pte_low & _PAGE_PSE; }

/*以下函数在pte_present()为真时才有效*/
/*pte_file用于非线性映射，通过操作页表提供了文件内容的一种不同视图mmap。该函数检查页
表项是否属于这样的一个映射。只有在pte_present返回false时，才能调用pte_file，即与该页
表项相关的页不在内存中。由于内核的通用代码对pte_file的依赖，在某个体系结构并不支持非
线性映射的情况下也需要定义该函数。在这种情况下，该函数总是返回0*/
static inline int pte_file(pte_t pte)		{ return (pte).pte_low & _PAGE_FILE; }
/*清除页脏标识*/
static inline pte_t pte_mkclean(pte_t pte)	{ (pte).pte_low &= ~_PAGE_DIRTY; return pte; }
/*清除页访问标识*/
static inline pte_t pte_mkold(pte_t pte)	{ (pte).pte_low &= ~_PAGE_ACCESSED; return pte; }
/*清除页的写保护*/
static inline pte_t pte_wrprotect(pte_t pte)	{ (pte).pte_low &= ~_PAGE_RW; return pte; }
/*设置页脏标识*/
static inline pte_t pte_mkdirty(pte_t pte)	{ (pte).pte_low |= _PAGE_DIRTY; return pte; }
/*设置页刚被cpu访问过*/
static inline pte_t pte_mkyoung(pte_t pte)	{ (pte).pte_low |= _PAGE_ACCESSED; return pte; }
/*设置页的写保护标识*/
static inline pte_t pte_mkwrite(pte_t pte)	{ (pte).pte_low |= _PAGE_RW; return pte; }
/*设置页为巨型页标识*/
static inline pte_t pte_mkhuge(pte_t pte)	{ (pte).pte_low |= _PAGE_PSE; return pte; }

#ifdef CONFIG_X86_PAE
#include <asm/pgtable-3level.h>
#else
#include <asm/pgtable-2level.h>
#endif

#ifndef CONFIG_PARAVIRT
/*
 * Rules for using pte_update - it must be called after any PTE update which
 * has not been done using the set_pte / clear_pte interfaces.  It is used by
 * shadow mode hypervisors to resynchronize the shadow page tables.  Kernel PTE
 * updates should either be sets, clears, or set_pte_atomic for P->P
 * transitions, which means this hook should only be called for user PTEs.
 * This hook implies a P->P protection or access change has taken place, which
 * requires a subsequent TLB flush.  The notification can optionally be delayed
 * until the TLB flush event by using the pte_update_defer form of the
 * interface, but care must be taken to assure that the flush happens while
 * still holding the same page table lock so that the shadow and primary pages
 * do not become out of sync on SMP.
 */
#define pte_update(mm, addr, ptep)		do { } while (0)
#define pte_update_defer(mm, addr, ptep)	do { } while (0)
#endif

/*获取并清除页表项，返回之前页表项的信息。本地pte更新不需要使用锁定的xchg*/
static inline pte_t native_local_ptep_get_and_clear(pte_t *ptep)
{
	pte_t res = *ptep;

	/* Pure native function needs no input for mm, addr */
	native_pte_clear(NULL, 0, ptep);
	return res;
}

/*
 * We only update the dirty/accessed state if we set
 * the dirty bit by hand in the kernel, since the hardware
 * will do the accessed bit for us, and we don't want to
 * race with other CPU's that might be updating the dirty
 * bit at the same time.
 */
#define  __HAVE_ARCH_PTEP_SET_ACCESS_FLAGS
/**/
#define ptep_set_access_flags(vma, address, ptep, entry, dirty)		\
({																			\
	int __changed = !pte_same(*(ptep), entry);								\
	if (__changed && dirty)													\
	{																		\
		(ptep)->pte_low = (entry).pte_low;									\
		pte_update_defer((vma)->vm_mm, (address), (ptep));					\
		flush_tlb_page(vma, address);										\
	}																		\
	__changed;																\
})

#define __HAVE_ARCH_PTEP_TEST_AND_CLEAR_YOUNG
#define ptep_test_and_clear_young(vma, addr, ptep)										\
({																						\
	int __ret = 0;																		\
	if (pte_young(*(ptep)))																\
		__ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,	&(ptep)->pte_low);	\
	if (__ret)																			\
		pte_update((vma)->vm_mm, addr, ptep);											\
	__ret;																				\
})

#define __HAVE_ARCH_PTEP_CLEAR_YOUNG_FLUSH
#define ptep_clear_flush_young(vma, address, ptep)								\
({																				\
	int __young;																\
	__young = ptep_test_and_clear_young((vma), (address), (ptep));				\
	if (__young)																\
		flush_tlb_page(vma, address);											\
	__young;																	\
})

#define __HAVE_ARCH_PTEP_GET_AND_CLEAR
static inline pte_t ptep_get_and_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
	pte_t pte = native_ptep_get_and_clear(ptep);
	pte_update(mm, addr, ptep);
	return pte;
}

#define __HAVE_ARCH_PTEP_GET_AND_CLEAR_FULL
static inline pte_t ptep_get_and_clear_full(struct mm_struct *mm, unsigned long addr, pte_t *ptep, int full)
{
	pte_t pte;
	if (full) {
		/*
		 * Full address destruction in progress; paravirt does not
		 * care about updates and native needs no locking
		 */
		pte = native_local_ptep_get_and_clear(ptep);
	} else {
		pte = ptep_get_and_clear(mm, addr, ptep);
	}
	return pte;
}

#define __HAVE_ARCH_PTEP_SET_WRPROTECT
static inline void ptep_set_wrprotect(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
	clear_bit(_PAGE_BIT_RW, &ptep->pte_low);
	pte_update(mm, addr, ptep);
}

/*
 * clone_pgd_range(pgd_t *dst, pgd_t *src, int count);
 *
 *  dst - pointer to pgd range anwhere on a pgd page
 *  src - ""
 *  count - the number of pgds to copy.
 *
 * dst and src can be on the same page, but the range must not overlap,
 * and must not cross a page boundary.
 */
static inline void clone_pgd_range(pgd_t *dst, pgd_t *src, int count)
{
       memcpy(dst, src, count * sizeof(pgd_t));
}

/*
 * Macro to mark a page protection value as "uncacheable".  On processors which do not support
 * it, this is a no-op.
 */
#define pgprot_noncached(prot)	((boot_cpu_data.x86 > 3)					  \
				 ? (__pgprot(pgprot_val(prot) | _PAGE_PCD | _PAGE_PWT)) : (prot))

/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */

#define mk_pte(page, pgprot)	pfn_pte(page_to_pfn(page), (pgprot))

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	pte.pte_low &= _PAGE_CHG_MASK;
	pte.pte_low |= pgprot_val(newprot);
#ifdef CONFIG_X86_PAE
	/*
	 * Chop off the NX bit (if present), and add the NX portion of
	 * the newprot (if present):
	 */
	pte.pte_high &= ~(1 << (_PAGE_BIT_NX - 32));
	pte.pte_high |= (pgprot_val(newprot) >> 32) & \
					(__supported_pte_mask >> 32);
#endif
	return pte;
}

#define pmd_large(pmd) \
		((pmd_val(pmd) & (_PAGE_PSE|_PAGE_PRESENT)) == (_PAGE_PSE|_PAGE_PRESENT))

/*页目录页可以认为是一个类似pgd_t[PTRS_PER_PGD]的数组。该宏返回给定虚拟地址对应的
页目录表索引*/
#define pgd_index(address)		(((address) >> PGDIR_SHIFT) & (PTRS_PER_PGD-1))
#define pgd_index_k(addr) 		pgd_index(addr)

/*返回一个pgd_t类型地址。pgd_index()经常用来获取pgd_t类型数组中pgd页的偏移*/
#define pgd_offset(mm, address) ((mm)->pgd+pgd_index(address))

/*简称暗示使用内核pgd而非进程pgd*/
#define pgd_offset_k(address) pgd_offset(&init_mm, address)

/*pmd页也可以被认为是一个类似pmd_t[PTRS_PER_PMD]的数组，该宏返回pmd页的索引*/
#define pmd_index(address) 	(((address) >> PMD_SHIFT) & (PTRS_PER_PMD-1))

/*pte页表项也可以认为是pte_t[PTRS_PER_PTE]数组，该宏返回pte项索引*/
#define pte_index(address)		(((address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
/**/
#define pte_offset_kernel(dir, address) \
		((pte_t *) pmd_page_vaddr(*(dir)) +  pte_index(address))

/*根据页中间目录项获取所属的页实例*/
#define pmd_page(pmd) (pfn_to_page(pmd_val(pmd) >> PAGE_SHIFT))
/*获取指定中间页目录项对应页的虚拟地址*/
#define pmd_page_vaddr(pmd) ((unsigned long) __va(pmd_val(pmd) & PAGE_MASK))

/*
 * Helper function that returns the kernel pagetable entry controlling
 * the virtual address 'address'. NULL means no pagetable entry present.
 * NOTE: the return type is pte_t but if the pmd is PSE then we return it
 * as a pte too.
 */
extern pte_t *lookup_address(unsigned long address);

/*
 * Make a given kernel text page executable/non-executable.
 * Returns the previous executability setting of that page (which
 * is used to restore the previous state). Used by the SMP bootup code.
 * NOTE: this is an __init function for security reasons.
 */
#ifdef CONFIG_X86_PAE
 extern int set_kernel_exec(unsigned long vaddr, int enable);
#else
 static inline int set_kernel_exec(unsigned long vaddr, int enable) { return 0;}
#endif

#if defined(CONFIG_HIGHPTE)
#define pte_offset_map(dir, address) \
	((pte_t *)kmap_atomic_pte(pmd_page(*(dir)),KM_PTE0) + pte_index(address))
#define pte_offset_map_nested(dir, address) \
	((pte_t *)kmap_atomic_pte(pmd_page(*(dir)),KM_PTE1) + pte_index(address))
#define pte_unmap(pte) kunmap_atomic(pte, KM_PTE0)
#define pte_unmap_nested(pte) kunmap_atomic(pte, KM_PTE1)
#else
#define pte_offset_map(dir, address) \
	((pte_t *)page_address(pmd_page(*(dir))) + pte_index(address))
#define pte_offset_map_nested(dir, address) pte_offset_map(dir, address)
#define pte_unmap(pte) do { } while (0)
#define pte_unmap_nested(pte) do { } while (0)
#endif

/* Clear a kernel PTE and flush it from the TLB */
/*清除一个内核页表项并且将它从TLb中刷出*/
#define kpte_clear_flush(ptep, vaddr)	\
do											\
{											\
	/*清除指定页表项内容*/							\
	pte_clear(&init_mm, vaddr, ptep);		\
	__flush_tlb_one(vaddr);					\
} while (0)

/*
 * The i386 doesn't have any external MMU info: the kernel page
 * tables contain all the necessary information.
 */
#define update_mmu_cache(vma,address,pte) do { } while (0)

void native_pagetable_setup_start(pgd_t *base);
void native_pagetable_setup_done(pgd_t *base);

#ifndef CONFIG_PARAVIRT
static inline void paravirt_pagetable_setup_start(pgd_t *base)
{
	native_pagetable_setup_start(base);
}

static inline void paravirt_pagetable_setup_done(pgd_t *base)
{
	native_pagetable_setup_done(base);
}
#endif	/* !CONFIG_PARAVIRT */

#endif /* !__ASSEMBLY__ */

#ifdef CONFIG_FLATMEM
#define kern_addr_valid(addr)	(1)
#endif /* CONFIG_FLATMEM */

#define io_remap_pfn_range(vma, vaddr, pfn, size, prot)		\
		remap_pfn_range(vma, vaddr, pfn, size, prot)

#include <asm-generic/pgtable.h>

#endif /* _I386_PGTABLE_H */
