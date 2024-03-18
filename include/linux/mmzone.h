#ifndef _LINUX_MMZONE_H
#define _LINUX_MMZONE_H

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/bitops.h>
#include <linux/cache.h>
#include <linux/threads.h>
#include <linux/numa.h>
#include <linux/init.h>
#include <linux/seqlock.h>
#include <linux/nodemask.h>
#include <linux/pageblock-flags.h>
#include <asm/atomic.h>
#include <asm/page.h>

/*空闲内存管理 伙伴区域分配阶*/
#ifndef CONFIG_FORCE_MAX_ZONEORDER
#define MAX_ORDER 11
#else
#define MAX_ORDER CONFIG_FORCE_MAX_ZONEORDER
#endif

/*最大分配阶对应的分配连续页数目*/
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

/*
 * PAGE_ALLOC_COSTLY_ORDER is the order at which allocations are deemed
 * costly to service.  That is between allocation orders which should
 * coelesce naturally under reasonable reclaim pressure and those which
 * will not.
 */
#define PAGE_ALLOC_COSTLY_ORDER 3
/*依据页的可移动性组织页是防止物理内存碎片的一种可能方法，另一种阻止该问题的手段是
：虚拟内存域ZONE_MOVABLE。基本思想很简单：可用的物理内存划分为两个内存域，一个用于
可移动分配，一个用于不可移动分配。只会自动防止不可移动页向可移动内存域引入碎片*/

/*页面的可移动性，依赖该页属于3中类别的哪一种。内核使用的反碎片技术，即基于将具有相
同可移动性的页分组的思想*/
/*不可移动页：在内存中有固定位置，不能移动到其它地方。核心内核分配的大多数内存属于该
类型*/
#define MIGRATE_UNMOVABLE		    0
/*可回收页：不能直接移动，但可以删除，其内容可用从某些源重新生成。例如，映射自文件的
数据属于该类别。kswap守护进程会根据可回收页访问的频繁程度，周期性释放此内存。内核会在
可回收页占据太多内存时进行回收。另外，在内存短缺（即分配失败）时也可以发起页面回收*/
#define MIGRATE_RECLAIMABLE   		1
/*可移动页可以随意移动。属于用户空间应用程序的页属于该类别。它们时通过页表映射的。如
果它们复制到新位置，页表项可以相应地更新，应用程序不会注意到任何事*/
#define MIGRATE_MOVABLE       		2
/*如果向具有特定可移动性的列表请求分配内存失败，在这种情况下可以从MIGRATE_RESERVE分配
内存（对应的列表在内存子系统初始化期间用setup_zone_migrate_reserve填充）*/
#define MIGRATE_RESERVE       		3
/*特殊的虚拟内存区域，用于跨越NUMA结点移动物理内存页。在大型系统上，它有益于将物理内存
页移动到接近于使用该页最频繁的cpu*/
#define MIGRATE_ISOLATE       		4 /* can't allocate from here */
/*页面迁移类型的数目，不代表具体的区域*/
#define MIGRATE_TYPES         		5

/*遍历所有分配阶的所有迁移类型*/
#define for_each_migratetype_order(order, type) 				\
	for (order = 0; order < MAX_ORDER; order++) 				\
		for (type = 0; type < MIGRATE_TYPES; type++)

extern int page_group_by_mobility_disabled;

/*获得页的迁移类型*/
static inline int get_pageblock_migratetype(struct page *page)
{
	/*如果禁用了页面迁移特性，则所有页都是不可移动的*/
	if (unlikely(page_group_by_mobility_disabled))
		return MIGRATE_UNMOVABLE;

	return get_pageblock_flags_group(page, PB_migrate, PB_migrate_end);
}

/*对伙伴系统数据结构的主要调整，是将空闲列表分解为MIGRATE_TYPE个列表*/
struct free_area
{
	/*每种迁移类型都对应了一个空闲列表*/
	struct list_head	free_list[MIGRATE_TYPES];
	/*统计了所有列表空闲页的数目*/
	unsigned long		nr_free;
};

struct pglist_data;

/*
 * zone->lock and zone->lru_lock are two of the hottest locks in the kernel.
 * So add a wild amount of padding here to ensure that they fall into separate
 * cachelines.  There are very few zone structures in the machine, so space
 * consumption is not a concern here.
 */
#if defined(CONFIG_SMP)
/*填充数据*/
struct zone_padding
{
	/*sizeof(struct zone_padding)为0，填充结构一般放在末尾用作对齐*/
	char x[0];
} ____cacheline_internodealigned_in_smp;
#define ZONE_PADDING(name)	struct zone_padding name;
#else
#define ZONE_PADDING(name)
#endif

/*系统内存页的状态*/
enum zone_stat_item
{
	/*第一个128字节缓存行（假定字长时64比特位）*/
	/*空闲页的数目*/
	NR_FREE_PAGES,
	/*惰性lru链表上的页数目*/
	NR_INACTIVE,
	/*活动lru链表上的页数目*/
	NR_ACTIVE,
	/*已映射的匿名页数目*/
	NR_ANON_PAGES,
	/*被页表机制映射的页的数目（只计算基于文件的页，直接的内核映射不包含在内）只在
	进程上下文中修改*/
	NR_FILE_MAPPED,
	/**/
	NR_FILE_PAGES,
	/*基于文件的脏页数目*/
	NR_FILE_DIRTY,
	/*当前正在回写的页的数目*/
	NR_WRITEBACK,
	/* Second 128 byte cacheline */
	/**/
	NR_SLAB_RECLAIMABLE,
	/**/
	NR_SLAB_UNRECLAIMABLE,
	/*用于存放页表的页的数目*/
	NR_PAGETABLE,
	/**/
	NR_UNSTABLE_NFS,	/* NFS unstable pages */
	/**/
	NR_BOUNCE,
	/**/
	NR_VMSCAN_WRITE,
#ifdef CONFIG_NUMA
	/**/
	NUMA_HIT,		/* allocated in intended node */
	/**/
	NUMA_MISS,		/* allocated in non intended node */
	/**/
	NUMA_FOREIGN,		/* was intended here, hit elsewhere */
	/**/
	NUMA_INTERLEAVE_HIT,	/* interleaver preferred this zone */
	/**/
	NUMA_LOCAL,		/* allocation from local node */
	/**/
	NUMA_OTHER,		/* allocation from other node */
#endif
	/**/
	NR_VM_ZONE_STAT_ITEMS
};

/**/
struct per_cpu_pages
{
	/*记录与该列表相关的页数目*/
	int count;
	/*水印值，如果count>high，则表明列表中的页太多了，需要清空，对容量过低的状态没有
	显式使用水印，如果列表中没有成员，则重新填充*/
	int high;
	/*如有可能，CPU的高速缓存不是用单个页来填充或删除的，而是用多个页组成的块。batch时
	每次添加或删除页数的一个参考值*/
	int batch;
	/*保存了当前cpu的冷页或热页，可使用内核的标准方法处理*/
	struct list_head list;
};

/*实现冷热分配器（hot-n-cold allocator）。内核说页是热的，意味着页已经加载到cpu高速
缓存中，与在内存中的页相比，其数据能够更快的访问。相反，冷页则不在高速缓存中。在多处
理器系统上，每个cpu都有一个或多个高速缓存，各个cpu的管理必须是独立的。尽管内存域可能
属于一个特定的NUMA结点，因而关联到某个特定的cpu，但其它cpu高速缓存仍然可以包含该内存
域中页，最终的效果时，每个处理器都可以访问系统中所有的页，尽管速度不同。因此，特定于
内存域的数据结构不仅需要考虑到所属NUMA结点相关的cpu，还必须照顾到系统中其它的cpu*/
struct per_cpu_pageset
{
	/*索引0对应热页，索引1对应冷页*/
	struct per_cpu_pages pcp[2];
#ifdef CONFIG_NUMA
	/**/
	s8 expire;
#endif
#ifdef CONFIG_SMP
	/**/
	s8 stat_threshold;
	/*保存内存域中页面统计信息*/
	s8 vm_stat_diff[NR_VM_ZONE_STAT_ITEMS];
#endif
} ____cacheline_aligned_in_smp;

/*获取指定内存域中特定的per-CPU高速缓存*/
#ifdef CONFIG_NUMA
/**/
#define zone_pcp(__z, __cpu) ((__z)->pageset[(__cpu)])
#else
#define zone_pcp(__z, __cpu) (&(__z)->pageset[(__cpu)])
#endif

/*内存域类型*/
enum zone_type
{
#ifdef CONFIG_ZONE_DMA
	/*
	 * ZONE_DMA is used when there are devices that are not able
	 * to do DMA to all of addressable memory (ZONE_NORMAL). Then we
	 * carve out the portion of memory that is needed for these devices.
	 * The range is arch specific.
	 *
	 * Some examples
	 *
	 * Architecture		Limit
	 * ---------------------------
	 * parisc, ia64, sparc	<4G
	 * s390			<2G
	 * arm			Various
	 * alpha		Unlimited or 0-16MB.
	 *
	 * i386, x86_64 and multiple other arches
	 * 			<16M.
	 */
	/*标识适合DMA的内存域。该区域的长度依赖于处理器类型。在IA-32计算机上，一般的限制
	是16M，这是由古老的ISA设备强加的边界，但更现代的计算机也可能接受这一限制的影响*/
	ZONE_DMA,
#endif
#ifdef CONFIG_ZONE_DMA32
	/*
	 * x86_64 needs two ZONE_DMAs because it supports devices that are
	 * only able to do DMA to the lower 16M but also 32 bit devices that
	 * can only do DMA areas below 4G.
	 */
	/*标记了使用32位地址字可寻址、适合DMA的内存域。显然，只有在64位系统上，两种DMA
	内存域才有差别。在32位计算机上，本区域时空的。在Alpha和AMD64系统上，该区域的长度
	可能从0到4G*/
	ZONE_DMA32,
#endif
	/*
	 * Normal addressable memory is in ZONE_NORMAL. DMA operations can be
	 * performed on pages in ZONE_NORMAL if the DMA devices support
	 * transfers to all addressable memory.
	 */
	/*标记了可直接映射到内核段的普通内存域。这是在所有体系结构上保证都会存在的唯一内存
	域，但无法保证该地址范围对应了实际的物理内存。例如，如果在AMD64系统有2G内存，那么
	所有内存都属于ZONE_DMA32范围，erZONE_NORMAL则为空*/
	ZONE_NORMAL,
#ifdef CONFIG_HIGHMEM
	/*
	 * A memory area that is only addressable by the kernel through
	 * mapping portions into its own address space. This is for example
	 * used by i386 to allow the kernel to address the memory beyond
	 * 900MB. The kernel will set up special mappings (page
	 * table entries on i386) for each page that the kernel needs to
	 * access.
	 */
	/*超出内核段的物理内存。根据编译时的配置，可能无需考虑某些内存域。例如在64位系统
	中，并不需要考虑高端内存域，如果支持了只能访问4GB以下内存的32位外设，才需要DMA32
	内存域。在IA-32系统上，可以直接管理的物理内存数量不会超过896M，超过该值（直到最大
	4G为止）的内存只能通过高端内存寻址*/
	ZONE_HIGHMEM,
#endif
	/*伪内存域，在防止物理内存碎片的机制中需要使用该内存域，该特性必须有管理员显示激活
	，并不关联到任何硬件上有意义的内存范围，该内存域的页面来自高端内存域或普通内存域。
	该区域中的页全部都是可以迁移的，主要是为了防止内存碎片和支持内存的热插拔*/
	ZONE_MOVABLE,
	/*内存域数目结束标记，在内核想要迭代系统中所有内存域时，会用到该常量*/
	MAX_NR_ZONES
};

/*
 * When a memory allocation must conform to specific limitations (such
 * as being suitable for DMA) the caller will pass in hints to the
 * allocator in the gfp_mask, in the zone modifier bits.  These bits
 * are used to select a priority ordered list of memory zones which
 * match the requested limits. See gfp_zone() in include/linux/gfp.h
 */

/*
 * Count the active zones.  Note that the use of defined(X) outside
 * #if and family is not necessarily defined so ensure we cannot use
 * it later.  Use __ZONE_COUNT to work out how many shift bits we need.
 */
/*活动内存域数目统计。使用__ZONE_COUNT计算需要的偏移位，以IA-32 4G内存为例，活动
内存域数目为__ZONE_COUNT = 4，ZONES_SHIFT = 2，该偏移位是为了和page->flags标识与
不同内存模型中或有的内存段、结点、内存域组合，加速页操作*/
#define __ZONE_COUNT				\
(									\
	  defined(CONFIG_ZONE_DMA)		\
	+ defined(CONFIG_ZONE_DMA32)	\
	+ 1								\
	+ defined(CONFIG_HIGHMEM)		\
	+ 1								\
)

#if __ZONE_COUNT < 2
#define ZONES_SHIFT 0
#elif __ZONE_COUNT <= 2
#define ZONES_SHIFT 1
#elif __ZONE_COUNT <= 4
#define ZONES_SHIFT 2
#else
#error ZONES_SHIFT -- too many zones configured adjust calculation
#endif
#undef __ZONE_COUNT

/*内核使用该结构来描述内存域。该结构比较特殊的方面时它有ZONE_PADDING分割为几个部分，这
是因为对zone结构的访问非常频繁。在多处理器系统上，通常会有不同的cpu试图同时访问结构成员
，因此，使用锁防止它们彼此干扰，避免错误和不一致。由于内核对该结构的访问非常频繁，因此
会经常性地获取该结构的两个自旋锁zone->lock和zone->lru_lock。如果数据保存在高速缓存行中
，那么处理起来会更快速。高速缓存分为行，每一行负责不同的内存区。内核使用ZONE_PADDING宏
生成“填充“字段添加到结构中，以确保每个自旋锁都处于自身的缓存行中，还使用了编译器关键字
__cacheline_maxaligned_in_smp，用以实现最优的高速缓存对齐方式。该结构的最后两个部分也通
过填充字段彼此分隔开来。两者都不包锁，主要目的是将数据保持在一个缓存行中，便于快速访问
，从而无需从内存加载数据（与cpu高速缓存行相比，内存比较慢），由于填充造成该结构的增加是
可以忽略的，特别是在内核内存中zone结构的实例很少使用*/
struct zone
{
	/*通常由页分配器访问的字段，三者是页换出时使用的“水印”，如果内存不足，内核可以将页
	写到硬盘，这三个成员会影响到交换守护进程的行为*/
	/*如果空闲页数目低于pages_min，那么页回收工作的压力就比较大，因为内存域中急需空闲页*/
	unsigned long pages_min;
	/*如果空闲页的数目低于pages_low，则内核开始将页换出到硬盘*/
	unsigned long pages_low;
	/*如果空闲页多域pages_high，则内存域的状态时理想的*/
	unsigned long pages_high;
	/*
	 * We don't know if the memory that we're going to allocate will be freeable
	 * or/and it will be released eventually, so to avoid totally wasting several
	 * GB of ram we must reserve some of the lower zone memory (otherwise we risk
	 * to run OOM on the lower zones despite there's tons of freeable ram
	 * on the higher zones). This array is recalculated at runtime if the
	 * sysctl_lowmem_reserve_ratio sysctl changes.
	 */
	/*lowmem_reserve数组分别为各种内存域指定了若干页，用于一些无论如何都不能失败的关键
	性内存分配，各个内存域的份额根据重要性确定。
		 当进程请求内核分配内存时，如果此时内存比较充裕，那么进程的请求会被立刻满足，如
	果此时内存已经比较紧张，内核就需要将一部分不经常使用的内存进行回收，从而腾出一部分
	内存满足进程的内存分配的请求，在这个回收内存的过程中，进程会一直阻塞等待。另一种内
	存分配场景，进程是不允许阻塞的，内存分配的请求必须马上得到满足，比如执行中断处理程
	序或者执行持有自旋锁等临界区内的代码时，进程就不允许睡眠，因为中断程序无法被重新调
	度。这时就需要内核提前为这些核心操作预留一部分内存，当内存紧张时，可以使用这部分预
	留的内存给这些操作分配
	*/
	unsigned long lowmem_reserve[MAX_NR_ZONES];

#ifdef CONFIG_NUMA
	/**/
	int node;
	/*
	 * zone reclaim becomes active if more unmapped pages exist.
	 */
	/**/
	unsigned long		min_unmapped_pages;
	/**/
	unsigned long		min_slab_pages;
	struct per_cpu_pageset	*pageset[NR_CPUS];
#else
	/*pageset是一个数组，用于实现每个cpu的冷热页帧列表。内核使用这些列表来保存可用于满
	足实现的新鲜页，但冷热页帧对应的高速缓存状态不同：有些也可能仍然在高速缓存中，因此
	可以快速访问，故称之为热的，为缓存的页帧与此相对，称之为冷的*/
	struct per_cpu_pageset	pageset[NR_CPUS];
#endif
	/*不同长度的空闲区域*/
	/*伙伴系统保护锁*/
	spinlock_t		lock;
#ifdef CONFIG_MEMORY_HOTPLUG
	/* see spanned/present_pages for more description */
	seqlock_t		span_seqlock;
#endif
	/*处理内存域中空闲页的伙伴系统，每个数组元素都表示某种固定长度的一些连续内存区，对
	于包含在每个区域中的空闲内存页的管理，free_area是一个起点*/
	struct free_area	free_area[MAX_ORDER];

#ifndef CONFIG_SPARSEMEM
	/*pageblock_nr_pages块标识，可查看pageblock_flags.h，在SPARSEMEM，这个位图存储在
	strct mem_section段中*/
	unsigned long		*pageblock_flags;
#endif /* CONFIG_SPARSEMEM */
	/*填充字段，对齐到高速缓存行*/
	ZONE_PADDING(_pad1_)

	/*通常由页面回收扫描程序访问的字段。该部分涉及的结构成员，用来根据活动情况对内存域
	中使用的页进行编目。如果页访问频繁，则内核认为它是活动的，而不活动的页则显然相反。
	在需要换出页时，这种区别是很重要的。如果可能的话，频繁使用的页应该保持不动，而多余
	的不活动的页则可以换出而没什么损害*/
	/*保护活动和不活动lru链表上的页*/
	spinlock_t		lru_lock;
	/*活动页集合*/
	struct list_head	active_list;
	/*不活动页集合*/
	struct list_head	inactive_list;
	/*指定在回收内存时需要扫描的活动页数目*/
	unsigned long		nr_scan_active;
	/*回收内存时需要扫描的不活动页的数目*/
	unsigned long		nr_scan_inactive;
	/*指定了上次换出一页以来，有多少页未能成功扫描*/
	unsigned long		pages_scanned;
	/*内存域当前状态*/
	unsigned long		flags;
	/*内存域统计数据*/
	/*维护当量有关该内存域的统计信息*/
	atomic_long_t		vm_stat[NR_VM_ZONE_STAT_ITEMS];

	/*
	 * prev_priority holds the scanning priority for this zone.  It is
	 * defined as the scanning priority at which we achieved our reclaim
	 * target at the previous try_to_free_pages() or balance_pgdat()
	 * invokation.
	 *
	 * We use prev_priority as a measure of how much stress page reclaim is
	 * under - it drives the swappiness decision: whether to unmap mapped
	 * pages.
	 *
	 * Access to both this field is quite racy even on uniprocessor.  But
	 * it is expected to average out OK.
	 */
	/*存储了上一次扫描操作扫描该内存域的优先级*/
	int prev_priority;

	/*对齐高速缓存行的填充字节*/
	ZONE_PADDING(_pad2_)
	/*很少使用或大多数情况下只读的字段*/

	/*
	 * wait_table		-- the array holding the hash table
	 * wait_table_hash_nr_entries	-- the size of the hash table array
	 * wait_table_bits	-- wait_table_size == (1 << wait_table_bits)
	 *
	 * The purpose of all these is to keep track of the people
	 * waiting for a page to become available and make them
	 * runnable again when possible. The trouble is that this
	 * consumes a lot of space, especially when so few things
	 * wait on pages at a given time. So instead of using
	 * per-page waitqueues, we use a waitqueue hash table.
	 *
	 * The bucket discipline is to sleep on the same queue when
	 * colliding and wake all in that wait queue when removing.
	 * When something wakes, it must check to be sure its page is
	 * truly available, a la thundering herd. The cost of a
	 * collision is great, but given the expected load of the
	 * table, they should be so rare as to be outweighed by the
	 * benefits from the saved space.
	 *
	 * __wait_on_page_locked() and unlock_page() in mm/filemap.c, are the
	 * primary users of these fields, and in mm/page_alloc.c
	 * free_area_init_core() performs the initialization of them.
	 */
	/*以下三个变量实现了一个等待队列，可供等待某一页变为可用的进程使用。进程排成一个
	队列，等待某些条件，在条件为真时，内核会通知进程恢复工作*/
	/**/
	wait_queue_head_t	*wait_table;
	/**/
	unsigned long		wait_table_hash_nr_entries;
	/**/
	unsigned long		wait_table_bits;

	/*支持不连续内存模型的字段*/
	/*内存域所属的结点实例的指针*/
	struct pglist_data	*zone_pgdat;
	/* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
	/*内存域第一个页帧的索引*/
	unsigned long		zone_start_pfn;

	/*
	 * zone_start_pfn, spanned_pages and present_pages are all
	 * protected by span_seqlock.  It is a seqlock because it has
	 * to be read outside of zone->lock, and it is done in the main
	 * allocator path.  But, it is written quite infrequently.
	 *
	 * The lock is declared along with zone->lock because it is
	 * frequently read in proximity to zone->lock.  It's good to
	 * give them a chance of being in the same cacheline.
	 */
	/*包含空洞的页数目*/
	unsigned long		spanned_pages;
	/*不含空洞实际可用的页数目*/
	unsigned long		present_pages;

	/*该内存域的名称，如Normal、DMA、HighMen。很少使用*/
	const char		*name;
} ____cacheline_internodealigned_in_smp;

/*内存域处于正常状态时也可能没有设置这些标识。ZONE_ALL_UNRECLAIMABLE状态出现在内核试
图重用该内存域的一些页时（页面回收），但因为所有的页都被钉住而无法回收。例如，用户空
间使用mlock系统调用通知内核页不能从物理内存溢出，比如换出到磁盘上。这样的页称之为钉住
的，如果一个内存域中的所有页都被钉住，那么该内存域是无法回收的，即设置该标识。为了不
浪费时间，交换守护进程在寻找可供回收的页是，只会简要地扫描一下此类内存域。但扫描时无法
完全省去的，因为该内存域经过若干时间后，在将来可能再次包含可回收的页。倘若如此，则消除
该标识，而kswapd守护进程将该内存域与其它内存域同等对待。在SMP系统上，多个cpu可能试图并
发地回收一个内存域。ZONE_RECLAIM_LOCKED标识防止这种情形：如果一个cpu在回收某个内存域，
则设置该标识，这防止其它cpu的尝试。ZONE_OOM_LOCKED专用于某种不走运的情况：如果进程消耗
了大量内存，致使必要的操作都无法完成，那么内核会试图杀死消耗内存最多的进程，以获得更多
的空闲页。该标识可以防止多个cpu同时进行这种操作*/
typedef enum
{
	/*所有页都被钉住而无法回收*/
	ZONE_ALL_UNRECLAIMABLE,
	/*防止并发回收*/
	ZONE_RECLAIM_LOCKED,
	/*内存域即可被回收*/
	ZONE_OOM_LOCKED,
} zone_flags_t;

/*将指定内存域设置为特定状态*/
static inline void zone_set_flag(struct zone *zone, zone_flags_t flag)
{
	set_bit(flag, &zone->flags);
}
/*测试并设置指定内存域的特定状态，然后返回该内存域之前的状态*/
static inline int zone_test_and_set_flag(struct zone *zone, zone_flags_t flag)
{
	return test_and_set_bit(flag, &zone->flags);
}
/*清除指定内存域特定标识*/
static inline void zone_clear_flag(struct zone *zone, zone_flags_t flag)
{
	clear_bit(flag, &zone->flags);
}
/*测试指定内存域是否处于不可回收状态*/
static inline int zone_is_all_unreclaimable(const struct zone *zone)
{
	return test_bit(ZONE_ALL_UNRECLAIMABLE, &zone->flags);
}
/*测试指定内存域是否处于回收锁定状态*/
static inline int zone_is_reclaim_locked(const struct zone *zone)
{
	return test_bit(ZONE_RECLAIM_LOCKED, &zone->flags);
}
/*测试指定内存域是否处于OOM锁定状态*/
static inline int zone_is_oom_locked(const struct zone *zone)
{
	return test_bit(ZONE_OOM_LOCKED, &zone->flags);
}

/*
 * The "priority" of VM scanning is how much of the queues we will scan in one
 * go. A value of 12 for DEF_PRIORITY implies that we will scan 1/4096th of the
 * queues ("queue_length >> 12") during an aging round.
 */
#define DEF_PRIORITY 12
/*一个备用分配域列表上最大的分配域数目*/
#define MAX_ZONES_PER_ZONELIST (MAX_NUMNODES * MAX_NR_ZONES)

#ifdef CONFIG_NUMA

/*
 * The NUMA zonelists are doubled becausse we need zonelists that restrict the
 * allocations to a single node for GFP_THISNODE.
 * [0 .. MAX_NR_ZONES -1] 		: Zonelists with fallback
 * [MAZ_NR_ZONES ... MAZ_ZONELISTS -1]  : No fallback (GFP_THISNODE)
 */
/**/
#define MAX_ZONELISTS (2 * MAX_NR_ZONES)


/*
 * We cache key information from each zonelist for smaller cache
 * footprint when scanning for free pages in get_page_from_freelist().
 *
 * 1) The BITMAP fullzones tracks which zones in a zonelist have come
 *    up short of free memory since the last time (last_fullzone_zap)
 *    we zero'd fullzones.
 * 2) The array z_to_n[] maps each zone in the zonelist to its node
 *    id, so that we can efficiently evaluate whether that node is
 *    set in the current tasks mems_allowed.
 *
 * Both fullzones and z_to_n[] are one-to-one with the zonelist,
 * indexed by a zones offset in the zonelist zones[] array.
 *
 * The get_page_from_freelist() routine does two scans.  During the
 * first scan, we skip zones whose corresponding bit in 'fullzones'
 * is set or whose corresponding node in current->mems_allowed (which
 * comes from cpusets) is not set.  During the second scan, we bypass
 * this zonelist_cache, to ensure we look methodically at each zone.
 *
 * Once per second, we zero out (zap) fullzones, forcing us to
 * reconsider nodes that might have regained more free memory.
 * The field last_full_zap is the time we last zapped fullzones.
 *
 * This mechanism reduces the amount of time we waste repeatedly
 * reexaming zones for free memory when they just came up low on
 * memory momentarilly ago.
 *
 * The zonelist_cache struct members logically belong in struct
 * zonelist.  However, the mempolicy zonelists constructed for
 * MPOL_BIND are intentionally variable length (and usually much
 * shorter).  A general purpose mechanism for handling structs with
 * multiple variable length members is more mechanism than we want
 * here.  We resort to some special case hackery instead.
 *
 * The MPOL_BIND zonelists don't need this zonelist_cache (in good
 * part because they are shorter), so we put the fixed length stuff
 * at the front of the zonelist struct, ending in a variable length
 * zones[], as is needed by MPOL_BIND.
 *
 * Then we put the optional zonelist cache on the end of the zonelist
 * struct.  This optional stuff is found by a 'zlcache_ptr' pointer in
 * the fixed length portion at the front of the struct.  This pointer
 * both enables us to find the zonelist cache, and in the case of
 * MPOL_BIND zonelists, (which will just set the zlcache_ptr to NULL)
 * to know that the zonelist cache is not there.
 *
 * The end result is that struct zonelists come in two flavors:
 *  1) The full, fixed length version, shown below, and
 *  2) The custom zonelists for MPOL_BIND.
 * The custom MPOL_BIND zonelists have a NULL zlcache_ptr and no zlcache.
 *
 * Even though there may be multiple CPU cores on a node modifying
 * fullzones or last_full_zap in the same zonelist_cache at the same
 * time, we don't lock it.  This is just hint data - if it is wrong now
 * and then, the allocator will still function, perhaps a bit slower.
 */


struct zonelist_cache
{
	unsigned short z_to_n[MAX_ZONES_PER_ZONELIST];		/* zone->nid */
	DECLARE_BITMAP(fullzones, MAX_ZONES_PER_ZONELIST);	/* zone full? */
	unsigned long last_full_zap;		/* when last zap'd (jiffies) */
};
#else
#define MAX_ZONELISTS MAX_NR_ZONES
struct zonelist_cache;
#endif

/*
 * One allocation request operates on a zonelist. A zonelist
 * is a list of zones, the first one is the 'goal' of the
 * allocation, the other zones are fallback zones, in decreasing
 * priority.
 *
 * If zlcache_ptr is not NULL, then it is just the address of zlcache,
 * as explained above.  If zlcache_ptr is NULL, there is no zlcache.
 */
/*备用内存域信息*/
struct zonelist
{
	/*NULL or &zlcache*/
	struct zonelist_cache *zlcache_ptr;
	/*备用内存域指针数组，最后多出来的是NULL分隔符*/
	struct zone *zones[MAX_ZONES_PER_ZONELIST + 1];
#ifdef CONFIG_NUMA
	struct zonelist_cache zlcache;			     // optional ...
#endif
};

#ifdef CONFIG_NUMA
/*
 * Only custom zonelists like MPOL_BIND need to be filtered as part of
 * policies. As described in the comment for struct zonelist_cache, these
 * zonelists will not have a zlcache so zlcache_ptr will not be set. Use
 * that to determine if the zonelists needs to be filtered or not.
 */
static inline int alloc_should_filter_zonelist(struct zonelist *zonelist)
{
	return !zonelist->zlcache_ptr;
}
#else
static inline int alloc_should_filter_zonelist(struct zonelist *zonelist)
{
	return 0;
}
#endif /* CONFIG_NUMA */

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP

/*结点活动区域*/
struct node_active_region
{
	/*活动区域起始页帧号*/
	unsigned long start_pfn;
	/*活动区域结束页帧号*/
	unsigned long end_pfn;
	/*结点编号*/
	int nid;
};
#endif /* CONFIG_ARCH_POPULATES_NODE_MAP */

#ifndef CONFIG_DISCONTIGMEM
/* The array of struct pages - for discontigmem use pgdat->lmem_map */
extern struct page *mem_map;
#endif

/*
 * The pg_data_t structure is used in machines with CONFIG_DISCONTIGMEM
 * (mostly NUMA machines?) to denote a higher-level memory zone than the
 * zone denotes.
 *
 * On NUMA machines, each NUMA node would have a pg_data_t to describe
 * it's memory layout.
 *
 * Memory statistics and page replacement data structures are maintained on a
 * per-zone basis.
 */
struct bootmem_data;
/*结点数据结构*/
typedef struct pglist_data
{
	/*数组项包含了结点中各内存域的数据结构*/
	struct zone node_zones[MAX_NR_ZONES];
	/*指定了备用结点及其内存域的列表，以便在当前结点雷雨可用空间时，在备用结点分配内存
	。出于性能考虑，在为进程分配内存时，内核总是试图在当前运行的cpu相关联的NUMA结点上
	进行分配，但这并不总是可行的，例如，该结点的内存可能已经用尽，对此类情况，每个结点
	都提供了一个备用列表。该列表包了其它结点（和相关的内存域），可用于代替当前结点分配
	内存。列表项位置越靠后，就越不适合分配*/
	struct zonelist node_zonelists[MAX_ZONELISTS];
	/*保存结点中不同内存域的数目*/
	int nr_zones;
#ifdef CONFIG_FLAT_NODE_MEM_MAP
	/*指向page实例数组的指针，用于描述结点的所有物理内存页，它包含了结点中所有内存域的页*/
	struct page *node_mem_map;
#endif
	/*在系统启动期间，内存管理子系统初始化之前，内核也需要使用内存（另外，还必须保留部分
	内存用于初始化内存管理子系统）。该自举内存分配器（boot memory allocator）实例指针指
	向解决这个问题的实例*/
	struct bootmem_data *bdata;
#ifdef CONFIG_MEMORY_HOTPLUG
	/*
	 * Must be held any time you expect node_start_pfn, node_present_pages
	 * or node_spanned_pages stay constant.  Holding this will also
	 * guarantee that any pfn_valid() stays that way.
	 *
	 * Nests above zone->lock and zone->size_seqlock.
	 */
	spinlock_t node_size_lock;
#endif
	/*是该NUMA结点第一个页帧的逻辑编号。系统中所有结点的页帧是依次编号的，每个页帧的
	号码都是全局唯一的（不只是结点内唯一）。该值在UMA系统中总是0，因为只有一个结点，
	因此其第一个页帧号总是0*/
	unsigned long node_start_pfn;
	/*结点中可用页帧数目*/
	unsigned long node_present_pages;
	/*结点以页帧为单位的数目（包含空洞数目）*/
	unsigned long node_spanned_pages;
	/*全局结点ID，系统中的NUMA结点都从0开始编号*/
	int node_id;
	/*交换守护进程的等待队列，在将页帧换出结点时会用到*/
	wait_queue_head_t kswapd_wait;
	/*指向负责该结点的交换守护进程（swap daemon）的task_strcut实例*/
	struct task_struct *kswapd;
	/*用于页交换子系统的实现，用来定义需要释放的区域的长度*/
	int kswapd_max_order;
} pg_data_t;

/*获取结点中可用页帧的数目*/
#define node_present_pages(nid)	(NODE_DATA(nid)->node_present_pages)
/*获取结点中包含空洞的页帧数目*/
#define node_spanned_pages(nid)	(NODE_DATA(nid)->node_spanned_pages)
#ifdef CONFIG_FLAT_NODE_MEM_MAP
/*根据结点编号及页帧编号，获取对应该结点上的struct page实例*/
#define pgdat_page_nr(pgdat, pagenr)	((pgdat)->node_mem_map + (pagenr))
#else
#define pgdat_page_nr(pgdat, pagenr)	pfn_to_page((pgdat)->node_start_pfn + (pagenr))
#endif
/*获取指定结点上指定页号的页实例*/
#define nid_page_nr(nid, pagenr) 	pgdat_page_nr(NODE_DATA(nid),(pagenr))

#include <linux/memory_hotplug.h>

void get_zone_counts(unsigned long *active, unsigned long *inactive, unsigned long *free);
void build_all_zonelists(void);
void wakeup_kswapd(struct zone *zone, int order);
int zone_watermark_ok(struct zone *z, int order, unsigned long mark, int classzone_idx, int alloc_flags);
enum memmap_context
{
	MEMMAP_EARLY,
	MEMMAP_HOTPLUG,
};
extern int init_currently_empty_zone(struct zone *zone, unsigned long start_pfn,
				     unsigned long size, enum memmap_context context);

#ifdef CONFIG_HAVE_MEMORY_PRESENT
void memory_present(int nid, unsigned long start, unsigned long end);
#else
static inline void memory_present(int nid, unsigned long start, unsigned long end) {}
#endif

#ifdef CONFIG_NEED_NODE_MEMMAP_SIZE
unsigned long __init node_memmap_size_bytes(int, unsigned long, unsigned long);
#endif

/*获取内存域在所属结点的内存域中的索引，ZONE_DMA时返回0，ZONE_NORMAL返回1，等等*/
#define zone_idx(zone)		((zone) - (zone)->zone_pgdat->node_zones)

/*测试内存域中是否有有效页帧*/
static inline int populated_zone(struct zone *zone)
{
	/*两次取反操作后的返回值只能是1或0*/
	return (!!zone->present_pages);
}

extern int movable_zone;

/*虚拟内存域是否是高端内存域*/
static inline int zone_movable_is_highmem(void)
{
#if defined(CONFIG_HIGHMEM) && defined(CONFIG_ARCH_POPULATES_NODE_MAP)
	return movable_zone == ZONE_HIGHMEM;
#else
	return 0;
#endif
}

/*测试指定内存域是否是ZONE_HIGHMEM或ZONE_MOVABLE*/
static inline int is_highmem_idx(enum zone_type idx)
{
#ifdef CONFIG_HIGHMEM
	return (idx == ZONE_HIGHMEM || (idx == ZONE_MOVABLE && zone_movable_is_highmem()));
#else
	return 0;
#endif
}

/*测试指定内存域是否是ZONE_NORMAL*/
static inline int is_normal_idx(enum zone_type idx)
{
	return (idx == ZONE_NORMAL);
}

/**
 * is_highmem - helper function to quickly check if a struct zone is a
 *              highmem zone or not.  This is an attempt to keep references
 *              to ZONE_{DMA/NORMAL/HIGHMEM/etc} in general code to a minimum.
 * @zone - pointer to struct zone variable
 */
/*测试指定内存域是否是高端内存域。只有在启用高端内存域并且该内存域编号是该结点所属的
高端域或者该高端内存域中的虚拟内存域时，才返回true*/
static inline int is_highmem(struct zone *zone)
{
#ifdef CONFIG_HIGHMEM
	int zone_idx = zone - zone->zone_pgdat->node_zones;
	return zone_idx == ZONE_HIGHMEM || (zone_idx == ZONE_MOVABLE && zone_movable_is_highmem());
#else
	return 0;
#endif
}
/*测试指定内存域是否是普通内存域*/
static inline int is_normal(struct zone *zone)
{
	return zone == zone->zone_pgdat->node_zones + ZONE_NORMAL;
}
/*测试指定内存域是否是ZONE_DMA32内存域*/
static inline int is_dma32(struct zone *zone)
{
#ifdef CONFIG_ZONE_DMA32
	return zone == zone->zone_pgdat->node_zones + ZONE_DMA32;
#else
	return 0;
#endif
}
/*测试指定内存域是否是ZONE_DMA内存域*/
static inline int is_dma(struct zone *zone)
{
#ifdef CONFIG_ZONE_DMA
	return zone == zone->zone_pgdat->node_zones + ZONE_DMA;
#else
	return 0;
#endif
}

/* These two functions are used to setup the per zone pages min values */
struct ctl_table;
struct file;
int min_free_kbytes_sysctl_handler(struct ctl_table *, int, struct file *,
											void __user *, size_t *, loff_t *);
extern int sysctl_lowmem_reserve_ratio[MAX_NR_ZONES-1];
int lowmem_reserve_ratio_sysctl_handler(struct ctl_table *, int, struct file *,
											void __user *, size_t *, loff_t *);
int percpu_pagelist_fraction_sysctl_handler(struct ctl_table *, int, struct file *,
											void __user *, size_t *, loff_t *);
int sysctl_min_unmapped_ratio_sysctl_handler(struct ctl_table *, int,struct file *,
											void __user *, size_t *, loff_t *);
int sysctl_min_slab_ratio_sysctl_handler(struct ctl_table *, int, struct file *,
											void __user *, size_t *, loff_t *);

extern int numa_zonelist_order_handler(struct ctl_table *, int, struct file *,
											void __user *, size_t *, loff_t *);
extern char numa_zonelist_order[];
/*缓冲字符串长度*/
#define NUMA_ZONELIST_ORDER_LEN			16

#include <linux/topology.h>
/*获取当前进程运行的cpu对应的结点编号*/
#ifndef numa_node_id
#define numa_node_id()		(cpu_to_node(raw_smp_processor_id()))
#endif

#ifndef CONFIG_NEED_MULTIPLE_NODES

extern struct pglist_data contig_page_data;
/*所有平台上都实现了特定于体系结构的NODE_DATA宏，用于通过编号，来查询与一个NUMA结点
相关的pgdata_t实例，该宏有一个形式参数用于选择NUMA结点，但在UMA系统上只有一个伪结点
，因此，总是返回同样的数据*/
#define NODE_DATA(nid)		(&contig_page_data)
/*获取结点的所有物理内存页*/
#define NODE_MEM_MAP(nid)	mem_map
/**/
#define MAX_NODES_SHIFT		1

#else /* CONFIG_NEED_MULTIPLE_NODES */

#include <asm/mmzone.h>

#endif /* !CONFIG_NEED_MULTIPLE_NODES */

extern struct pglist_data *first_online_pgdat(void);
extern struct pglist_data *next_online_pgdat(struct pglist_data *pgdat);
extern struct zone *next_zone(struct zone *zone);


/*遍历系统中所有在线结点*/
#define for_each_online_pgdat(pgdat)\
	for (pgdat = first_online_pgdat(); pgdat; pgdat = next_online_pgdat(pgdat))

/*遍历系统中所有结点的所有内存域*/
#define for_each_zone(zone)\
	for (zone = (first_online_pgdat())->node_zones; zone;	zone = next_zone(zone))

#ifdef CONFIG_SPARSEMEM
#include <asm/sparsemem.h>
#endif

#if BITS_PER_LONG == 32
/*32位page->flags，为结点和内存域信息保留9位，4个内存域（3位），给结点余下6位*/
#define FLAGS_RESERVED		9
#elif BITS_PER_LONG == 64
/*
 * with 64 bit flags field, there's plenty of room.
 */
#define FLAGS_RESERVED		32
#else
#error BITS_PER_LONG not defined
#endif

#if !defined(CONFIG_HAVE_ARCH_EARLY_PFN_TO_NID) && !defined(CONFIG_ARCH_POPULATES_NODE_MAP)
#define early_pfn_to_nid(nid)  (0UL)
#endif

#ifdef CONFIG_FLATMEM
/*UMA只有一个结点*/
#define pfn_to_nid(pfn)		(0)
#endif

#define pfn_to_section_nr(pfn) ((pfn) >> PFN_SECTION_SHIFT)
#define section_nr_to_pfn(sec) ((sec) << PFN_SECTION_SHIFT)

#ifdef CONFIG_SPARSEMEM

/*
 * SECTION_SHIFT    		#bits space required to store a section #
 *
 * PA_SECTION_SHIFT		physical address to/from section number
 * PFN_SECTION_SHIFT		pfn to/from section number
 */
#define SECTIONS_SHIFT		(MAX_PHYSMEM_BITS - SECTION_SIZE_BITS)

#define PA_SECTION_SHIFT	(SECTION_SIZE_BITS)
#define PFN_SECTION_SHIFT	(SECTION_SIZE_BITS - PAGE_SHIFT)

#define NR_MEM_SECTIONS		(1UL << SECTIONS_SHIFT)

#define PAGES_PER_SECTION       (1UL << PFN_SECTION_SHIFT)
#define PAGE_SECTION_MASK	(~(PAGES_PER_SECTION-1))

#define SECTION_BLOCKFLAGS_BITS \
	((1UL << (PFN_SECTION_SHIFT - pageblock_order)) * NR_PAGEBLOCK_BITS)

#if (MAX_ORDER - 1 + PAGE_SHIFT) > SECTION_SIZE_BITS
#error Allocator MAX_ORDER exceeds SECTION_SIZE
#endif

struct page;
struct mem_section {
	/*
	 * This is, logically, a pointer to an array of struct
	 * pages.  However, it is stored with some other magic.
	 * (see sparse.c::sparse_init_one_section())
	 *
	 * Additionally during early boot we encode node id of
	 * the location of the section here to guide allocation.
	 * (see sparse.c::memory_present())
	 *
	 * Making it a UL at least makes someone do a cast
	 * before using it wrong.
	 */
	unsigned long section_mem_map;

	/* See declaration of similar field in struct zone */
	unsigned long *pageblock_flags;
};

#ifdef CONFIG_SPARSEMEM_EXTREME
#define SECTIONS_PER_ROOT       (PAGE_SIZE / sizeof (struct mem_section))
#else
#define SECTIONS_PER_ROOT	1
#endif

#define SECTION_NR_TO_ROOT(sec)	((sec) / SECTIONS_PER_ROOT)
#define NR_SECTION_ROOTS	(NR_MEM_SECTIONS / SECTIONS_PER_ROOT)
#define SECTION_ROOT_MASK	(SECTIONS_PER_ROOT - 1)

#ifdef CONFIG_SPARSEMEM_EXTREME
extern struct mem_section *mem_section[NR_SECTION_ROOTS];
#else
extern struct mem_section mem_section[NR_SECTION_ROOTS][SECTIONS_PER_ROOT];
#endif

static inline struct mem_section *__nr_to_section(unsigned long nr)
{
	if (!mem_section[SECTION_NR_TO_ROOT(nr)])
		return NULL;
	return &mem_section[SECTION_NR_TO_ROOT(nr)][nr & SECTION_ROOT_MASK];
}
extern int __section_nr(struct mem_section* ms);

/*
 * We use the lower bits of the mem_map pointer to store
 * a little bit of information.  There should be at least
 * 3 bits here due to 32-bit alignment.
 */
#define	SECTION_MARKED_PRESENT	(1UL<<0)
#define SECTION_HAS_MEM_MAP	(1UL<<1)
#define SECTION_MAP_LAST_BIT	(1UL<<2)
#define SECTION_MAP_MASK	(~(SECTION_MAP_LAST_BIT-1))
#define SECTION_NID_SHIFT	2

static inline struct page *__section_mem_map_addr(struct mem_section *section)
{
	unsigned long map = section->section_mem_map;
	map &= SECTION_MAP_MASK;
	return (struct page *)map;
}

static inline int present_section(struct mem_section *section)
{
	return (section && (section->section_mem_map & SECTION_MARKED_PRESENT));
}

static inline int present_section_nr(unsigned long nr)
{
	return present_section(__nr_to_section(nr));
}

static inline int valid_section(struct mem_section *section)
{
	return (section && (section->section_mem_map & SECTION_HAS_MEM_MAP));
}

static inline int valid_section_nr(unsigned long nr)
{
	return valid_section(__nr_to_section(nr));
}

static inline struct mem_section *__pfn_to_section(unsigned long pfn)
{
	return __nr_to_section(pfn_to_section_nr(pfn));
}

static inline int pfn_valid(unsigned long pfn)
{
	if (pfn_to_section_nr(pfn) >= NR_MEM_SECTIONS)
		return 0;
	return valid_section(__nr_to_section(pfn_to_section_nr(pfn)));
}

static inline int pfn_present(unsigned long pfn)
{
	if (pfn_to_section_nr(pfn) >= NR_MEM_SECTIONS)
		return 0;
	return present_section(__nr_to_section(pfn_to_section_nr(pfn)));
}

/*
 * These are _only_ used during initialisation, therefore they
 * can use __initdata ...  They could have names to indicate
 * this restriction.
 */
#ifdef CONFIG_NUMA
#define pfn_to_nid(pfn)							\
({									\
	unsigned long __pfn_to_nid_pfn = (pfn);				\
	page_to_nid(pfn_to_page(__pfn_to_nid_pfn));			\
})
#else
#define pfn_to_nid(pfn)		(0)
#endif

#define early_pfn_valid(pfn)	pfn_valid(pfn)
void sparse_init(void);
#else
#define sparse_init()	do {} while (0)
#define sparse_index_init(_sec, _nid)  do {} while (0)
#endif /* CONFIG_SPARSEMEM */

#ifdef CONFIG_NODES_SPAN_OTHER_NODES
#define early_pfn_in_nid(pfn, nid)	(early_pfn_to_nid(pfn) == (nid))
#else
#define early_pfn_in_nid(pfn, nid)	(1)
#endif

#ifndef early_pfn_valid
#define early_pfn_valid(pfn)	(1)
#endif

void memory_present(int nid, unsigned long start, unsigned long end);
unsigned long __init node_memmap_size_bytes(int, unsigned long, unsigned long);

/*
 * If it is possible to have holes within a MAX_ORDER_NR_PAGES, then we
 * need to check pfn validility within that MAX_ORDER_NR_PAGES block.
 * pfn_valid_within() should be used in this case; we optimise this away
 * when we have no holes within a MAX_ORDER_NR_PAGES block.
 */
#ifdef CONFIG_HOLES_IN_ZONE
#define pfn_valid_within(pfn) pfn_valid(pfn)
#else
#define pfn_valid_within(pfn) (1)
#endif

#endif /* !__ASSEMBLY__ */
#endif /* __KERNEL__ */
#endif /* _LINUX_MMZONE_H */
