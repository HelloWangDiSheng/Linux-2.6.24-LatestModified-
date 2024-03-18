/* internal.h: mm/�ڲ�����*/
#ifndef __MM_INTERNAL_H
#define __MM_INTERNAL_H

#include <linux/mm.h>
/*����ҳ�����ü���*/
static inline void set_page_count(struct page *page, int v)
{
	atomic_set(&page->_count, v);
}

/*��һ��û�������ҷǸ���ҳβҳ�ĵ�ҳ����Ϊֻ��һ������*/
static inline void set_page_refcounted(struct page *page)
{
	/*ҳ�����Ǹ���ҳ��βҳ*/
	VM_BUG_ON(PageCompound(page) && PageTail(page));
	/*ҳ�����ü���ֻ��Ϊ0*/
	VM_BUG_ON(atomic_read(&page->_count));
	/*����ҳ�����ü���Ϊ1*/
	set_page_count(page, 1);
}

/*������ָ��ҳ������*/
static inline void __put_page(struct page *page)
{
	atomic_dec(&page->_count);
}

extern void fastcall __init __free_pages_bootmem(struct page *page,	unsigned int order);

/*���ػ��ϵͳ�е�ҳ�ķ���ס�zone->lock�Ѿ����룬��˸ú����ڲ���Ҫ��page->flags��
��ԭ�Ӳ���*/
static inline unsigned long page_order(struct page *page)
{
	/*ֻ���ǻ��ϵͳ�е�ҳ*/
	VM_BUG_ON(!PageBuddy(page));
	/*����ҳ�ķ����*/
	return page_private(page);
}
#endif
