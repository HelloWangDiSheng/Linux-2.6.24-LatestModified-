/*操作和测试page->flags的宏*/

#ifndef PAGE_FLAGS_H
#define PAGE_FLAGS_H

#include <linux/types.h>
#include <linux/mm_types.h>

/*
 * Various page->flags bits:
 *
 * PG_reserved is set for special pages, which can never be swapped out. Some
 * of them might not even exist (eg empty_bad_page)...
 *
 * The PG_private bitflag is set on pagecache pages if they contain filesystem
 * specific data (which is normally at page->private). It can be used by
 * private allocations for its own usage.
 *
 * During initiation of disk I/O, PG_locked is set. This bit is set before I/O
 * and cleared when writeback _starts_ or when read _completes_. PG_writeback
 * is set before writeback starts and cleared when it finishes.
 *
 * PG_locked also pins a page in pagecache, and blocks truncation of the file
 * while it is held.
 *
 * page_waitqueue(page) is a wait queue of all tasks waiting for the page
 * to become unlocked.
 *
 * PG_uptodate tells whether the page's contents is valid.  When a read
 * completes, the page becomes uptodate, unless a disk I/O error happened.
 *
 * PG_referenced, PG_reclaim are used for page reclaim for anonymous and
 * file-backed pagecache (see mm/vmscan.c).
 *
 * PG_error is set to indicate that an I/O error occurred on this page.
 *
 * PG_arch_1 is an architecture specific page state bit.  The generic code
 * guarantees that this bit is cleared for a page when it first is entered into
 * the page cache.
 *
 * PG_highmem pages are not permanently mapped into the kernel virtual address
 * space, they need to be kmapped separately for doing IO on the pages.  The
 * struct page (these bits with information) are always mapped into kernel
 * address space...
 *
 * PG_buddy is set to indicate that the page is free and in the buddy system
 * (see mm/page_alloc.c).
 *
 */

/*
 * Don't use the *_dontuse flags.  Use the macros.  Otherwise you'll break
 * locked- and dirty-page accounting.
 *
 * The page flags field is split into two parts, the main flags area
 * which extends from the low bits upwards, and the fields area which
 * extends from the high bits downwards.
 *
 *  | FIELD | ... | FLAGS |
 *  N-1     ^             0
 *          (N-FLAGS_RESERVED)
 *
 * The fields area is reserved for fields mapping zone, node and SPARSEMEM
 * section.  The boundry between these two areas is defined by
 * FLAGS_RESERVED which defines the width of the fields section
 * (see linux/mmzone.h).  New flags must _not_ overlap with this area.
 */
/*页被锁定标识，内核的其他部分不允许访问该页，这防止了内存管理出现竟态条件，例如，在
从硬盘上读取数据到页帧时。磁盘I/O启动期间，PG_lock被设置，该标识在IO前被设置，当回写
开始或读完成时被清除。也可以钉住一个页缓存的页，文件块被截断时页页被设置为该标识*/
#define PG_locked	 	 	0	/* Page is locked. Don't touch. */
/*如果页在I/O操作期间发生错误，则PG_error置位*/
#define PG_error		 	1
/*PG_reference和PG_reclaim被用于匿名页和文件后备页缓存的页回收*/
#define PG_referenced		2
/*页内容有效。当读完成，页设置该标识，除非发生磁盘io错误*/
#define PG_uptodate		 	3
/*如果与硬盘上的数据相比，页的内容已经改变，这置位PG_dirty。出于性能考虑，页并不在每
次改变后立即回写，因此，内核使用该标记注明该页已经改变，可以在稍后刷出。设置了该标识
的页成为脏的（通常，这意味着内存中的数据没有与外存储器介质如硬盘上的数据同步）*/
#define PG_dirty	 	 	4
/*有助于实现页面回收和切换。内核使用了两个最近最少使用（Latest Recently Used LRU）的
链表来区别活动和不活动的页。如果页在其中一个链表上，则设置该比特位。还有一个PG_active
标识，如果页在活动链表上，则设置该标识*/
#define PG_lru			 	5
/*页在活动lru链表上*/
#define PG_active		 	6
/*slab调试*/
#define PG_slab			 	7
/*拥有者使用，如果是页缓存，文件系统可以使用*/
#define PG_owner_priv_1	8
/**/
#define PG_arch_1		 	9
/*永远不会被换出的特别页被标记为保留页，有些甚至可能不存在，例如empty_bad_page*/
#define PG_reserved			10
/*如果page结构的private非空，则必须设置PG_private位。用于I/O的页，可使用该字段将页细
分为多个缓冲区，但内核的其他部分也有各种不同的方法，将私有数据附加到页上*/
#define PG_private			11	/* If pagecache, has fs-private data */
/*页数据向块设备回写开始时被设置，完成时被清除*/
#define PG_writeback		12	/* Page is under writeback */
/*页属于一个更大的复合页，复合页有多个毗邻的普通页组成*/
#define PG_compound			14	/* Part of a compound page */
/*页处于交换缓存，此时，页的private包含一个类型为swp_entry_t变量*/
#define PG_swapcache		15

/*Has blocks allocated on-disk*/
#define PG_mappedtodisk		16
/*在可用内存的数量变少时，内核试图周期性地回收页，即剔除不活动、未使用的页，在内核决
定回收某个特定的页之后，需要设置PG_reclaim标识*/
#define PG_reclaim			17	/* To be reclaimed asap */
/*伙伴系统页，页是空闲的，在伙伴系统链表上*/
#define PG_buddy			19
/* PG_readahead is only used for file reads; PG_reclaim is only for writes */
/*仅在文件异步预读时使用*/
#define PG_readahead		PG_reclaim /* Reminder to do async read-ahead */

/*被一些文件系统使用*/
#define PG_checked			PG_owner_priv_1
/**/
#define PG_pinned			PG_owner_priv_1	/* Xen pinned pagetable */

#if (BITS_PER_LONG > 32)
/*
 * 64-bit-only flags build down from bit 31
 *
 * 32 bit  -------------------------------| FIELDS |       FLAGS         |
 * 64 bit  |           FIELDS             | ??????         FLAGS         |
 *         63                            32                              0
 */
/*页已经被映射，但还没有缓存*/
#define PG_uncached			31	/* Page has been mapped as uncached */
#endif

/*页状态标识操作*/
/*
	PageXXX(page)				检查页是否设置了PG_XXX位。如Page_Dirty检查PG_dirty标识位。
	SetPageXXX(page)			设置PG_XXX标识位
	TestSetPageXXX(page)		检查PG_XXX标识位，没有设置则设置，并返回原值
	ClearPageXXX(page)			清除PG_XXX标识位
	TestClearPageXXX(page)检查PG_XXX标识位，如果已设置则清除，并返回原值
*/

#define PageLocked(page)					test_bit(PG_locked, &(page)->flags)
#define SetPageLocked(page)					set_bit(PG_locked, &(page)->flags)
#define TestSetPageLocked(page)				test_and_set_bit(PG_locked, &(page)->flags)
#define ClearPageLocked(page)				clear_bit(PG_locked, &(page)->flags)
#define TestClearPageLocked(page) 			test_and_clear_bit(PG_locked, &(page)->flags)

#define PageError(page)						test_bit(PG_error, &(page)->flags)
#define SetPageError(page)					set_bit(PG_error, &(page)->flags)
#define ClearPageError(page)				clear_bit(PG_error, &(page)->flags)

#define PageReferenced(page)				test_bit(PG_referenced, &(page)->flags)
#define SetPageReferenced(page)				set_bit(PG_referenced, &(page)->flags)
#define ClearPageReferenced(page)			clear_bit(PG_referenced, &(page)->flags)
#define TestClearPageReferenced(page)	test_and_clear_bit(PG_referenced, &(page)->flags)

#define PageUptodate(page)					test_bit(PG_uptodate, &(page)->flags)
#ifdef CONFIG_S390
static inline void SetPageUptodate(struct page *page)
{
	if (!test_and_set_bit(PG_uptodate, &page->flags))
		page_clear_dirty(page);
}
#else
#define SetPageUptodate(page)				set_bit(PG_uptodate, &(page)->flags)
#endif
#define ClearPageUptodate(page)				clear_bit(PG_uptodate, &(page)->flags)

#define PageDirty(page)						test_bit(PG_dirty, &(page)->flags)
#define SetPageDirty(page)					set_bit(PG_dirty, &(page)->flags)
#define TestSetPageDirty(page)				test_and_set_bit(PG_dirty, &(page)->flags)
#define ClearPageDirty(page)				clear_bit(PG_dirty, &(page)->flags)
#define __ClearPageDirty(page)				__clear_bit(PG_dirty, &(page)->flags)
#define TestClearPageDirty(page) 			test_and_clear_bit(PG_dirty, &(page)->flags)

#define PageLRU(page)						test_bit(PG_lru, &(page)->flags)
#define SetPageLRU(page)					set_bit(PG_lru, &(page)->flags)
#define ClearPageLRU(page)					clear_bit(PG_lru, &(page)->flags)
#define __ClearPageLRU(page)				__clear_bit(PG_lru, &(page)->flags)

#define PageActive(page)					test_bit(PG_active, &(page)->flags)
#define SetPageActive(page)					set_bit(PG_active, &(page)->flags)
#define ClearPageActive(page)				clear_bit(PG_active, &(page)->flags)
#define __ClearPageActive(page)				__clear_bit(PG_active, &(page)->flags)

#define PageSlab(page)						test_bit(PG_slab, &(page)->flags)
#define __SetPageSlab(page)					__set_bit(PG_slab, &(page)->flags)
#define __ClearPageSlab(page)				__clear_bit(PG_slab, &(page)->flags)

#ifdef CONFIG_HIGHMEM
/*测试一个页实例是否属于高端内存页*/
#define PageHighMem(page)					is_highmem(page_zone(page))
#else
#define PageHighMem(page)	0 /* needed to optimize away at compile time */
#endif

#define PageChecked(page)					test_bit(PG_checked, &(page)->flags)
#define SetPageChecked(page)				set_bit(PG_checked, &(page)->flags)
#define ClearPageChecked(page)				clear_bit(PG_checked, &(page)->flags)

#define PagePinned(page)					test_bit(PG_pinned, &(page)->flags)
#define SetPagePinned(page)					set_bit(PG_pinned, &(page)->flags)
#define ClearPagePinned(page)				clear_bit(PG_pinned, &(page)->flags)

#define PageReserved(page)					test_bit(PG_reserved, &(page)->flags)
#define SetPageReserved(page)				set_bit(PG_reserved, &(page)->flags)
#define ClearPageReserved(page)				clear_bit(PG_reserved, &(page)->flags)
#define __ClearPageReserved(page)			__clear_bit(PG_reserved, &(page)->flags)

#define SetPagePrivate(page)				set_bit(PG_private, &(page)->flags)
#define ClearPagePrivate(page)				clear_bit(PG_private, &(page)->flags)
#define PagePrivate(page)					test_bit(PG_private, &(page)->flags)
#define __SetPagePrivate(page)			  	__set_bit(PG_private, &(page)->flags)
#define __ClearPagePrivate(page) 			__clear_bit(PG_private, &(page)->flags)

/*
 * Only test-and-set exist for PG_writeback.  The unconditional operators are
 * risky: they bypass page accounting.
 */
#define PageWriteback(page)					test_bit(PG_writeback, &(page)->flags)
#define TestSetPageWriteback(page) 			test_and_set_bit(PG_writeback,			&(page)->flags)
#define TestClearPageWriteback(page)		test_and_clear_bit(PG_writeback, &(page)->flags)

#define PageBuddy(page)						test_bit(PG_buddy, &(page)->flags)
#define __SetPageBuddy(page)				__set_bit(PG_buddy, &(page)->flags)
#define __ClearPageBuddy(page)				__clear_bit(PG_buddy, &(page)->flags)

#define PageMappedToDisk(page)				test_bit(PG_mappedtodisk, &(page)->flags)
#define SetPageMappedToDisk(page) 			set_bit(PG_mappedtodisk, &(page)->flags)
#define ClearPageMappedToDisk(page) 		clear_bit(PG_mappedtodisk, &(page)->flags)

#define PageReadahead(page)					test_bit(PG_readahead, &(page)->flags)
#define SetPageReadahead(page)				set_bit(PG_readahead, &(page)->flags)
#define ClearPageReadahead(page) 			clear_bit(PG_readahead, &(page)->flags)

#define PageReclaim(page)					test_bit(PG_reclaim, &(page)->flags)
#define SetPageReclaim(page)				set_bit(PG_reclaim, &(page)->flags)
#define ClearPageReclaim(page)				clear_bit(PG_reclaim, &(page)->flags)
#define TestClearPageReclaim(page) 			test_and_clear_bit(PG_reclaim, &(page)->flags)

/*测试页是否是复合页*/
#define PageCompound(page)					test_bit(PG_compound, &(page)->flags)
/*设置页为复合页*/
#define __SetPageCompound(page)				__set_bit(PG_compound, &(page)->flags)
/*清除复合页标识*/
#define __ClearPageCompound(page) 			__clear_bit(PG_compound, &(page)->flags)

/*PG_reclaim和PG_compound一块使用来标识复合页的首页和尾页。PG_compound & PG_reclaim同时
使用表示尾页，PG_compund &~ PG_reclaim表示首页*/
/*设置页为复合页的尾页尾页标识*/
#define PG_head_tail_mask 					((1L << PG_compound) | (1L << PG_reclaim))
/*测试指定页是否是复合页中的尾页*/
#define PageTail(page)				((page->flags & PG_head_tail_mask)	== PG_head_tail_mask)

/*设置指定的页是复合页的尾页*/
static inline void __SetPageTail(struct page *page)
{
	page->flags |= PG_head_tail_mask;
}

/*清除复合页尾页标识*/
static inline void __ClearPageTail(struct page *page)
{
	page->flags &= ~PG_head_tail_mask;
}
/*测试指定页是否是复合页的首页*/
#define PageHead(page)	((page->flags & PG_head_tail_mask) == (1L << PG_compound))
/*设置复合页的首页标识*/
#define __SetPageHead(page)					__SetPageCompound(page)
/*清除复合页首页标识*/
#define __ClearPageHead(page)				__ClearPageCompound(page)

#ifdef CONFIG_SWAP
#define PageSwapCache(page)					test_bit(PG_swapcache, &(page)->flags)
#define SetPageSwapCache(page)				set_bit(PG_swapcache, &(page)->flags)
#define ClearPageSwapCache(page) 			clear_bit(PG_swapcache, &(page)->flags)
#else
#define PageSwapCache(page)					0
#endif

#define PageUncached(page)					test_bit(PG_uncached, &(page)->flags)
#define SetPageUncached(page)				set_bit(PG_uncached, &(page)->flags)
#define ClearPageUncached(page)				clear_bit(PG_uncached, &(page)->flags)

struct page;	/* forward declaration */

extern void cancel_dirty_page(struct page *page, unsigned int account_size);

int test_clear_page_writeback(struct page *page);
int test_set_page_writeback(struct page *page);

static inline void set_page_writeback(struct page *page)
{
	test_set_page_writeback(page);
}

#endif	/* PAGE_FLAGS_H */
