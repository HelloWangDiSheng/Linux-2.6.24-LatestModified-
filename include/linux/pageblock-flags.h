/*pageblock_nr_pagesҳ��Ŀ��صĲ����Ͳ��Ա�ʶ��*/
#ifndef PAGEBLOCK_FLAGS_H
#define PAGEBLOCK_FLAGS_H

#include <linux/types.h>

/*��bitλ��Χ����*/
#define PB_range(name, required_bits) name, name##_end = (name + required_bits) - 1

/*
enum pageblock_bits
{
	PB_migrate,
	PB_migrate_end = 2,
	NR_PAGEBLOCK_BITS
};
*/
enum pageblock_bits
{
	/*Ǩ��������Ҫ3��λ������չ��:PB_migrate, PB_migrate_end=2*/
	PB_range(PB_migrate, 3),
	NR_PAGEBLOCK_BITS
};


#ifdef CONFIG_HUGETLB_PAGE

#ifdef CONFIG_HUGETLB_PAGE_SIZE_VARIABLE

/*���õľ���ҳ�����*/
extern int pageblock_order;

#else /* CONFIG_HUGETLB_PAGE_SIZE_VARIABLE */

/*����ҳ�����һ������ֵ*/
#define pageblock_order		HUGETLB_PAGE_ORDER

#endif /* CONFIG_HUGETLB_PAGE_SIZE_VARIABLE */

#else /* CONFIG_HUGETLB_PAGE */

/*����ҳ������������ҳû�б�ʹ�ø���MAX_ORDER_NR_PAGES����*/
#define pageblock_order		(MAX_ORDER-1)

#endif /* CONFIG_HUGETLB_PAGE */
/*����ҳ������������ҳ��Ŀ*/
#define pageblock_nr_pages	(1UL << pageblock_order)

struct page;

unsigned long get_pageblock_flags_group(struct page *page, int start_bitidx, int end_bitidx);
void set_pageblock_flags_group(struct page *page, unsigned long flags, int start_bitidx,
									int end_bitidx);
#define get_pageblock_flags(page)	get_pageblock_flags_group(page, 0, NR_PAGEBLOCK_BITS-1)
#define set_pageblock_flags(page)	set_pageblock_flags_group(page, 0, NR_PAGEBLOCK_BITS-1)

#endif	/* PAGEBLOCK_FLAGS_H */
