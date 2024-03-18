/*pageblock_nr_pages页数目相关的操作和测试标识宏*/
#ifndef PAGEBLOCK_FLAGS_H
#define PAGEBLOCK_FLAGS_H

#include <linux/types.h>

/*宏bit位范围定义*/
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
	/*迁移类型需要3个位。宏扩展后:PB_migrate, PB_migrate_end=2*/
	PB_range(PB_migrate, 3),
	NR_PAGEBLOCK_BITS
};


#ifdef CONFIG_HUGETLB_PAGE

#ifdef CONFIG_HUGETLB_PAGE_SIZE_VARIABLE

/*可用的巨型页分配阶*/
extern int pageblock_order;

#else /* CONFIG_HUGETLB_PAGE_SIZE_VARIABLE */

/*巨型页分配阶一个常量值*/
#define pageblock_order		HUGETLB_PAGE_ORDER

#endif /* CONFIG_HUGETLB_PAGE_SIZE_VARIABLE */

#else /* CONFIG_HUGETLB_PAGE */

/*巨型页分配阶如果巨型页没有被使用根据MAX_ORDER_NR_PAGES分组*/
#define pageblock_order		(MAX_ORDER-1)

#endif /* CONFIG_HUGETLB_PAGE */
/*巨型页包含的连续的页数目*/
#define pageblock_nr_pages	(1UL << pageblock_order)

struct page;

unsigned long get_pageblock_flags_group(struct page *page, int start_bitidx, int end_bitidx);
void set_pageblock_flags_group(struct page *page, unsigned long flags, int start_bitidx,
									int end_bitidx);
#define get_pageblock_flags(page)	get_pageblock_flags_group(page, 0, NR_PAGEBLOCK_BITS-1)
#define set_pageblock_flags(page)	set_pageblock_flags_group(page, 0, NR_PAGEBLOCK_BITS-1)

#endif	/* PAGEBLOCK_FLAGS_H */
