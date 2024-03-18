#ifndef _LINUX_VMALLOC_H
#define _LINUX_VMALLOC_H

#include <linux/spinlock.h>
#include <asm/page.h>

struct vm_area_struct;

/*vm_struct->flags标识位*/
/*表示将几乎随机的物理内存区域映射到vmalloc区域中，这个一个特定于体系结构的操作*/
#define VM_IOREMAP		0x00000001
/*指定由vmalloc产生的子区域*/
#define VM_ALLOC		0x00000002
/*表示将现存pages集合映射到连续的虚拟地址空间中*/
#define VM_MAP			0x00000004
/*suitable for remap_vmalloc_range*/
#define VM_USERMAP		0x00000008
/*buffer for pages was vmalloc'ed*/
#define VM_VPAGES		0x00000010
/*20至32比特位为体系结构内部特别的ioremap保留*/

/*可被特定体系结构修改的ioremap()区域的最大的对齐长度*/
#ifndef IOREMAP_MAX_ORDER
#define IOREMAP_MAX_ORDER		(7 + PAGE_SHIFT)	/* 128 pages */
#endif

/*内核管理虚拟地址空间中的VMALLOC区域时，使用该结构跟踪已分配使用的VMALLOC区域*/
struct vm_struct
{
	/*前三个变量放在一块，置于一个缓存行中，便于快速查找*/
	/*将vmalloc区域中所有子区域保存在一个单链表上*/
	struct vm_struct	*next;
	/*分配的子区域在虚拟地址空间中的起始地址*/
	void *addr;
	/*子区域长度*/
	unsigned long size;
	/*存储与该内存区关联的标志集合，它只用于指定内存区类型，如VM_ALLOC指定有vmalloc
	产生的区域*/
	unsigned long flags;
	/*指针数组，每个数组成员都表示为一个映射到虚拟地址空间中的物理内存页的page实例*/
	struct page		**pages;
	/*指定pages数组中数组项数目，即涉及的内存页数目*/
	unsigned int nr_pages;
	/*仅当用ioremap映射了由物理地址描述的物理内存区域时才需要，该信息保存在该变量中*/
	unsigned long phys_addr;
};

/*驱动使用的高级APIs*/
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

/*非驱动使用的低级APIs*/
static inline size_t get_vm_area_size(const struct vm_struct *area)
{
	/*获取vmalloc子区域无保护页的实际分配长度*/
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

/*分配指定大小的vmalloc子区域*/
extern struct vm_struct *alloc_vm_area(size_t size);
/*释放指定的vmalloc子区域*/
extern void free_vm_area(struct vm_struct *area);

/*仅供内核内部使用*/
/*操作vmlist单链表时的保护锁*/
extern rwlock_t vmlist_lock;
/*vmlist为vmalloc区域锁分配子区域的组成的单链表表头*/
extern struct vm_struct *vmlist;

#endif /* _LINUX_VMALLOC_H */
