#ifndef _I386_PGTABLE_2LEVEL_H
#define _I386_PGTABLE_2LEVEL_H

/*打印页表项错误提示信息（文件名：行号及页表项）*/
#define pte_ERROR(e) printk("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, (e).pte_low)
/*打印全局页目录项的错误提示信息（文件名：行号及全局页目录项的值）*/
#define pgd_ERROR(e) printk("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

/*
 * Certain architectures need to do special things when PTEs
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */
/*设置页表项的值*/
static inline void native_set_pte(pte_t *ptep , pte_t pte)
{
	*ptep = pte;
}
/*设置虚拟地址空间中指定地址的页表项的值*/
static inline void native_set_pte_at(struct mm_struct *mm, unsigned long addr, pte_t *ptep,
			pte_t pte)
{
	native_set_pte(ptep, pte);
}
/*设置中间页目录项的值*/
static inline void native_set_pmd(pmd_t *pmdp, pmd_t pmd)
{
	*pmdp = pmd;
}
#ifndef CONFIG_PARAVIRT
#define set_pte(pteptr, pteval)					native_set_pte(pteptr, pteval)
#define set_pte_at(mm,addr,ptep,pteval)			native_set_pte_at(mm, addr, ptep, pteval)
#define set_pmd(pmdptr, pmdval)					native_set_pmd(pmdptr, pmdval)
#endif

#define set_pte_atomic(pteptr, pteval) 				set_pte(pteptr,pteval)
#define set_pte_present(mm,addr,ptep,pteval)		set_pte_at(mm,addr,ptep,pteval)
/*清空虚拟地址空间中指定地址对应的页表项*/
#define pte_clear(mm,addr,xp)	do { set_pte_at(mm, addr, xp, __pte(0)); } while (0)
/*清空指定中间页目录项的值*/
#define pmd_clear(xp)	do { set_pmd(xp, __pmd(0)); } while (0)
/*清空虚拟地址空间中指定地址对应的页表项*/
static inline void native_pte_clear(struct mm_struct *mm, unsigned long addr, pte_t *xp)
{
	*xp = __pte(0);
}

#ifdef CONFIG_SMP
static inline pte_t native_ptep_get_and_clear(pte_t *xp)
{
	return __pte(xchg(&xp->pte_low, 0));
}
#else
#define native_ptep_get_and_clear(xp) native_local_ptep_get_and_clear(xp)
#endif
/*获取页表项对应的页实例*/
#define pte_page(x)		pfn_to_page(pte_pfn(x))
/*将页表项的值取反，如果为NULL，则说明页表项不在内存中*/
#define pte_none(x)		(!(x).pte_low)
/*获取页表项对应的页帧号*/
#define pte_pfn(x)		(pte_val(x) >> PAGE_SHIFT)
/*设置页帧号对应的页表项的页保护标识*/
#define pfn_pte(pfn, prot)		__pte(((pfn) << PAGE_SHIFT) | pgprot_val(prot))
/*设置页帧号对应的中间页目录项所在页的页保护标识*/
#define pfn_pmd(pfn, prot)		__pmd(((pfn) << PAGE_SHIFT) | pgprot_val(prot))

/*All present pages are kernel-executable*/
/*所有内存中的页都是可执行内核页*/
static inline int pte_exec_kernel(pte_t pte)
{
	return 1;
}

/*Bits 0, 6 and 7 are taken, split up the 29 bits of offset into this range*/
#define PTE_FILE_MAX_BITS	29
/*将页表项的0（_PAGE_PRESENT）、6（_PAGE_FILE）、7（_PAGE_PROTNONE）比特位值值去掉，
拼接为一个29位的偏移值。也就是页表项的1至5位作为低5位，第6位开始至29位是其原子的第
8位至31位。将页表项转换为换出到交换区的信息*/
#define pte_to_pgoff(pte)	((((pte).pte_low >> 1) & 0x1f ) + (((pte).pte_low >> 8) << 5 ))
/*将交换区信息转换为非线性映射的页表项*/
#define pgoff_to_pte(off) \
	((pte_t) { (((off) & 0x1f) << 1) + (((off) >> 5) << 8) + _PAGE_FILE })

/* Encode and de-code a swap entry */
/*编码和解码交换区*/
/*根据页表项获取页所在交换区的编号（页表项的比特位1至5位）*/
#define __swp_type(x)			(((x).val >> 1) & 0x1f)
/*根据页表项获取页所在交换区中偏移（页表项额的高24位）*/
#define __swp_offset(x)			((x).val >> 8)
/*根据换出页的信息（所在交换区编号及其偏移）组合为swap_entry_t类型变量*/
#define __swp_entry(type, offset)	((swp_entry_t) { ((type) << 1) | ((offset) << 8) })
/*将页表项强制转换为swap_entry_t类型*/
#define __pte_to_swp_entry(pte)		((swp_entry_t) { (pte).pte_low })
/*将swap_entry_t类型转换为页表项类型*/
#define __swp_entry_to_pte(x)		((pte_t) { (x).val })

#endif /* _I386_PGTABLE_2LEVEL_H */
