/*头插法将指定页添加到所属内存域中的活动lru链表，并将（全局和per-CPU的）活动页数目加1*/
static inline void add_page_to_active_list(struct zone *zone, struct page *page)
{	list_add(&page->lru, &zone->active_list);
	__inc_zone_state(zone, NR_ACTIVE);
}

/*头插法将指定页添加到所属内存域的惰性lru链表，并将（全局和per-CPU的）不活动页数目加1*/
static inline void add_page_to_inactive_list(struct zone *zone, struct page *page)
{
	list_add(&page->lru, &zone->inactive_list);
	__inc_zone_state(zone, NR_INACTIVE);
}
/*将指定页从所属的内存域活动lru链表中删除，并将（全局和per-CPU的）活动页数目减1*/
static inline void del_page_from_active_list(struct zone *zone, struct page *page)
{
	/*将页从活动lru链表上删除*/
	list_del(&page->lru);
	__dec_zone_state(zone, NR_ACTIVE);
}
/*将指定页从所属内存域的惰性lru链表中删除，并将（全局和per-CPU的）不活动页数目减1*/
static inline void del_page_from_inactive_list(struct zone *zone, struct page *page)
{
	/*将页从惰性lru链表上删除*/
	list_del(&page->lru);
	/*更新不活动页的数目*/
	__dec_zone_state(zone, NR_INACTIVE);
}
/*将指定页从其所属的内存域lru链表中删除，，并将（活动或不活动）页数目减1*/
static inline void del_page_from_lru(struct zone *zone, struct page *page)
{
	/*将页从对应lru链表上删除*/
	list_del(&page->lru);
	/*如果页在活动lru链表上，则清除其PG_active标识，并将活动页的数目减1*/
	if (PageActive(page))
	{
		/*清除页的PG_active标识*/
		__ClearPageActive(page);
		/*将活动页的数目减1*/
		__dec_zone_state(zone, NR_ACTIVE);
	}
	else
	{
		/*将不活动页的数目减1*/
		__dec_zone_state(zone, NR_INACTIVE);
	}
}

