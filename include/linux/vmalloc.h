#ifndef _LINUX_VMALLOC_H
#define _LINUX_VMALLOC_H

#include <linux/spinlock.h>
#include <asm/page.h>

struct vm_area_struct;

/*vm_struct->flags��ʶλ*/
/*��ʾ����������������ڴ�����ӳ�䵽vmalloc�����У����һ���ض�����ϵ�ṹ�Ĳ���*/
#define VM_IOREMAP		0x00000001
/*ָ����vmalloc������������*/
#define VM_ALLOC		0x00000002
/*��ʾ���ִ�pages����ӳ�䵽�����������ַ�ռ���*/
#define VM_MAP			0x00000004
/*suitable for remap_vmalloc_range*/
#define VM_USERMAP		0x00000008
/*buffer for pages was vmalloc'ed*/
#define VM_VPAGES		0x00000010
/*20��32����λΪ��ϵ�ṹ�ڲ��ر��ioremap����*/

/*�ɱ��ض���ϵ�ṹ�޸ĵ�ioremap()��������Ķ��볤��*/
#ifndef IOREMAP_MAX_ORDER
#define IOREMAP_MAX_ORDER		(7 + PAGE_SHIFT)	/* 128 pages */
#endif

/*�ں˹��������ַ�ռ��е�VMALLOC����ʱ��ʹ�øýṹ�����ѷ���ʹ�õ�VMALLOC����*/
struct vm_struct
{
	/*ǰ������������һ�飬����һ���������У����ڿ��ٲ���*/
	/*��vmalloc���������������򱣴���һ����������*/
	struct vm_struct	*next;
	/*������������������ַ�ռ��е���ʼ��ַ*/
	void *addr;
	/*�����򳤶�*/
	unsigned long size;
	/*�洢����ڴ��������ı�־���ϣ���ֻ����ָ���ڴ������ͣ���VM_ALLOCָ����vmalloc
	����������*/
	unsigned long flags;
	/*ָ�����飬ÿ�������Ա����ʾΪһ��ӳ�䵽�����ַ�ռ��е������ڴ�ҳ��pageʵ��*/
	struct page		**pages;
	/*ָ��pages��������������Ŀ�����漰���ڴ�ҳ��Ŀ*/
	unsigned int nr_pages;
	/*������ioremapӳ�����������ַ�����������ڴ�����ʱ����Ҫ������Ϣ�����ڸñ�����*/
	unsigned long phys_addr;
};

/*����ʹ�õĸ߼�APIs*/
extern void *vmalloc(unsigned long size);
extern void *vmalloc_user(unsigned long size);
extern void *vmalloc_node(unsigned long size, int node);
extern void *vmalloc_exec(unsigned long size);
extern void *vmalloc_32(unsigned long size);
extern void *vmalloc_32_user(unsigned long size);
extern void *__vmalloc(unsigned long size, gfp_t gfp_mask, pgprot_t prot);
extern void *__vmalloc_area(struct vm_struct *area, gfp_t gfp_mask, pgprot_t prot);
extern void vfree(void *addr);
extern void *vmap(struct page **pages, unsigned int count, unsigned long flags, pgprot_t prot);
extern void vunmap(void *addr);
extern int remap_vmalloc_range(struct vm_area_struct *vma, void *addr, unsigned long pgoff);
void vmalloc_sync_all(void);

/*������ʹ�õĵͼ�APIs*/
static inline size_t get_vm_area_size(const struct vm_struct *area)
{
	/*��ȡvmalloc�������ޱ���ҳ��ʵ�ʷ��䳤��*/
	return area->size - PAGE_SIZE;
}

extern struct vm_struct *get_vm_area(unsigned long size, unsigned long flags);
extern struct vm_struct *__get_vm_area(unsigned long size, unsigned long flags,
			unsigned long start, unsigned long end);
 extern struct vm_struct *get_vm_area_node(unsigned long size, unsigned long flags,	int node,
 			gfp_t gfp_mask);
extern struct vm_struct *remove_vm_area(void *addr);
extern int map_vm_area(struct vm_struct *area, pgprot_t prot, struct page ***pages);
extern void unmap_kernel_range(unsigned long addr, unsigned long size);

/*����ָ����С��vmalloc������*/
extern struct vm_struct *alloc_vm_area(size_t size);
/*�ͷ�ָ����vmalloc������*/
extern void free_vm_area(struct vm_struct *area);

/*�����ں��ڲ�ʹ��*/
/*����vmlist������ʱ�ı�����*/
extern rwlock_t vmlist_lock;
/*vmlistΪvmalloc�������������������ɵĵ������ͷ*/
extern struct vm_struct *vmlist;

#endif /* _LINUX_VMALLOC_H */
