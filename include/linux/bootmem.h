/*支持不连续内存*/
#ifndef _LINUX_BOOTMEM_H
#define _LINUX_BOOTMEM_H

#include <linux/mmzone.h>
#include <asm/dma.h>

/*系统启动期间，尽管内存管理尚未初始化，但内核仍然需要分配内存以创建各种数据结构。
bootmem分配器用于在启动阶段早期分配内存。实现的是一个最先适配分配器用于在启动阶段管
理内存。该分配器使用一个位图来管理页，位图比特位的数目与系统中物理内存页的数目相同。
比特位为1，表示页已被使用；比特位为0，表示页处于空闲状态。在需要分配内存时，分配器逐
位扫描位图，直到找到一个能够提供足够连续页的位置，即所谓的最先最佳（first-best）或最
先（first-fit）适配位置。该过程不是很高效，因为每次分配都必须从头扫描比特位。因此，在
内核完全初始化之后，不能将该分配器用于内存管理。伙伴系统（buddy system）连同slab或slob
或slub分配器是一个好的多的备选方案*/
extern unsigned long max_low_pfn;
extern unsigned long min_low_pfn;

/*最高页*/
extern unsigned long max_pfn;

#ifdef CONFIG_CRASH_DUMP
extern unsigned long saved_max_pfn;
#endif

/*内核（为系统中的每个结点都）提供了一个最先适配分配器来管理系统启动时的直接映射区
内存，该结构所需的内存无法动态分配，必须在编译时分配给内核*/
typedef struct bootmem_data
{
	/*保存系统中第一个页的物理地址（而非PLKA书上说的第一个页编号），大多数体系结构下都是零*/
	unsigned long node_boot_start;
	/*保存可以直接管理的物理地址空间中最后一个页的编号。也就是直接映射区的结束页*/
	unsigned long node_low_pfn;
	/*指向存储分配位图的内存区指针，位图中的所有位代表包含空洞的结点上的所有物理内存页
	。在IA-32系统上，用于该用途的内存区紧接着内核映像之后，对应的地址保存在_end变量中，
	该变量在链接期间自动地插入到内核映像中*/
	void *node_bootmem_map;
	/*保存上次分配时的偏移量，如果为零，则说明上次分配的是一整页*/
	unsigned long last_offset;
	/*保存上次分配的页的编号。如果没有请求分配整个页，则last_offset用作该页内部的偏移
	量。这使得bootmem分配器可以分配小于一整页的内存区（伙伴系统无法做到这一点）。*/
	unsigned long last_pos;
	/*为了加快寻找，保存位图中上次成功分配内存的位置。新的分配将由此开始*/
	unsigned long last_success;
	/*内存不连续的系统可能需要多个bootmem分配器。一个典型的例子时NUMA计算机，其中每个
	结点注册了一个bootmem分配器，但如果物理内存地址空间中散布着较小的空洞，也可以为每
	个连续内存区注册一个bootmem分配器*/
	struct list_head list;
} bootmem_data_t;

extern unsigned long bootmem_bootmap_pages(unsigned long);
extern unsigned long init_bootmem(unsigned long addr, unsigned long memend);
extern void free_bootmem(unsigned long addr, unsigned long size);
extern void *__alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal);
extern void *__alloc_bootmem_nopanic(unsigned long size, unsigned long align, unsigned long goal);
extern void *__alloc_bootmem_low(unsigned long size, unsigned long align, unsigned long goal);
extern void *__alloc_bootmem_low_node(pg_data_t *pgdat,	unsigned long size, unsigned long align,
				      						unsigned long goal);
extern void *__alloc_bootmem_core(struct bootmem_data *bdata, unsigned long size,
				  unsigned long align, unsigned long goal, unsigned long limit);

#ifndef CONFIG_HAVE_ARCH_BOOTMEM_NODE
extern void reserve_bootmem(unsigned long addr, unsigned long size);
/*系统启动期间的物理内存分配。在ZONE_DMA结束位置开始，分配一个x字节长且硬件缓存行对
齐的内存区*/
#define alloc_bootmem(x) __alloc_bootmem(x, SMP_CACHE_BYTES, __pa(MAX_DMA_ADDRESS))
/*系统启动期间的物理内存分配。从头开始，分配一个x字节长且硬件缓存行对齐的内存区*/
#define alloc_bootmem_low(x)	__alloc_bootmem_low(x, SMP_CACHE_BYTES, 0)
/*系统启动期间的物理内存分配。在ZONE_DMA结束位置开始，分配一个x字节长且页长度对齐的
内存区*/
#define alloc_bootmem_pages(x)	__alloc_bootmem(x, PAGE_SIZE, __pa(MAX_DMA_ADDRESS))
/*系统启动期间的物理内存分配。从头开始，分配一个x字节长且页长度对齐的内存区*/
#define alloc_bootmem_low_pages(x)	__alloc_bootmem_low(x, PAGE_SIZE, 0)
#endif /* !CONFIG_HAVE_ARCH_BOOTMEM_NODE */

extern unsigned long free_all_bootmem(void);
extern unsigned long free_all_bootmem_node(pg_data_t *pgdat);
extern void *__alloc_bootmem_node(pg_data_t *pgdat, unsigned long size, unsigned long align,
				  						unsigned long goal);
extern unsigned long init_bootmem_node(pg_data_t *pgdat, unsigned long freepfn,
										unsigned long startpfn, unsigned long endpfn);
extern void reserve_bootmem_node(pg_data_t *pgdat, unsigned long physaddr, unsigned long size);
extern void free_bootmem_node(pg_data_t *pgdat,	unsigned long addr, unsigned long size);

#ifndef CONFIG_HAVE_ARCH_BOOTMEM_NODE
/*系统启动期间，在指定结点上的ZONE_DMA结束位置开始，分配一个特定长度且硬件缓存行对齐
的内存区*/
#define alloc_bootmem_node(pgdat, x) \
	__alloc_bootmem_node(pgdat, x, SMP_CACHE_BYTES, __pa(MAX_DMA_ADDRESS))
/*系统启动期间，在指定结点上的ZONE_DMA结束位置开始，分配一个特定长度且页长度对齐的
内存区*/
#define alloc_bootmem_pages_node(pgdat, x) \
	__alloc_bootmem_node(pgdat, x, PAGE_SIZE, __pa(MAX_DMA_ADDRESS))
/*系统启动期间，在指定结点上的内存起始处开始，分配一个特定长度且页长度对齐的内存区*/
#define alloc_bootmem_low_pages_node(pgdat, x) \
	__alloc_bootmem_low_node(pgdat, x, PAGE_SIZE, 0)
#endif /* !CONFIG_HAVE_ARCH_BOOTMEM_NODE */

#ifdef CONFIG_HAVE_ARCH_ALLOC_REMAP
extern void *alloc_remap(int nid, unsigned long size);
#else
static inline void *alloc_remap(int nid, unsigned long size)
{
	return NULL;
}
#endif /* CONFIG_HAVE_ARCH_ALLOC_REMAP */

extern unsigned long __meminitdata nr_kernel_pages;
extern unsigned long __meminitdata nr_all_pages;

extern void *alloc_large_system_hash(const char *tablename,
				     unsigned long bucketsize,
				     unsigned long numentries,
				     int scale,
				     int flags,
				     unsigned int *_hash_shift,
				     unsigned int *_hash_mask,
				     unsigned long limit);

#define HASH_EARLY	0x00000001	/* Allocating during early boot? */

/*仅NUMA需要散列分配。IA-64和X86_64有足够的VMALLOC空间*/
#if defined(CONFIG_NUMA) && (defined(CONFIG_IA64) || defined(CONFIG_X86_64))
#define HASHDIST_DEFAULT 1
#else
#define HASHDIST_DEFAULT 0
#endif
extern int hashdist;		/* Distribute hashes across NUMA nodes? */


#endif /* _LINUX_BOOTMEM_H */
