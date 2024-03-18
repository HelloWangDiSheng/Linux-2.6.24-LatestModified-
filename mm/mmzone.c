/*��������ڴ���Ĵ���*/

#include <linux/stddef.h>
#include <linux/mmzone.h>
#include <linux/module.h>

/*��ȡ��һ�����ߣ��ɱ��ں˹����Ľ��*/
struct pglist_data *first_online_pgdat(void)
{
	return NODE_DATA(first_online_node);
}

/*��ȡָ��������һ�����߽��*/
struct pglist_data *next_online_pgdat(struct pglist_data *pgdat)
{
	/*��ȡָ��������һ�����ı��*/
	int nid = next_online_node(pgdat->node_id);
	/*ָ������Ѿ������һ����㣬��һ���������Ч*/
	if (nid == MAX_NUMNODES)
		return NULL;
	/*����ָ����ŵĽ��ʵ��*/
	return NODE_DATA(nid);
}

/*��ȡ��ǰ�ڴ������һ���ڴ��������ǰ�ڴ����ǵ�ǰ�������һ���ڴ������ȡ��ǰ
������һ����㣬��һ������ʱ�����ȡ��һ���ĵ�һ���ڴ��򣬷�������Ϊ��*/
struct zone *next_zone(struct zone *zone)
{
	/*��ȡ��ǰ�ڴ��������Ľ��*/
	pg_data_t *pgdat = zone->zone_pgdat;
	/*�����ǰ�ڴ����Ǹý�������ڴ�����λ���¸��ڴ���*/
	if (zone < pgdat->node_zones + MAX_NR_ZONES - 1)
	{
		zone++;
	}
	else
	{
		/*��ȡ�ý�����һ�����߽��*/
		pgdat = next_online_pgdat(pgdat);
		/*������ʱ�ͻ�ȡ�ý��ĵ�һ���ڴ��򣬷�������Ϊ��*/
		if (pgdat)
			zone = pgdat->node_zones;
		else
			zone = NULL;
	}
	return zone;
}

