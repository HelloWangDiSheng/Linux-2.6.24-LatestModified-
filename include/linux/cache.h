#ifndef __LINUX_CACHE_H
#define __LINUX_CACHE_H

#include <linux/kernel.h>
#include <asm/cache.h>

/*SMP_CACHE_BYTES会对齐到数据，使之在大多数体系结构上能够理想地置于L1高速缓存中
（尽管名字含有SMP字样，但处理器系统也会定义该常数）*/

#ifndef L1_CACHE_ALIGN
/*L1缓存行对齐*/
#define L1_CACHE_ALIGN(x) ALIGN(x, L1_CACHE_BYTES)
#endif

#ifndef SMP_CACHE_BYTES
 /*L1缓存行对齐*/
 #define SMP_CACHE_BYTES L1_CACHE_BYTES
#endif

#ifndef __read_mostly
#define __read_mostly
#endif

#ifndef ____cacheline_aligned
 /*SMP_CACHE_BYTES对齐*/
#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))
#endif

#ifndef ____cacheline_aligned_in_smp
#ifdef CONFIG_SMP
 /*SMP_CACHE_BYTES对齐*/
#define ____cacheline_aligned_in_smp ____cacheline_aligned
#else
#define ____cacheline_aligned_in_smp
#endif /* CONFIG_SMP */
#endif

#ifndef __cacheline_aligned
 /*SMP_CACHE_BYTES对齐，并将数据保存在".data.cacheline_aligned"数据段中*/
#define __cacheline_aligned	 __attribute__((__aligned__(SMP_CACHE_BYTES),			\
		 __section__(".data.cacheline_aligned")))
#endif /* __cacheline_aligned */

#ifndef __cacheline_aligned_in_smp
#ifdef CONFIG_SMP
#define __cacheline_aligned_in_smp __cacheline_aligned
#else
#define __cacheline_aligned_in_smp
#endif /* CONFIG_SMP */
#endif

/*一些关键数据结构需要最大的对齐方式，这些可以是内部结点缓存行大小或L3缓存行
 大小等，这种方式定义的对齐在告诉缓存使用方面时最佳的，但浪费了更多的时间，因
 而该属性的使用需要谨慎地考虑。在体系结构中asm/cache.h中定义*/
#ifndef INTERNODE_CACHE_SHIFT
/*结点内部最大的缓存行对应的字节偏移*/
#define INTERNODE_CACHE_SHIFT L1_CACHE_SHIFT
#endif

#if !defined(____cacheline_internodealigned_in_smp)
#if defined(CONFIG_SMP)
/*SMP系统上没有定义结点内部缓存行偏移对齐时，定义该对齐长度为1UL左移结点内部最大的缓存行
对应的字节后的长度*/
#define ____cacheline_internodealigned_in_smp \
	__attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))
#else
#define ____cacheline_internodealigned_in_smp
#endif
#endif

#endif /* __LINUX_CACHE_H */
