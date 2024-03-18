#include <linux/mm.h>
#include <linux/module.h>
#include <linux/swap.h>
#include <linux/bio.h>
#include <linux/pagemap.h>
#include <linux/mempool.h>
#include <linux/blkdev.h>
#include <linux/init.h>
#include <linux/hash.h>
#include <linux/highmem.h>
#include <linux/blktrace_api.h>
#include <asm/tlbflush.h>

/*����������������ļ�����0��ζ�����ҳû�б�ӳ�䣬�Դ�һ��TLB��������û�б�ӳ�䡣1
��ζ��û���û�ʹ�ã��Դ��ϴ�TBLˢ�����������Ѿ�����ӳ�䣬������cpu��TLBû�и��¶���
��ʹ�á�n��ζ���е�ǰn-1���û�����ʹ����*/
#ifdef CONFIG_HIGHMEM

/*�߶��ڴ�����ҳ��Ŀ�ܼƣ��󲿷������ֻ��*/
unsigned long totalhigh_pages __read_mostly;

/*����������߽��ĸ߶��ڴ���ZONE_HIGHMEM�������ZONE_MOVABLE�����Ҹ������е�ҳ
����ZONE_HIGHMEM�ڴ���ҲӦ���ۼƣ��п���ҳ����Ŀ*/
unsigned int nr_free_highpages (void)
{
	pg_data_t *pgdat;
	unsigned int pages = 0;
	/*����ÿһ�����߽��*/
	for_each_online_pgdat(pgdat)
	{
		/*��ȡZONE_HIGHMEM�ڴ����п���ҳ����Ŀ*/
		pages += zone_page_state(&pgdat->node_zones[ZONE_HIGHMEM], NR_FREE_PAGES);
		/*���ZONE_MOVABLE�����Ҹ��ڴ����е�ҳȡ�Ը߶��ڴ���Ҳ�ۼƸ��ڴ���Ŀ���ҳ*/
		if (zone_movable_is_highmem())
			pages += zone_page_state(		&pgdat->node_zones[ZONE_MOVABLE], 		NR_FREE_PAGES);
	}
	return pages;
}

/*page_addresses_map��Ӧ������ü���*/
/*pkmap_count��һ����ΪLAST_PKMAP���������飬����ÿ��Ԫ�ض���Ӧ��һ���־�ӳ��ҳ����ʵ
�����Ǳ�ӳ��ҳ��һ��ʹ�ü��������ü������������ں�ʹ�ø�ҳ�Ĵ�����1�����������ֵΪ2��
���ں���ֻ��һ��ʹ�ø�ӳ��ҳ��������ֵΪ5��ʾ��4��ʹ�á�һ���˵��������ֵΪn�����ں�
����n-1��ʹ�ø�ҳ����ͨ����ʹ�ü�����һ����0��ζ����ص�ҳû��ʹ�á�������ֵ1��������
�塣���ʾ��λ�ù�����ҳ�Ѿ�ӳ�䣬������CPU��TLBû�и��¶��޷�ʹ�ã���ʱ���ʸ�ҳ����
��ʧ�ܣ����߻���ʵ�һ������ȷ�ĵ�ַ��*/
static int pkmap_count[LAST_PKMAP];
/*�ϴη���pkmap_count�����������*/
static unsigned int last_pkmap_nr;
/*pkmap_count���鱣����*/
static  __cacheline_aligned_in_smp DEFINE_SPINLOCK(kmap_lock);
/**/
pte_t * pkmap_page_table;
/*���岢��ʼ��һ���ȴ�����*/
static DECLARE_WAIT_QUEUE_HEAD(pkmap_map_wait);


/*flush_all_zero_pkmaps�������ͷ�ӳ��Ĺؼ�����map_new_virtual��ͷ��ʼ��������λ��ʱ
�����ǵ��øú���������������3��������(1)  flush_cache_kmaps���ں�ӳ����ִ��ˢ��������
Ҫ��ʽˢ���Ĵ������ϵ�ṹ�ϣ���ʹ��flush_cache_allˢ��CPU��ȫ���ĸ��ٻ��棩����Ϊ��
�˵�ȫ��ҳ���Ѿ��޸ġ� ����һ�����ۺܸߵĲ��������˵�����ദ������ϵ�ṹ����Ҫ�ò�
��������������£�������Ϊ�ղ�����(2) ɨ������pkmap_count���顣������ֵΪ1��������Ϊ
0����ҳ��ɾ����ص�����ɾ����ӳ�䡣(3) ���ʹ��flush_tlb_kernel_range����ˢ����
����PKMAP������ص�TLB��*/
static void flush_all_zero_pkmaps(void)
{
	int i;

	flush_cache_kmaps();
	/*��pkmap_count����ͷ��ʼ����*/
	for (i = 0; i < LAST_PKMAP; i++)
	{
		struct page *page;
		/*0��ζ�����ǲ���Ҫ��ʲô������1��ζ�Ÿ�ҳ��Ȼ��ʹ��֮�У�1��ζ�Ÿ�ҳ�ǿ��е�
		��Ҫ�����ӳ��*/
		if (pkmap_count[i] != 1)
			continue;
		/*��ӳ��δʹ�ã�����ӳ��*/
		pkmap_count[i] = 0;

		/* sanity check */
		BUG_ON(pte_none(pkmap_page_table[i]));

		/*
		 * Don't need an atomic fetch-and-clear op here;
		 * no-one has the page mapped, and cannot get at
		 * its virtual address (and hence PTE) without first
		 * getting the kmap_lock (which is held here).
		 * So no dangers, even with speculative execution.
		 */
		/*��ȡҳ�����Ӧ��ҳʵ��*/
		page = pte_page(pkmap_page_table[i]);
		/*��ҳ��Ӧ��pteҳ�������*/
		pte_clear(&init_mm, (unsigned long)page_address(page), &pkmap_page_table[i]);
		/*���ҳ��ӳ��*/
		set_page_address(page, NULL);
	}
	/**/
	flush_tlb_kernel_range(PKMAP_ADDR(0), PKMAP_ADDR(LAST_PKMAP));
}

/* Flush all unused kmap mappings in order to remove stray mappings. */
void kmap_flush_unused(void)
{
	spin_lock(&kmap_lock);
	flush_all_zero_pkmaps();
	spin_unlock(&kmap_lock);
}

/*����ҳʵ���������ַ֮���ӳ��*/
/*ʹ��map_new_virtualӳ��ҳ���ú�����ִ��������Ҫ�Ĳ��裺(1) �����ʹ�õ�λ�ã�������
ȫ�ֱ���last_pkmap_nr�У���ʼ������ɨ��pkmap_count���飬ֱ���ҵ�һ������λ�á����û
�п���λ�ã��ú�������˯��״̬��ֱ���ں˵���һ����ִ�н��ӳ������ڳ���λ���ڵ���
pkmap_count���������ֵʱ��������λ��0��ʼ������������£�������flush_all_zero_pkmaps
����ˢ��CPU���ٻ��档(2) �޸��ں˵�ҳ������ҳӳ����ָ��λ�á�����δ����TLB��(3) ��
λ�õ�ʹ�ü���������Ϊ1����ҳ�ѷ��䵫�޷�ʹ�ã���ΪTLB��δ���¡�(4) set_page_address
����ҳ��ӵ��־��ں�ӳ������ݽṹ���ú���������ӳ��ҳ�������ַ���ڲ���Ҫ�߶��ڴ�ҳ
����ϵ�ṹ�ϣ���û������CONFIG_HIGHMEM������ʹ��ͨ�ð汾��kmap����ҳ�ĵ�ַ���Ҳ��޸�
�����ڴ�*/
static inline unsigned long map_new_virtual(struct page *page)
{
	unsigned long vaddr;
	int count;

start:
	/**/
	count = LAST_PKMAP;
	/*��pkmap_count�����������ѯδ����ӳ��Ŀ���λ*/
	for (;;)
	{
		/*���ϴ��ѷ��ʵĿ��н����һ�ʼ*/
		last_pkmap_nr = (last_pkmap_nr + 1) & LAST_PKMAP_MASK;
		/*����pkmap_count����ͷʱ����Ҫˢ��cpu���ٻ���*/
		if (!last_pkmap_nr)
		{
			flush_all_zero_pkmaps();
			count = LAST_PKMAP;
		}
		/*�����pkmap_count���ҵ�һ������λ�ã�˵����λ�ö�Ӧ�������ַû��ʹ�ã�����
		ʹ�ø������ַ������ҳ��ַ��ӳ��*/
		if (!pkmap_count[last_pkmap_nr])
			break;
		/*��������pkmap_count����λ*/
		if (--count)
			continue;

		/*
		 * Sleep for somebody else to unmap their entries
		 */
		/*����˯��ֱ�����������ͷ�ӳ��*/
		{
			DECLARE_WAITQUEUE(wait, current);

			__set_current_state(TASK_UNINTERRUPTIBLE);
			add_wait_queue(&pkmap_map_wait, &wait);
			spin_unlock(&kmap_lock);
			schedule();
			remove_wait_queue(&pkmap_map_wait, &wait);
			spin_lock(&kmap_lock);

			/* Somebody else might have mapped it while we slept */
			if (page_address(page))
				return (unsigned long)page_address(page);

			/* Re-start */
			goto start;
		}
	}
	vaddr = PKMAP_ADDR(last_pkmap_nr);
	set_pte_at(&init_mm, vaddr,
		   &(pkmap_page_table[last_pkmap_nr]), mk_pte(page, kmap_prot));

	pkmap_count[last_pkmap_nr] = 1;
	set_page_address(page, (void *)vaddr);

	return vaddr;
}

/*ָ��ҳʵ�����û�н���ӳ�䣬����ҳʵ���������ַ֮���ӳ�䣬���¸�ҳʵ����Ӧ����
���ַ��ӳ�����*/
void fastcall *kmap_high(struct page *page)
{
	unsigned long vaddr;

	/*���ڸ߶��ڴ�ҳ������Ҫ���������²���������Ҳ���ܴӿ��ܱ��������ж��е���
	��������ҳ��*/
	spin_lock(&kmap_lock);
	/*��ȡҳʵ����Ӧ�������ڴ��ַ*/
	vaddr = (unsigned long)page_address(page);
	/*�����û�н�����ҳ�������ַ��ӳ�䣬����ӳ��*/
	if (!vaddr)
		vaddr = map_new_virtual(page);
	/*����ҳ��Ӧ�������ַ���ü�������1*/
	pkmap_count[PKMAP_NR(vaddr)]++;
	/**/
	BUG_ON(pkmap_count[PKMAP_NR(vaddr)] < 2);
	spin_unlock(&kmap_lock);
	return (void*) vaddr;
}

EXPORT_SYMBOL(kmap_high);

/**/
void fastcall kunmap_high(struct page *page)
{
	unsigned long vaddr;
	unsigned long nr;
	int need_wakeup;

	spin_lock(&kmap_lock);
	/*��ȡҳʵ����Ӧ�������ַ*/
	vaddr = (unsigned long)page_address(page);
	/*��Ҫ���ӳ���ҳʵ��Ϊ��ʱ��bug*/
	BUG_ON(!vaddr);
	/*��ȡ�������ַ��ʹ�õĴ���*/
	nr = PKMAP_NR(vaddr);

	/*û��һ��TLBˢ��һ��ҳ�����ü������ܱ�Ϊ0
	 * A count must never go down to zero without a TLB flush!
	 */
	need_wakeup = 0;
	switch (--pkmap_count[nr])
	{
		case 0:
			BUG();
		case 1:
			/*����û�б�Ҫ��wakp_up�������ã�����������ʱpkmap_count[]=1������ʱû��
			�ȴ����̡��ȴ������е��Ŷӽ��̱�wait_queue_head_t��Ա����kmap_lock��������
			��Ϊ�ú����ѳ���kmap_lock������˾�û�б��г���wait_queue_head_t��Ա����
			�������Ϊ����򵥲���*/
			/*���Եȴ��������Ƿ��еȴ�����*/
			need_wakeup = waitqueue_active(&pkmap_map_wait);
	}
	spin_unlock(&kmap_lock);

	/* do wake-up, if needed, race-free outside of the spin lock */
	if (need_wakeup)
		wake_up(&pkmap_map_wait);
}

EXPORT_SYMBOL(kunmap_high);
#endif

#if defined(HASHED_PAGE_VIRTUAL)

/*128λɢ�м�ֵ*/
#define PA_HASH_ORDER		7

/*�ں������������ݽṹ�������������ڴ��pageʵ�������������ڴ�����λ��֮��Ĺ�����
�ýṹ������page->virtual��ӳ��*/
struct page_address_map
{
	/*һ��ָ��ȫ��mem_map�����е�pageʵ����ָ��*/
	struct page *page;
	/*ָ����ҳ���ں������ַ�ռ��з����λ��*/
	void *virtual;
	/*Ϊ�˱�����֯��ӳ�䱣����ɢ�б��У��ṹ�е�����Ԫ�����ڽ�����������Դ���ɢ��
	��ײ*/
	struct list_head list;
};

/*page_address_map����������page_address_maps����*/
static struct list_head page_address_pool;
/*������page_address_pool*/
static spinlock_t pool_lock;

/*smp��L1�����ж����page_address_htableɢ�б����飬ɢ�б�ͨ��������ʵ��*/
static struct page_address_slot
{
	/*page_address_maps�������������������ɢ����ײ*/
	struct list_head lh;
	/*����������������*/
	spinlock_t lock;
} ____cacheline_aligned_in_smp page_address_htable[1<<PA_HASH_ORDER];

/*����ҳʵ����ַ��ͨ����ƽ�ָ��������ѧ���㣬��ȡ��[0, (1<<PA_HASH_SHIFT)-1]֮��
��������Ȼ����ݸ�����ȷ������ɢ�б��еĵ�ַ*/
static struct page_address_slot *page_slot(struct page *page)
{
	return &page_address_htable[hash_ptr(page, PA_HASH_ORDER)];
}

/*��ȡָ��ҳ�������ַ�����ҳ�ǷǸ߶��ڴ�ҳ����ֱ�ӻ�ȡ��ֱ��ӳ��������ַ������
�����ҳʵ����ַ��ȡ��ҳ��Ӧ��ɢ�б�����ɢ�б��в�ѯ��ҳ�Ƿ��Ѵ��ڣ������ҳ����
��ɢ�б��У��򷵻ظ�ҳ��Ӧ�������ַ�����򣬷���NULL*/
void *page_address(struct page *page)
{
	unsigned long flags;
	void *ret;
	struct page_address_slot *pas;
	/*���ҳ�ǷǸ߶��ڴ����е�ҳ�����ȡ��ҳ��ֱ��ӳ�����е������ַ*/
	if (!PageHighMem(page))
		return lowmem_page_address(page);
	/*��ȡ�ø߶��ڴ�ҳ������ɢ�б�ͷ*/
	pas = page_slot(page);
	ret = NULL;
	spin_lock_irqsave(&pas->lock, flags);
	/*�����ҳ��Ӧɢ�б�ǿգ����ѯ��ҳ�Ƿ��Ѿ������ڸ�ɢ�б���*/
	if (!list_empty(&pas->lh))
	{
		struct page_address_map *pam;
		/*������ɢ�б���ѯ��ҳ��Ӧ�������ַ*/
		list_for_each_entry(pam, &pas->lh, list)
		{
			/*�����ҳ��ɢ�б��У���ȡ�������ַ*/
			if (pam->page == page)
			{
				ret = pam->virtual;
				goto done;
			}
		}
	}
done:
	spin_unlock_irqrestore(&pas->lock, flags);
	return ret;
}
EXPORT_SYMBOL(page_address);

/*����ҳʵ����ָ�������ַ֮���ӳ�䣬ҳ�����ǷǸ߶��ڴ�ҳ�����ָ��ӳ��������ַ
��Ч����ӿ���ӳ�����ѡ��һ�����н�㣬��������ҳʵ���������ַ֮���ӳ�䣬Ȼ��
��page_address_mapʵ����ӵ���Ӧ��ɢ�б�ĩβ�����ָ���������ַ��Ч�����ѯɢ�б�
���Ƿ��Ѵ��ڸ�ҳ�ѽ������ӳ�䣬������ɾ����ӳ�䣬������page_address_mapʵ�������
��ӳ�����*/
void set_page_address(struct page *page, void *virtual)
{
	unsigned long flags;
	struct page_address_slot *pas;
	struct page_address_map *pam;
	/*pageֻ���Ǹ߶��ڴ�����ҳ�棬������ֱ��ӳ������ҳ��*/
	BUG_ON(!PageHighMem(page));
	/*����gageʵ����ַ��ȡ��������page_addeesses_htableɢ��ͷ*/
	pas = page_slot(page);
	/*�����ҳ�����������ַ�ǿգ���ӿ���������л�ȡ��һ��struct page_address_map
	ʵ�������ú�������ҳʵ��ָ��������ַ��ʼ����������ҳ���뵽��Ӧ��ɢ�б�ĩβ*/
	if (virtual)
	{
		/*page_address_poolȫ�ֿ���������*/
		BUG_ON(list_empty(&page_address_pool));
		/*�����������������ж�*/
		spin_lock_irqsave(&pool_lock, flags);
		/*��ȡȫ�ֿ��������ϵ�һ������Ӧ��struct page_address_mapʵ��ָ��*/
		pam = list_entry(page_address_pool.next,	struct page_address_map, list);
		/*���ý��ӿ����������ɾ��*/
		list_del(&pam->list);
		spin_unlock_irqrestore(&pool_lock, flags);
		/*��ʼ���ѻ�ȡ��struct page_address_mapʵ��*/
		pam->page = page;
		pam->virtual = virtual;
		/*�������������ʼ����struct page_address_mapʵ�����뵽��Ӧ��ɢ�б�λ*/
		spin_lock_irqsave(&pas->lock, flags);
		list_add_tail(&pam->list, &pas->lh);
		spin_unlock_irqrestore(&pas->lock, flags);
	}
	/*��������Ӧҳʵ���������ַΪ�գ������ҳ������page_address_htableɢ�б��У�
	�򽫸�ҳ�Ӹ�ɢ�б���ɾ����������struct page_address_mapʵ����ӵ����г���*/
	else
	{
		spin_lock_irqsave(&pas->lock, flags);
		list_for_each_entry(pam, &pas->lh, list)
		{
			/*���ҳʵ��������ɢ�б��У��򽫸�ҳ��Ӧ��struct page_address_mapʵ����
			ɢ�б���ɾ����������ҳ��Ӧ��page_address_mapʵ����ӵ����г�ĩβ*/
			if (pam->page == page)
			{
				list_del(&pam->list);
				spin_unlock_irqrestore(&pas->lock, flags);
				spin_lock_irqsave(&pool_lock, flags);
				list_add_tail(&pam->list, &page_address_pool);
				spin_unlock_irqrestore(&pool_lock, flags);
				goto done;
			}
		}
		spin_unlock_irqrestore(&pas->lock, flags);
	}
done:
	return;
}

/*������֯ҳʵ���������ַ֮��Ĺ�ϵ��ӳ�䱣����ɢ�б��У��ṹ������Ԫ�����ڽ������
�����Դ���ɢ����ײ*/
static struct page_address_map page_address_maps[LAST_PKMAP];

/*��ʼ��pkmap�ڴ��򣬸��������ڳ־�ӳ��*/
void __init page_address_init(void)
{
	int i;
	/*��ʼ��ɢ�г�����*/
	INIT_LIST_HEAD(&page_address_pool);
	/*ͷ�巨��page_addresses_maps�����Աlist���ӵ�page_address_pool����ѭ�������
	��page_address_maps�����Ա����������page_address_pool������*/
	for (i = 0; i < ARRAY_SIZE(page_address_maps); i++)
		list_add(&page_address_maps[i].list, &page_address_pool);
	/*��ʼ��page_address_htable������ɢ��ͷ�ͱ�������Ա*/
	for (i = 0; i < ARRAY_SIZE(page_address_htable); i++)
	{
		INIT_LIST_HEAD(&page_address_htable[i].lh);
		spin_lock_init(&page_address_htable[i].lock);
	}
	/*��ʼ��page_addresses_pool������*/
	spin_lock_init(&pool_lock);
}

#endif	/* defined(CONFIG_HIGHMEM) && !defined(WANT_PAGE_VIRTUAL) */
