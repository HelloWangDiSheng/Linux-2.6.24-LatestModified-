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

/*�����ڴ���� �����������*/
#ifndef CONFIG_FORCE_MAX_ZONEORDER
#define MAX_ORDER 11
#else
#define MAX_ORDER CONFIG_FORCE_MAX_ZONEORDER
#endif

/*������׶�Ӧ�ķ�������ҳ��Ŀ*/
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

/*
 * PAGE_ALLOC_COSTLY_ORDER is the order at which allocations are deemed
 * costly to service.  That is between allocation orders which should
 * coelesce naturally under reasonable reclaim pressure and those which
 * will not.
 */
#define PAGE_ALLOC_COSTLY_ORDER 3
/*����ҳ�Ŀ��ƶ�����֯ҳ�Ƿ�ֹ�����ڴ���Ƭ��һ�ֿ��ܷ�������һ����ֹ��������ֶ���
�������ڴ���ZONE_MOVABLE������˼��ܼ򵥣����õ������ڴ滮��Ϊ�����ڴ���һ������
���ƶ����䣬һ�����ڲ����ƶ����䡣ֻ���Զ���ֹ�����ƶ�ҳ����ƶ��ڴ���������Ƭ*/

/*ҳ��Ŀ��ƶ��ԣ�������ҳ����3��������һ�֡��ں�ʹ�õķ���Ƭ�����������ڽ�������
ͬ���ƶ��Ե�ҳ�����˼��*/
/*�����ƶ�ҳ�����ڴ����й̶�λ�ã������ƶ��������ط��������ں˷���Ĵ�����ڴ����ڸ�
����*/
#define MIGRATE_UNMOVABLE		    0
/*�ɻ���ҳ������ֱ���ƶ���������ɾ���������ݿ��ô�ĳЩԴ�������ɡ����磬ӳ�����ļ���
�������ڸ����kswap�ػ����̻���ݿɻ���ҳ���ʵ�Ƶ���̶ȣ��������ͷŴ��ڴ档�ں˻���
�ɻ���ҳռ��̫���ڴ�ʱ���л��ա����⣬���ڴ��ȱ��������ʧ�ܣ�ʱҲ���Է���ҳ�����*/
#define MIGRATE_RECLAIMABLE   		1
/*���ƶ�ҳ���������ƶ��������û��ռ�Ӧ�ó����ҳ���ڸ��������ʱͨ��ҳ��ӳ��ġ���
�����Ǹ��Ƶ���λ�ã�ҳ���������Ӧ�ظ��£�Ӧ�ó��򲻻�ע�⵽�κ���*/
#define MIGRATE_MOVABLE       		2
/*���������ض����ƶ��Ե��б���������ڴ�ʧ�ܣ�����������¿��Դ�MIGRATE_RESERVE����
�ڴ棨��Ӧ���б����ڴ���ϵͳ��ʼ���ڼ���setup_zone_migrate_reserve��䣩*/
#define MIGRATE_RESERVE       		3
/*����������ڴ��������ڿ�ԽNUMA����ƶ������ڴ�ҳ���ڴ���ϵͳ�ϣ��������ڽ������ڴ�
ҳ�ƶ����ӽ���ʹ�ø�ҳ��Ƶ����cpu*/
#define MIGRATE_ISOLATE       		4 /* can't allocate from here */
/*ҳ��Ǩ�����͵���Ŀ����������������*/
#define MIGRATE_TYPES         		5

/*�������з���׵�����Ǩ������*/
#define for_each_migratetype_order(order, type) 				\
	for (order = 0; order < MAX_ORDER; order++) 				\
		for (type = 0; type < MIGRATE_TYPES; type++)

extern int page_group_by_mobility_disabled;

/*���ҳ��Ǩ������*/
static inline int get_pageblock_migratetype(struct page *page)
{
	/*���������ҳ��Ǩ�����ԣ�������ҳ���ǲ����ƶ���*/
	if (unlikely(page_group_by_mobility_disabled))
		return MIGRATE_UNMOVABLE;

	return get_pageblock_flags_group(page, PB_migrate, PB_migrate_end);
}

/*�Ի��ϵͳ���ݽṹ����Ҫ�������ǽ������б�ֽ�ΪMIGRATE_TYPE���б�*/
struct free_area
{
	/*ÿ��Ǩ�����Ͷ���Ӧ��һ�������б�*/
	struct list_head	free_list[MIGRATE_TYPES];
	/*ͳ���������б����ҳ����Ŀ*/
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
/*�������*/
struct zone_padding
{
	/*sizeof(struct zone_padding)Ϊ0�����ṹһ�����ĩβ��������*/
	char x[0];
} ____cacheline_internodealigned_in_smp;
#define ZONE_PADDING(name)	struct zone_padding name;
#else
#define ZONE_PADDING(name)
#endif

/*ϵͳ�ڴ�ҳ��״̬*/
enum zone_stat_item
{
	/*��һ��128�ֽڻ����У��ٶ��ֳ�ʱ64����λ��*/
	/*����ҳ����Ŀ*/
	NR_FREE_PAGES,
	/*����lru�����ϵ�ҳ��Ŀ*/
	NR_INACTIVE,
	/*�lru�����ϵ�ҳ��Ŀ*/
	NR_ACTIVE,
	/*��ӳ�������ҳ��Ŀ*/
	NR_ANON_PAGES,
	/*��ҳ�����ӳ���ҳ����Ŀ��ֻ��������ļ���ҳ��ֱ�ӵ��ں�ӳ�䲻�������ڣ�ֻ��
	�������������޸�*/
	NR_FILE_MAPPED,
	/**/
	NR_FILE_PAGES,
	/*�����ļ�����ҳ��Ŀ*/
	NR_FILE_DIRTY,
	/*��ǰ���ڻ�д��ҳ����Ŀ*/
	NR_WRITEBACK,
	/* Second 128 byte cacheline */
	/**/
	NR_SLAB_RECLAIMABLE,
	/**/
	NR_SLAB_UNRECLAIMABLE,
	/*���ڴ��ҳ���ҳ����Ŀ*/
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
	/*��¼����б���ص�ҳ��Ŀ*/
	int count;
	/*ˮӡֵ�����count>high��������б��е�ҳ̫���ˣ���Ҫ��գ����������͵�״̬û��
	��ʽʹ��ˮӡ������б���û�г�Ա�����������*/
	int high;
	/*���п��ܣ�CPU�ĸ��ٻ��治���õ���ҳ������ɾ���ģ������ö��ҳ��ɵĿ顣batchʱ
	ÿ����ӻ�ɾ��ҳ����һ���ο�ֵ*/
	int batch;
	/*�����˵�ǰcpu����ҳ����ҳ����ʹ���ں˵ı�׼��������*/
	struct list_head list;
};

/*ʵ�����ȷ�������hot-n-cold allocator�����ں�˵ҳ���ȵģ���ζ��ҳ�Ѿ����ص�cpu����
�����У������ڴ��е�ҳ��ȣ��������ܹ�����ķ��ʡ��෴����ҳ���ڸ��ٻ����С��ڶദ
����ϵͳ�ϣ�ÿ��cpu����һ���������ٻ��棬����cpu�Ĺ�������Ƕ����ġ������ڴ������
����һ���ض���NUMA��㣬���������ĳ���ض���cpu��������cpu���ٻ�����Ȼ���԰������ڴ�
����ҳ�����յ�Ч��ʱ��ÿ�������������Է���ϵͳ�����е�ҳ�������ٶȲ�ͬ����ˣ��ض���
�ڴ�������ݽṹ������Ҫ���ǵ�����NUMA�����ص�cpu���������չ˵�ϵͳ��������cpu*/
struct per_cpu_pageset
{
	/*����0��Ӧ��ҳ������1��Ӧ��ҳ*/
	struct per_cpu_pages pcp[2];
#ifdef CONFIG_NUMA
	/**/
	s8 expire;
#endif
#ifdef CONFIG_SMP
	/**/
	s8 stat_threshold;
	/*�����ڴ�����ҳ��ͳ����Ϣ*/
	s8 vm_stat_diff[NR_VM_ZONE_STAT_ITEMS];
#endif
} ____cacheline_aligned_in_smp;

/*��ȡָ���ڴ������ض���per-CPU���ٻ���*/
#ifdef CONFIG_NUMA
/**/
#define zone_pcp(__z, __cpu) ((__z)->pageset[(__cpu)])
#else
#define zone_pcp(__z, __cpu) (&(__z)->pageset[(__cpu)])
#endif

/*�ڴ�������*/
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
	/*��ʶ�ʺ�DMA���ڴ��򡣸�����ĳ��������ڴ��������͡���IA-32������ϣ�һ�������
	��16M�������ɹ��ϵ�ISA�豸ǿ�ӵı߽磬�����ִ��ļ����Ҳ���ܽ�����һ���Ƶ�Ӱ��*/
	ZONE_DMA,
#endif
#ifdef CONFIG_ZONE_DMA32
	/*
	 * x86_64 needs two ZONE_DMAs because it supports devices that are
	 * only able to do DMA to the lower 16M but also 32 bit devices that
	 * can only do DMA areas below 4G.
	 */
	/*�����ʹ��32λ��ַ�ֿ�Ѱַ���ʺ�DMA���ڴ�����Ȼ��ֻ����64λϵͳ�ϣ�����DMA
	�ڴ�����в����32λ������ϣ�������ʱ�յġ���Alpha��AMD64ϵͳ�ϣ�������ĳ���
	���ܴ�0��4G*/
	ZONE_DMA32,
#endif
	/*
	 * Normal addressable memory is in ZONE_NORMAL. DMA operations can be
	 * performed on pages in ZONE_NORMAL if the DMA devices support
	 * transfers to all addressable memory.
	 */
	/*����˿�ֱ��ӳ�䵽�ں˶ε���ͨ�ڴ���������������ϵ�ṹ�ϱ�֤������ڵ�Ψһ�ڴ�
	�򣬵��޷���֤�õ�ַ��Χ��Ӧ��ʵ�ʵ������ڴ档���磬�����AMD64ϵͳ��2G�ڴ棬��ô
	�����ڴ涼����ZONE_DMA32��Χ��erZONE_NORMAL��Ϊ��*/
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
	/*�����ں˶ε������ڴ档���ݱ���ʱ�����ã��������迼��ĳЩ�ڴ���������64λϵͳ
	�У�������Ҫ���Ǹ߶��ڴ������֧����ֻ�ܷ���4GB�����ڴ��32λ���裬����ҪDMA32
	�ڴ�����IA-32ϵͳ�ϣ�����ֱ�ӹ���������ڴ��������ᳬ��896M��������ֵ��ֱ�����
	4GΪֹ�����ڴ�ֻ��ͨ���߶��ڴ�Ѱַ*/
	ZONE_HIGHMEM,
#endif
	/*α�ڴ����ڷ�ֹ�����ڴ���Ƭ�Ļ�������Ҫʹ�ø��ڴ��򣬸����Ա����й���Ա��ʾ����
	�������������κ�Ӳ������������ڴ淶Χ�����ڴ����ҳ�����Ը߶��ڴ������ͨ�ڴ���
	�������е�ҳȫ�����ǿ���Ǩ�Ƶģ���Ҫ��Ϊ�˷�ֹ�ڴ���Ƭ��֧���ڴ���Ȳ��*/
	ZONE_MOVABLE,
	/*�ڴ�����Ŀ������ǣ����ں���Ҫ����ϵͳ�������ڴ���ʱ�����õ��ó���*/
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
/*��ڴ�����Ŀͳ�ơ�ʹ��__ZONE_COUNT������Ҫ��ƫ��λ����IA-32 4G�ڴ�Ϊ�����
�ڴ�����ĿΪ__ZONE_COUNT = 4��ZONES_SHIFT = 2����ƫ��λ��Ϊ�˺�page->flags��ʶ��
��ͬ�ڴ�ģ���л��е��ڴ�Ρ���㡢�ڴ�����ϣ�����ҳ����*/
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

/*�ں�ʹ�øýṹ�������ڴ��򡣸ýṹ�Ƚ�����ķ���ʱ����ZONE_PADDING�ָ�Ϊ�������֣���
����Ϊ��zone�ṹ�ķ��ʷǳ�Ƶ�����ڶദ����ϵͳ�ϣ�ͨ�����в�ͬ��cpu��ͼͬʱ���ʽṹ��Ա
����ˣ�ʹ������ֹ���Ǳ˴˸��ţ��������Ͳ�һ�¡������ں˶Ըýṹ�ķ��ʷǳ�Ƶ�������
�ᾭ���Եػ�ȡ�ýṹ������������zone->lock��zone->lru_lock��������ݱ����ڸ��ٻ�������
����ô��������������١����ٻ����Ϊ�У�ÿһ�и���ͬ���ڴ������ں�ʹ��ZONE_PADDING��
���ɡ���䡰�ֶ���ӵ��ṹ�У���ȷ��ÿ������������������Ļ������У���ʹ���˱������ؼ���
__cacheline_maxaligned_in_smp������ʵ�����ŵĸ��ٻ�����뷽ʽ���ýṹ�������������Ҳͨ
������ֶα˴˷ָ����������߶�����������ҪĿ���ǽ����ݱ�����һ���������У����ڿ��ٷ���
���Ӷ�������ڴ�������ݣ���cpu���ٻ�������ȣ��ڴ�Ƚ����������������ɸýṹ��������
���Ժ��Եģ��ر������ں��ڴ���zone�ṹ��ʵ������ʹ��*/
struct zone
{
	/*ͨ����ҳ���������ʵ��ֶΣ�������ҳ����ʱʹ�õġ�ˮӡ��������ڴ治�㣬�ں˿��Խ�ҳ
	д��Ӳ�̣���������Ա��Ӱ�쵽�����ػ����̵���Ϊ*/
	/*�������ҳ��Ŀ����pages_min����ôҳ���չ�����ѹ���ͱȽϴ���Ϊ�ڴ����м������ҳ*/
	unsigned long pages_min;
	/*�������ҳ����Ŀ����pages_low�����ں˿�ʼ��ҳ������Ӳ��*/
	unsigned long pages_low;
	/*�������ҳ����pages_high�����ڴ����״̬ʱ�����*/
	unsigned long pages_high;
	/*
	 * We don't know if the memory that we're going to allocate will be freeable
	 * or/and it will be released eventually, so to avoid totally wasting several
	 * GB of ram we must reserve some of the lower zone memory (otherwise we risk
	 * to run OOM on the lower zones despite there's tons of freeable ram
	 * on the higher zones). This array is recalculated at runtime if the
	 * sysctl_lowmem_reserve_ratio sysctl changes.
	 */
	/*lowmem_reserve����ֱ�Ϊ�����ڴ���ָ��������ҳ������һЩ������ζ�����ʧ�ܵĹؼ�
	���ڴ���䣬�����ڴ���ķݶ������Ҫ��ȷ����
		 �����������ں˷����ڴ�ʱ�������ʱ�ڴ�Ƚϳ�ԣ����ô���̵�����ᱻ�������㣬��
	����ʱ�ڴ��Ѿ��ȽϽ��ţ��ں˾���Ҫ��һ���ֲ�����ʹ�õ��ڴ���л��գ��Ӷ��ڳ�һ����
	�ڴ�������̵��ڴ�������������������ڴ�Ĺ����У����̻�һֱ�����ȴ�����һ����
	����䳡���������ǲ����������ģ��ڴ���������������ϵõ����㣬����ִ���жϴ����
	�����ִ�г������������ٽ����ڵĴ���ʱ�����̾Ͳ�����˯�ߣ���Ϊ�жϳ����޷������µ�
	�ȡ���ʱ����Ҫ�ں���ǰΪ��Щ���Ĳ���Ԥ��һ�����ڴ棬���ڴ����ʱ������ʹ���ⲿ��Ԥ
	�����ڴ����Щ��������
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
	/*pageset��һ�����飬����ʵ��ÿ��cpu������ҳ֡�б��ں�ʹ����Щ�б��������������
	��ʵ�ֵ�����ҳ��������ҳ֡��Ӧ�ĸ��ٻ���״̬��ͬ����ЩҲ������Ȼ�ڸ��ٻ����У����
	���Կ��ٷ��ʣ��ʳ�֮Ϊ�ȵģ�Ϊ�����ҳ֡�����ԣ���֮Ϊ���*/
	struct per_cpu_pageset	pageset[NR_CPUS];
#endif
	/*��ͬ���ȵĿ�������*/
	/*���ϵͳ������*/
	spinlock_t		lock;
#ifdef CONFIG_MEMORY_HOTPLUG
	/* see spanned/present_pages for more description */
	seqlock_t		span_seqlock;
#endif
	/*�����ڴ����п���ҳ�Ļ��ϵͳ��ÿ������Ԫ�ض���ʾĳ�̶ֹ����ȵ�һЩ�����ڴ�������
	�ڰ�����ÿ�������еĿ����ڴ�ҳ�Ĺ���free_area��һ�����*/
	struct free_area	free_area[MAX_ORDER];

#ifndef CONFIG_SPARSEMEM
	/*pageblock_nr_pages���ʶ���ɲ鿴pageblock_flags.h����SPARSEMEM�����λͼ�洢��
	strct mem_section����*/
	unsigned long		*pageblock_flags;
#endif /* CONFIG_SPARSEMEM */
	/*����ֶΣ����뵽���ٻ�����*/
	ZONE_PADDING(_pad1_)

	/*ͨ����ҳ�����ɨ�������ʵ��ֶΡ��ò����漰�Ľṹ��Ա���������ݻ������ڴ���
	��ʹ�õ�ҳ���б�Ŀ�����ҳ����Ƶ�������ں���Ϊ���ǻ�ģ��������ҳ����Ȼ�෴��
	����Ҫ����ҳʱ�����������Ǻ���Ҫ�ġ�������ܵĻ���Ƶ��ʹ�õ�ҳӦ�ñ��ֲ�����������
	�Ĳ����ҳ����Ի�����ûʲô��*/
	/*������Ͳ��lru�����ϵ�ҳ*/
	spinlock_t		lru_lock;
	/*�ҳ����*/
	struct list_head	active_list;
	/*���ҳ����*/
	struct list_head	inactive_list;
	/*ָ���ڻ����ڴ�ʱ��Ҫɨ��Ļҳ��Ŀ*/
	unsigned long		nr_scan_active;
	/*�����ڴ�ʱ��Ҫɨ��Ĳ��ҳ����Ŀ*/
	unsigned long		nr_scan_inactive;
	/*ָ�����ϴλ���һҳ�������ж���ҳδ�ܳɹ�ɨ��*/
	unsigned long		pages_scanned;
	/*�ڴ���ǰ״̬*/
	unsigned long		flags;
	/*�ڴ���ͳ������*/
	/*ά�������йظ��ڴ����ͳ����Ϣ*/
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
	/*�洢����һ��ɨ�����ɨ����ڴ�������ȼ�*/
	int prev_priority;

	/*������ٻ����е�����ֽ�*/
	ZONE_PADDING(_pad2_)
	/*����ʹ�û����������ֻ�����ֶ�*/

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
	/*������������ʵ����һ���ȴ����У��ɹ��ȴ�ĳһҳ��Ϊ���õĽ���ʹ�á������ų�һ��
	���У��ȴ�ĳЩ������������Ϊ��ʱ���ں˻�֪ͨ���ָ̻�����*/
	/**/
	wait_queue_head_t	*wait_table;
	/**/
	unsigned long		wait_table_hash_nr_entries;
	/**/
	unsigned long		wait_table_bits;

	/*֧�ֲ������ڴ�ģ�͵��ֶ�*/
	/*�ڴ��������Ľ��ʵ����ָ��*/
	struct pglist_data	*zone_pgdat;
	/* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
	/*�ڴ����һ��ҳ֡������*/
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
	/*�����ն���ҳ��Ŀ*/
	unsigned long		spanned_pages;
	/*�����ն�ʵ�ʿ��õ�ҳ��Ŀ*/
	unsigned long		present_pages;

	/*���ڴ�������ƣ���Normal��DMA��HighMen������ʹ��*/
	const char		*name;
} ____cacheline_internodealigned_in_smp;

/*�ڴ���������״̬ʱҲ����û��������Щ��ʶ��ZONE_ALL_UNRECLAIMABLE״̬�������ں���
ͼ���ø��ڴ����һЩҳʱ��ҳ����գ�������Ϊ���е�ҳ������ס���޷����ա����磬�û���
��ʹ��mlockϵͳ����֪ͨ�ں�ҳ���ܴ������ڴ���������绻���������ϡ�������ҳ��֮Ϊ��ס
�ģ����һ���ڴ����е�����ҳ������ס����ô���ڴ������޷����յģ������øñ�ʶ��Ϊ�˲�
�˷�ʱ�䣬�����ػ�������Ѱ�ҿɹ����յ�ҳ�ǣ�ֻ���Ҫ��ɨ��һ�´����ڴ��򡣵�ɨ��ʱ�޷�
��ȫʡȥ�ģ���Ϊ���ڴ��򾭹�����ʱ����ڽ��������ٴΰ����ɻ��յ�ҳ��������ˣ�������
�ñ�ʶ����kswapd�ػ����̽����ڴ����������ڴ���ͬ�ȶԴ�����SMPϵͳ�ϣ����cpu������ͼ��
���ػ���һ���ڴ���ZONE_RECLAIM_LOCKED��ʶ��ֹ�������Σ����һ��cpu�ڻ���ĳ���ڴ���
�����øñ�ʶ�����ֹ����cpu�ĳ��ԡ�ZONE_OOM_LOCKEDר����ĳ�ֲ����˵�����������������
�˴����ڴ棬��ʹ��Ҫ�Ĳ������޷���ɣ���ô�ں˻���ͼɱ�������ڴ����Ľ��̣��Ի�ø���
�Ŀ���ҳ���ñ�ʶ���Է�ֹ���cpuͬʱ�������ֲ���*/
typedef enum
{
	/*����ҳ������ס���޷�����*/
	ZONE_ALL_UNRECLAIMABLE,
	/*��ֹ��������*/
	ZONE_RECLAIM_LOCKED,
	/*�ڴ��򼴿ɱ�����*/
	ZONE_OOM_LOCKED,
} zone_flags_t;

/*��ָ���ڴ�������Ϊ�ض�״̬*/
static inline void zone_set_flag(struct zone *zone, zone_flags_t flag)
{
	set_bit(flag, &zone->flags);
}
/*���Բ�����ָ���ڴ�����ض�״̬��Ȼ�󷵻ظ��ڴ���֮ǰ��״̬*/
static inline int zone_test_and_set_flag(struct zone *zone, zone_flags_t flag)
{
	return test_and_set_bit(flag, &zone->flags);
}
/*���ָ���ڴ����ض���ʶ*/
static inline void zone_clear_flag(struct zone *zone, zone_flags_t flag)
{
	clear_bit(flag, &zone->flags);
}
/*����ָ���ڴ����Ƿ��ڲ��ɻ���״̬*/
static inline int zone_is_all_unreclaimable(const struct zone *zone)
{
	return test_bit(ZONE_ALL_UNRECLAIMABLE, &zone->flags);
}
/*����ָ���ڴ����Ƿ��ڻ�������״̬*/
static inline int zone_is_reclaim_locked(const struct zone *zone)
{
	return test_bit(ZONE_RECLAIM_LOCKED, &zone->flags);
}
/*����ָ���ڴ����Ƿ���OOM����״̬*/
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
/*һ�����÷������б������ķ�������Ŀ*/
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
/*�����ڴ�����Ϣ*/
struct zonelist
{
	/*NULL or &zlcache*/
	struct zonelist_cache *zlcache_ptr;
	/*�����ڴ���ָ�����飬�����������NULL�ָ���*/
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

/*�������*/
struct node_active_region
{
	/*�������ʼҳ֡��*/
	unsigned long start_pfn;
	/*��������ҳ֡��*/
	unsigned long end_pfn;
	/*�����*/
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
/*������ݽṹ*/
typedef struct pglist_data
{
	/*����������˽���и��ڴ�������ݽṹ*/
	struct zone node_zones[MAX_NR_ZONES];
	/*ָ���˱��ý�㼰���ڴ�����б��Ա��ڵ�ǰ���������ÿռ�ʱ���ڱ��ý������ڴ�
	���������ܿ��ǣ���Ϊ���̷����ڴ�ʱ���ں�������ͼ�ڵ�ǰ���е�cpu�������NUMA�����
	���з��䣬���Ⲣ�����ǿ��еģ����磬�ý����ڴ�����Ѿ��þ����Դ��������ÿ�����
	���ṩ��һ�������б����б����������㣨����ص��ڴ��򣩣������ڴ��浱ǰ������
	�ڴ档�б���λ��Խ���󣬾�Խ���ʺϷ���*/
	struct zonelist node_zonelists[MAX_ZONELISTS];
	/*�������в�ͬ�ڴ������Ŀ*/
	int nr_zones;
#ifdef CONFIG_FLAT_NODE_MEM_MAP
	/*ָ��pageʵ�������ָ�룬���������������������ڴ�ҳ���������˽���������ڴ����ҳ*/
	struct page *node_mem_map;
#endif
	/*��ϵͳ�����ڼ䣬�ڴ������ϵͳ��ʼ��֮ǰ���ں�Ҳ��Ҫʹ���ڴ棨���⣬�����뱣������
	�ڴ����ڳ�ʼ���ڴ������ϵͳ�������Ծ��ڴ��������boot memory allocator��ʵ��ָ��ָ
	������������ʵ��*/
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
	/*�Ǹ�NUMA����һ��ҳ֡���߼���š�ϵͳ�����н���ҳ֡�����α�ŵģ�ÿ��ҳ֡��
	���붼��ȫ��Ψһ�ģ���ֻ�ǽ����Ψһ������ֵ��UMAϵͳ������0����Ϊֻ��һ����㣬
	������һ��ҳ֡������0*/
	unsigned long node_start_pfn;
	/*����п���ҳ֡��Ŀ*/
	unsigned long node_present_pages;
	/*�����ҳ֡Ϊ��λ����Ŀ�������ն���Ŀ��*/
	unsigned long node_spanned_pages;
	/*ȫ�ֽ��ID��ϵͳ�е�NUMA��㶼��0��ʼ���*/
	int node_id;
	/*�����ػ����̵ĵȴ����У��ڽ�ҳ֡�������ʱ���õ�*/
	wait_queue_head_t kswapd_wait;
	/*ָ����ý��Ľ����ػ����̣�swap daemon����task_strcutʵ��*/
	struct task_struct *kswapd;
	/*����ҳ������ϵͳ��ʵ�֣�����������Ҫ�ͷŵ�����ĳ���*/
	int kswapd_max_order;
} pg_data_t;

/*��ȡ����п���ҳ֡����Ŀ*/
#define node_present_pages(nid)	(NODE_DATA(nid)->node_present_pages)
/*��ȡ����а����ն���ҳ֡��Ŀ*/
#define node_spanned_pages(nid)	(NODE_DATA(nid)->node_spanned_pages)
#ifdef CONFIG_FLAT_NODE_MEM_MAP
/*���ݽ���ż�ҳ֡��ţ���ȡ��Ӧ�ý���ϵ�struct pageʵ��*/
#define pgdat_page_nr(pgdat, pagenr)	((pgdat)->node_mem_map + (pagenr))
#else
#define pgdat_page_nr(pgdat, pagenr)	pfn_to_page((pgdat)->node_start_pfn + (pagenr))
#endif
/*��ȡָ�������ָ��ҳ�ŵ�ҳʵ��*/
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

/*��ȡ�ڴ��������������ڴ����е�������ZONE_DMAʱ����0��ZONE_NORMAL����1���ȵ�*/
#define zone_idx(zone)		((zone) - (zone)->zone_pgdat->node_zones)

/*�����ڴ������Ƿ�����Чҳ֡*/
static inline int populated_zone(struct zone *zone)
{
	/*����ȡ��������ķ���ֵֻ����1��0*/
	return (!!zone->present_pages);
}

extern int movable_zone;

/*�����ڴ����Ƿ��Ǹ߶��ڴ���*/
static inline int zone_movable_is_highmem(void)
{
#if defined(CONFIG_HIGHMEM) && defined(CONFIG_ARCH_POPULATES_NODE_MAP)
	return movable_zone == ZONE_HIGHMEM;
#else
	return 0;
#endif
}

/*����ָ���ڴ����Ƿ���ZONE_HIGHMEM��ZONE_MOVABLE*/
static inline int is_highmem_idx(enum zone_type idx)
{
#ifdef CONFIG_HIGHMEM
	return (idx == ZONE_HIGHMEM || (idx == ZONE_MOVABLE && zone_movable_is_highmem()));
#else
	return 0;
#endif
}

/*����ָ���ڴ����Ƿ���ZONE_NORMAL*/
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
/*����ָ���ڴ����Ƿ��Ǹ߶��ڴ���ֻ�������ø߶��ڴ����Ҹ��ڴ������Ǹý��������
�߶�����߸ø߶��ڴ����е������ڴ���ʱ���ŷ���true*/
static inline int is_highmem(struct zone *zone)
{
#ifdef CONFIG_HIGHMEM
	int zone_idx = zone - zone->zone_pgdat->node_zones;
	return zone_idx == ZONE_HIGHMEM || (zone_idx == ZONE_MOVABLE && zone_movable_is_highmem());
#else
	return 0;
#endif
}
/*����ָ���ڴ����Ƿ�����ͨ�ڴ���*/
static inline int is_normal(struct zone *zone)
{
	return zone == zone->zone_pgdat->node_zones + ZONE_NORMAL;
}
/*����ָ���ڴ����Ƿ���ZONE_DMA32�ڴ���*/
static inline int is_dma32(struct zone *zone)
{
#ifdef CONFIG_ZONE_DMA32
	return zone == zone->zone_pgdat->node_zones + ZONE_DMA32;
#else
	return 0;
#endif
}
/*����ָ���ڴ����Ƿ���ZONE_DMA�ڴ���*/
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
/*�����ַ�������*/
#define NUMA_ZONELIST_ORDER_LEN			16

#include <linux/topology.h>
/*��ȡ��ǰ�������е�cpu��Ӧ�Ľ����*/
#ifndef numa_node_id
#define numa_node_id()		(cpu_to_node(raw_smp_processor_id()))
#endif

#ifndef CONFIG_NEED_MULTIPLE_NODES

extern struct pglist_data contig_page_data;
/*����ƽ̨�϶�ʵ�����ض�����ϵ�ṹ��NODE_DATA�꣬����ͨ����ţ�����ѯ��һ��NUMA���
��ص�pgdata_tʵ�����ú���һ����ʽ��������ѡ��NUMA��㣬����UMAϵͳ��ֻ��һ��α���
����ˣ����Ƿ���ͬ��������*/
#define NODE_DATA(nid)		(&contig_page_data)
/*��ȡ�������������ڴ�ҳ*/
#define NODE_MEM_MAP(nid)	mem_map
/**/
#define MAX_NODES_SHIFT		1

#else /* CONFIG_NEED_MULTIPLE_NODES */

#include <asm/mmzone.h>

#endif /* !CONFIG_NEED_MULTIPLE_NODES */

extern struct pglist_data *first_online_pgdat(void);
extern struct pglist_data *next_online_pgdat(struct pglist_data *pgdat);
extern struct zone *next_zone(struct zone *zone);


/*����ϵͳ���������߽��*/
#define for_each_online_pgdat(pgdat)\
	for (pgdat = first_online_pgdat(); pgdat; pgdat = next_online_pgdat(pgdat))

/*����ϵͳ�����н��������ڴ���*/
#define for_each_zone(zone)\
	for (zone = (first_online_pgdat())->node_zones; zone;	zone = next_zone(zone))

#ifdef CONFIG_SPARSEMEM
#include <asm/sparsemem.h>
#endif

#if BITS_PER_LONG == 32
/*32λpage->flags��Ϊ�����ڴ�����Ϣ����9λ��4���ڴ���3λ�������������6λ*/
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
/*UMAֻ��һ�����*/
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
