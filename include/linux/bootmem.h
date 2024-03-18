/*֧�ֲ������ڴ�*/
#ifndef _LINUX_BOOTMEM_H
#define _LINUX_BOOTMEM_H

#include <linux/mmzone.h>
#include <asm/dma.h>

/*ϵͳ�����ڼ䣬�����ڴ������δ��ʼ�������ں���Ȼ��Ҫ�����ڴ��Դ����������ݽṹ��
bootmem�����������������׶����ڷ����ڴ档ʵ�ֵ���һ��������������������������׶ι�
���ڴ档�÷�����ʹ��һ��λͼ������ҳ��λͼ����λ����Ŀ��ϵͳ�������ڴ�ҳ����Ŀ��ͬ��
����λΪ1����ʾҳ�ѱ�ʹ�ã�����λΪ0����ʾҳ���ڿ���״̬������Ҫ�����ڴ�ʱ����������
λɨ��λͼ��ֱ���ҵ�һ���ܹ��ṩ�㹻����ҳ��λ�ã�����ν��������ѣ�first-best������
�ȣ�first-fit������λ�á��ù��̲��Ǻܸ�Ч����Ϊÿ�η��䶼�����ͷɨ�����λ����ˣ���
�ں���ȫ��ʼ��֮�󣬲��ܽ��÷����������ڴ�������ϵͳ��buddy system����ͬslab��slob
��slub��������һ���õĶ�ı�ѡ����*/
extern unsigned long max_low_pfn;
extern unsigned long min_low_pfn;

/*���ҳ*/
extern unsigned long max_pfn;

#ifdef CONFIG_CRASH_DUMP
extern unsigned long saved_max_pfn;
#endif

/*�ںˣ�Ϊϵͳ�е�ÿ����㶼���ṩ��һ���������������������ϵͳ����ʱ��ֱ��ӳ����
�ڴ棬�ýṹ������ڴ��޷���̬���䣬�����ڱ���ʱ������ں�*/
typedef struct bootmem_data
{
	/*����ϵͳ�е�һ��ҳ�������ַ������PLKA����˵�ĵ�һ��ҳ��ţ����������ϵ�ṹ�¶�����*/
	unsigned long node_boot_start;
	/*�������ֱ�ӹ���������ַ�ռ������һ��ҳ�ı�š�Ҳ����ֱ��ӳ�����Ľ���ҳ*/
	unsigned long node_low_pfn;
	/*ָ��洢����λͼ���ڴ���ָ�룬λͼ�е�����λ��������ն��Ľ���ϵ����������ڴ�ҳ
	����IA-32ϵͳ�ϣ����ڸ���;���ڴ����������ں�ӳ��֮�󣬶�Ӧ�ĵ�ַ������_end�����У�
	�ñ����������ڼ��Զ��ز��뵽�ں�ӳ����*/
	void *node_bootmem_map;
	/*�����ϴη���ʱ��ƫ���������Ϊ�㣬��˵���ϴη������һ��ҳ*/
	unsigned long last_offset;
	/*�����ϴη����ҳ�ı�š����û�������������ҳ����last_offset������ҳ�ڲ���ƫ��
	������ʹ��bootmem���������Է���С��һ��ҳ���ڴ��������ϵͳ�޷�������һ�㣩��*/
	unsigned long last_pos;
	/*Ϊ�˼ӿ�Ѱ�ң�����λͼ���ϴγɹ������ڴ��λ�á��µķ��佫�ɴ˿�ʼ*/
	unsigned long last_success;
	/*�ڴ治������ϵͳ������Ҫ���bootmem��������һ�����͵�����ʱNUMA�����������ÿ��
	���ע����һ��bootmem������������������ڴ��ַ�ռ���ɢ���Ž�С�Ŀն���Ҳ����Ϊÿ
	�������ڴ���ע��һ��bootmem������*/
	struct list_head list;
} bootmem_data_t;

extern unsigned long bootmem_bootmap_pages(unsigned long);
extern unsigned long init_bootmem(unsigned long addr, unsigned long memend);
extern void free_bootmem(unsigned long addr, unsigned long size);
extern void *__alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal);
extern void *__alloc_bootmem_nopanic(unsigned long size, unsigned long align, unsigned long goal);
extern void *__alloc_bootmem_low(unsigned long size, unsigned long align, unsigned long goal);
extern void *__alloc_bootmem_low_node(pg_data_t *pgdat,	unsigned long size, unsigned long align,
				      						unsigned long goal);
extern void *__alloc_bootmem_core(struct bootmem_data *bdata, unsigned long size,
				  unsigned long align, unsigned long goal, unsigned long limit);

#ifndef CONFIG_HAVE_ARCH_BOOTMEM_NODE
extern void reserve_bootmem(unsigned long addr, unsigned long size);
/*ϵͳ�����ڼ�������ڴ���䡣��ZONE_DMA����λ�ÿ�ʼ������һ��x�ֽڳ���Ӳ�������ж�
����ڴ���*/
#define alloc_bootmem(x) __alloc_bootmem(x, SMP_CACHE_BYTES, __pa(MAX_DMA_ADDRESS))
/*ϵͳ�����ڼ�������ڴ���䡣��ͷ��ʼ������һ��x�ֽڳ���Ӳ�������ж�����ڴ���*/
#define alloc_bootmem_low(x)	__alloc_bootmem_low(x, SMP_CACHE_BYTES, 0)
/*ϵͳ�����ڼ�������ڴ���䡣��ZONE_DMA����λ�ÿ�ʼ������һ��x�ֽڳ���ҳ���ȶ����
�ڴ���*/
#define alloc_bootmem_pages(x)	__alloc_bootmem(x, PAGE_SIZE, __pa(MAX_DMA_ADDRESS))
/*ϵͳ�����ڼ�������ڴ���䡣��ͷ��ʼ������һ��x�ֽڳ���ҳ���ȶ�����ڴ���*/
#define alloc_bootmem_low_pages(x)	__alloc_bootmem_low(x, PAGE_SIZE, 0)
#endif /* !CONFIG_HAVE_ARCH_BOOTMEM_NODE */

extern unsigned long free_all_bootmem(void);
extern unsigned long free_all_bootmem_node(pg_data_t *pgdat);
extern void *__alloc_bootmem_node(pg_data_t *pgdat, unsigned long size, unsigned long align,
				  						unsigned long goal);
extern unsigned long init_bootmem_node(pg_data_t *pgdat, unsigned long freepfn,
										unsigned long startpfn, unsigned long endpfn);
extern void reserve_bootmem_node(pg_data_t *pgdat, unsigned long physaddr, unsigned long size);
extern void free_bootmem_node(pg_data_t *pgdat,	unsigned long addr, unsigned long size);

#ifndef CONFIG_HAVE_ARCH_BOOTMEM_NODE
/*ϵͳ�����ڼ䣬��ָ������ϵ�ZONE_DMA����λ�ÿ�ʼ������һ���ض�������Ӳ�������ж���
���ڴ���*/
#define alloc_bootmem_node(pgdat, x) \
	__alloc_bootmem_node(pgdat, x, SMP_CACHE_BYTES, __pa(MAX_DMA_ADDRESS))
/*ϵͳ�����ڼ䣬��ָ������ϵ�ZONE_DMA����λ�ÿ�ʼ������һ���ض�������ҳ���ȶ����
�ڴ���*/
#define alloc_bootmem_pages_node(pgdat, x) \
	__alloc_bootmem_node(pgdat, x, PAGE_SIZE, __pa(MAX_DMA_ADDRESS))
/*ϵͳ�����ڼ䣬��ָ������ϵ��ڴ���ʼ����ʼ������һ���ض�������ҳ���ȶ�����ڴ���*/
#define alloc_bootmem_low_pages_node(pgdat, x) \
	__alloc_bootmem_low_node(pgdat, x, PAGE_SIZE, 0)
#endif /* !CONFIG_HAVE_ARCH_BOOTMEM_NODE */

#ifdef CONFIG_HAVE_ARCH_ALLOC_REMAP
extern void *alloc_remap(int nid, unsigned long size);
#else
static inline void *alloc_remap(int nid, unsigned long size)
{
	return NULL;
}
#endif /* CONFIG_HAVE_ARCH_ALLOC_REMAP */

extern unsigned long __meminitdata nr_kernel_pages;
extern unsigned long __meminitdata nr_all_pages;

extern void *alloc_large_system_hash(const char *tablename,
				     unsigned long bucketsize,
				     unsigned long numentries,
				     int scale,
				     int flags,
				     unsigned int *_hash_shift,
				     unsigned int *_hash_mask,
				     unsigned long limit);

#define HASH_EARLY	0x00000001	/* Allocating during early boot? */

/*��NUMA��Ҫɢ�з��䡣IA-64��X86_64���㹻��VMALLOC�ռ�*/
#if defined(CONFIG_NUMA) && (defined(CONFIG_IA64) || defined(CONFIG_X86_64))
#define HASHDIST_DEFAULT 1
#else
#define HASHDIST_DEFAULT 0
#endif
extern int hashdist;		/* Distribute hashes across NUMA nodes? */


#endif /* _LINUX_BOOTMEM_H */
