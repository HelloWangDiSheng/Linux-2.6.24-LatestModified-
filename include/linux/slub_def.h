#ifndef _LINUX_SLUB_DEF_H
#define _LINUX_SLUB_DEF_H

/*slub：一个没有对象等待队列的slab分配器*/
#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>

/**/
struct kmem_cache_cpu
{
	/*指向被CPU本地缓存的slab中第一个空闲的对象*/
	void **freelist;
	/*slab cache中CPU本地所缓存的slab，由于slab底层的存储结构是内存页page，所以这里
	直接用内存页page表示slab*/
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
	/*部分空闲链表和空闲数目的保护锁*/
	spinlock_t list_lock;
	/*空闲链表的数目*/
	unsigned long nr_partial;
	/*该节点中缓存的slab个数*/
	atomic_long_t nr_slabs;
	/*该链表用于组织串联节点中缓存的slabs partial链表中缓存的slab为部分空闲的
	（slab中的对象部分被分配出去）    */
	struct list_head partial;
#ifdef CONFIG_SLUB_DEBUG
	/*full链表中包含的slab全部是已经被分配完毕的full slab*/
	struct list_head full;
#endif
};

/*
 * Slab cache management.
 */
/**/
struct kmem_cache
{
	/*slab cache的管理标志位，用于设置slab的一些特性。比如：slab 中的对象按照什么
	方式对齐、对象是否需要POISON  毒化、是否插入red zone在对象内存周围，是否追踪对象
	的分配和释放信息等    */
	unsigned long flags;
	/*slab对象在内存中的真实占用，包括为了内存对齐填充的字节数，red zone等    */
	int size;
	/*slab中对象的实际大小，不包含填充的字节数*/
	int objsize;
	/*slab对象池中的对象在没有被分配之前，我们是不关心对象里边存储的内容的。内核巧
	妙的利用对象占用的内存空间存储下一个空闲对象的地址。offset表示用于存储下一个空
	闲对象指针的位置距离对象首地址的偏移*/
	int offset;
	/**/
	int order;

	/*
	 * Avoid an extra cache line for UP, SMP and for the node local to
	 * struct kmem_cache.
	 */
	struct kmem_cache_node local_node;

	/*slab的分配和释放*/
	/*slab中对象的数目*/
	int objects;
	/*slab cache的引用计数，为0时就可以销毁并释放内存回伙伴系统*/
	int refcount;
	/*池化对象的构造函数，用于创建slab对象池中的对象*/
	void (*ctor)(struct kmem_cache *, void *);
	/*元数据偏移。表示对象的object size按照word size对齐之后的大小，如果我们设置了
	SLAB_RED_ZONE，inuse也会包括对象右侧red zone区域的大小*/
	int inuse;
	/*对象按照指定的align进行对齐。在创建slab cache的时候，我们可以向内核指定slab中
	的对象按照align的值进行对齐，内核会综合word size、cache line、align计算出一个合
	理的对齐尺寸*/
	int align;
	/*slab缓存名字（仅供显示）。这里指定的name将会在cat /proc/slabinfo命令中显示，
	该命令用于查看系统中所有slab cache的信息*/
	const char *name;
	/*用于组织串联系统中所有类型的struct kmem_cache实例*/
	struct list_head list;
#ifdef CONFIG_SLUB_DEBUG
	/*sysfs文件系统使用*/
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
 * have trouble with constant propagation传播 and loops.
 */
/*获取预申请内存大小对应的kmalloc_cache数组索引，返回0为无效申请长度*/
static __always_inline int kmalloc_index(size_t size)
{
	/*无效申请长度*/
	if (!size)
		return 0;
	/*kmalloc能分配的最小空间，默认是8个字节，对其取以2为底的对数值是其对应的索引*/
	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;
	/*申请空间在(64, 96]之间时，对应的kmalloc_cache数组索引1*/
	if (size > 64 && size <= 96)
		return 1;
	/*申请空间在(128, 192]时，对应的kmalloc_cache数组索引2*/
	if (size > 128 && size <= 192)
		return 2;
	/*申请8个字节，对应的索引是3*/
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
/*以下超过4K的大页需要体系结构的支持
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
	/*编译时确定该变量是常数*/
	if (__builtin_constant_p(size))
	{
		/*如果申请的长度大于页长的一半，则按长度申请复合页*/
		if (size > PAGE_SIZE / 2)
			return (void *)__get_free_pages(flags | __GFP_COMP,	get_order(size));
		/*如果slub系统中不是在ZONE_DMA内存域中申请内存，则按给定标识和长度组合分配slab
		缓存*/
		if (!(flags & SLUB_DMA))
		{
			/**/
			struct kmem_cache *s = kmalloc_slab(size);
			/*分配失败则返回ZERO_SIZE_PTR*/
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
