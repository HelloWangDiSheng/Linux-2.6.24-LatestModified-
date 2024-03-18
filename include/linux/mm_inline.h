/*ͷ�巨��ָ��ҳ��ӵ������ڴ����еĻlru����������ȫ�ֺ�per-CPU�ģ��ҳ��Ŀ��1*/
static inline void add_page_to_active_list(struct zone *zone, struct page *page)
{	list_add(&page->lru, &zone->active_list);
	__inc_zone_state(zone, NR_ACTIVE);
}

/*ͷ�巨��ָ��ҳ��ӵ������ڴ���Ķ���lru����������ȫ�ֺ�per-CPU�ģ����ҳ��Ŀ��1*/
static inline void add_page_to_inactive_list(struct zone *zone, struct page *page)
{
	list_add(&page->lru, &zone->inactive_list);
	__inc_zone_state(zone, NR_INACTIVE);
}
/*��ָ��ҳ���������ڴ���lru������ɾ����������ȫ�ֺ�per-CPU�ģ��ҳ��Ŀ��1*/
static inline void del_page_from_active_list(struct zone *zone, struct page *page)
{
	/*��ҳ�ӻlru������ɾ��*/
	list_del(&page->lru);
	__dec_zone_state(zone, NR_ACTIVE);
}
/*��ָ��ҳ�������ڴ���Ķ���lru������ɾ����������ȫ�ֺ�per-CPU�ģ����ҳ��Ŀ��1*/
static inline void del_page_from_inactive_list(struct zone *zone, struct page *page)
{
	/*��ҳ�Ӷ���lru������ɾ��*/
	list_del(&page->lru);
	/*���²��ҳ����Ŀ*/
	__dec_zone_state(zone, NR_INACTIVE);
}
/*��ָ��ҳ�����������ڴ���lru������ɾ��������������򲻻��ҳ��Ŀ��1*/
static inline void del_page_from_lru(struct zone *zone, struct page *page)
{
	/*��ҳ�Ӷ�Ӧlru������ɾ��*/
	list_del(&page->lru);
	/*���ҳ�ڻlru�����ϣ��������PG_active��ʶ�������ҳ����Ŀ��1*/
	if (PageActive(page))
	{
		/*���ҳ��PG_active��ʶ*/
		__ClearPageActive(page);
		/*���ҳ����Ŀ��1*/
		__dec_zone_state(zone, NR_ACTIVE);
	}
	else
	{
		/*�����ҳ����Ŀ��1*/
		__dec_zone_state(zone, NR_INACTIVE);
	}
}

