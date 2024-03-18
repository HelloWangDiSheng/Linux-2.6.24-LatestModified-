#ifndef _I386_PAGE_H
#define _I386_PAGE_H

/*PAGE_SHIFT����PAGE_SIZE��С*/
#define PAGE_SHIFT	12
/*ҳ����*/
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
/*��PAGE_SHIFTλȫ�����㣬��λȫ����λ*/
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define LARGE_PAGE_SIZE (1UL << PMD_SHIFT)
#define LARGE_PAGE_MASK (~(LARGE_PAGE_SIZE-1))

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#ifdef CONFIG_X86_USE_3DNOW

#include <asm/mmx.h>
/*��ҳ����������*/
#define clear_page(page)	mmx_clear_page((void *)(page))
/*��һ��ҳ�����ݸ�ֵΪһ��ҳ*/
#define copy_page(to,from)	mmx_copy_page(to,from)

#else

/*�ڽϾɵ�x86�������ϲ���ʹ��MMX*/
/*��ҳ����������*/
#define clear_page(page)		memset((void *)(page), 0, PAGE_SIZE)
/*����ҳ����*/
#define copy_page(to,from)		memcpy((void *)(to), (void *)(from), PAGE_SIZE)
#endif

/*�����û�ҳ����*/
#define clear_user_page(page, vaddr, pg)			clear_page(page)
/*��ֵ�û�ҳ����*/
#define copy_user_page(to, from, vaddr, pg)			copy_page(to, from)
/**/
#define __alloc_zeroed_user_highpage(movableflags, vma, vaddr) \
	alloc_page_vma(GFP_HIGHUSER | __GFP_ZERO | movableflags, vma, vaddr)
#define __HAVE_ARCH_ALLOC_ZEROED_USER_HIGHPAGE

/*��������C���ͼ��*/
extern int nx_enabled;
/*����PAE����*/
#ifdef CONFIG_X86_PAE
/*�����ڴ���ڴ������ĵ�ַ�ռ�ʱ������ָ����Ȼֻ��32������λ������Ϊ�û��ռ�Ӧ�ó���ѡ��������ڴ�ռ�
��һ���ʵ��Ӽ���ʹÿ��������Ȼֻ�ܿ���4GB��ַ�ռ�*/
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
/*ҳ���Page Table Entry��*/
typedef struct { unsigned long pte_low; } pte_t;
/*ȫ��ҳĿ¼�Page Global Directory��*/
typedef struct { unsigned long pgd; } pgd_t;
/*ҳ������ʶ��Page Protect Flags��*/
typedef struct { unsigned long pgprot; } pgprot_t;
/*�����typedef�������ͣ������ͼ�飬��ֻ�滻��*/
#define boot_pte_t pte_t

/*��ȡȫ��ҳĿ¼��*/
static inline unsigned long native_pgd_val(pgd_t pgd)
{
	return pgd.pgd;
}
/*��ȡҳ����*/
static inline unsigned long native_pte_val(pte_t pte)
{
	return pte.pte_low;
}

/*��pgdֵǿ��ת����pgd_t���ͣ��򵥷�װ����*/
static inline pgd_t native_make_pgd(unsigned long val)
{
	return (pgd_t) { val };
}
/*��unsigned long����ֵת����pte_t����*/
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
/*����ҳ��С��ÿ�ο��Է���4M�����������ڴ�ҳ*/
#define HPAGE_SIZE	((1UL) << HPAGE_SHIFT)
/*��Huge Page�ĵ�PAGE_SHIFTλȫ�����㣬��λȫ����λ*/
#define HPAGE_MASK	(~(HPAGE_SIZE - 1))
/*�������ҳ�����10*/
#define HUGETLB_PAGE_ORDER	(HPAGE_SHIFT - PAGE_SHIFT)
#define HAVE_ARCH_HUGETLB_UNMAPPED_AREA
#endif
/*��ȡҳ�汣����ʶ*/
#define pgprot_val(x)		((x).pgprot)
/*��unsigned long����ֵת��Ϊpgprot_t����*/
#define __pgprot(x)			((pgprot_t) { (x) } )

/*x86�����е����ָ���Ҫ���滻Ϊ�������*/
#ifndef CONFIG_PARAVIRT
#define pgd_val(x)		native_pgd_val(x)
#define __pgd(x)		native_make_pgd(x)
#define pte_val(x)		native_pte_val(x)
#define __pte(x)		native_make_pte(x)
#endif

#endif /* !__ASSEMBLY__ */

/*ҳָ����뵽��ҳ�߽�*/
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

/*ָ��������ҳ֡�������ַ�ռ���ӳ���λ�á��ڴ������ϵ�ṹ�ϣ��������ض������û���ַ
�ռ�ĳ��ȣ���������ַ�ռ仮��Ϊ�ں˵�ַ�ռ���û���ַ�ռ䡣���Ⲣ��������������ϵ��
������AMD64����һ�����⣬��Ϊ�������ַ�ռ����в���һ������Ѱַ�Ŀռ䡣���������ʹ��
asm-arch/processor.h�ж����TASK_SIZE������ȷ���û��ռ�ĳ��ȣ�������ʹ��PAGE_OFFSET*/
#define PAGE_OFFSET					((unsigned long)__PAGE_OFFSET)
/*������vmalloc����ĳ���*/
#define VMALLOC_RESERVE				((unsigned long)__VMALLOC_RESERVE)
/*�ں˿���ֱ��Ѱַ�������ڴ������������*/
#define MAXMEM						(-__PAGE_OFFSET - __VMALLOC_RESERVE)
/*ֱ��ӳ�����������ַת��Ϊ�����ַ*/
#define __pa(x)						((unsigned long)(x) - PAGE_OFFSET)
/* __pa_symbol should be used for C visible symbols.
   This seems to be the official gcc blessed way to do such arithmetic. */
#define __pa_symbol(x)          	__pa(RELOC_HIDE((unsigned long)(x), 0))
/*ֱ��ӳ�����������ַת��Ϊ�����ַ*/
#define __va(x)						((void *)((unsigned long)(x)  + PAGE_OFFSET))
/*ֱ��ӳ�����е�ҳ�ɸ���ҳ�Ż�ȡ��Ӧ�������ַ*/
#define pfn_to_kaddr(pfn)			__va((pfn) << PAGE_SHIFT)
#ifdef CONFIG_FLATMEM
/*����ҳ�ŵ���Ч��*/
#define pfn_valid(pfn)				((pfn) < max_mapnr)
#endif /* CONFIG_FLATMEM */
/*ֱ��ӳ�����е������ַת��Ϊҳʵ�����Ƚ�ֱ��ӳ�����������ַת��Ϊ�����ַ��Ȼ���
�ݸ������ַ����ҳ֡�ţ��ٸ���ҳ֡�Ų�ѯ���Ӧ��ȫ�ֵ�mem_map�е�ҳʵ��*/
#define virt_to_page(kaddr)			pfn_to_page(__pa(kaddr) >> PAGE_SHIFT)
/*���������ַ�Ƿ���ֱ��ӳ������*/
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
