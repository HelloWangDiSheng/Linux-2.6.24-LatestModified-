#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <linux/module.h>
#include <linux/nodemask.h>
#include <asm/numaq.h>
#include <asm/topology.h>
#include <asm/processor.h>

/*适用于NUMA-Q计算机*/

#define	MB_TO_PAGES(addr) ((addr) << (20 - PAGE_SHIFT))

/*
 * Function: smp_dump_qct()
 *
 * Description: gets memory layout from the quad config table.  This
 * function also updates node_online_map with the nodes (quads) present.
 */
static void __init smp_dump_qct(void)
{
	int node;
	struct eachquadmem *eq;
	struct sys_cfg_data *scd = (struct sys_cfg_data *)__va(SYS_CFG_DATA_PRIV_ADDR);
	/*清空在线结点位图*/
	nodes_clear(node_online_map);
	/*遍历所有结点*/
	for_each_node(node)
	{
		if (scd->quads_present31_0 & (1 << node))
		{
			node_set_online(node);
			eq = &scd->eq[node];
			/* Convert to pages */
			node_start_pfn[node] = MB_TO_PAGES(		eq->hi_shrd_mem_start - eq->priv_mem_size);
			node_end_pfn[node] = MB_TO_PAGES(eq->hi_shrd_mem_start + eq->hi_shrd_mem_size);
			memory_present(node,	node_start_pfn[node], node_end_pfn[node]);
			node_remap_size[node] = node_memmap_size_bytes(node,	node_start_pfn[node],
										node_end_pfn[node]);
		}
	}
}

/*
 * Unlike Summit, we don't really care to let the NUMA-Q
 * fall back to flat mode.  Don't compile for NUMA-Q
 * unless you really need it!
 */
int __init get_memcfg_numaq(void)
{
	smp_dump_qct();
	return 1;
}

/*在线结点多于1个时禁用tsc功能*/
static int __init numaq_tsc_disable(void)
{

	if (num_online_nodes() > 1)
	{
		printk(KERN_DEBUG "NUMAQ: disabling TSC\n");
		tsc_disable = 1;
	}
	return 0;
}
arch_initcall(numaq_tsc_disable);
