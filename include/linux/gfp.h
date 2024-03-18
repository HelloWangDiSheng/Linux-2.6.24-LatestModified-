#ifndef __LINUX_GFP_H
#define __LINUX_GFP_H

#include <linux/mmzone.h>
#include <linux/stddef.h>
#include <linux/linkage.h>

struct vm_area_struct;

/*内存域修饰符*/
/*在ZONE_DMA内存域中分配空闲页*/
#define __GFP_DMA			((__force gfp_t)0x01u)
/*在ZONE_HIGHMEM内存域中分配空闲页*/
#define __GFP_HIGHMEM		((__force gfp_t)0x02u)
/*在ZONE_DMA32内存域中分配空闲页*/
#define __GFP_DMA32			((__force gfp_t)0x04u)

/*不改变内存域修饰符。不限制从哪个内存域分配内存，但可以改变分配器的行为*/
/*分配空闲页期间可以等待和调度。分配内存的请求可以中断。调度器在该请求期间可以任意
选择另一个进程执行，或者该请求可以被另一个更重要的事件中断。分配器还可以在返回内存
之前，在队列上等待一个事件（相关进程会进入睡眠状态）*/
#define __GFP_WAIT				((__force gfp_t)0x10u)
/*启用紧急分配池。如果请求非常重要，则设置该选项，即内核急切地需要内存时，在分配失败
可能给内核带来严重后果时（比如威胁到系统的稳定性或系统崩溃），总是会使用该标识*/
#define __GFP_HIGH				((__force gfp_t)0x20u)
/*在查找空闲内存页期间，内核可以进行I/O操作。实际上，这意味着如果内核在内存分配期间
换出页，那么仅当设置该标识时，才能将选择的页写入硬盘*/
#define __GFP_IO				((__force gfp_t)0x40u)
/*允许内核执行低级VFS操作。在与VFS层有联系的内核子系统中必须禁用，因为这可能引起循环
递归调用*/
#define __GFP_FS				((__force gfp_t)0x80u)
/*如果需要分配不在cpu高速缓存中的冷页时，则设置该标识*/
#define __GFP_COLD				((__force gfp_t)0x100u)
/*在分配失败时禁止内核故障警告。在极少数场合，该标识有用*/
#define __GFP_NOWARN			((__force gfp_t)0x200u)
/*在分配失败后自动重试，但在尝试若干次之后会停止*/
#define __GFP_REPEAT			((__force gfp_t)0x400u)
/*在分配失败时一直重试，直至成功*/
#define __GFP_NOFAIL			((__force gfp_t)0x800u)
/*在分配失败时直接返回*/
#define __GFP_NORETRY			((__force gfp_t)0x1000u)
/*分配巨型页*/
#define __GFP_COMP				((__force gfp_t)0x4000u)/* Add compound page metadata */
/*在分配成功时，将返回填充字节0的页*/
#define __GFP_ZERO				((__force gfp_t)0x8000u)
/*不使用紧急储备lowmem_reserve*/
#define __GFP_NOMEMALLOC 		((__force gfp_t)0x10000u)
/*只在NUMA系统上有意义。它限制只在分配到当前进程的各个cpu所关联的结点分配内存。如果
进程允许在所有cpu上运行（默认情况），该标识是无意义的，只有进程可以运行的cpu受限时，
该标识才有效果*/
#define __GFP_HARDWALL			((__force gfp_t)0x20000u)
/*也只有在NUMA系统上有意义。分配失败的情况下不允许使用其它结点作为备用。需要保证在当
前结点或者明确指定的结点上成功分配内存*/
#define __GFP_THISNODE			((__force gfp_t)0x40000u)
/*分配可回收的页面*/
#define __GFP_RECLAIMABLE		((__force gfp_t)0x80000u)
/*分配可移动的页面*/
#define __GFP_MOVABLE			((__force gfp_t)0x100000u)  

/*21个__GFP_*位空间*/
#define __GFP_BITS_SHIFT 		21
/*__GFP_*位掩码*/
#define __GFP_BITS_MASK 		((__force gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

/*等于0，假如会改变时应该使用常数*/
#define GFP_NOWAIT	(GFP_ATOMIC & ~__GFP_HIGH)
/* GFP_ATOMIC means both !wait (__GFP_WAIT not set) and use emergency pool */
/*原子分配，在任何情况下都不能中断，可使用紧急分配链表中的内存*/
#define GFP_ATOMIC	(__GFP_HIGH)
/*禁止I/O操作*/
#define GFP_NOIO	(__GFP_WAIT)
/*禁止VFS操作*/
#define GFP_NOFS	(__GFP_WAIT | __GFP_IO)
/*内核分配的默认设置，内核代码总最常使用的标识*/
#define GFP_KERNEL	(__GFP_WAIT | __GFP_IO | __GFP_FS)
/*分配可回收的内核页面*/
#define GFP_TEMPORARY	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_RECLAIMABLE)
/*用户分配的默认设置。在进程允许运行的cpu对应的内存结点上分配*/
#define GFP_USER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
/*GFP_USER的基础上在高端内存域中分配*/
#define GFP_HIGHUSER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | \
									__GFP_HIGHMEM | __GFP_MOVABLE)
#define GFP_NOFS_PAGECACHE	(__GFP_WAIT | __GFP_IO | __GFP_MOVABLE)
#define GFP_USER_PAGECACHE	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | __GFP_MOVABLE)
#define GFP_HIGHUSER_PAGECACHE	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | \
									__GFP_HIGHMEM | __GFP_MOVABLE)

#ifdef CONFIG_NUMA
/*配置NUMA选项时在允许进程运行的cpu对应的内存结点上分配，并且分配失败后不警告，不重试*/
#define GFP_THISNODE	(__GFP_THISNODE | __GFP_NOWARN | __GFP_NORETRY)
#else
/*无NUMA配置时只能在当前结点上分配空闲内存页*/
#define GFP_THISNODE	((__force gfp_t)0)
#endif

/*在可回收页和可移动页中分配空闲页*/
#define GFP_MOVABLE_MASK (__GFP_RECLAIMABLE|__GFP_MOVABLE)

/*控制页面分配器的页面回收行为*/
#define GFP_RECLAIM_MASK (__GFP_WAIT|__GFP_HIGH|__GFP_IO|__GFP_FS|__GFP_NOWARN|\
				__GFP_REPEAT|__GFP_NOFAIL| __GFP_NORETRY|__GFP_NOMEMALLOC)

/*控制分配限制*/
#define GFP_CONSTRAINT_MASK (__GFP_HARDWALL|__GFP_THISNODE)

/*不能在slab中使用这些分配标识*/
#define GFP_SLAB_BUG_MASK (__GFP_DMA32|__GFP_HIGHMEM|~__GFP_BITS_MASK)

/* Flag - indicates that the buffer will be suitable for DMA.  Ignored on some
   platforms, used as appropriate on others */
/**/
#define GFP_DMA		__GFP_DMA

/*一些平台上被认为是4GDMA*/
#define GFP_DMA32	__GFP_DMA32

/*将GFP标识转换成迁移类型*/
static inline int allocflags_to_migratetype(gfp_t gfp_flags)
{
	/**/
	WARN_ON((gfp_flags & GFP_MOVABLE_MASK) == GFP_MOVABLE_MASK);
	/**/
	if (unlikely(page_group_by_mobility_disabled))
		return MIGRATE_UNMOVABLE;

	/* Group based on mobility */
	return (((gfp_flags & __GFP_MOVABLE) != 0) << 1) |
		((gfp_flags & __GFP_RECLAIMABLE) != 0);
}

/*根据分配页掩码获取对应的内存域*/
static inline enum zone_type gfp_zone(gfp_t flags)
{
	int base = 0;
/*配置NUMA的当前结点结点*/
#ifdef CONFIG_NUMA
	if (flags & __GFP_THISNODE)
		base = MAX_NR_ZONES;
#endif

#ifdef CONFIG_ZONE_DMA
/*如果配置了ZONE_DMA内存域，分配时指定在该区域分配，则返回结点的ZONE_DMA内存域编号*/

	if (flags & __GFP_DMA)
		return base + ZONE_DMA;
#endif

#ifdef CONFIG_ZONE_DMA32
/*如果配置了ZONE_DMA32内存域，分配时指定在该区域分配，则返回结点的ZONE_DMA32内存域编号*/

	if (flags & __GFP_DMA32)
		return base + ZONE_DMA32;
#endif
/*配置高端内存域，并且启用虚拟内存域时，返回结点对应的虚拟内存域编号*/
	if ((flags & (__GFP_HIGHMEM | __GFP_MOVABLE)) == (__GFP_HIGHMEM | __GFP_MOVABLE))
		return base + ZONE_MOVABLE;
	
#ifdef CONFIG_HIGHMEM
/*如果配置了ZONE_HIGHMEM内存域，分配时指定在该区域分配，则返回结点的ZONE_HIGHMEM内存域编号*/
	if (flags & __GFP_HIGHMEM)
		return base + ZONE_HIGHMEM;
#endif

/*返回结点的ZONE_NORMAL内存域编号*/
	return base + ZONE_NORMAL;
}

/*根据页分配的掩码，设置迁移类型。注意！页分配掩码不能指定为虚拟内存域*/
static inline gfp_t set_migrateflags(gfp_t gfp, gfp_t migrate_flags)
{
	/*页分配掩码中不能在虚拟内存域中分配可回收内存页*/
	BUG_ON((gfp & GFP_MOVABLE_MASK) == GFP_MOVABLE_MASK);
	return (gfp & ~(GFP_MOVABLE_MASK)) | migrate_flags;
}

/*
 * There is only one page-allocator function, and two main namespaces to
 * it. The alloc_page*() variants return 'struct page *' and as such
 * can allocate highmem pages, the *get*page*() variants return
 * virtual kernel addresses to the allocated page(s).
 */

/*
 * We get the zone list from the current node and the gfp_mask.
 * This zone list contains a maximum of MAXNODES*MAX_NR_ZONES zones.
 *
 * For the normal case of non-DISCONTIGMEM systems the NODE_DATA() gets
 * optimized to &contig_page_data at compile-time.
 */

#ifndef HAVE_ARCH_FREE_PAGE
static inline void arch_free_page(struct page *page, int order) { }
#endif
#ifndef HAVE_ARCH_ALLOC_PAGE
static inline void arch_alloc_page(struct page *page, int order) { }
#endif

extern struct page *FASTCALL(__alloc_pages(gfp_t, unsigned int, struct zonelist *));

/*在指定结点（指定结点为负值时默认为当前结点）上分配页*/
static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order)
{
	/*分配阶无效*/
	if (unlikely(order >= MAX_ORDER))
		return NULL;

	/*未指定结点时默认为当前结点*/
	if (nid < 0)
		nid = numa_node_id();
	/*从指定结点的备用内存域上由分配掩码gfp_mask获取的内存域开始分配*/
	return __alloc_pages(gfp_mask, order, NODE_DATA(nid)->node_zonelists + gfp_zone(gfp_mask));
}

#ifdef CONFIG_NUMA
extern struct page *alloc_pages_current(gfp_t gfp_mask, unsigned order);

/*从当前结点分配内存页*/
static inline struct page *alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	/*分配阶无效*/
	if (unlikely(order >= MAX_ORDER))
		return NULL;
	/*在当前结点上分配*/
	return alloc_pages_current(gfp_mask, order);
}
extern struct page *alloc_page_vma(gfp_t gfp_mask, struct vm_area_struct *vma, unsigned long addr);
#else
/*在当前结点获取指定分配类型和分配阶的空闲页*/
#define alloc_pages(gfp_mask, order) alloc_pages_node(numa_node_id(), gfp_mask, order)
#define alloc_page_vma(gfp_mask, vma, addr) alloc_pages(gfp_mask, 0)
#endif
/*申请一个页*/
#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)
extern unsigned long FASTCALL(__get_free_pages(gfp_t gfp_mask, unsigned int order));
extern unsigned long FASTCALL(get_zeroed_page(gfp_t gfp_mask));

/*获取一个空闲页*/
#define __get_free_page(gfp_mask) 		__get_free_pages((gfp_mask), 0)
/*在ZONE_DMA内存域中获取指定分配阶和分配标的空闲页*/
#define __get_dma_pages(gfp_mask, order) __get_free_pages((gfp_mask) | GFP_DMA, (order))
extern void FASTCALL(__free_pages(struct page *page, unsigned int order));
extern void FASTCALL(free_pages(unsigned long addr, unsigned int order));
extern void FASTCALL(free_hot_page(struct page *page));
extern void FASTCALL(free_cold_page(struct page *page));
/*释放页。注意！该参数是struct page实例*/
#define __free_page(page) __free_pages((page), 0)
/*释放虚拟地址对应的页*/
#define free_page(addr) free_pages((addr),0)

void page_alloc_init(void);
void drain_zone_pages(struct zone *zone, struct per_cpu_pages *pcp);

#endif /* __LINUX_GFP_H */
