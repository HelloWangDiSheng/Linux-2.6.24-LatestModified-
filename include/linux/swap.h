#ifndef _LINUX_SWAP_H
#define _LINUX_SWAP_H

#include <linux/spinlock.h>
#include <linux/linkage.h>
#include <linux/mmzone.h>
#include <linux/list.h>
#include <linux/sched.h>

#include <asm/atomic.h>
#include <asm/page.h>

struct notifier_block;

struct bio;

#define SWAP_FLAG_PREFER			0x8000	/* set if swap priority specified */
#define SWAP_FLAG_PRIO_MASK			0x7fff
#define SWAP_FLAG_PRIO_SHIFT		0

/*当前进程时页面交换守护进程*/
static inline int current_is_kswapd(void)
{
	return current->flags & PF_KSWAPD;
}

/*
 * MAX_SWAPFILES defines the maximum number of swaptypes: things which can
 * be swapped to.  The swap type and the offset into that swap type are
 * encoded into pte's and into pgoff_t's in the swapcache.  Using five bits
 * for the type means that the maximum number of swapcache pages is 27 bits
 * on 32-bit-pgoff_t architectures.  And that assumes that the architecture packs
 * the type/offset into the pte as 5/27 as well.
 */
#define MAX_SWAPFILES_SHIFT	5
#ifndef CONFIG_MIGRATION
#define MAX_SWAPFILES		(1 << MAX_SWAPFILES_SHIFT)
#else
/* Use last two entries for page migration swap entries */
#define MAX_SWAPFILES		((1 << MAX_SWAPFILES_SHIFT)-2)
#define SWP_MIGRATION_READ	MAX_SWAPFILES
#define SWP_MIGRATION_WRITE	(MAX_SWAPFILES + 1)
#endif

/*
 * Magic header for a swap area. The first part of the union is
 * what the swap magic looks like for the old (limited to 128MB)
 * swap area format, the second part of the union adds - in the
 * old reserved area - some extra information. Note that the first
 * kilobyte is reserved for boot loader or disk label stuff...
 *
 * Having the magic at the end of the PAGE_SIZE makes detecting swap
 * areas somewhat tricky on machines that support multiple page sizes.
 * For 2.5 we'll probably want to move the magic to just beyond the
 * bootbits...
 */
/**/
union swap_header
{
	/**/
	struct
	{
		/**/
		char reserved[PAGE_SIZE - 10];
		/**/
		char magic[10];			/* SWAP-SPACE or SWAPSPACE2 */
	} magic;
	/**/
	struct
	{
		/**/
		char		bootbits[1024];	/* Space for disklabel etc. */
		/**/
		__u32		version;
		/**/
		__u32		last_page;
		/**/
		__u32		nr_badpages;
		/**/
		unsigned char	sws_uuid[16];
		/**/
		unsigned char	sws_volume[16];
		/**/
		__u32		padding[117];
		/**/
		__u32		badpages[1];
	} info;
};

 /* A swap entry has to fit into a "unsigned long", as
  * the entry is hidden in the "index" field of the
  * swapper address space.
  */
/*根据内存页的虚拟地址，需要使用一整套页表，才能找到相关页帧在物理内存中的地址。仅
当数据实际存在于内存中时，该机制才是有效的。否则，没有对应的页表项。内核还必须能够
正确标识换出页，换言之，必须能够根据给定的虚拟地址，找到内存页在交换区中的地址。换
出页在页表中通过一种专门的页表项来标记，其格式取决于所用的处理器体系结构。每个系统
都使用了特定的编码，以满足特定的需求。
在换出页的页表项中，所有CPU都会存储下列信息。（1）一个标志，表明页已经换出（2）该页
所在交换区的编号（3）对应槽位的偏移量，用于在交换区中查找该页所在的槽位。
内核定义了一种体系结构无关的格式，可用于在交换区中确定页所在的位置，该格式可以（通过
特定于处理器的代码）从体系结构相关的数据得出。该方法的优点是显然的，它使得所有页交换
算法的实现都与硬件无关，无须对每种处理器类型重写。与实际硬件的唯一接口，就是用于转换
体系结构相关和无关两种数据表示的函数。
在体系结构无关的表示中，内核必须存储交换分区的标识（也称为类型）和该交换区内部的偏移
量，以便唯一确定一页。该信息保存在一个专门的数据类型中。该结构只使用了一个成员变量，
而需要存储两个不同的信息项。可以通过选择该成员中不同的比特位，来得出两个对应的信息项
*/
typedef struct
{
	unsigned long val;
} swp_entry_t;

/*
 * current->reclaim_state points to one of these when a task is running
 * memory reclaim
 */
/**/
struct reclaim_state
{
	unsigned long reclaimed_slab;
};

#ifdef __KERNEL__

struct address_space;
struct sysinfo;
struct writeback_control;
struct zone;

/*
 * A swap extent maps a range of a swapfile's PAGE_SIZE pages onto a range of
 * disk blocks.  A list of swap extents maps the entire swapfile.  (Where the
 * term `swapfile' refers to either a blockdevice or an IS_REG file.  Apart
 * from setup, they're handled identically.
 *
 * We always assume that blocks are of size PAGE_SIZE.
 */
/*区间结果结构*/
struct swap_extent
{
	/*链表将区间结构置于标准双链表中管理，其它成员描述了一个连续的块组*/
	struct list_head list;
	/*块组中第一个槽位的编号*/
	pgoff_t start_page;
	/*块组中可容纳页的数目*/
	pgoff_t nr_pages;
	/*块组的第一块在硬盘上的块号*/
	sector_t start_block;
};

/*
 * Max bad pages in the new format..
 */
/**/
#define __swapoffset(x) ((unsigned long)&((union swap_header *)0)->x)
/**/
#define MAX_SWAP_BADPAGES ((__swapoffset(magic.magic) - __swapoffset(info.badpages)) / sizeof(int))
/*交换区状态*/
enum
{
	/*当前项在交换数组中处于使用状态。否则，相应的数组项会用字节0填充，使得很容易区分
	使用和未使用的数组项*/
	SWP_USED	= 		(1 << 0),
	/*当前项对应的交换区可写*/
	SWP_WRITEOK	= 		(1 << 1),
	/*在交换区插入到内核之后，这两个标识都会设置*/
	SWP_ACTIVE	= 		(SWP_USED | SWP_WRITEOK),
	/*其它状态可以插入此处*/
	/*scan_swap_map引用次数*/
	SWP_SCANNING	= 	(1 << 8),
};

/*交换区*/
#define SWAP_CLUSTER_MAX 32

#define SWAP_MAP_MAX	0x7fff
#define SWAP_MAP_BAD	0x8000

/*
 * The in-memory structure used to track swap areas.
 */
/*内存中跟踪交换区结构*/
struct swap_info_struct
{
	/*交换区状态*/
	unsigned int flags;
	/*交换区的相对优先级优先级。有符号整数，所以正负优先级都有可能，交换分区的优先级
	越高，表明该交换分区越重要*/
	int prio;
	/*指向交换区关联的file结构，对于交换分区，这是一个指向块设备上分区的设备文件指针
	，如/dev/hda5。对于交换文件，该指针指向相关文件的struct file实例，如/mnt/swap1或
	/tmp/swap2的情形*/
	struct file *swap_file;
	/*指向文件/分区所在的低层块设备的block_device结构*/
	struct block_device *bdev;
	/*内核使用extend_list和current_swap_extent成员来实现区间，用于建立假定连续的交换
	区槽位与交换文件的磁盘块之间的映射。如果使用分区作为交换区，这是不必要的，因为内
	核可以依赖一个事实，即分区中的块在磁盘上线性排列的。因而槽位与磁盘块之间的映射会
	非常简单，从第一个磁盘块开始，加上所需的页数乘以一个常量得到的偏移值，即可获得所
	需地址*/
	struct list_head extent_list;
	/*当前区间*/
	struct swap_extent *curr_swap_extent;
	/**/
	unsigned old_block_size;
	/*指针指向短整形数组（交换映射），其中包含的项数与与交换区槽位数目相同，该数组用作
	一个访问计数器，每个数组项都表示共享对应换出页的进程的数目*/
	unsigned short * swap_map;
	/*为减少扫描整个交换区查找空闲槽位的搜索时间，内核指定lowest_bit为搜索区域的下界，
	[lowest_bit, highest_bit]之外是没有空闲槽位*/
	unsigned int lowest_bit;
	/*搜索区域的上界*/
	unsigned int highest_bit;
	/*交换区中接下来使用的槽位（在某个现存聚集中）的索引*/
	unsigned int cluster_next;
	/*当前聚集总仍然有可用槽位的数目，在消耗了空闲槽位之后，则必须建立一个新的聚集，
	否则（如果没有足够空闲槽位可用于建立新的聚集）就只能进行细粒度分配（不再按聚集分
	配槽位）*/
	unsigned int cluster_nr;
	/*交换区中可用槽位的总数，每个槽位可容纳一个内存域*/
	unsigned int pages;
	/*交换区当前包含的页数目。不同于pages，该成员不仅计算可用的槽位，而且包括那些（因
	为块设备故障）损坏或用于管理目的的槽位。因为在当今的硬盘上，坏块时极端罕见的（也
	没有必要在这样的区域上创建交换分区），max通常等于pages加1*/
	unsigned int max;
	/**/
	unsigned int inuse_pages;
	/**/
	int next;			/* next entry on swap list */
};

/*内核使用该结构将交换数组中的各个数组项按优先级连接起来。因为标识各个交换区的数据位
于一个线性数组的各个数组项中，所以在固定的数组项位置之外，使用了next成员来建立一个相
对的顺序。next实际上是下一个交换区swap_info[]数组中的索引，使得内核能够根据优先级来跟
踪各个数组项*/
struct swap_list_t
{
	/*优先级已排序的交换文件第一个索引，也用于选择优先级最高的交换区，内核根据head和
	swap_info_struct的next成员，按优先级由高到低的顺序，即可遍历所有交换区的列表。next
	用于实现魂环过程，以便在有多个交换区优先级相同的情况下，均匀地使用内存页填充这个多
	交换区，因为入口点是第一个数组项，因此head的值为0*/
	int head;
	/*指定接下来将使用的缓冲区，这个未必总是优先级最高的交换区，如果该交换区已满，则
	next指向另一个交换区*/
	int next;
};

/* Swap 50% full? Release swapcache more aggressively.. */
#define vm_swap_full() (nr_swap_pages*2 < total_swap_pages)

/* linux/mm/memory.c */
extern void swapin_readahead(swp_entry_t, unsigned long, struct vm_area_struct *);

/* linux/mm/page_alloc.c */
extern unsigned long totalram_pages;
extern unsigned long totalreserve_pages;
extern long nr_swap_pages;
extern unsigned int nr_free_buffer_pages(void);
extern unsigned int nr_free_pagecache_pages(void);

/*获取系统空闲页数目*/
#define nr_free_pages() global_page_state(NR_FREE_PAGES)


/* linux/mm/swap.c */
extern void FASTCALL(lru_cache_add(struct page *));
extern void FASTCALL(lru_cache_add_active(struct page *));
extern void FASTCALL(activate_page(struct page *));
extern void FASTCALL(mark_page_accessed(struct page *));
extern void lru_add_drain(void);
extern int lru_add_drain_all(void);
extern int rotate_reclaimable_page(struct page *page);
extern void swap_setup(void);

/* linux/mm/vmscan.c */
extern unsigned long try_to_free_pages(struct zone **zones, int order,
					gfp_t gfp_mask);
extern unsigned long shrink_all_memory(unsigned long nr_pages);
extern int vm_swappiness;
extern int remove_mapping(struct address_space *mapping, struct page *page);
extern long vm_total_pages;

#ifdef CONFIG_NUMA
extern int zone_reclaim_mode;
extern int sysctl_min_unmapped_ratio;
extern int sysctl_min_slab_ratio;
extern int zone_reclaim(struct zone *, gfp_t, unsigned int);
#else
#define zone_reclaim_mode 0
static inline int zone_reclaim(struct zone *z, gfp_t mask, unsigned int order)
{
	return 0;
}
#endif

extern int kswapd_run(int nid);

#ifdef CONFIG_MMU
/* linux/mm/shmem.c */
extern int shmem_unuse(swp_entry_t entry, struct page *page);
#endif /* CONFIG_MMU */

extern void swap_unplug_io_fn(struct backing_dev_info *, struct page *);

#ifdef CONFIG_SWAP
/* linux/mm/page_io.c */
extern int swap_readpage(struct file *, struct page *);
extern int swap_writepage(struct page *page, struct writeback_control *wbc);
extern void end_swap_bio_read(struct bio *bio, int err);

/* linux/mm/swap_state.c */
extern struct address_space swapper_space;
#define total_swapcache_pages  swapper_space.nrpages
extern void show_swap_cache_info(void);
extern int add_to_swap(struct page *, gfp_t);
extern void __delete_from_swap_cache(struct page *);
extern void delete_from_swap_cache(struct page *);
extern int move_to_swap_cache(struct page *, swp_entry_t);
extern int move_from_swap_cache(struct page *, unsigned long, struct address_space *);
extern void free_page_and_swap_cache(struct page *);
extern void free_pages_and_swap_cache(struct page **, int);
extern struct page * lookup_swap_cache(swp_entry_t);
extern struct page * read_swap_cache_async(swp_entry_t, struct vm_area_struct *vma,
					   unsigned long addr);
/* linux/mm/swapfile.c */
extern long total_swap_pages;
extern unsigned int nr_swapfiles;
extern void si_swapinfo(struct sysinfo *);
extern swp_entry_t get_swap_page(void);
extern swp_entry_t get_swap_page_of_type(int);
extern int swap_duplicate(swp_entry_t);
extern int valid_swaphandles(swp_entry_t, unsigned long *);
extern void swap_free(swp_entry_t);
extern void free_swap_and_cache(swp_entry_t);
extern int swap_type_of(dev_t, sector_t, struct block_device **);
extern unsigned int count_swap_pages(int, int);
extern sector_t map_swap_page(struct swap_info_struct *, pgoff_t);
extern sector_t swapdev_block(int, pgoff_t);
extern struct swap_info_struct *get_swap_info_struct(unsigned);
extern int can_share_swap_page(struct page *);
extern int remove_exclusive_swap_page(struct page *);
struct backing_dev_info;

extern spinlock_t swap_lock;

/* linux/mm/thrash.c */
extern struct mm_struct * swap_token_mm;
extern void grab_swap_token(void);
extern void __put_swap_token(struct mm_struct *);

static inline int has_swap_token(struct mm_struct *mm)
{
	return (mm == swap_token_mm);
}

static inline void put_swap_token(struct mm_struct *mm)
{
	if (has_swap_token(mm))
		__put_swap_token(mm);
}

static inline void disable_swap_token(void)
{
	put_swap_token(swap_token_mm);
}

#else /* CONFIG_SWAP */

#define total_swap_pages			0
#define total_swapcache_pages			0UL

#define si_swapinfo(val) do { (val)->freeswap = (val)->totalswap = 0; } while (0)
/* only sparc can not include linux/pagemap.h in this file
 * so leave page_cache_release and release_pages undeclared... */
/**/
#define free_page_and_swap_cache(page) page_cache_release(page)
#define free_pages_and_swap_cache(pages, nr)	release_pages((pages), (nr), 0);

static inline void show_swap_cache_info(void)
{
}

static inline void free_swap_and_cache(swp_entry_t swp)
{
}

static inline int swap_duplicate(swp_entry_t swp)
{
	return 0;
}

static inline void swap_free(swp_entry_t swp)
{
}

static inline struct page *read_swap_cache_async(swp_entry_t swp,
			struct vm_area_struct *vma, unsigned long addr)
{
	return NULL;
}

static inline struct page *lookup_swap_cache(swp_entry_t swp)
{
	return NULL;
}

static inline int valid_swaphandles(swp_entry_t entry, unsigned long *offset)
{
	return 0;
}

#define can_share_swap_page(p)			(page_mapcount(p) == 1)

static inline int move_to_swap_cache(struct page *page, swp_entry_t entry)
{
	return 1;
}

static inline int move_from_swap_cache(struct page *page, unsigned long index,
					struct address_space *mapping)
{
	return 1;
}

static inline void __delete_from_swap_cache(struct page *page)
{
}

static inline void delete_from_swap_cache(struct page *page)
{
}

#define swap_token_default_timeout		0

static inline int remove_exclusive_swap_page(struct page *p)
{
	return 0;
}

/**/
static inline swp_entry_t get_swap_page(void)
{
	swp_entry_t entry;
	entry.val = 0;
	return entry;
}

/* linux/mm/thrash.c */
#define put_swap_token(x) do { } while(0)
#define grab_swap_token()  do { } while(0)
#define has_swap_token(x) 0
#define disable_swap_token() do { } while(0)

#endif /* CONFIG_SWAP */
#endif /* __KERNEL__*/
#endif /* _LINUX_SWAP_H */
