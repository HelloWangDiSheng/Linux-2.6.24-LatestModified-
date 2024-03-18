/*管理结点和内存域的代码*/

#include <linux/stddef.h>
#include <linux/mmzone.h>
#include <linux/module.h>

/*获取第一个在线（可被内核管理）的结点*/
struct pglist_data *first_online_pgdat(void)
{
	return NODE_DATA(first_online_node);
}

/*获取指定结点的下一个在线结点*/
struct pglist_data *next_online_pgdat(struct pglist_data *pgdat)
{
	/*获取指定结点的下一个结点的编号*/
	int nid = next_online_node(pgdat->node_id);
	/*指定结点已经是最后一个结点，下一个结点编号无效*/
	if (nid == MAX_NUMNODES)
		return NULL;
	/*返回指定编号的结点实例*/
	return NODE_DATA(nid);
}

/*获取当前内存域的下一个内存域。如果当前内存域是当前结点的最后一个内存域，则获取当前
结点的下一个结点，下一结点存在时，则获取下一结点的第一个内存域，否则，设置为空*/
struct zone *next_zone(struct zone *zone)
{
	/*获取当前内存域所属的结点*/
	pg_data_t *pgdat = zone->zone_pgdat;
	/*如果当前内存域不是该结点的最后内存域，则定位到下个内存域*/
	if (zone < pgdat->node_zones + MAX_NR_ZONES - 1)
	{
		zone++;
	}
	else
	{
		/*获取该结点的下一个在线结点*/
		pgdat = next_online_pgdat(pgdat);
		/*结点存在时就获取该结点的第一个内存域，否则，设置为空*/
		if (pgdat)
			zone = pgdat->node_zones;
		else
			zone = NULL;
	}
	return zone;
}

