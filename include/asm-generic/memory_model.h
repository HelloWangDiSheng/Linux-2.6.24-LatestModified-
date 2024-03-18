#ifndef __ASM_MEMORY_MODEL_H
#define __ASM_MEMORY_MODEL_H

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

/*
ʹ��FLATMEM��ģ�ͷǳ���Ч�ͼ򵥣�ֱ�ӽ�����ҳͨ������ӳ����mem_map��Ӧ������������ģ
���и����������⣬�����ڴ��ڴ����ն��ڴ�ĳ����£�mem_map������ܻ�ܴ�����ڴ��˷ѡ�
Ϊ�˽���������ڴ棨NUMA�ܹ�����ɵ��ڴ��˷����⣬Linux��1999��������һ���µ��ڴ�ģ�ͣ�
�����DISCONTIGMEM������ͨ�������ʱ������CONFIG_DISCONTIGMEM�������������ġ����
FLATMEMģ���ڲ������ڴ�������˷ѣ�DISCONTIGMEM�Ľ��˼·Ҳͦ�򵥵ģ�����ÿ����������
node��ά��һ��mem_map��������ʹ��һ��ȫ�ֵ�mem_map�������ͱ���mem_map�д����Ŀն���ַ
ӳ��.���Ͻ�������˼·����ּ�ҳ��ͨ���༶ҳ��ķ�ʽ�������ڴ����ġ�������Ҳ������
һ�����⣺�����FLATMEM��ͬ���Ǹ���һ��ҳ֡�ţ�pfn������ô�ҵ���Ӧ��struct page�أ���
Ϊ�м����һ����������ô֪�����pfn�����ĸ�node������أ�
��Linux-2.6.25ǰ��DISCONTIGMEM��ͨ����pfn����PAGE_SHIFT��Ĭ����12��λ�󣬼������ҳ֡
����ʵ�����ַ����ͨ����ϣӳ����ҵ���Ӧ��node id�š���pfn_to_page�У�ʵ�ʾ��ǽ�pfn��
ȥ��node����ʼpfn��������õ�ֵ����node_mem_map���±�ֵ������page_to_pfn�����෴�Ĺ���
��ֱ�Ӵ�struct page��flag�л�ȡ��nidֵ��Ȼ��page��ַ��ȥ��node��node_mem_map��ʼ��ַ
�����㵽�ľ���pfn����ڸ�node��ҳ֡�ţ����ռ��ϸ�node��ʼҳ֡�ţ��������ȫ��ҳ֡�š�
ע�⣺��x86_64�ܹ��У�Linux-2.6.25��CONFIG_DISCONTIGMEMģ��phys_to_nid������ʵ���Ѿ���
ɾ�������仰˵Linux-2.6.25��İ汾�Ѿ�����֧��CONFIG_DISCONTIGMEMģ���ˣ�׼ȷ��˵Ӧ��
ֻ֧��SPARSEMEM�ˣ�����������ȷ�ϣ�Linux 5.10.68 x86_64�ܹ�����CONFIG_DISCONTIGMEM�Ĵ�
�붼��Ч�ˡ�DISCONTIGMEMģ��ͬ�����ڲ�С�ı׶ˣ�����������ӳ��Ͳ�֧���ڴ��Ȱβ塣
DISCONTIGMEMģ�ͱ�����һ��node�ϵ�FLATMEM������node�����ӻ����ڴ��Ȱβ峤�����ĳ��֣�
ͬһ��node�ڣ�Ҳ���ܳ��ִ����������ڴ棬����DISCONTIGMEMģ�Ϳ���Խ��Խ����ʱ��һ��
ȫ�µ�ϡ���ڴ�ģ��(sparse memory model)�����뵽�ں��С�SPARSEMEMģ��ʹ��һ��struct
mem_section **mem_section�Ķ�ά��������¼�ڴ沼�֣�������ÿһ��һ��ָ�붼ָ��һҳ����
���ڴ�ռ䣬��Ӧ���� PAGE_SIZE / sizeof(struct mem_section)��mem_section����ΪҪ�趨һ
��mem_section��Ӧ128M(2^27)�������ڴ棬����һ��mem_section��Ҫ��Ӧ128M / 4k = 2^(27 -
12)��struct page��ÿһ��ҳ����һ��struct page�ṹ���Ӧ��
*/

#if defined(CONFIG_FLATMEM)

#ifndef ARCH_PFN_OFFSET
/*��ϵ�ṹ�߼���ʼҳ֡�Ŵ�0��ʼ*/
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

/*֧�������ڴ�ģ��*/
#if defined(CONFIG_FLATMEM)
/*����ҳ�Ŷ�Ӧ��struct pageʵ������ȡȫ��ҳ֡ʵ��������ƫ�����*/
#define __pfn_to_page(pfn)	(mem_map + ((pfn) - ARCH_PFN_OFFSET))
/*����struct pageʵ����ȡ��Ӧ��ҳ֡�š��ȼ����ҳ��ȫ��ҳ֡ʵ�������е�����ֵ��
Ȼ���ټ����ֵ��������ҳ֡��ŵ�ƫ��*/
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
/*��������pfn_to_page̫��ʱ�ú���������*/
extern struct page *pfn_to_page(unsigned long pfn);
extern unsigned long page_to_pfn(struct page *page);
#else
#define page_to_pfn __page_to_pfn
#define pfn_to_page __pfn_to_page
#endif /* CONFIG_OUT_OF_LINE_PFN_TO_PAGE */

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL__ */

#endif
