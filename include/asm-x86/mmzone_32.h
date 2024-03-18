#ifndef _ASM_MMZONE_H_
#define _ASM_MMZONE_H_

#include <asm/smp.h>

#ifdef CONFIG_NUMA
/*声明全局结点数组变量*/
extern struct pglist_data *node_data[];
/*获取NUMA结点数组中指定编号的结点*/
#define NODE_DATA(nid)	(node_data[nid])

#ifdef CONFIG_X86_NUMAQ
	#include <asm/numaq.h>
#elif defined(CONFIG_ACPI_SRAT)/* summit or generic arch */
	#include <asm/srat.h>
#endif

extern int get_memcfg_numa_flat(void );
/*
 * This allows any one NUMA architecture to be compiled
 * for, and still fall back to the flat function if it
 * fails.
 */
static inline void get_memcfg_numa(void)
{
#ifdef CONFIG_X86_NUMAQ
	if (get_memcfg_numaq())
		return;
#elif defined(CONFIG_ACPI_SRAT)
	if (get_memcfg_from_srat())
		return;
#endif

	get_memcfg_numa_flat();
}

extern int early_pfn_to_nid(unsigned long pfn);
extern void numa_kva_reserve(void);

#else /* !CONFIG_NUMA */

#define get_memcfg_numa get_memcfg_numa_flat
#define get_zholes_size(n) (0)

static inline void numa_kva_reserve(void)
{
}
#endif /* CONFIG_NUMA */

#ifdef CONFIG_DISCONTIGMEM

/*通用结点内存支持。应用以下假定：（1）256M的连续内存块组成的内存（2）不超过64G内存
假定整个系统中最大64GRAM，页大小为4K，因此共有MAX_NR_PAGES个页
64G内存分为256个256M连续内存块，每块中有256M/4K=65536个页*/
#define MAX_NR_PAGES 16777216
/**/
#define MAX_ELEMENTS 256
/*65536*/
#define PAGES_PER_ELEMENT (MAX_NR_PAGES/MAX_ELEMENTS)
/*物理结点编号数组，每一项中包含PAGES_PER_ELEMENT个页*/
extern s8 physnode_map[];
/**/
static inline int pfn_to_nid(unsigned long pfn)
{
#ifdef CONFIG_NUMA
	/*页帧号除于*/
	return((int) physnode_map[(pfn) / PAGES_PER_ELEMENT]);
#else
	return 0;
#endif
}

/*
 * Following are macros that each numa implmentation must define.
 */

/*获取指定结点的起始页帧号*/
#define node_start_pfn(nid)	(NODE_DATA(nid)->node_start_pfn)
/*获取指定结点的结束页帧号，先获取结点，然后将结点的起始页帧号与结点内包含空洞的
页的数目之和就计算出结束页帧号*/
#define node_end_pfn(nid)										\
({																\
	pg_data_t *__pgdat = NODE_DATA(nid);						\
	__pgdat->node_start_pfn + __pgdat->node_spanned_pages;		\
})

#define kern_addr_valid(kaddr)	(0)

/*NUMA-Q上有连续内存*/
#ifdef CONFIG_X86_NUMAQ
/*测试页帧号的有效性*/
#define pfn_valid(pfn)          ((pfn) < num_physpages)
#else
/*测试页帧号的有效性*/

static inline int pfn_valid(int pfn)
{
	/*获取页帧号对应的结点编号*/
	int nid = pfn_to_nid(pfn);
	/*如果结点有效，且页帧号小于该结点的结束页帧号，则该页帧号有效*/
	if (nid >= 0)
		return (pfn < node_end_pfn(nid));
	return 0;
}
#endif /* CONFIG_X86_NUMAQ */

#endif /* CONFIG_DISCONTIGMEM */

#ifdef CONFIG_NEED_MULTIPLE_NODES

/**/
#define reserve_bootmem(addr, size) reserve_bootmem_node(NODE_DATA(0), (addr), (size))
/*在0结点的普通内存域分配x字节长度，按硬件（L1）缓存行对齐的内存*/
#define alloc_bootmem(x) \
	__alloc_bootmem_node(NODE_DATA(0), (x), SMP_CACHE_BYTES, __pa(MAX_DMA_ADDRESS))
/*在0结点的DMA内存域中分配x字节长度，按硬件缓存行对齐的内存*/
#define alloc_bootmem_low(x) __alloc_bootmem_node(NODE_DATA(0), (x), SMP_CACHE_BYTES, 0)
/*在0结点的普通内存域中分配x字节长度，按页长度对齐的内存*/
#define alloc_bootmem_pages(x) \
	__alloc_bootmem_node(NODE_DATA(0), (x), PAGE_SIZE, __pa(MAX_DMA_ADDRESS))
/*在0结点的DMAA内存域中分配x字节长度，按页长度对齐的内存*/
#define alloc_bootmem_low_pages(x) __alloc_bootmem_node(NODE_DATA(0), (x), PAGE_SIZE, 0)
#define alloc_bootmem_node(pgdat, x)																\
({																									\
	struct pglist_data  __maybe_unused *__alloc_bootmem_node__pgdat = (pgdat);						\
	__alloc_bootmem_node(NODE_DATA(0), (x), SMP_CACHE_BYTES,	__pa(MAX_DMA_ADDRESS));\
})
/*在0结点的普通内存域中分配x字节长度按页长度对齐的内存*/
#define alloc_bootmem_pages_node(pgdat, x)													\
({																							\
	struct pglist_data  __maybe_unused	*__alloc_bootmem_node__pgdat = (pgdat);				\
	__alloc_bootmem_node(NODE_DATA(0), (x), PAGE_SIZE,	__pa(MAX_DMA_ADDRESS))\
})
/*在0结点的DMA内存域中分配x字节长度，按页长度对齐的内存*/
#define alloc_bootmem_low_pages_node(pgdat, x)												\
({																							\
	struct pglist_data  __maybe_unused	*__alloc_bootmem_node__pgdat = (pgdat);				\
	__alloc_bootmem_node(NODE_DATA(0), (x), PAGE_SIZE, 0);									\
})
#endif /* CONFIG_NEED_MULTIPLE_NODES */

#endif /* _ASM_MMZONE_H_ */
