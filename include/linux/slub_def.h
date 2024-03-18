#ifndef _LINUX_SLUB_DEF_H
#define _LINUX_SLUB_DEF_H

/*slub��һ��û�ж���ȴ����е�slab������*/
#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>

/**/
struct kmem_cache_cpu
{
	/*ָ��CPU���ػ����slab�е�һ�����еĶ���*/
	void **freelist;
	/*slab cache��CPU�����������slab������slab�ײ�Ĵ洢�ṹ���ڴ�ҳpage����������
	ֱ�����ڴ�ҳpage��ʾslab*/
	struct page *page;
	/**/
	int node;
	/**/
	unsigned int offset;
	/**/
	unsigned int objsize;
};

/**/
struct kmem_cache_node
{
	/*���ֿ�������Ϳ�����Ŀ�ı�����*/
	spinlock_t list_lock;
	/*�����������Ŀ*/
	unsigned long nr_partial;
	/*�ýڵ��л����slab����*/
	atomic_long_t nr_slabs;
	/*������������֯�����ڵ��л����slabs partial�����л����slabΪ���ֿ��е�
	��slab�еĶ��󲿷ֱ������ȥ��    */
	struct list_head partial;
#ifdef CONFIG_SLUB_DEBUG
	/*full�����а�����slabȫ�����Ѿ���������ϵ�full slab*/
	struct list_head full;
#endif
};

/*
 * Slab cache management.
 */
/**/
struct kmem_cache
{
	/*slab cache�Ĺ����־λ����������slab��һЩ���ԡ����磺slab �еĶ�����ʲô
	��ʽ���롢�����Ƿ���ҪPOISON  �������Ƿ����red zone�ڶ����ڴ���Χ���Ƿ�׷�ٶ���
	�ķ�����ͷ���Ϣ��    */
	unsigned long flags;
	/*slab�������ڴ��е���ʵռ�ã�����Ϊ���ڴ���������ֽ�����red zone��    */
	int size;
	/*slab�ж����ʵ�ʴ�С�������������ֽ���*/
	int objsize;
	/*slab������еĶ�����û�б�����֮ǰ�������ǲ����Ķ�����ߴ洢�����ݵġ��ں���
	������ö���ռ�õ��ڴ�ռ�洢��һ�����ж���ĵ�ַ��offset��ʾ���ڴ洢��һ����
	�ж���ָ���λ�þ�������׵�ַ��ƫ��*/
	int offset;
	/**/
	int order;

	/*
	 * Avoid an extra cache line for UP, SMP and for the node local to
	 * struct kmem_cache.
	 */
	struct kmem_cache_node local_node;

	/*slab�ķ�����ͷ�*/
	/*slab�ж������Ŀ*/
	int objects;
	/*slab cache�����ü�����Ϊ0ʱ�Ϳ������ٲ��ͷ��ڴ�ػ��ϵͳ*/
	int refcount;
	/*�ػ�����Ĺ��캯�������ڴ���slab������еĶ���*/
	void (*ctor)(struct kmem_cache *, void *);
	/*Ԫ����ƫ�ơ���ʾ�����object size����word size����֮��Ĵ�С���������������
	SLAB_RED_ZONE��inuseҲ����������Ҳ�red zone����Ĵ�С*/
	int inuse;
	/*������ָ����align���ж��롣�ڴ���slab cache��ʱ�����ǿ������ں�ָ��slab��
	�Ķ�����align��ֵ���ж��룬�ں˻��ۺ�word size��cache line��align�����һ����
	��Ķ���ߴ�*/
	int align;
	/*slab�������֣�������ʾ��������ָ����name������cat /proc/slabinfo��������ʾ��
	���������ڲ鿴ϵͳ������slab cache����Ϣ*/
	const char *name;
	/*������֯����ϵͳ���������͵�struct kmem_cacheʵ��*/
	struct list_head list;
#ifdef CONFIG_SLUB_DEBUG
	/*sysfs�ļ�ϵͳʹ��*/
	struct kobject kobj;
#endif

#ifdef CONFIG_NUMA
	int defrag_ratio;
	struct kmem_cache_node *node[MAX_NUMNODES];
#endif
#ifdef CONFIG_SMP
	/**/
	struct kmem_cache_cpu *cpu_slab[NR_CPUS];
#else
	struct kmem_cache_cpu cpu_slab;
#endif
};

/*
 * Kmalloc subsystem.
 */
#if defined(ARCH_KMALLOC_MINALIGN) && ARCH_KMALLOC_MINALIGN > 8
#define KMALLOC_MIN_SIZE 		ARCH_KMALLOC_MINALIGN
#else
#define KMALLOC_MIN_SIZE 		8
#endif

#define KMALLOC_SHIFT_LOW		ilog2(KMALLOC_MIN_SIZE)

/*
 * We keep the general caches in an array of slab caches that are used for
 * 2^x bytes of allocations.
 */
extern struct kmem_cache kmalloc_caches[PAGE_SHIFT];

/*
 * Sorry that the following has to be that ugly but some versions of GCC
 * have trouble with constant propagation���� and loops.
 */
/*��ȡԤ�����ڴ��С��Ӧ��kmalloc_cache��������������0Ϊ��Ч���볤��*/
static __always_inline int kmalloc_index(size_t size)
{
	/*��Ч���볤��*/
	if (!size)
		return 0;
	/*kmalloc�ܷ������С�ռ䣬Ĭ����8���ֽڣ�����ȡ��2Ϊ�׵Ķ���ֵ�����Ӧ������*/
	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;
	/*����ռ���(64, 96]֮��ʱ����Ӧ��kmalloc_cache��������1*/
	if (size > 64 && size <= 96)
		return 1;
	/*����ռ���(128, 192]ʱ����Ӧ��kmalloc_cache��������2*/
	if (size > 128 && size <= 192)
		return 2;
	/*����8���ֽڣ���Ӧ��������3*/
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
/*���³���4K�Ĵ�ҳ��Ҫ��ϵ�ṹ��֧��
 * The following is only needed to support architectures with a larger page
 * size than 4k.
 */
	if (size <=   4 * 1024) return 12;
	if (size <=   8 * 1024) return 13;
	if (size <=  16 * 1024) return 14;
	if (size <=  32 * 1024) return 15;
	if (size <=  64 * 1024) return 16;
	if (size <= 128 * 1024) return 17;
	if (size <= 256 * 1024) return 18;
	if (size <= 512 * 1024) return 19;
	if (size <= 1024 * 1024) return 20;
	if (size <=  2 * 1024 * 1024) return 21;
	return -1;

/*
 * What we really wanted to do and cannot do because of compiler issues is:
 *	int i;
 *	for (i = KMALLOC_SHIFT_LOW; i <= KMALLOC_SHIFT_HIGH; i++)
 *		if (size <= (1 << i))
 *			return i;
 */
}

/*
 * Find the slab cache for a given combination of allocation flags and size.
 *
 * This ought to end up with a global pointer to the right cache
 * in kmalloc_caches.
 */
static __always_inline struct kmem_cache *kmalloc_slab(size_t size)
{
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return &kmalloc_caches[index];
}

#ifdef CONFIG_ZONE_DMA
#define SLUB_DMA __GFP_DMA
#else
/* Disable DMA functionality */
#define SLUB_DMA (__force gfp_t)0
#endif

void *kmem_cache_alloc(struct kmem_cache *, gfp_t);
void *__kmalloc(size_t size, gfp_t flags);

/**/
static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
	/*����ʱȷ���ñ����ǳ���*/
	if (__builtin_constant_p(size))
	{
		/*�������ĳ��ȴ���ҳ����һ�룬�򰴳������븴��ҳ*/
		if (size > PAGE_SIZE / 2)
			return (void *)__get_free_pages(flags | __GFP_COMP,	get_order(size));
		/*���slubϵͳ�в�����ZONE_DMA�ڴ����������ڴ棬�򰴸�����ʶ�ͳ�����Ϸ���slab
		����*/
		if (!(flags & SLUB_DMA))
		{
			/**/
			struct kmem_cache *s = kmalloc_slab(size);
			/*����ʧ���򷵻�ZERO_SIZE_PTR*/
			if (!s)
				return ZERO_SIZE_PTR;
			/**/
			return kmem_cache_alloc(s, flags);
		}
	}
	return __kmalloc(size, flags);
}

#ifdef CONFIG_NUMA
void *__kmalloc_node(size_t size, gfp_t flags, int node);
void *kmem_cache_alloc_node(struct kmem_cache *, gfp_t flags, int node);

static __always_inline void *kmalloc_node(size_t size, gfp_t flags, int node)
{
	if (__builtin_constant_p(size) &&
		size <= PAGE_SIZE / 2 && !(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

		if (!s)
			return ZERO_SIZE_PTR;

		return kmem_cache_alloc_node(s, flags, node);
	}
	return __kmalloc_node(size, flags, node);
}
#endif

#endif /* _LINUX_SLUB_DEF_H */
