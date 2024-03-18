#ifndef __LINUX_GFP_H
#define __LINUX_GFP_H

#include <linux/mmzone.h>
#include <linux/stddef.h>
#include <linux/linkage.h>

struct vm_area_struct;

/*�ڴ������η�*/
/*��ZONE_DMA�ڴ����з������ҳ*/
#define __GFP_DMA			((__force gfp_t)0x01u)
/*��ZONE_HIGHMEM�ڴ����з������ҳ*/
#define __GFP_HIGHMEM		((__force gfp_t)0x02u)
/*��ZONE_DMA32�ڴ����з������ҳ*/
#define __GFP_DMA32			((__force gfp_t)0x04u)

/*���ı��ڴ������η��������ƴ��ĸ��ڴ�������ڴ棬�����Ըı����������Ϊ*/
/*�������ҳ�ڼ���Եȴ��͵��ȡ������ڴ����������жϡ��������ڸ������ڼ��������
ѡ����һ������ִ�У����߸�������Ա���һ������Ҫ���¼��жϡ��������������ڷ����ڴ�
֮ǰ���ڶ����ϵȴ�һ���¼�����ؽ��̻����˯��״̬��*/
#define __GFP_WAIT				((__force gfp_t)0x10u)
/*���ý�������ء��������ǳ���Ҫ�������ø�ѡ����ں˼��е���Ҫ�ڴ�ʱ���ڷ���ʧ��
���ܸ��ں˴������غ��ʱ��������в��ϵͳ���ȶ��Ի�ϵͳ�����������ǻ�ʹ�øñ�ʶ*/
#define __GFP_HIGH				((__force gfp_t)0x20u)
/*�ڲ��ҿ����ڴ�ҳ�ڼ䣬�ں˿��Խ���I/O������ʵ���ϣ�����ζ������ں����ڴ�����ڼ�
����ҳ����ô�������øñ�ʶʱ�����ܽ�ѡ���ҳд��Ӳ��*/
#define __GFP_IO				((__force gfp_t)0x40u)
/*�����ں�ִ�еͼ�VFS����������VFS������ϵ���ں���ϵͳ�б�����ã���Ϊ���������ѭ��
�ݹ����*/
#define __GFP_FS				((__force gfp_t)0x80u)
/*�����Ҫ���䲻��cpu���ٻ����е���ҳʱ�������øñ�ʶ*/
#define __GFP_COLD				((__force gfp_t)0x100u)
/*�ڷ���ʧ��ʱ��ֹ�ں˹��Ͼ��档�ڼ��������ϣ��ñ�ʶ����*/
#define __GFP_NOWARN			((__force gfp_t)0x200u)
/*�ڷ���ʧ�ܺ��Զ����ԣ����ڳ������ɴ�֮���ֹͣ*/
#define __GFP_REPEAT			((__force gfp_t)0x400u)
/*�ڷ���ʧ��ʱһֱ���ԣ�ֱ���ɹ�*/
#define __GFP_NOFAIL			((__force gfp_t)0x800u)
/*�ڷ���ʧ��ʱֱ�ӷ���*/
#define __GFP_NORETRY			((__force gfp_t)0x1000u)
/*�������ҳ*/
#define __GFP_COMP				((__force gfp_t)0x4000u)/* Add compound page metadata */
/*�ڷ���ɹ�ʱ������������ֽ�0��ҳ*/
#define __GFP_ZERO				((__force gfp_t)0x8000u)
/*��ʹ�ý�������lowmem_reserve*/
#define __GFP_NOMEMALLOC 		((__force gfp_t)0x10000u)
/*ֻ��NUMAϵͳ�������塣������ֻ�ڷ��䵽��ǰ���̵ĸ���cpu�������Ľ������ڴ档���
��������������cpu�����У�Ĭ����������ñ�ʶ��������ģ�ֻ�н��̿������е�cpu����ʱ��
�ñ�ʶ����Ч��*/
#define __GFP_HARDWALL			((__force gfp_t)0x20000u)
/*Ҳֻ����NUMAϵͳ�������塣����ʧ�ܵ�����²�����ʹ�����������Ϊ���á���Ҫ��֤�ڵ�
ǰ��������ȷָ���Ľ���ϳɹ������ڴ�*/
#define __GFP_THISNODE			((__force gfp_t)0x40000u)
/*����ɻ��յ�ҳ��*/
#define __GFP_RECLAIMABLE		((__force gfp_t)0x80000u)
/*������ƶ���ҳ��*/
#define __GFP_MOVABLE			((__force gfp_t)0x100000u)  

/*21��__GFP_*λ�ռ�*/
#define __GFP_BITS_SHIFT 		21
/*__GFP_*λ����*/
#define __GFP_BITS_MASK 		((__force gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

/*����0�������ı�ʱӦ��ʹ�ó���*/
#define GFP_NOWAIT	(GFP_ATOMIC & ~__GFP_HIGH)
/* GFP_ATOMIC means both !wait (__GFP_WAIT not set) and use emergency pool */
/*ԭ�ӷ��䣬���κ�����¶������жϣ���ʹ�ý������������е��ڴ�*/
#define GFP_ATOMIC	(__GFP_HIGH)
/*��ֹI/O����*/
#define GFP_NOIO	(__GFP_WAIT)
/*��ֹVFS����*/
#define GFP_NOFS	(__GFP_WAIT | __GFP_IO)
/*�ں˷����Ĭ�����ã��ں˴������ʹ�õı�ʶ*/
#define GFP_KERNEL	(__GFP_WAIT | __GFP_IO | __GFP_FS)
/*����ɻ��յ��ں�ҳ��*/
#define GFP_TEMPORARY	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_RECLAIMABLE)
/*�û������Ĭ�����á��ڽ����������е�cpu��Ӧ���ڴ����Ϸ���*/
#define GFP_USER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
/*GFP_USER�Ļ������ڸ߶��ڴ����з���*/
#define GFP_HIGHUSER	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | \
									__GFP_HIGHMEM | __GFP_MOVABLE)
#define GFP_NOFS_PAGECACHE	(__GFP_WAIT | __GFP_IO | __GFP_MOVABLE)
#define GFP_USER_PAGECACHE	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | __GFP_MOVABLE)
#define GFP_HIGHUSER_PAGECACHE	(__GFP_WAIT | __GFP_IO | __GFP_FS | __GFP_HARDWALL | \
									__GFP_HIGHMEM | __GFP_MOVABLE)

#ifdef CONFIG_NUMA
/*����NUMAѡ��ʱ������������е�cpu��Ӧ���ڴ����Ϸ��䣬���ҷ���ʧ�ܺ󲻾��棬������*/
#define GFP_THISNODE	(__GFP_THISNODE | __GFP_NOWARN | __GFP_NORETRY)
#else
/*��NUMA����ʱֻ���ڵ�ǰ����Ϸ�������ڴ�ҳ*/
#define GFP_THISNODE	((__force gfp_t)0)
#endif

/*�ڿɻ���ҳ�Ϳ��ƶ�ҳ�з������ҳ*/
#define GFP_MOVABLE_MASK (__GFP_RECLAIMABLE|__GFP_MOVABLE)

/*����ҳ���������ҳ�������Ϊ*/
#define GFP_RECLAIM_MASK (__GFP_WAIT|__GFP_HIGH|__GFP_IO|__GFP_FS|__GFP_NOWARN|\
				__GFP_REPEAT|__GFP_NOFAIL| __GFP_NORETRY|__GFP_NOMEMALLOC)

/*���Ʒ�������*/
#define GFP_CONSTRAINT_MASK (__GFP_HARDWALL|__GFP_THISNODE)

/*������slab��ʹ����Щ�����ʶ*/
#define GFP_SLAB_BUG_MASK (__GFP_DMA32|__GFP_HIGHMEM|~__GFP_BITS_MASK)

/* Flag - indicates that the buffer will be suitable for DMA.  Ignored on some
   platforms, used as appropriate on others */
/**/
#define GFP_DMA		__GFP_DMA

/*һЩƽ̨�ϱ���Ϊ��4GDMA*/
#define GFP_DMA32	__GFP_DMA32

/*��GFP��ʶת����Ǩ������*/
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

/*���ݷ���ҳ�����ȡ��Ӧ���ڴ���*/
static inline enum zone_type gfp_zone(gfp_t flags)
{
	int base = 0;
/*����NUMA�ĵ�ǰ�����*/
#ifdef CONFIG_NUMA
	if (flags & __GFP_THISNODE)
		base = MAX_NR_ZONES;
#endif

#ifdef CONFIG_ZONE_DMA
/*���������ZONE_DMA�ڴ��򣬷���ʱָ���ڸ�������䣬�򷵻ؽ���ZONE_DMA�ڴ�����*/

	if (flags & __GFP_DMA)
		return base + ZONE_DMA;
#endif

#ifdef CONFIG_ZONE_DMA32
/*���������ZONE_DMA32�ڴ��򣬷���ʱָ���ڸ�������䣬�򷵻ؽ���ZONE_DMA32�ڴ�����*/

	if (flags & __GFP_DMA32)
		return base + ZONE_DMA32;
#endif
/*���ø߶��ڴ��򣬲������������ڴ���ʱ�����ؽ���Ӧ�������ڴ�����*/
	if ((flags & (__GFP_HIGHMEM | __GFP_MOVABLE)) == (__GFP_HIGHMEM | __GFP_MOVABLE))
		return base + ZONE_MOVABLE;
	
#ifdef CONFIG_HIGHMEM
/*���������ZONE_HIGHMEM�ڴ��򣬷���ʱָ���ڸ�������䣬�򷵻ؽ���ZONE_HIGHMEM�ڴ�����*/
	if (flags & __GFP_HIGHMEM)
		return base + ZONE_HIGHMEM;
#endif

/*���ؽ���ZONE_NORMAL�ڴ�����*/
	return base + ZONE_NORMAL;
}

/*����ҳ��������룬����Ǩ�����͡�ע�⣡ҳ�������벻��ָ��Ϊ�����ڴ���*/
static inline gfp_t set_migrateflags(gfp_t gfp, gfp_t migrate_flags)
{
	/*ҳ���������в����������ڴ����з���ɻ����ڴ�ҳ*/
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

/*��ָ����㣨ָ�����Ϊ��ֵʱĬ��Ϊ��ǰ��㣩�Ϸ���ҳ*/
static inline struct page *alloc_pages_node(int nid, gfp_t gfp_mask, unsigned int order)
{
	/*�������Ч*/
	if (unlikely(order >= MAX_ORDER))
		return NULL;

	/*δָ�����ʱĬ��Ϊ��ǰ���*/
	if (nid < 0)
		nid = numa_node_id();
	/*��ָ�����ı����ڴ������ɷ�������gfp_mask��ȡ���ڴ���ʼ����*/
	return __alloc_pages(gfp_mask, order, NODE_DATA(nid)->node_zonelists + gfp_zone(gfp_mask));
}

#ifdef CONFIG_NUMA
extern struct page *alloc_pages_current(gfp_t gfp_mask, unsigned order);

/*�ӵ�ǰ�������ڴ�ҳ*/
static inline struct page *alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	/*�������Ч*/
	if (unlikely(order >= MAX_ORDER))
		return NULL;
	/*�ڵ�ǰ����Ϸ���*/
	return alloc_pages_current(gfp_mask, order);
}
extern struct page *alloc_page_vma(gfp_t gfp_mask, struct vm_area_struct *vma, unsigned long addr);
#else
/*�ڵ�ǰ����ȡָ���������ͺͷ���׵Ŀ���ҳ*/
#define alloc_pages(gfp_mask, order) alloc_pages_node(numa_node_id(), gfp_mask, order)
#define alloc_page_vma(gfp_mask, vma, addr) alloc_pages(gfp_mask, 0)
#endif
/*����һ��ҳ*/
#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)
extern unsigned long FASTCALL(__get_free_pages(gfp_t gfp_mask, unsigned int order));
extern unsigned long FASTCALL(get_zeroed_page(gfp_t gfp_mask));

/*��ȡһ������ҳ*/
#define __get_free_page(gfp_mask) 		__get_free_pages((gfp_mask), 0)
/*��ZONE_DMA�ڴ����л�ȡָ������׺ͷ����Ŀ���ҳ*/
#define __get_dma_pages(gfp_mask, order) __get_free_pages((gfp_mask) | GFP_DMA, (order))
extern void FASTCALL(__free_pages(struct page *page, unsigned int order));
extern void FASTCALL(free_pages(unsigned long addr, unsigned int order));
extern void FASTCALL(free_hot_page(struct page *page));
extern void FASTCALL(free_cold_page(struct page *page));
/*�ͷ�ҳ��ע�⣡�ò�����struct pageʵ��*/
#define __free_page(page) __free_pages((page), 0)
/*�ͷ������ַ��Ӧ��ҳ*/
#define free_page(addr) free_pages((addr),0)

void page_alloc_init(void);
void drain_zone_pages(struct zone *zone, struct per_cpu_pages *pcp);

#endif /* __LINUX_GFP_H */
