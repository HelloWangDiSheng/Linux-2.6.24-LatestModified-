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

/*虚拟计数不是真正的计数。0意味着相关页没有被映射，自从一个TLB更新以来没有被映射。1
意味着没有用户使用，自从上次TBL刷出操作以来已经建立映射，但由于cpu的TLB没有更新而无
法使用。n意味着有当前n-1个用户正在使用它*/
#ifdef CONFIG_HIGHMEM

/*高端内存域中页数目总计，大部分情况下只读*/
unsigned long totalhigh_pages __read_mostly;

/*获得所有在线结点的高端内存域（ZONE_HIGHMEM）（如果ZONE_MOVABLE存在且该区域中的页
来自ZONE_HIGHMEM内存域，也应该累计）中空闲页的数目*/
unsigned int nr_free_highpages (void)
{
	pg_data_t *pgdat;
	unsigned int pages = 0;
	/*遍历每一个在线结点*/
	for_each_online_pgdat(pgdat)
	{
		/*获取ZONE_HIGHMEM内存域中空闲页的数目*/
		pages += zone_page_state(&pgdat->node_zones[ZONE_HIGHMEM], NR_FREE_PAGES);
		/*如果ZONE_MOVABLE存在且该内存域中的页取自高端内存域，也累计该内存域的空闲页*/
		if (zone_movable_is_highmem())
			pages += zone_page_state(		&pgdat->node_zones[ZONE_MOVABLE], 		NR_FREE_PAGES);
	}
	return pages;
}

/*page_addresses_map对应项的引用计数*/
/*pkmap_count是一容量为LAST_PKMAP的整数数组，其中每个元素都对应于一个持久映射页。它实
际上是被映射页的一个使用计数器。该计数器计算了内核使用该页的次数加1。如果计数器值为2，
则内核中只有一处使用该映射页。计数器值为5表示有4处使用。一般地说，计数器值为n代表内核
中有n-1处使用该页。和通常的使用计数器一样，0意味着相关的页没有使用。计数器值1有特殊语
义。这表示该位置关联的页已经映射，但由于CPU的TLB没有更新而无法使用，此时访问该页，或
者失败，或者会访问到一个不正确的地址。*/
static int pkmap_count[LAST_PKMAP];
/*上次访问pkmap_count数组项的索引*/
static unsigned int last_pkmap_nr;
/*pkmap_count数组保护锁*/
static  __cacheline_aligned_in_smp DEFINE_SPINLOCK(kmap_lock);
/**/
pte_t * pkmap_page_table;
/*定义并初始化一个等待队列*/
static DECLARE_WAIT_QUEUE_HEAD(pkmap_map_wait);


/*flush_all_zero_pkmaps是最终释放映射的关键。在map_new_virtual从头开始搜索空闲位置时
，总是调用该函数。它负责以下3个操作：(1)  flush_cache_kmaps在内核映射上执行刷出（在需
要显式刷出的大多数体系结构上，将使用flush_cache_all刷出CPU的全部的高速缓存），因为内
核的全局页表已经修改。 这是一个代价很高的操作，幸运的是许多处理器体系结构不需要该操
作。在这种情况下，将定义为空操作。(2) 扫描整个pkmap_count数组。计数器值为1的项设置为
0，从页表删除相关的项，最后删除该映射。(3) 最后，使用flush_tlb_kernel_range函数刷出所
有与PKMAP区域相关的TLB项*/
static void flush_all_zero_pkmaps(void)
{
	int i;

	flush_cache_kmaps();
	/*从pkmap_count数组头开始，将*/
	for (i = 0; i < LAST_PKMAP; i++)
	{
		struct page *page;
		/*0意味着我们不需要做什么，大于1意味着该页仍然在使用之中，1意味着该页是空闲的
		需要被解除映射*/
		if (pkmap_count[i] != 1)
			continue;
		/*已映射未使用，则解除映射*/
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
		/*获取页表项对应的页实例*/
		page = pte_page(pkmap_page_table[i]);
		/*将页对应的pte页表项清空*/
		pte_clear(&init_mm, (unsigned long)page_address(page), &pkmap_page_table[i]);
		/*解除页的映射*/
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

/*建立页实例与虚拟地址之间的映射*/
/*使用map_new_virtual映射页。该函数将执行下列主要的步骤：(1) 从最后使用的位置（保存在
全局变量last_pkmap_nr中）开始，反向扫描pkmap_count数组，直至找到一个空闲位置。如果没
有空闲位置，该函数进入睡眠状态，直至内核的另一部分执行解除映射操作腾出空位。在到达
pkmap_count的最大索引值时，搜索从位置0开始。在这种情况下，还调用flush_all_zero_pkmaps
函数刷出CPU高速缓存。(2) 修改内核的页表，将该页映射在指定位置。但尚未更新TLB。(3) 新
位置的使用计数器设置为1。该页已分配但无法使用，因为TLB项未更新。(4) set_page_address
将该页添加到持久内核映射的数据结构。该函数返回新映射页的虚拟地址。在不需要高端内存页
的体系结构上（或没有设置CONFIG_HIGHMEM），则使用通用版本的kmap返回页的地址，且不修改
虚拟内存*/
static inline unsigned long map_new_virtual(struct page *page)
{
	unsigned long vaddr;
	int count;

start:
	/**/
	count = LAST_PKMAP;
	/*从pkmap_count数组中逆向查询未建立映射的空闲位*/
	for (;;)
	{
		/*从上次已访问的空闲结点下一项开始*/
		last_pkmap_nr = (last_pkmap_nr + 1) & LAST_PKMAP_MASK;
		/*到达pkmap_count数组头时，需要刷出cpu高速缓存*/
		if (!last_pkmap_nr)
		{
			flush_all_zero_pkmaps();
			count = LAST_PKMAP;
		}
		/*如果从pkmap_count中找到一个空闲位置（说明该位置对应的虚拟地址没有使用），则
		使用该虚拟地址建立与页地址的映射*/
		if (!pkmap_count[last_pkmap_nr])
			break;
		/*逆向搜索pkmap_count空闲位*/
		if (--count)
			continue;

		/*
		 * Sleep for somebody else to unmap their entries
		 */
		/*进程睡眠直到其它进程释放映射*/
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

/*指定页实例如果没有建立映射，则建立页实例与虚拟地址之间的映射，更新该页实例对应的虚
拟地址的映射计数*/
void fastcall *kmap_high(struct page *page)
{
	unsigned long vaddr;

	/*对于高端内存页，必须要在锁保护下操作，我们也不能从可能被阻塞的中断中调用
	（该类型页）*/
	spin_lock(&kmap_lock);
	/*获取页实例对应的虚拟内存地址*/
	vaddr = (unsigned long)page_address(page);
	/*如果还没有建立该页与虚拟地址的映射，则建立映射*/
	if (!vaddr)
		vaddr = map_new_virtual(page);
	/*将该页对应的虚拟地址引用计数自增1*/
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
	/*获取页实例对应的虚拟地址*/
	vaddr = (unsigned long)page_address(page);
	/*需要解除映射的页实例为空时是bug*/
	BUG_ON(!vaddr);
	/*获取该虚拟地址被使用的次数*/
	nr = PKMAP_NR(vaddr);

	/*没有一个TLB刷新一个页的引用计数不能变为0
	 * A count must never go down to zero without a TLB flush!
	 */
	need_wakeup = 0;
	switch (--pkmap_count[nr])
	{
		case 0:
			BUG();
		case 1:
			/*避免没有必要的wakp_up函数调用，常见的例子时pkmap_count[]=1，但此时没有
			等待进程。等待队列中的排队进程被wait_queue_head_t成员锁和kmap_lock锁保护，
			因为该函数已持有kmap_lock锁，因此就没有必有持有wait_queue_head_t成员锁。
			如果队列为空则简单测试*/
			/*测试等待队列中是否有等待进程*/
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

/*128位散列键值*/
#define PA_HASH_ORDER		7

/*内核利用下列数据结构，来建立物理内存的page实例与其在虚拟内存区中位置之间的关联。
该结构来构建page->virtual的映射*/
struct page_address_map
{
	/*一个指向全局mem_map数组中的page实例的指针*/
	struct page *page;
	/*指定该页在内核虚拟地址空间中分配的位置*/
	void *virtual;
	/*为了便于组织，映射保存在散列表中，结构中的链表元素用于建立溢出链表，以处理散列
	碰撞*/
	struct list_head list;
};

/*page_address_map空闲链表，从page_address_maps分配*/
static struct list_head page_address_pool;
/*锁保护page_address_pool*/
static spinlock_t pool_lock;

/*smp中L1缓存行对齐的page_address_htable散列表数组，散列表通过该数组实现*/
static struct page_address_slot
{
	/*page_address_maps链表，建立溢出链表，处理散列碰撞*/
	struct list_head lh;
	/*保护该数组项链表*/
	spinlock_t lock;
} ____cacheline_aligned_in_smp page_address_htable[1<<PA_HASH_ORDER];

/*根据页实例地址，通过与黄金分割比素数数学计算，获取在[0, (1<<PA_HASH_SHIFT)-1]之间
的索引，然后根据该索引确定其在散列表中的地址*/
static struct page_address_slot *page_slot(struct page *page)
{
	return &page_address_htable[hash_ptr(page, PA_HASH_ORDER)];
}

/*获取指定页的虚拟地址。如果页是非高端内存页，则直接获取其直接映射的虚拟地址，否则，
则根据页实例地址获取该页对应的散列表，并从散列表中查询该页是否已存在，如果该页存在
与散列表中，则返回该页对应的虚拟地址，否则，返回NULL*/
void *page_address(struct page *page)
{
	unsigned long flags;
	void *ret;
	struct page_address_slot *pas;
	/*如果页是非高端内存域中的页，则获取该页在直接映射区中的虚拟地址*/
	if (!PageHighMem(page))
		return lowmem_page_address(page);
	/*获取该高端内存页所属的散列表头*/
	pas = page_slot(page);
	ret = NULL;
	spin_lock_irqsave(&pas->lock, flags);
	/*如果该页对应散列表非空，则查询该页是否已经存在于该散列表中*/
	if (!list_empty(&pas->lh))
	{
		struct page_address_map *pam;
		/*遍历该散列表，查询该页对应的虚拟地址*/
		list_for_each_entry(pam, &pas->lh, list)
		{
			/*如果该页在散列表中，获取其虚拟地址*/
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

/*设置页实例与指定虚拟地址之间的映射，页不能是非高端内存页，如果指定映射的虚拟地址
有效，则从空闲映射池中选第一个空闲结点，并初设置页实例与虚拟地址之间的映射，然后将
该page_address_map实例添加到对应的散列表末尾。如果指定的虚拟地址无效，则查询散列表
中是否已存在该页已建立额的映射，存在则删除该映射，并将该page_address_map实例放入空
闲映射池中*/
void set_page_address(struct page *page, void *virtual)
{
	unsigned long flags;
	struct page_address_slot *pas;
	struct page_address_map *pam;
	/*page只能是高端内存域中页面，不能是直接映射区中页面*/
	BUG_ON(!PageHighMem(page));
	/*根据gage实例地址获取其所属的page_addeesses_htable散列头*/
	pas = page_slot(page);
	/*如果与页关联的虚拟地址非空，则从空闲链表池中获取第一个struct page_address_map
	实例，并用函数输入页实例指针和虚拟地址初始化，并将该页插入到对应的散列表末尾*/
	if (virtual)
	{
		/*page_address_pool全局空闲链表中*/
		BUG_ON(list_empty(&page_address_pool));
		/*申请自旋锁并禁用中断*/
		spin_lock_irqsave(&pool_lock, flags);
		/*获取全局空闲链表上第一个结点对应的struct page_address_map实例指针*/
		pam = list_entry(page_address_pool.next,	struct page_address_map, list);
		/*将该结点从空闲链表池中删除*/
		list_del(&pam->list);
		spin_unlock_irqrestore(&pool_lock, flags);
		/*初始化已获取的struct page_address_map实例*/
		pam->page = page;
		pam->virtual = virtual;
		/*将用输入参数初始化的struct page_address_map实例插入到对应的散列表位*/
		spin_lock_irqsave(&pas->lock, flags);
		list_add_tail(&pam->list, &pas->lh);
		spin_unlock_irqrestore(&pas->lock, flags);
	}
	/*如果输入对应页实例的虚拟地址为空，如果该页存在于page_address_htable散列表中，
	则将该页从该散列表中删除，并将该struct page_address_map实例添加到空闲池中*/
	else
	{
		spin_lock_irqsave(&pas->lock, flags);
		list_for_each_entry(pam, &pas->lh, list)
		{
			/*如果页实例出现在散列表中，则将该页对应的struct page_address_map实例从
			散列表中删除，并将该页对应的page_address_map实例添加到空闲池末尾*/
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

/*便于组织页实例和虚拟地址之间的关系，映射保存在散列表中，结构的链表元素用于建立溢出
链表，以处理散列碰撞*/
static struct page_address_map page_address_maps[LAST_PKMAP];

/*初始化pkmap内存域，该区域用于持久映射*/
void __init page_address_init(void)
{
	int i;
	/*初始化散列池链表*/
	INIT_LIST_HEAD(&page_address_pool);
	/*头插法将page_addresses_maps数组成员list连接到page_address_pool，该循环结果就
	是page_address_maps数组成员逆序排列于page_address_pool链表中*/
	for (i = 0; i < ARRAY_SIZE(page_address_maps); i++)
		list_add(&page_address_maps[i].list, &page_address_pool);
	/*初始化page_address_htable数组中散列头和保护锁成员*/
	for (i = 0; i < ARRAY_SIZE(page_address_htable); i++)
	{
		INIT_LIST_HEAD(&page_address_htable[i].lh);
		spin_lock_init(&page_address_htable[i].lock);
	}
	/*初始化page_addresses_pool保护锁*/
	spin_lock_init(&pool_lock);
}

#endif	/* defined(CONFIG_HIGHMEM) && !defined(WANT_PAGE_VIRTUAL) */
