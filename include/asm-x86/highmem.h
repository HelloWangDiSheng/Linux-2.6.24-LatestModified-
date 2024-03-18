/*�߶��ڴ���������ں��ڴ�ӳ�䡣���ڲ��ܱ�ֱ���ں������ַѰַ��CONFIG_HIGHMEMϵͳ
�ڴ�ҳ��������X86 32λ����ܹ����Դ���ߴ�16TB�������ڴ档��ǰx86cpu���֧��64GB��
��RAM*/
#ifndef _ASM_HIGHMEM_H
#define _ASM_HIGHMEM_H

#ifdef __KERNEL__

#include <linux/interrupt.h>
#include <linux/threads.h>
#include <asm/kmap_types.h>
#include <asm/tlbflush.h>
#include <asm/paravirt.h>
/*�����߶��ڴ������ʼ��ַ��Ӧ��ҳ���*/
extern unsigned long highstart_pfn;
/*�����߶��ڴ���Ľ�����ַ��Ӧ��ҳ���*/
extern unsigned long highend_pfn;

extern pte_t *kmap_pte;
extern pgprot_t kmap_prot;
extern pte_t *pkmap_page_table;

/*��ǰֻ��ʼ����һ��PTE���ñ��������չ����subsequent pte��ֻ�ܷ�����һ��RAM�����*/
/*����־�ӳ���ڴ����������ڴ�ҳ����Ŀ*/
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
/*�ڹ̶�ӳ��������λ��֮������һ��ҳ�����ں������ڴ�ռ��е����һ�����򣬳�Ϊ��ʱ
ӳ��������ô�����ʱӳ������������ʲô���أ�һ����Ҫʹ��kmap_atomic������ҳ��ʱӳ��
���ں˿ռ��һ�������ַ�ϣ���������ַ��λ���ں������ڴ�ռ��е���ʱӳ�����ϣ�Ȼ��
���û��ռ�ָ���������еĴ�д������ͨ�����ӳ��������ַ������pagecache�е���Ӧ����
ҳ�С���ʱ�ļ���д��������Ѿ�����ˡ���������ʱӳ�䣬�����ڿ������֮�󣬵���
kunmap_atomic�����ӳ���ٽ������*/

/*�־�ӳ���ڴ������ڽ��߶��ڴ����еķǳ־�ҳӳ�䵽�ں��С���������LAST_PKMAP��ҳ����
���Ĺ̶�ӳ�������м����һ����������ҳ*/
#define PKMAP_BASE ( (FIXADDR_BOOT_START - PAGE_SIZE*(LAST_PKMAP + 1)) & PMD_MASK )
/*�־�ӳ���ڴ�����ҳ����*/
#define LAST_PKMAP_MASK (LAST_PKMAP-1)
/*��ȡ�־�ӳ�������еĵ�ַ��Ӧ��ҳ���*/
#define PKMAP_NR(virt)  ((virt-PKMAP_BASE) >> PAGE_SHIFT)
/*��ȡ�־�ӳ��������ҳ��Ŷ�Ӧ�������ַ*/
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
