#ifndef _I386_PGTABLE_2LEVEL_H
#define _I386_PGTABLE_2LEVEL_H

/*��ӡҳ���������ʾ��Ϣ���ļ������кż�ҳ���*/
#define pte_ERROR(e) printk("%s:%d: bad pte %08lx.\n", __FILE__, __LINE__, (e).pte_low)
/*��ӡȫ��ҳĿ¼��Ĵ�����ʾ��Ϣ���ļ������кż�ȫ��ҳĿ¼���ֵ��*/
#define pgd_ERROR(e) printk("%s:%d: bad pgd %08lx.\n", __FILE__, __LINE__, pgd_val(e))

/*
 * Certain architectures need to do special things when PTEs
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */
/*����ҳ�����ֵ*/
static inline void native_set_pte(pte_t *ptep , pte_t pte)
{
	*ptep = pte;
}
/*���������ַ�ռ���ָ����ַ��ҳ�����ֵ*/
static inline void native_set_pte_at(struct mm_struct *mm, unsigned long addr, pte_t *ptep,
			pte_t pte)
{
	native_set_pte(ptep, pte);
}
/*�����м�ҳĿ¼���ֵ*/
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
/*��������ַ�ռ���ָ����ַ��Ӧ��ҳ����*/
#define pte_clear(mm,addr,xp)	do { set_pte_at(mm, addr, xp, __pte(0)); } while (0)
/*���ָ���м�ҳĿ¼���ֵ*/
#define pmd_clear(xp)	do { set_pmd(xp, __pmd(0)); } while (0)
/*��������ַ�ռ���ָ����ַ��Ӧ��ҳ����*/
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
/*��ȡҳ�����Ӧ��ҳʵ��*/
#define pte_page(x)		pfn_to_page(pte_pfn(x))
/*��ҳ�����ֵȡ�������ΪNULL����˵��ҳ������ڴ���*/
#define pte_none(x)		(!(x).pte_low)
/*��ȡҳ�����Ӧ��ҳ֡��*/
#define pte_pfn(x)		(pte_val(x) >> PAGE_SHIFT)
/*����ҳ֡�Ŷ�Ӧ��ҳ�����ҳ������ʶ*/
#define pfn_pte(pfn, prot)		__pte(((pfn) << PAGE_SHIFT) | pgprot_val(prot))
/*����ҳ֡�Ŷ�Ӧ���м�ҳĿ¼������ҳ��ҳ������ʶ*/
#define pfn_pmd(pfn, prot)		__pmd(((pfn) << PAGE_SHIFT) | pgprot_val(prot))

/*All present pages are kernel-executable*/
/*�����ڴ��е�ҳ���ǿ�ִ���ں�ҳ*/
static inline int pte_exec_kernel(pte_t pte)
{
	return 1;
}

/*Bits 0, 6 and 7 are taken, split up the 29 bits of offset into this range*/
#define PTE_FILE_MAX_BITS	29
/*��ҳ�����0��_PAGE_PRESENT����6��_PAGE_FILE����7��_PAGE_PROTNONE������λֵֵȥ����
ƴ��Ϊһ��29λ��ƫ��ֵ��Ҳ����ҳ�����1��5λ��Ϊ��5λ����6λ��ʼ��29λ����ԭ�ӵĵ�
8λ��31λ����ҳ����ת��Ϊ����������������Ϣ*/
#define pte_to_pgoff(pte)	((((pte).pte_low >> 1) & 0x1f ) + (((pte).pte_low >> 8) << 5 ))
/*����������Ϣת��Ϊ������ӳ���ҳ����*/
#define pgoff_to_pte(off) \
	((pte_t) { (((off) & 0x1f) << 1) + (((off) >> 5) << 8) + _PAGE_FILE })

/* Encode and de-code a swap entry */
/*����ͽ��뽻����*/
/*����ҳ�����ȡҳ���ڽ������ı�ţ�ҳ����ı���λ1��5λ��*/
#define __swp_type(x)			(((x).val >> 1) & 0x1f)
/*����ҳ�����ȡҳ���ڽ�������ƫ�ƣ�ҳ�����ĸ�24λ��*/
#define __swp_offset(x)			((x).val >> 8)
/*���ݻ���ҳ����Ϣ�����ڽ�������ż���ƫ�ƣ����Ϊswap_entry_t���ͱ���*/
#define __swp_entry(type, offset)	((swp_entry_t) { ((type) << 1) | ((offset) << 8) })
/*��ҳ����ǿ��ת��Ϊswap_entry_t����*/
#define __pte_to_swp_entry(pte)		((swp_entry_t) { (pte).pte_low })
/*��swap_entry_t����ת��Ϊҳ��������*/
#define __swp_entry_to_pte(x)		((pte_t) { (x).val })

#endif /* _I386_PGTABLE_2LEVEL_H */
