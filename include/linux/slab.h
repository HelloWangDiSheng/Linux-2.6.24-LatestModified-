#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#ifdef __KERNEL__

#include <linux/gfp.h>
#include <linux/types.h>

/* DEBUG: Perform (expensive) checks on free */
/*���ݸ�kmem_cache_create()�ı�ʶ�����DEBUG�ı�ʶ��������CONFIG_SLAB_DEBUGʱ��Ч*/
#define SLAB_DEBUG_FREE				0x00000100UL
/*DEBUG:�����еĶ���Σ������Σ��������ÿ������Ŀ�ʼ�ͽ���������һ��������ڴ�������
�������֪���ֽ�ģʽ�����ģʽ���޸ģ�����Ա�ڷ����ں��ڴ�ʱע�⵽������ĳЩ�������
�˲��������ǵ��ڴ���*/
#define SLAB_RED_ZONE				0x00000400UL
/*DEBUG:���󶾻����ڽ������ͷ�slabʱ����������Ԥ�����ģʽ��䡣����ڶ������ʱע��
����ģʽ�Ѿ��ı䣬����Ա��֪���Ѿ�������δ��Ȩ�ķ���*/
#define SLAB_POISON					0x00000800UL
/*��������뵽�����С�slab�еĶ�����CPUӲ�����ٻ�����cache line(64�ֽ�)���ж���*/
#define SLAB_HWCACHE_ALIGN			0x00002000UL
/*ʹ��DMA�ڴ��򡣵�flags������SLAB_CACHE_DMA����SLAB_CACHE_DMA32ʱ����ʾָ��slab�е�
�ڴ��������ĸ��ڴ�����DMA����DMA32�������û������ָ����slab�е��ڴ�һ��������
NORMALֱ��ӳ������*/
#define SLAB_CACHE_DMA				0x00004000UL
/*DEBUG:Ϊ����bug���洢���һ��ӵ���ߡ���ʾ��Ҫ׷�ٶ���ķ�����ͷ������Ϣ��������
��slab�����ڴ������ж�����������sizeof(struct track)��С��������������ڴ洢slab����
�ķ�����ͷ���Ϣ*/
#define SLAB_STORE_USER				0x00010000UL
/*kmem_cache_create()����ʧ��ʱ��ϵͳpanic*/
#define SLAB_PANIC					0x00040000UL
/*Defer freeing slabs to RCU*/
#define SLAB_DESTROY_BY_RCU			0x00080000UL
/*Spread some memory over cpuset*/
#define SLAB_MEM_SPREAD				0x00100000UL
/*���ٷ�����ͷ�*/
#define SLAB_TRACE					0x00200000UL

/*�������ı�ʶӰ��ҳ�ķ�����������ƶ�ҳ��*/
/*�ɻ��յĶ���*/
#define SLAB_RECLAIM_ACCOUNT		0x00020000UL
/*��ʱ���ڵĶ���*/
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
/*slab��������֧��kmalloc�������32M����С��32M��ҳ�����ɷ���ף�Ĭ��11+12-1=22��
��Ӧ4M*/
#define KMALLOC_SHIFT_HIGH		((MAX_ORDER + PAGE_SHIFT - 1) <= 25 ? \
				(MAX_ORDER + PAGE_SHIFT - 1) : 25)
/*ʹ��kmalloc�����ɷ���ĳ���*/
#define KMALLOC_MAX_SIZE		(1UL << KMALLOC_SHIFT_HIGH)
/*ʹ��kmalloc֧�ֵ����ɷ����*/
#define KMALLOC_MAX_ORDER		(KMALLOC_SHIFT_HIGH - PAGE_SHIFT)

/*���з��������ṩ��ͨ��kmalloc����*/
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
 /*�Ȳ鿴CONFIG_SLUB��CONFIG_SLOB�Ƿ����ã���ѡ���Ӧ�ķ��䷽ʽ��Ĭ��ʱslab���䷽ʽ*/
#ifdef CONFIG_SLUB
#include <linux/slub_def.h>
#elif defined(CONFIG_SLOB)
#include <linux/slob_def.h>
#else
#include <linux/slab_def.h>
#endif

/*��ʶ����������
 *  GFP_USER        �����û��ڴ棬���ܻ�˯��
 * GFP_KERNEL   ������ͨ�ں��ڴ棬���ܻ�˯��
 * GFP_ATOMIC   ���䲻��˯�ߣ���ʹ�ý�������أ������ж��ڲ�����ʹ�øñ�ʶ
 * GFP_HIGHUSER �Ӹ߶��ڴ�������ڴ�
 * GFP_NOIO         ��ȡ�ڴ��ڼ䲻�����κ�IO����
 * GFP_NOFS         ��ȡ�ڴ��ڼ䲻�����κ��ļ�ϵͳ����
 * GFP_NOWAIT   �����ڼ��ֹ˯��
 * GFP_THISNODE �����䱾�ؽڵ��ڴ�
 * GFP_DMA          ʹ��kmalloc��DMA�ڴ����з����ڴ棬����ʹ���Ѵ����Ĵ���SLAB_DMA��ʶ���ڴ�
 * ���б�ʶ����ͨ����λ������ò�ͬ�ı�ʶ�����Ϊ���ӱ�ʶ
 * __GFP_COLD   �����仺��ҳ
 * __GFP_HIGH   ���ý�������صĸ�Ȩ�޷���
 * __GFP_NOFAIL ���������ʧ�ܣ�һֱ���ԣ�ֱ���ɹ�
 * __GFP_NORETRY ����ʧ�ܺ��������
 * __GFP_NOWARN      ����ʧ�ܺ󲻾���
 * __GFP_REPEAT  �տ�ʼ����ʧ�ܺ����Լ���
 */
/*���������ڴ棬�ڴ����ɹ������㡣ʹ��������ʶ��Ϊ��������*/
static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{	/*Ԥ����Ԫ�ظ���Ϊ���Ԫ����ռ�ռ�������ɷ���ռ�ʱ������ʧ��*/
	if (n != 0 && size > ULONG_MAX / n)
		return NULL;
	/*����ɹ����ҳ������*/
	return __kmalloc(n * size, flags | __GFP_ZERO);
}

#if !defined(CONFIG_NUMA) && !defined(CONFIG_SLOB)
/*�ڵ�ǰ��㣨�������ָ����㣩�Ϸ�������ʶΪflags��size���ֽڵ��ڴ�*/
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
