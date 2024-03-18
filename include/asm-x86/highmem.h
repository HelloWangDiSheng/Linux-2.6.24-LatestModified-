/*高端内存域的虚拟内核内存映射。用在不能被直接内核虚拟地址寻址的CONFIG_HIGHMEM系统
内存页。部署在X86 32位虚拟架构可以处理高达16TB的物理内存。当前x86cpu最多支持64GB物
理RAM*/
#ifndef _ASM_HIGHMEM_H
#define _ASM_HIGHMEM_H

#ifdef __KERNEL__

#include <linux/interrupt.h>
#include <linux/threads.h>
#include <asm/kmap_types.h>
#include <asm/tlbflush.h>
#include <asm/paravirt.h>
/*声明高端内存域的起始地址对应的页编号*/
extern unsigned long highstart_pfn;
/*声明高端内存域的结束地址对应的页编号*/
extern unsigned long highend_pfn;

extern pte_t *kmap_pte;
extern pgprot_t kmap_prot;
extern pte_t *pkmap_page_table;

/*当前只初始化了一个PTE表，该表很容易扩展，子subsequent pte表只能分配在一个RAM物理块*/
/*定义持久映射内存域中物理内存页的数目*/
#ifdef CONFIG_X86_PAE
#define LAST_PKMAP 512
#else
#define LAST_PKMAP 1024
#endif
/*
 * Ordering is:
 *
 * FIXADDR_TOP
 * 			fixed_addresses
 * FIXADDR_START
 * 			temp fixed addresses
 * FIXADDR_BOOT_START
 * 			Persistent kmap area
 * PKMAP_BASE
 * VMALLOC_END
 * 			Vmalloc area
 * VMALLOC_START
 * high_memory
 */
/*在固定映射区结束位置之后的最后一个页，是内核虚拟内存空间中的最后一块区域，称为临时
映射区，那么这块临时映射区是用来干什么的呢？一般需要使用kmap_atomic将缓存页临时映射
到内核空间的一段虚拟地址上，这段虚拟地址就位于内核虚拟内存空间中的临时映射区上，然后
将用户空间指定缓存区中的待写入数据通过这段映射的虚拟地址拷贝到pagecache中的相应缓存
页中。这时文件的写入操作就已经完成了。由于是临时映射，所以在拷贝完成之后，调用
kunmap_atomic将这段映射再解除掉。*/

/*持久映射内存区用于将高端内存域中的非持久页映射到内核中。该区域有LAST_PKMAP个页，与
其后的固定映射区域中间隔了一个（保护）页*/
#define PKMAP_BASE ( (FIXADDR_BOOT_START - PAGE_SIZE*(LAST_PKMAP + 1)) & PMD_MASK )
/*持久映射内存区中页掩码*/
#define LAST_PKMAP_MASK (LAST_PKMAP-1)
/*获取持久映射区域中的地址对应的页编号*/
#define PKMAP_NR(virt)  ((virt-PKMAP_BASE) >> PAGE_SHIFT)
/*获取持久映射区域中页编号对应的虚拟地址*/
#define PKMAP_ADDR(nr)  (PKMAP_BASE + ((nr) << PAGE_SHIFT))

extern void * FASTCALL(kmap_high(struct page *page));
extern void FASTCALL(kunmap_high(struct page *page));

void *kmap(struct page *page);
void kunmap(struct page *page);
void *kmap_atomic_prot(struct page *page, enum km_type type, pgprot_t prot);
void *kmap_atomic(struct page *page, enum km_type type);
void kunmap_atomic(void *kvaddr, enum km_type type);
void *kmap_atomic_pfn(unsigned long pfn, enum km_type type);
struct page *kmap_atomic_to_page(void *ptr);

#ifndef CONFIG_PARAVIRT
#define kmap_atomic_pte(page, type)	kmap_atomic(page, type)
#endif

#define flush_cache_kmaps()	do { } while (0)

#endif /* __KERNEL__ */

#endif /* _ASM_HIGHMEM_H */
