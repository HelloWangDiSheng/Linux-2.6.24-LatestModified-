#ifndef _I386_PAGE_H
#define _I386_PAGE_H

/*PAGE_SHIFT决定PAGE_SIZE大小*/
#define PAGE_SHIFT	12
/*页长度*/
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
/*低PAGE_SHIFT位全部清零，高位全部置位*/
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define LARGE_PAGE_SIZE (1UL << PMD_SHIFT)
#define LARGE_PAGE_MASK (~(LARGE_PAGE_SIZE-1))

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#ifdef CONFIG_X86_USE_3DNOW

#include <asm/mmx.h>
/*将页面内容清零*/
#define clear_page(page)	mmx_clear_page((void *)(page))
/*将一个页的内容赋值为一个页*/
#define copy_page(to,from)	mmx_copy_page(to,from)

#else

/*在较旧的x86处理器上不能使用MMX*/
/*将页面内容清零*/
#define clear_page(page)		memset((void *)(page), 0, PAGE_SIZE)
/*复制页内容*/
#define copy_page(to,from)		memcpy((void *)(to), (void *)(from), PAGE_SIZE)
#endif

/*清零用户页内容*/
#define clear_user_page(page, vaddr, pg)			clear_page(page)
/*赋值用户页内容*/
#define copy_user_page(to, from, vaddr, pg)			copy_page(to, from)
/**/
#define __alloc_zeroed_user_highpage(movableflags, vma, vaddr) \
	alloc_page_vma(GFP_HIGHUSER | __GFP_ZERO | movableflags, vma, vaddr)
#define __HAVE_ARCH_ALLOC_ZEROED_USER_HIGHPAGE

/*经常用于C类型检查*/
extern int nx_enabled;
/*启用PAE特性*/
#ifdef CONFIG_X86_PAE
/*可用内存大于处理器的地址空间时，由于指针仍然只有32个比特位宽，必须为用户空间应用程序选择扩大的内存空间
的一个适当子集，使每个进程仍然只能看到4GB地址空间*/
typedef struct { unsigned long pte_low, pte_high; } pte_t;
typedef struct { unsigned long long pmd; } pmd_t;
typedef struct { unsigned long long pgd; } pgd_t;
typedef struct { unsigned long long pgprot; } pgprot_t;

static inline unsigned long long native_pgd_val(pgd_t pgd)
{
	return pgd.pgd;
}

static inline unsigned long long native_pmd_val(pmd_t pmd)
{
	return pmd.pmd;
}

static inline unsigned long long native_pte_val(pte_t pte)
{
	return pte.pte_low | ((unsigned long long)pte.pte_high << 32);
}

static inline pgd_t native_make_pgd(unsigned long long val)
{
	return (pgd_t) { val };
}

static inline pmd_t native_make_pmd(unsigned long long val)
{
	return (pmd_t) { val };
}

static inline pte_t native_make_pte(unsigned long long val)
{
	return (pte_t) { .pte_low = val, .pte_high = (val >> 32) } ;
}

#ifndef CONFIG_PARAVIRT
#define pmd_val(x)	native_pmd_val(x)
#define __pmd(x)	native_make_pmd(x)
#endif

#define HPAGE_SHIFT	21
#include <asm-generic/pgtable-nopud.h>
#else  /* !CONFIG_X86_PAE */
/*页表项（Page Table Entry）*/
typedef struct { unsigned long pte_low; } pte_t;
/*全局页目录项（Page Global Directory）*/
typedef struct { unsigned long pgd; } pgd_t;
/*页保护标识（Page Protect Flags）*/
typedef struct { unsigned long pgprot; } pgprot_t;
/*最好用typedef定义类型（有类型检查，宏只替换）*/
#define boot_pte_t pte_t

/*获取全局页目录项*/
static inline unsigned long native_pgd_val(pgd_t pgd)
{
	return pgd.pgd;
}
/*获取页表项*/
static inline unsigned long native_pte_val(pte_t pte)
{
	return pte.pte_low;
}

/*将pgd值强制转换成pgd_t类型，简单封装保护*/
static inline pgd_t native_make_pgd(unsigned long val)
{
	return (pgd_t) { val };
}
/*将unsigned long类型值转换成pte_t类型*/
static inline pte_t native_make_pte(unsigned long val)
{
	return (pte_t) { .pte_low = val };
}
/*Huge Page Shift*/
#define HPAGE_SHIFT	22
#include <asm-generic/pgtable-nopmd.h>
#endif	/* CONFIG_X86_PAE */

#define PTE_MASK	PAGE_MASK

#ifdef CONFIG_HUGETLB_PAGE
/*巨型页大小，每次可以分配4M的连续物理内存页*/
#define HPAGE_SIZE	((1UL) << HPAGE_SHIFT)
/*将Huge Page的低PAGE_SHIFT位全部清零，高位全部置位*/
#define HPAGE_MASK	(~(HPAGE_SIZE - 1))
/*定义巨型页分配阶10*/
#define HUGETLB_PAGE_ORDER	(HPAGE_SHIFT - PAGE_SHIFT)
#define HAVE_ARCH_HUGETLB_UNMAPPED_AREA
#endif
/*获取页面保护标识*/
#define pgprot_val(x)		((x).pgprot)
/*将unsigned long类型值转换为pgprot_t类型*/
#define __pgprot(x)			((pgprot_t) { (x) } )

/*x86上运行的许多指令都需要被替换为虚拟参数*/
#ifndef CONFIG_PARAVIRT
#define pgd_val(x)		native_pgd_val(x)
#define __pgd(x)		native_make_pgd(x)
#define pte_val(x)		native_pte_val(x)
#define __pte(x)		native_make_pte(x)
#endif

#endif /* !__ASSEMBLY__ */

/*页指针对齐到下页边界*/
#define PAGE_ALIGN(addr)		(((addr)+PAGE_SIZE-1)&PAGE_MASK)

/*
 * This handles the memory map.. We could make this a config
 * option, but too many people screw it up, and too few need
 * it.
 *
 * A __PAGE_OFFSET of 0xC0000000 means that the kernel has
 * a virtual address space of one gigabyte, which limits the
 * amount of physical memory you can use to about 950MB.
 *
 * If you want more physical memory than this then see the CONFIG_HIGHMEM4G
 * and CONFIG_HIGHMEM64G options in the kernel configuration.
 */

#ifndef __ASSEMBLY__

struct vm_area_struct;

/*
 * This much address space is reserved for vmalloc() and iomap()
 * as well as fixmap mappings.
 */
extern unsigned int __VMALLOC_RESERVE;
extern int sysctl_legacy_va_layout;
extern int page_is_ram(unsigned long pagenr);

#endif /* __ASSEMBLY__ */

#ifdef __ASSEMBLY__
#define __PAGE_OFFSET		CONFIG_PAGE_OFFSET
#else
#define __PAGE_OFFSET		((unsigned long)CONFIG_PAGE_OFFSET)
#endif

/*指定了物理页帧在虚拟地址空间中映射的位置。在大多数体系结构上，这隐含地定义了用户地址
空间的长度，或将整个地址空间划分为内核地址空间和用户地址空间。但这并不适用于所有体系结
构。如AMD64是另一个例外，因为其虚拟地址空间在中部有一个不可寻址的空间。因而，必须使用
asm-arch/processor.h中定义的TASK_SIZE常数来确定用户空间的长度，而不能使用PAGE_OFFSET*/
#define PAGE_OFFSET					((unsigned long)__PAGE_OFFSET)
/*设置了vmalloc区域的长度*/
#define VMALLOC_RESERVE				((unsigned long)__VMALLOC_RESERVE)
/*内核可以直接寻址的物理内存的最大可能数量*/
#define MAXMEM						(-__PAGE_OFFSET - __VMALLOC_RESERVE)
/*直接映射区中虚拟地址转换为物理地址*/
#define __pa(x)						((unsigned long)(x) - PAGE_OFFSET)
/* __pa_symbol should be used for C visible symbols.
   This seems to be the official gcc blessed way to do such arithmetic. */
#define __pa_symbol(x)          	__pa(RELOC_HIDE((unsigned long)(x), 0))
/*直接映射区中物理地址转换为虚拟地址*/
#define __va(x)						((void *)((unsigned long)(x)  + PAGE_OFFSET))
/*直接映射区中的页可根据页号获取对应的虚拟地址*/
#define pfn_to_kaddr(pfn)			__va((pfn) << PAGE_SHIFT)
#ifdef CONFIG_FLATMEM
/*测试页号的有效性*/
#define pfn_valid(pfn)				((pfn) < max_mapnr)
#endif /* CONFIG_FLATMEM */
/*直接映射区中的虚拟地址转换为页实例。先将直接映射区的虚拟地址转换为物理地址，然后根
据该物理地址计算页帧号，再根据页帧号查询其对应的全局的mem_map中的页实例*/
#define virt_to_page(kaddr)			pfn_to_page(__pa(kaddr) >> PAGE_SHIFT)
/*测试虚拟地址是否在直接映射区内*/
#define virt_addr_valid(kaddr)	pfn_valid(__pa(kaddr) >> PAGE_SHIFT)

/**/
#define VM_DATA_DEFAULT_FLAGS (VM_READ | VM_WRITE | 								\
		((current->personality & READ_IMPLIES_EXEC) ? VM_EXEC : 0 ) | 	\
		 VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)

#include <asm-generic/memory_model.h>
#include <asm-generic/page.h>

#define __HAVE_ARCH_GATE_AREA 1
#endif /* __KERNEL__ */

#endif /* _I386_PAGE_H */
