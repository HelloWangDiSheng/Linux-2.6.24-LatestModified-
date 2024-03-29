#ifndef _ASM_GENERIC_PGTABLE_H
#define _ASM_GENERIC_PGTABLE_H

#ifndef __ASSEMBLY__
#ifdef CONFIG_MMU

#ifndef __HAVE_ARCH_PTEP_SET_ACCESS_FLAGS
/*
 * Largely same as above, but only sets the access flags (dirty,
 * accessed, and writable). Furthermore, we know it always gets set
 * to a "more permissive" setting, which allows most architectures
 * to optimize this. We return whether the PTE actually changed, which
 * in turn instructs the caller to do things like update__mmu_cache.
 * This used to be done in the caller, but sparc needs minor faults to
 * force that call on sun4c so we changed this macro slightly
 */
/**/
#define ptep_set_access_flags(__vma, __address, __ptep, __entry, __dirty) \
({									  													\
	/*测试虚拟地址空间中指定地址所在页表项与指定页表项是否相等*/													\
	int __changed = !pte_same(*(__ptep), __entry);			  							\
	/*如果不相等，则设置指定地址页表项为指定页表项*/															\
	if (__changed)																		\
	{						  															\
		/*设置指定地址所属页表项为指定页表项*/															\
		set_pte_at((__vma)->vm_mm, (__address), __ptep, __entry); 						\
		/**/\
		flush_tlb_page(__vma, __address);			  									\
	}								  													\
	__changed;							  												\
})
#endif

/*如果页表项中_PAGE_ACCESSED被设置，则清除页表项中_PAGE_ACCESSED标识，返回页表项
之前是否具有_PAGE_ACCESSED标识*/
#ifndef __HAVE_ARCH_PTEP_TEST_AND_CLEAR_YOUNG
#define ptep_test_and_clear_young(__vma, __address, __ptep)									\
({																							\
	pte_t __pte = *(__ptep);																\
	int r = 1;																				\
	if (!pte_young(__pte))																	\
		r = 0;																				\
	else																					\
		set_pte_at((__vma)->vm_mm, (__address), (__ptep), pte_mkold(__pte));				\
	r;																						\
})
#endif

/*测试页表项是否设置_PAGE_ACCESSED标识，如果设置则*/
#ifndef __HAVE_ARCH_PTEP_CLEAR_YOUNG_FLUSH
#define ptep_clear_flush_young(__vma, __address, __ptep)						\
({																				\
	int __young;																\
	__young = ptep_test_and_clear_young(__vma, __address, __ptep);		\
	if (__young)																\
		flush_tlb_page(__vma, __address);										\
	__young;																	\
})
#endif

/*获取并清除虚拟地址空间中指定地址所属的页表项，返回之前的页表项*/
#ifndef __HAVE_ARCH_PTEP_GET_AND_CLEAR
#define ptep_get_and_clear(__mm, __address, __ptep)				\
({																\
	pte_t __pte = *(__ptep);									\
	pte_clear((__mm), (__address), (__ptep));					\
	__pte;														\
})
#endif

/*获取并清除虚拟地址空间中指定地址所属的页表项，返回之前的页表项*/
#ifndef __HAVE_ARCH_PTEP_GET_AND_CLEAR_FULL
#define ptep_get_and_clear_full(__mm, __address, __ptep, __full)				\
({																				\
	pte_t __pte;																\
	__pte = ptep_get_and_clear((__mm), (__address), (__ptep));					\
	__pte;																		\
})
#endif

/*
 * Some architectures may be able to avoid expensive synchronization
 * primitives when modifications are made to PTE's which are already
 * not present, or in the process of an address space destruction.
 */
/*清除虚拟地址中指定地址所属的页表项*/
#ifndef __HAVE_ARCH_PTE_CLEAR_NOT_PRESENT_FULL
#define pte_clear_not_present_full(__mm, __address, __ptep, __full)					\
do																					\
{																					\
	pte_clear((__mm), (__address), (__ptep));										\
} while (0)
#endif

#ifndef __HAVE_ARCH_PTEP_CLEAR_FLUSH
/*清除vma所属虚拟地址空间中指定地址所属的页表项*/
#define ptep_clear_flush(__vma, __address, __ptep)								\
({																				\
	pte_t __pte;																\
	__pte = ptep_get_and_clear((__vma)->vm_mm, __address, __ptep);	\
	flush_tlb_page(__vma, __address);											\
	__pte;																		\
})
#endif

#ifndef __HAVE_ARCH_PTEP_SET_WRPROTECT
struct mm_struct;
/*清除页表项的_PAGE_RW标识*/
static inline void ptep_set_wrprotect(struct mm_struct *mm, unsigned long address, pte_t *ptep)
{
	pte_t old_pte = *ptep;
	set_pte_at(mm, address, ptep, pte_wrprotect(old_pte));
}
#endif

#ifndef __HAVE_ARCH_PTE_SAME
/*测试指定的两页表项是否相等*/
#define pte_same(A,B)	(pte_val(A) == pte_val(B))
#endif

#ifndef __HAVE_ARCH_PAGE_TEST_DIRTY
#define page_test_dirty(page)		(0)
#endif

#ifndef __HAVE_ARCH_PAGE_CLEAR_DIRTY
#define page_clear_dirty(page)		do { } while (0)
#endif

/*测试指定页表项是否自上次同步以来已修改过*/
#ifndef __HAVE_ARCH_PAGE_TEST_DIRTY
#define pte_maybe_dirty(pte)		pte_dirty(pte)
#else
#define pte_maybe_dirty(pte)		(1)
#endif

#ifndef __HAVE_ARCH_PAGE_TEST_AND_CLEAR_YOUNG
#define page_test_and_clear_young(page) (0)
#endif

#ifndef __HAVE_ARCH_PGD_OFFSET_GATE
/*获取全局页目录项pgd的偏移地址*/
#define pgd_offset_gate(mm, addr)	pgd_offset(mm, addr)
#endif

#ifndef __HAVE_ARCH_MOVE_PTE
#define move_pte(pte, prot, old_addr, new_addr)	(pte)
#endif

/*
 * When walking page tables, get the address of the next boundary,
 * or the end address of the range if that comes earlier.  Although no
 * vma end wraps to 0, rounded up __boundary may wrap to 0 throughout.
 */
/*获取全局页目录项的下一个边界或者页表的终止地址*/
#define pgd_addr_end(addr, end)													\
({	unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK;				\
	(__boundary - 1 < (end) - 1)? __boundary: (end);							\
})

/*获取上层页目录项的下一个边界或该上层页目录项的终止地址*/
#ifndef pud_addr_end
#define pud_addr_end(addr, end)													\
({	unsigned long __boundary = ((addr) + PUD_SIZE) & PUD_MASK;					\
	(__boundary - 1 < (end) - 1)? __boundary: (end);							\
})
#endif

/*获取中间页目录项的像一个边界或该中间目录项的终止地址*/
#ifndef pmd_addr_end
#define pmd_addr_end(addr, end)													\
({	unsigned long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;					\
	(__boundary - 1 < (end) - 1)? __boundary: (end);							\
})
#endif

/*
 * When walking page tables, we usually want to skip any p?d_none entries;
 * and any p?d_bad entries - reporting the error before resetting to none.
 * Do the tests inline, but report and clear the bad entry in mm/memory.c.
 */
void pgd_clear_bad(pgd_t *);
void pud_clear_bad(pud_t *);
void pmd_clear_bad(pmd_t *);

/*全局页目录项为空或错误时（清除错误信息）返回1，否则返回0*/
static inline int pgd_none_or_clear_bad(pgd_t *pgd)
{
	/*全局页目录项为空时返回1*/
	if (pgd_none(*pgd))
		return 1;
	/*检查全局页表项是否无效。如果函数从外部接收输入参数，则无法假定参数是有效的。
	为保证安全性，可以调用该函数进行检查 */
	if (unlikely(pgd_bad(*pgd)))
	{
		pgd_clear_bad(pgd);
		return 1;
	}
	return 0;
}

/*上层页目录为空或错误时（清除错误信息）返回1，否则返回0*/
static inline int pud_none_or_clear_bad(pud_t *pud)
{
	if (pud_none(*pud))
		return 1;
	if (unlikely(pud_bad(*pud)))
	{
		pud_clear_bad(pud);
		return 1;
	}
	return 0;
}

/*中间页目录项为空或错误时（清除错误信息）返回1，否则返回0*/
static inline int pmd_none_or_clear_bad(pmd_t *pmd)
{
	if (pmd_none(*pmd))
		return 1;
	if (unlikely(pmd_bad(*pmd)))
	{
		pmd_clear_bad(pmd);
		return 1;
	}
	return 0;
}
#endif /* CONFIG_MMU */

/*
 * A facility to provide lazy MMU batching.  This allows PTE updates and
 * page invalidations to be delayed until a call to leave lazy MMU mode
 * is issued.  Some architectures may benefit from doing this, and it is
 * beneficial for both shadow and direct mode hypervisors, which may batch
 * the PTE updates which happen during this window.  Note that using this
 * interface requires that read hazards be removed from the code.  A read
 * hazard could result in the direct mode hypervisor case, since the actual
 * write to the page tables may not yet have taken place, so reads though
 * a raw PTE pointer after it has been modified are not guaranteed to be
 * up to date.  This mode can only be entered and left under the protection of
 * the page table locks for all page tables which may be modified.  In the UP
 * case, this is required so that preemption is disabled, and in the SMP case,
 * it must synchronize the delayed page table writes properly on other CPUs.
 */
#ifndef __HAVE_ARCH_ENTER_LAZY_MMU_MODE
#define arch_enter_lazy_mmu_mode()	do {} while (0)
#define arch_leave_lazy_mmu_mode()	do {} while (0)
#define arch_flush_lazy_mmu_mode()	do {} while (0)
#endif

/*
 * A facility to provide batching of the reload of page tables with the
 * actual context switch code for paravirtualized guests.  By convention,
 * only one of the lazy modes (CPU, MMU) should be active at any given
 * time, entry should never be nested, and entry and exits should always
 * be paired.  This is for sanity of maintaining and reasoning about the
 * kernel code.
 */
#ifndef __HAVE_ARCH_ENTER_LAZY_CPU_MODE
#define arch_enter_lazy_cpu_mode()	do {} while (0)
#define arch_leave_lazy_cpu_mode()	do {} while (0)
#define arch_flush_lazy_cpu_mode()	do {} while (0)
#endif

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_GENERIC_PGTABLE_H */
