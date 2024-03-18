#include <linux/highmem.h>
#include <linux/module.h>

/**/
void *kmap(struct page *page)
{
	might_sleep();
	/*非高端内存域中的页，获取页在直接映射区中的虚拟地址*/
	if (!PageHighMem(page))
		return page_address(page);
	/**/
	return kmap_high(page);
}

/**/
void kunmap(struct page *page)
{
	/*系统不能处于软中断或硬中断中，否则是bug*/
	if (in_interrupt())
		BUG();
	/*非高端内存页则直接返回*/
	if (!PageHighMem(page))
		return;
	/*高端内存页则需要解除已建立的映射*/
	kunmap_high(page);
}

/*
 * kmap_atomic/kunmap_atomic is significantly faster than kmap/kunmap because
 * no global lock is needed and because the kmap code must perform a global TLB
 * invalidation when the kmap pool wraps.
 *
 * However when holding an atomic kmap is is not legal to sleep, so atomic
 * kmaps are appropriate for short, tight code paths only.
 */
/**/
void *kmap_atomic_prot(struct page *page, enum km_type type, pgprot_t prot)
{
	enum fixed_addresses idx;
	unsigned long vaddr;

	/* even !CONFIG_PREEMPT needs this, for in_atomic in do_page_fault */
	pagefault_disable();

	/**/
	if (!PageHighMem(page))
		return page_address(page);

	/**/
	idx = type + KM_TYPE_NR*smp_processor_id();
	/**/
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
	/**/
	BUG_ON(!pte_none(*(kmap_pte-idx)));
	/**/
	set_pte(kmap_pte-idx, mk_pte(page, prot));
	/**/
	arch_flush_lazy_mmu_mode();

	return (void *)vaddr;
}

/*普通的kmap函数不能用于中断处理程序，因为它可能进入睡眠状态。如果pkmap数组中没有空闲
位置，该函数会进入睡眠状态，直至情形有所改善。因此内核提供了一个备选的映射函数，其执
行是原子的，逻辑上称为kmap_atomic。该函数的一个主要优点是它比普通的kmap快速。但它不能
用于可能进入睡眠的代码。它对于很快就需要一个临时页的简短代码，是非常理想的。page是一
个指向高端内存页的管理结构的指针，type定义了所需的映射类型*/
void *kmap_atomic(struct page *page, enum km_type type)
{
	return kmap_atomic_prot(page, type, kmap_prot);
}

/*kunmap_atomic函数从虚拟内存解除一个现存的原子映射，该函数根据映射类型和虚拟地址，
从页表删除对应的项*/
void kunmap_atomic(void *kvaddr, enum km_type type)
{
	unsigned long vaddr = (unsigned long) kvaddr & PAGE_MASK;
	enum fixed_addresses idx = type + KM_TYPE_NR*smp_processor_id();

	/*
	 * Force other mappings to Oops if they'll try to access this pte
	 * without first remap it.  Keeping stale mappings around is a bad idea
	 * also, in case the page changes cacheability attributes or becomes
	 * a protected page in a hypervisor.
	 */
	if (vaddr == __fix_to_virt(FIX_KMAP_BEGIN+idx))
		kpte_clear_flush(kmap_pte-idx, vaddr);
	else
	{
#ifdef CONFIG_DEBUG_HIGHMEM
		/**/
		BUG_ON(vaddr < PAGE_OFFSET);
		/**/
		BUG_ON(vaddr >= (unsigned long)high_memory);
#endif
	}

	/**/
	arch_flush_lazy_mmu_mode();
	/**/
	pagefault_enable();
}

/* This is the same as kmap_atomic() but can map memory that doesn't
 * have a struct page associated with it.
 */
/**/
void *kmap_atomic_pfn(unsigned long pfn, enum km_type type)
{
	enum fixed_addresses idx;
	unsigned long vaddr;
	/**/
	pagefault_disable();
	/**/
	idx = type + KM_TYPE_NR*smp_processor_id();
	/**/
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
	/**/
	set_pte(kmap_pte-idx, pfn_pte(pfn, kmap_prot));
	/**/
	arch_flush_lazy_mmu_mode();

	return (void*) vaddr;
}

struct page *kmap_atomic_to_page(void *ptr)
{
	unsigned long idx, vaddr = (unsigned long)ptr;
	pte_t *pte;

	/**/
	if (vaddr < FIXADDR_START)
		return virt_to_page(ptr);
	/**/
	idx = virt_to_fix(vaddr);
	/**/
	pte = kmap_pte - (idx - FIX_KMAP_BEGIN);
	return pte_page(*pte);
}

EXPORT_SYMBOL(kmap);
EXPORT_SYMBOL(kunmap);
EXPORT_SYMBOL(kmap_atomic);
EXPORT_SYMBOL(kunmap_atomic);
EXPORT_SYMBOL(kmap_atomic_to_page);
