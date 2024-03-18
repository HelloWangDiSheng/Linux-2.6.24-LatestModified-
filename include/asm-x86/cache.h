#ifndef _ARCH_X86_CACHE_H
#define _ARCH_X86_CACHE_H

/*定义L1缓存行大小*/
#define L1_CACHE_SHIFT	(CONFIG_X86_L1_CACHE_SHIFT)
#define L1_CACHE_BYTES	(1 << L1_CACHE_SHIFT)

/*将该宏定义的变量保存在".data.read_mostly"数据段中*/
#define __read_mostly __attribute__((__section__(".data.read_mostly")))

#ifdef CONFIG_X86_VSMP
/*vSMP内部结点缓存行大小为4K*/
#define INTERNODE_CACHE_SHIFT (12)
#ifdef CONFIG_SMP
/*该类型变量4k大小对齐且变量保存在".data.page_aligned"数据段中*/
#define __cacheline_aligned_in_smp					\
	__attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))	\
	__attribute__((__section__(".data.page_aligned")))
#endif
#endif

#endif
