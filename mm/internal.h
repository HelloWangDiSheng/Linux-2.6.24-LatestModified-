/* internal.h: mm/内部定义*/
#ifndef __MM_INTERNAL_H
#define __MM_INTERNAL_H

#include <linux/mm.h>
/*设置页的引用计数*/
static inline void set_page_count(struct page *page, int v)
{
	atomic_set(&page->_count, v);
}

/*将一个没有引用且非复合页尾页的的页设置为只有一个引用*/
static inline void set_page_refcounted(struct page *page)
{
	/*页不能是复合页的尾页*/
	VM_BUG_ON(PageCompound(page) && PageTail(page));
	/*页的引用计数只能为0*/
	VM_BUG_ON(atomic_read(&page->_count));
	/*设置页的引用计数为1*/
	set_page_count(page, 1);
}

/*撤销对指定页的引用*/
static inline void __put_page(struct page *page)
{
	atomic_dec(&page->_count);
}

extern void fastcall __init __free_pages_bootmem(struct page *page,	unsigned int order);

/*返回伙伴系统中的页的分配阶。zone->lock已经申请，因此该函数内不需要对page->flags进
行原子操作*/
static inline unsigned long page_order(struct page *page)
{
	/*只能是伙伴系统中的页*/
	VM_BUG_ON(!PageBuddy(page));
	/*返回页的分配阶*/
	return page_private(page);
}
#endif
