#ifndef __ASM_MEMORY_MODEL_H
#define __ASM_MEMORY_MODEL_H

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

/*
使用FLATMEM的模型非常高效和简单，直接将物理页通过线性映射与mem_map对应起来。但这种模
型有个致命的问题，就是在存在大量空洞内存的场景下，mem_map数组可能会很大，造成内存浪费。
为了解决不连续内存（NUMA架构）造成的内存浪费问题，Linux在1999年引入了一种新的内存模型，
这就是DISCONTIGMEM。其是通过编译的时候设置CONFIG_DISCONTIGMEM配置项来开启的。针对
FLATMEM模型在不连续内存带来的浪费，DISCONTIGMEM的解决思路也挺简单的，就是每个不连续的
node都维护一个mem_map，而不是使用一个全局的mem_map，这样就避免mem_map有大量的空洞地址
映射.以上解决问题的思路很像分级页表，通过多级页表的方式来减少内存消耗。但这里也引出了
一个问题：相比于FLATMEM，同样是给定一个页帧号（pfn），怎么找到对应的struct page呢？因
为中间多了一层索引，怎么知道这个pfn是在哪个node里面的呢？
在Linux-2.6.25前，DISCONTIGMEM是通过将pfn左移PAGE_SHIFT（默认是12）位后，计算出该页帧
的真实物理地址，再通过哈希映射表找到对应的node id号。在pfn_to_page中，实际就是将pfn减
去该node的起始pfn，所计算得的值就是node_mem_map的下标值；而在page_to_pfn则是相反的过程
，直接从struct page的flag中获取到nid值，然后将page地址减去该node的node_mem_map起始地址
，计算到的就是pfn相对于该node的页帧号，最终加上该node起始页帧号，结果就是全局页帧号。
注意：在x86_64架构中，Linux-2.6.25后CONFIG_DISCONTIGMEM模型phys_to_nid函数的实现已经被
删除，换句话说Linux-2.6.25后的版本已经不再支持CONFIG_DISCONTIGMEM模型了（准确来说应该
只支持SPARSEMEM了）。经过反复确认，Linux 5.10.68 x86_64架构关于CONFIG_DISCONTIGMEM的代
码都无效了。DISCONTIGMEM模型同样存在不小的弊端：紧凑型线性映射和不支持内存热拔插。
DISCONTIGMEM模型本质是一个node上的FLATMEM，随着node的增加或者内存热拔插长场景的出现，
同一个node内，也可能出现大量不连续内存，导致DISCONTIGMEM模型开销越来越大。这时候，一个
全新的稀松内存模型(sparse memory model)被引入到内核中。SPARSEMEM模型使用一个struct
mem_section **mem_section的二维数组来记录内存布局，数组中每一个一级指针都指向一页的物
理内存空间，对应的是 PAGE_SIZE / sizeof(struct mem_section)个mem_section；因为要设定一
个mem_section对应128M(2^27)的物理内存，所以一个mem_section需要对应128M / 4k = 2^(27 -
12)个struct page。每一个页框都有一个struct page结构体对应。
*/

#if defined(CONFIG_FLATMEM)

#ifndef ARCH_PFN_OFFSET
/*体系结构逻辑起始页帧号从0开始*/
#define ARCH_PFN_OFFSET		(0UL)
#endif

#elif defined(CONFIG_DISCONTIGMEM)

#ifndef arch_pfn_to_nid
#define arch_pfn_to_nid(pfn)	pfn_to_nid(pfn)
#endif

#ifndef arch_local_page_offset
#define arch_local_page_offset(pfn, nid)	\
	((pfn) - NODE_DATA(nid)->node_start_pfn)
#endif

#endif /* CONFIG_DISCONTIGMEM */

/*支持三种内存模型*/
#if defined(CONFIG_FLATMEM)
/*查找页号对应的struct page实例，获取全局页帧实例数组中偏移项即可*/
#define __pfn_to_page(pfn)	(mem_map + ((pfn) - ARCH_PFN_OFFSET))
/*根据struct page实例获取对应的页帧号。先计算该页在全局页帧实例数组中的索引值，
然后再计算该值在在整个页帧编号的偏移*/
#define __page_to_pfn(page)	((unsigned long)((page) - mem_map) + ARCH_PFN_OFFSET)
#elif defined(CONFIG_DISCONTIGMEM)

#define __pfn_to_page(pfn)			\
({	unsigned long __pfn = (pfn);		\
	unsigned long __nid = arch_pfn_to_nid(pfn);  \
	NODE_DATA(__nid)->node_mem_map + arch_local_page_offset(__pfn, __nid);\
})

#define __page_to_pfn(pg)						\
({	struct page *__pg = (pg);					\
	struct pglist_data *__pgdat = NODE_DATA(page_to_nid(__pg));	\
	(unsigned long)(__pg - __pgdat->node_mem_map) +			\
	 __pgdat->node_start_pfn;					\
})

#elif defined(CONFIG_SPARSEMEM_VMEMMAP)

/* memmap is virtually contigious.  */
#define __pfn_to_page(pfn)	(vmemmap + (pfn))
#define __page_to_pfn(page)	((page) - vmemmap)

#elif defined(CONFIG_SPARSEMEM)
/*
 * Note: section's mem_map is encorded to reflect its start_pfn.
 * section[i].section_mem_map == mem_map's address - start_pfn;
 */
#define __page_to_pfn(pg)					\
({	struct page *__pg = (pg);				\
	int __sec = page_to_section(__pg);			\
	(unsigned long)(__pg - __section_mem_map_addr(__nr_to_section(__sec)));	\
})

#define __pfn_to_page(pfn)				\
({	unsigned long __pfn = (pfn);			\
	struct mem_section *__sec = __pfn_to_section(__pfn);	\
	__section_mem_map_addr(__sec) + __pfn;		\
})
#endif /* CONFIG_FLATMEM/DISCONTIGMEM/SPARSEMEM */

#ifdef CONFIG_OUT_OF_LINE_PFN_TO_PAGE
struct page;
/*当内联的pfn_to_page太大时该函数很有用*/
extern struct page *pfn_to_page(unsigned long pfn);
extern unsigned long page_to_pfn(struct page *page);
#else
#define page_to_pfn __page_to_pfn
#define pfn_to_page __pfn_to_page
#endif /* CONFIG_OUT_OF_LINE_PFN_TO_PAGE */

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL__ */

#endif
