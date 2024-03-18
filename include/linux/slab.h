#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#ifdef __KERNEL__

#include <linux/gfp.h>
#include <linux/types.h>

/* DEBUG: Perform (expensive) checks on free */
/*传递给kmem_cache_create()的标识，标记DEBUG的标识仅在配置CONFIG_SLAB_DEBUG时有效*/
#define SLAB_DEBUG_FREE				0x00000100UL
/*DEBUG:缓存中的对象危险区。危险区是在每个对象的开始和结束处增加一个额外的内存区，其
中填充已知的字节模式，如果模式被修改，程序员在分析内核内存时注意到，可能某些代码访问
了不属于它们的内存区*/
#define SLAB_RED_ZONE				0x00000400UL
/*DEBUG:对象毒化。在建立和释放slab时，将对象用预定义的模式填充。如果在对象分配时注意
到该模式已经改变，程序员就知道已经发生了未授权的访问*/
#define SLAB_POISON					0x00000800UL
/*将对象对齐到缓存行。slab中的对象按照CPU硬件高速缓存行cache line(64字节)进行对齐*/
#define SLAB_HWCACHE_ALIGN			0x00002000UL
/*使用DMA内存域。当flags设置了SLAB_CACHE_DMA或者SLAB_CACHE_DMA32时，表示指定slab中的
内存来自于哪个内存区域，DMA或者DMA32区域？如果没有特殊指定，slab中的内存一般来自于
NORMAL直接映射区域*/
#define SLAB_CACHE_DMA				0x00004000UL
/*DEBUG:为捕获bug而存储最后一个拥有者。表示需要追踪对象的分配和释放相关信息，这样会
在slab对象内存区域中额外增加两个sizeof(struct track)大小的区域出来，用于存储slab对象
的分配和释放信息*/
#define SLAB_STORE_USER				0x00010000UL
/*kmem_cache_create()调用失败时，系统panic*/
#define SLAB_PANIC					0x00040000UL
/*Defer freeing slabs to RCU*/
#define SLAB_DESTROY_BY_RCU			0x00080000UL
/*Spread some memory over cpuset*/
#define SLAB_MEM_SPREAD				0x00100000UL
/*跟踪分配和释放*/
#define SLAB_TRACE					0x00200000UL

/*接下来的标识影响页的分配器分组可移动页面*/
/*可回收的对象*/
#define SLAB_RECLAIM_ACCOUNT		0x00020000UL
/*短时存在的对象*/
#define SLAB_TEMPORARY				SLAB_RECLAIM_ACCOUNT
/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
/**/
#define ZERO_SIZE_PTR ((void *)16)
/**/
#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= (unsigned long)ZERO_SIZE_PTR)


void __init kmem_cache_init(void);
int slab_is_available(void);

struct kmem_cache *kmem_cache_create(const char *, size_t, size_t, unsigned long,
			void (*)(struct kmem_cache *, void *));
void kmem_cache_destroy(struct kmem_cache *);
int kmem_cache_shrink(struct kmem_cache *);
void kmem_cache_free(struct kmem_cache *, void *);
unsigned int kmem_cache_size(struct kmem_cache *);
const char *kmem_cache_name(struct kmem_cache *);
int kmem_ptr_validate(struct kmem_cache *cachep, const void *ptr);

/*
 * Please use this macro to create slab caches. Simply specify the
 * name of the structure and maybe some flags that are listed above.
 *
 * The alignment of the struct determines object alignment. If you
 * f.e. add ____cacheline_aligned_in_smp to the struct declaration
 * then the objects will be properly aligned in SMP configurations.
 */
#define KMEM_CACHE(__struct, __flags) kmem_cache_create(#__struct,\
		sizeof(struct __struct), __alignof__(struct __struct), (__flags), NULL)

/*
 * The largest kmalloc size supported by the slab allocators is
 * 32 megabyte (2^25) or the maximum allocatable page order if that is
 * less than 32 MB.
 *
 * WARNING: Its not easy to increase this value since the allocators have
 * to do various tricks to work around compiler limitations in order to
 * ensure proper constant folding.
 */
/*slab分配器可支持kmalloc分配最大32M或者小于32M的页面最大可分配阶，默认11+12-1=22，
对应4M*/
#define KMALLOC_SHIFT_HIGH		((MAX_ORDER + PAGE_SHIFT - 1) <= 25 ? \
				(MAX_ORDER + PAGE_SHIFT - 1) : 25)
/*使用kmalloc的最大可分配的长度*/
#define KMALLOC_MAX_SIZE		(1UL << KMALLOC_SHIFT_HIGH)
/*使用kmalloc支持的最大可分配阶*/
#define KMALLOC_MAX_ORDER		(KMALLOC_SHIFT_HIGH - PAGE_SHIFT)

/*所有分配器都提供的通用kmalloc函数*/
void * __must_check krealloc(const void *, size_t, gfp_t);
void kfree(const void *);
size_t ksize(const void *);

/*
 * Allocator specific definitions. These are mainly used to establish optimized
 * ways to convert kmalloc() calls to kmem_cache_alloc() invocations by
 * selecting the appropriate general cache at compile time.
 *
 * Allocators must define at least:
 *
 *	kmem_cache_alloc()
 *	__kmalloc()
 *	kmalloc()
 *
 * Those wishing to support NUMA must also define:
 *
 *	kmem_cache_alloc_node()
 *	kmalloc_node()
 *
 * See each allocator definition file for additional comments and
 * implementation notes.
 */
 /*先查看CONFIG_SLUB或CONFIG_SLOB是否配置，再选择对应的分配方式，默认时slab分配方式*/
#ifdef CONFIG_SLUB
#include <linux/slub_def.h>
#elif defined(CONFIG_SLOB)
#include <linux/slob_def.h>
#else
#include <linux/slab_def.h>
#endif

/*标识参数包含：
 *  GFP_USER        分配用户内存，可能会睡眠
 * GFP_KERNEL   分配普通内核内存，可能会睡眠
 * GFP_ATOMIC   分配不能睡眠，可使用紧急分配池，例如中断内部处理使用该标识
 * GFP_HIGHUSER 从高端内存域分配内存
 * GFP_NOIO         获取内存期间不进行任何IO操作
 * GFP_NOFS         获取内存期间不进行任何文件系统操作
 * GFP_NOWAIT   分配期间禁止睡眠
 * GFP_THISNODE 仅分配本地节点内存
 * GFP_DMA          使用kmalloc从DMA内存域中分配内存，否则，使用已创建的带有SLAB_DMA标识的内存
 * 下列标识可以通过按位或操设置不同的标识组合作为附加标识
 * __GFP_COLD   分配冷缓存页
 * __GFP_HIGH   可用紧急分配池的高权限分配
 * __GFP_NOFAIL 不允许分配失败，一直重试，直至成功
 * __GFP_NORETRY 分配失败后放弃尝试
 * __GFP_NOWARN      分配失败后不警告
 * __GFP_REPEAT  刚开始分配失败后重试几次
 */
/*分配数组内存，内存分配成功后被清零。使用上述标识作为分配类型*/
static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{	/*预分配元素个数为零或元素所占空间大于最大可分配空间时，分配失败*/
	if (n != 0 && size > ULONG_MAX / n)
		return NULL;
	/*分配成功后的页面清零*/
	return __kmalloc(n * size, flags | __GFP_ZERO);
}

#if !defined(CONFIG_NUMA) && !defined(CONFIG_SLOB)
/*在当前结点（代码忽略指定结点）上分配分配标识为flags的size长字节的内存*/
static inline void *kmalloc_node(size_t size, gfp_t flags, int node)
{
	return kmalloc(size, flags);
}

/**/
static inline void *__kmalloc_node(size_t size, gfp_t flags, int node)
{
	return __kmalloc(size, flags);
}

void *kmem_cache_alloc(struct kmem_cache *, gfp_t);
static inline void *kmem_cache_alloc_node(struct kmem_cache *cachep, gfp_t flags, int node)
{
	return kmem_cache_alloc(cachep, flags);
}
#endif /* !CONFIG_NUMA && !CONFIG_SLOB */

/*
 * kmalloc_track_caller is a special version of kmalloc that records the
 * calling function of the routine calling it for slab leak tracking instead
 * of just the calling function (confusing, eh?).
 * It's useful when the call to kmalloc comes from a widely-used standard
 * allocator where we care about the real place the memory allocation
 * request comes from.
 */
#if defined(CONFIG_DEBUG_SLAB) || defined(CONFIG_SLUB)
extern void *__kmalloc_track_caller(size_t, gfp_t, void*);
#define kmalloc_track_caller(size, flags) \
	__kmalloc_track_caller(size, flags, __builtin_return_address(0))
#else
#define kmalloc_track_caller(size, flags)			__kmalloc(size, flags)
#endif /* DEBUG_SLAB */

#ifdef CONFIG_NUMA
/*
 * kmalloc_node_track_caller is a special version of kmalloc_node that
 * records the calling function of the routine calling it for slab leak
 * tracking instead of just the calling function (confusing, eh?).
 * It's useful when the call to kmalloc_node comes from a widely-used
 * standard allocator where we care about the real place the memory
 * allocation request comes from.
 */
#if defined(CONFIG_DEBUG_SLAB) || defined(CONFIG_SLUB)
extern void *__kmalloc_node_track_caller(size_t, gfp_t, int, void *);
#define kmalloc_node_track_caller(size, flags, node) \
	__kmalloc_node_track_caller(size, flags, node, \
			__builtin_return_address(0))
#else
#define kmalloc_node_track_caller(size, flags, node) \
	__kmalloc_node(size, flags, node)
#endif

#else /* CONFIG_NUMA */

#define kmalloc_node_track_caller(size, flags, node) kmalloc_track_caller(size, flags)

#endif /* DEBUG_SLAB */

/*
 * Shortcuts
 */
static inline void *kmem_cache_zalloc(struct kmem_cache *k, gfp_t flags)
{
	return kmem_cache_alloc(k, flags | __GFP_ZERO);
}

/**
 * kzalloc - allocate memory. The memory is set to zero.
 * @size: how many bytes of memory are required.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kzalloc(size_t size, gfp_t flags)
{
	return kmalloc(size, flags | __GFP_ZERO);
}

#ifdef CONFIG_SLABINFO
extern const struct seq_operations slabinfo_op;
ssize_t slabinfo_write(struct file *, const char __user *, size_t, loff_t *);
#endif

#endif	/* __KERNEL__ */
#endif	/* _LINUX_SLAB_H */
