/*启动期间简单的物理内存分配器和释放器，经常被用于处理系统保留内存和系统空洞*/

#include <linux/init.h>
#include <linux/pfn.h>
#include <linux/bootmem.h>
#include <linux/module.h>

#include <asm/bug.h>
#include <asm/io.h>
#include <asm/processor.h>

#include "internal.h"

/*外部（反正启动进程时这样的）需要串行访问该子系统*/
/*直接映射区的最大页帧号*/
unsigned long max_low_pfn;
/*直接映射区的最小页帧号*/
unsigned long min_low_pfn;
/*最大页帧号*/
unsigned long max_pfn;
/*自举分配器双链表表头*/
static LIST_HEAD(bdata_list);
#ifdef CONFIG_CRASH_DUMP

/*如果因为崩溃需要重启，max_pfn将会是一个很小的值，需要直到内核之前使用的内存数量*/
unsigned long saved_max_pfn;
#endif

/*返回将要为启动位图分配的页数目。在IA-32系统上4G内存，直接映射区896M对应的内存，
需要896M/4K=224k=224*1024=229376个页，那么，函数中的mapsize=(229376+7)/8=28672，
mapsize = (mapsize + ~PAGE_MASK) & PAGE_MASK=(28672 + 4095) & (~4095)=0X7FFF的低
12位全部清零，再mapsize >>= PAGE_SHIFT结果为7，直观的结果是(224K+4096)/(4K*8)=7，
两者计算结果一致*/
unsigned long __init bootmem_bootmap_pages(unsigned long pages)
{
	unsigned long mapsize;
	/*页数目对齐到字节的数目。例如有100个页，那就需要13个unsigned char*/
	mapsize = (pages+7)/8;
	/*需要的页数目*/
	mapsize = (mapsize + ~PAGE_MASK) & PAGE_MASK;
	/*需要页的数目*/
	mapsize >>= PAGE_SHIFT;

	return mapsize;
}

/*按自举分配器的起始页编号由小到大的顺序插入已排序的链表*/
static void __init link_bootmem(bootmem_data_t *bdata)
{
	bootmem_data_t *ent;
	/*如果全局自举分配器的双链表为空，头插法插入指定的自举分配器*/
	if (list_empty(&bdata_list))
	{
		list_add(&bdata->list, &bdata_list);
		return;
	}
	/*按结点的起始页编号由小到大排列插入自举分配器*/
	list_for_each_entry(ent, &bdata_list, list)
	{
		/*测试该自举分配器和链表中已顺序排序的自举分配器的起始页大小关系，如果小于，
		则插入已查找到的结点之前*/
		if (bdata->node_boot_start < ent->node_boot_start) {
			list_add_tail(&bdata->list, &ent->list);
			return;
		}
	}
	/*原链表中的所有自举分配器的起始页编号都小于该自举分配器的起始页编号，因此插入的
	位置在最后*/
	list_add_tail(&bdata->list, &bdata_list);
}

/*给定一个已经初始化的自举分配器，返回它需要的以long类型变量表示的位图空间数目。例
如，IA-32系统上4G内存，对应896M的直接映射区，自举分配器的起始页编号是0，最后一个可
直接管理的页编号是896M/4K=224k=224*1024=229376，函数中计算的mapsize是28672，和
size(long)对齐后就是28672。计算的结果就是获取自举分配器中所有页对应位图所占字节数目*/
static unsigned long __init get_mapsize(bootmem_data_t *bdata)
{
	unsigned long mapsize;
	/*获取自举分配器中起始页编号对应的页的编号*/
	unsigned long start = PFN_DOWN(bdata->node_boot_start);
	/*获取自举分配器中最后一个页的编号*/
	unsigned long end = bdata->node_low_pfn;
	/*获取自举分配器中有效编号范围所需的字节数目*/
	mapsize = ((end - start) + 7) / 8;
	/*获取位图中有效字节数目和long类型对齐后的数目*/
	return ALIGN(mapsize, sizeof(long));
}

/*只调用一次，创建指定结点的自举分配器*/
static unsigned long __init init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart,
													unsigned long start, unsigned long end)
{
	/*获取结点所属的（第一个）自举分配器（如果结点内有较多不连续的空洞，则该结点应该
	对应多个自举分配器实例）*/
	bootmem_data_t *bdata = pgdat->bdata;
	unsigned long mapsize;
	/*获取自举分配器的位图，因为系统启动期间管理的是直接映射区，因此，可以直接根据页
	号获取其物理地址，然后转换成虚拟地址*/
	bdata->node_bootmem_map = phys_to_virt(PFN_PHYS(mapstart));
	/*获取起始页对应的映射区中的物理地址*/
	bdata->node_boot_start = PFN_PHYS(start);
	bdata->node_low_pfn = end;
	/*将自举分配器插入到已排序的全局自举分配器的链表中*/
	link_bootmem(bdata);

	/*期初的所有页都设置为保留页，setup_arch()必须显式注册空闲RAM区*/
	/*获取该自举分配器的中有效页范围所需要的long类型变量的数目*/
	mapsize = get_mapsize(bdata);
	/*将位图中前四分之一位置位*/
	memset(bdata->node_bootmem_map, 0xff, mapsize);

	return mapsize;
}

/*标记一个特别物理内存域是不可分配的。可用的RAM可能已经在启动期间已被分配使用，或者
稍后被添加到空闲页池（就是启动期间_end结束处的struct *node_mem_map页数组）中*/
static void __init reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr,
											unsigned long size)
{
	unsigned long sidx, eidx;
	unsigned long i;

	/*大体上，部分被保留的页面被认为是完全保留的*/
	 /*指定的长度不能为空*/
	BUG_ON(!size);
	/*指定的地址对应的页编号不能超出自举分配器的最大页编号*/
	BUG_ON(PFN_DOWN(addr) >= bdata->node_low_pfn);
	/*指定的地址区间的最后页的下一个页编号不能超过自举分配器的最后一个页编号*/
	BUG_ON(PFN_UP(addr + size) > bdata->node_low_pfn);
	/*获取指定地址向前偏移自举分配器起始页编号的页编号*/
	sidx = PFN_DOWN(addr - bdata->node_boot_start);
	/*获取指定地址空间最后地址相对于自举分配器起始页编号偏移的后一个页编号*/
	eidx = PFN_UP(addr + size - bdata->node_boot_start);
	/*将指定地址区域所对应的页设置为置位状态*/
	for (i = sidx; i < eidx; i++)
		if (test_and_set_bit(i, bdata->node_bootmem_map))
		{
#ifdef CONFIG_DEBUG_BOOTMEM
			printk("hm, page %08lx reserved twice.\n", i*PAGE_SIZE);
#endif
		}
}

/*设置自举分配器中指定的地址区间对应的页为未使用状态*/
static void __init free_bootmem_core(bootmem_data_t *bdata, unsigned long addr,
				     					unsigned long size)
{
	unsigned long sidx, eidx;
	unsigned long i;

	/*取可用内存的前一页，部分空闲页被认为是保留的*/
	/*指定的长度不能为零*/
	BUG_ON(!size);
	/*指定地址区间结束处的页编号不能大于所属的自举分配器最大页编号*/
	BUG_ON(PFN_DOWN(addr + size) > bdata->node_low_pfn);
	/*如果指定地址小于上次分配成功的地址，则重新设置上次成功分配地址为指定地址*/
	if (addr < bdata->last_success)
		bdata->last_success = addr;

	/*
	 * Round up the beginning of the address.
	 */
	/*获取指定地址后一个页编号和自举分配器起始页编号的偏移*/
	sidx = PFN_UP(addr) - PFN_DOWN(bdata->node_boot_start);
	/*获取指定地址区间结束处与自举分配器起始处的偏移对应的页号*/
	eidx = PFN_DOWN(addr + size - bdata->node_boot_start);
	/*设置指定地址区间对应的页为空闲状态*/
	for (i = sidx; i < eidx; i++)
	{
		/*如果之前区间内的页之前为未使用状态，则是BUG*/
		if (unlikely(!test_and_clear_bit(i, bdata->node_bootmem_map)))
			BUG();
	}
}

/*
 * We 'merge' subsequent allocations to save space. We might 'lose'
 * some fraction of a page if allocations cannot be satisfied due to
 * size constraints on boxes where there is physical RAM space
 * fragmentation - in these cases (mostly large memory boxes) this
 * is not a problem.
 *
 * On low memory boxes we get it right in 100% of the cases.
 *
 * alignment has to be a power of 2 value.
 *
 * NOTE:  This function is _not_ reentrant.
 */
/*在bdata自举分配器的[goal, limit]区间中分配size=ALIGN(size, align)字节长度的内存*/
void * __init __alloc_bootmem_core(struct bootmem_data *bdata, unsigned long size,
	      								unsigned long align, unsigned long goal, unsigned long limit)
{
	unsigned long offset, remaining_size, areasize, preferred;
	unsigned long i, start = 0, incr, eidx, end_pfn;
	void *ret;

	/*指定分配长度不能为零*/
	if (!size)
	{
		printk("__alloc_bootmem_core(): zero-sized request\n");
		BUG();
	}
	/*对齐长度必须是2的幂次方*/
	BUG_ON(align & (align-1));
	/*如果指定的搜索极值还没有超过自举分配器的起始页编号，则直接返回空*/
	if (limit && bdata->node_boot_start >= limit)
		return NULL;

	/*指定的自举分配器无内存（bootmem_map为空），则返回空*/
	if (!bdata->node_bootmem_map)
		return NULL;

	/*获取治治具分配器的结束页*/
	end_pfn = bdata->node_low_pfn;
	/*获取分配结束地址对应的页编号*/
	limit = PFN_DOWN(limit);
	/*如果自举分配器的结束页大于分配结束地址，则重新设置分配结束地址（缩小搜索范围）*/
	if (limit && end_pfn > limit)
		end_pfn = limit;
	/*获取分配结束的页号相对于自举分配器起始页号的偏移*/
	eidx = end_pfn - PFN_DOWN(bdata->node_boot_start);
	/*重新设置偏移值，并将偏移值对齐到页编号*/
	offset = 0;
	if (align && (bdata->node_boot_start & (align - 1UL)) != 0)
		offset = align - (bdata->node_boot_start & (align - 1UL));
	offset = PFN_DOWN(offset);

	/*首先尝试在指定分配域之上分配页，（失败后）再分配低地址的页*/
	if (goal && goal >= bdata->node_boot_start && PFN_DOWN(goal) < end_pfn)
	{
		/*设置目标搜索区相对于起始处的偏移位置*/
		preferred = goal - bdata->node_boot_start;
		/*如果上次分配成功的位置不小于目标搜索区的偏移位*/
		if (bdata->last_success >= preferred)
			/*如果没有设置搜索结束位置或者搜索结束位置在上次分配成功位置之后，则将上次
			搜索成功位置设置为搜索起始位置*/
			if (!limit || (limit && limit > bdata->last_success))
				preferred = bdata->last_success;
	} 
	else
		/*设置开始位置为搜索起始位置*/
		preferred = 0;
	/*重新设置开始搜索位置为偏移后的与指定对齐长度对齐后的页编号*/
	preferred = PFN_DOWN(ALIGN(preferred, align)) + offset;
	/*设置重新对齐页后的页数目*/
	areasize = (size + PAGE_SIZE-1) / PAGE_SIZE;
	/*设置递增因子，对齐长度大于页时对齐到页的倍数，否则，页对齐*/
	incr = align >> PAGE_SHIFT ? : 1;

restart_scan:
	/*查找位图中*/
	for (i = preferred; i < eidx; i += incr)
	{
		unsigned long j;
		/*查找位图[i, i+eidx]区间中未置位的位*/
		i = find_next_zero_bit(bdata->node_bootmem_map, eidx, i);
		/*找到后获取对齐因子后的值*/
		i = ALIGN(i, incr);
		/*区间搜索完毕，没有找到对应的空闲位，退出循环*/
		if (i >= eidx)
			break;
		/*测试找到位已被标识为已用，则继续查找*/
		if (test_bit(i, bdata->node_bootmem_map))
			continue;
		/*找到的位显示该页空闲，则循环找接下来areasize-1个页，如果都是空闲的，则成功
		*/
		for (j = i + 1; j < i + areasize; ++j)
		{
			/*查找到结束位置仍然没有指定连续的空闲页，失败*/
			if (j >= eidx)
				goto fail_block;
			/*查找到的位置显示对应的页已使用，则应该重新定位空闲开始查找问位，并继续查找*/
			if (test_bit(j, bdata->node_bootmem_map))
				goto fail_block;
		}
		/*已查到指定条件的连续长度，设置找到的起始页编号，并跳转到找到标号*/
		start = i;
		goto found;
	fail_block:
		i = ALIGN(j, incr);
	}

	/**/
	if (preferred > offset)
	{
		preferred = offset;
		goto restart_scan;
	}
	return NULL;

found:
	/*查找成功，重新设置上次查找成功的物理地址*/
	bdata->last_success = PFN_PHYS(start);
	/*如果查找到的位置不小于指定的查找结束位置，时BUG*/
	BUG_ON(start >= eidx);

	/*之前分配结束位置的下一页是当前分配缓冲区的开始位置吗？如果是则能合并之前部分页
	作为本次分配*/
	if (align < PAGE_SIZE &&	bdata->last_offset && bdata->last_pos+1 == start)
	{
		offset = ALIGN(bdata->last_offset, align);
		BUG_ON(offset > PAGE_SIZE);
		remaining_size = PAGE_SIZE - offset;
		if (size < remaining_size)
		{
			areasize = 0;
			/* last_pos unchanged */
			bdata->last_offset = offset + size;
			ret = phys_to_virt(bdata->last_pos * PAGE_SIZE +
					   offset +
					   bdata->node_boot_start);
		}
		else
		{
			remaining_size = size - remaining_size;
			areasize = (remaining_size + PAGE_SIZE-1) / PAGE_SIZE;
			ret = phys_to_virt(bdata->last_pos * PAGE_SIZE +
					   offset +
					   bdata->node_boot_start);
			bdata->last_pos = start + areasize - 1;
			bdata->last_offset = remaining_size;
		}
		bdata->last_offset &= ~PAGE_MASK;
	} else {
		bdata->last_pos = start + areasize - 1;
		bdata->last_offset = size & ~PAGE_MASK;
		ret = phys_to_virt(start * PAGE_SIZE + bdata->node_boot_start);
	}

	/*
	 * Reserve the area now:
	 */
	/*将分配成功的页标记为已使用*/
	for (i = start; i < start + areasize; i++)
		/*如果分配成功的页位图显示对应页已使用，程序BUG*/
		if (unlikely(test_and_set_bit(i, bdata->node_bootmem_map)))
			BUG();
	/*将分配成功的内存长度清零*/
	memset(ret, 0, size);
	return ret;
}

/*如果起始物理地址页框号是32对齐的，设置gofast为1。如果gofast等于1，并且从page开始的
32个页都未使用，则将page开始的32个页框回收到buddy系统中，这个通过调用函数
__free_pages(page, order)，一次回收32个页框。如果从page开始的32个页框中存在未使用的，
查找这32个页框中空的页框，并回收到buddy系统中，一次回收一个页框。如果32个页框都使用了
，则什么都不做。然后查找下一组32个页框回收，直到查找完bootmem分配器管理的所有页框*/
static unsigned long __init free_all_bootmem_core(pg_data_t *pgdat)
{
	struct page *page;
	unsigned long pfn;
	/*获取指定结点对应的自举分配器*/
	bootmem_data_t *bdata = pgdat->bdata;
	unsigned long i, count, total = 0;
	unsigned long idx;
	unsigned long *map; 
	int gofast = 0;
	/*指定自举分配器的内存不能为空*/
	BUG_ON(!bdata->node_bootmem_map);

	count = 0;
	/* first extant page of the node */
	/*获取自举分配器上第一个页编号*/
	pfn = PFN_DOWN(bdata->node_boot_start);
	/*获取自举分配器上的页数目*/
	idx = bdata->node_low_pfn - pfn;
	/*获取自举分配器上的页位图地址*/
	map = bdata->node_bootmem_map;
	/*检查起始物理地址是不是O(LOG2(BITS_PER_LONG))页对齐？*/
	/*起始页地址为0或页帧号是不是32位或64位页帧号对齐，ffs(BITS_PER_LONG)是6或7，因
	此，ffs(bdata->node_boot_start)要大于18或19（也即node_boot_start起始位置在512K或
	1M）时if第二个条件才成立*/
	if (bdata->node_boot_start == 0 ||
	    ffs(bdata->node_boot_start) - PAGE_SHIFT > ffs(BITS_PER_LONG))
		gofast = 1;
	/*循环遍历所有位图*/
	for (i = 0; i < idx; )
	{
		/*直接将位图当做无符号长整形数组处理，按位取反，如果该值为0，则说明该
		BITS_PER_LONG个位所对应的页都在使用中，否则，至少有部分页没用使用*/
		unsigned long v = ~map[i / BITS_PER_LONG];
		/*选定的位图段中所有页都没有使用，处理BITS_PER_LONG个页块的快速方法*/
		if (gofast && v == ~0UL)
		{
			int order;
			/*根据页编号获取对应的页*/
			page = pfn_to_page(pfn);
			/*累计空闲页数目*/
			count += BITS_PER_LONG;
			/*IA-32系统，4G内存，order就是5*/
			order = ffs(BITS_PER_LONG) - 1;
			/*释放page为首的32个页，将页放入伙伴系统中*/
			__free_pages_bootmem(page, order);
			/*更新已释放的页数目*/
			i += BITS_PER_LONG;
			/*更新页实例指针*/
			page += BITS_PER_LONG;
		}
		/*选定BITS_PER_LONG个页中至少有部分没有使用*/
		else if (v)
		{
			unsigned long m;
			/*获取指定页编号的页实例*/
			page = pfn_to_page(pfn);
			/*m初值为1，每次左移一位，测试页未占用时才会释放页，循环条件中最多循环
			BITS_PER_LONG次（小于时说明已经到位图末尾）*/
			for (m = 1; m && i < idx; m<<=1, page++, i++)
			{
				/*仅释放未使用的页*/
				if (v & m)
				{
					/*更新已释放的页数目*/
					count++;
					/*释放页*/
					__free_pages_bootmem(page, 0);
				}
			}
		}
		/*指定页块中所有页都是在使用中，直接跳过*/
		else
		{
			i += BITS_PER_LONG;
		}
		/*更新当前以处理过的页编号*/
		pfn += BITS_PER_LONG;
	}
	/*累计已处理的页数目*/
	total += count;
	
	/*现在需要释放自举分配器的位图，该位图以后不在需要了*/
	/*获取位图所在虚拟地址的页实例*/
	page = virt_to_page(bdata->node_bootmem_map);
	count = 0;
	/*获取位图所占的页数目*/
	idx = (get_mapsize(bdata) + PAGE_SIZE-1) >> PAGE_SHIFT;
	/*从头开始循环释放位图所占用的页*/
	for (i = 0; i < idx; i++, page++)
	{
		__free_pages_bootmem(page, 0);
		count++;
	}
	/*更新总共释放的页数目*/
	total += count;
	/*重置自举分配器的页位图为空*/
	bdata->node_bootmem_map = NULL;
	/*返回总共释放的页数目*/
	return total;
}

unsigned long __init init_bootmem_node(pg_data_t *pgdat, unsigned long freepfn,
						unsigned long startpfn, unsigned long endpfn)
{
	return init_bootmem_core(pgdat, freepfn, startpfn, endpfn);
}

/*将治具分配器内指定地址区间对应的页标记为已使用状态（设置保留页）*/
void __init reserve_bootmem_node(pg_data_t *pgdat, unsigned long physaddr, unsigned long size)
{
	reserve_bootmem_core(pgdat->bdata, physaddr, size);
}
/*设置自举分配器中指定的地址区间对应的页为未使用状态*/
void __init free_bootmem_node(pg_data_t *pgdat, unsigned long physaddr, unsigned long size)
{
	free_bootmem_core(pgdat->bdata, physaddr, size);
}

/*释放指定结点对应的所有自举分配器*/
unsigned long __init free_all_bootmem_node(pg_data_t *pgdat)
{
	return free_all_bootmem_core(pgdat);
}

/*创建0结点的自举分配器。初始化该结点的直接映射区所对应的位图*/
unsigned long __init init_bootmem(unsigned long start, unsigned long pages)
{
	max_low_pfn = pages;
	min_low_pfn = start;
	return init_bootmem_core(NODE_DATA(0), start, 0, pages);
}

#ifndef CONFIG_HAVE_ARCH_BOOTMEM_NODE
void __init reserve_bootmem(unsigned long addr, unsigned long size)
{
	reserve_bootmem_core(NODE_DATA(0)->bdata, addr, size);
}
#endif /* !CONFIG_HAVE_ARCH_BOOTMEM_NODE */

/*将0结点的自举分配器中指定地址空间所对应的页在页位图中标记为空闲状态*/
void __init free_bootmem(unsigned long addr, unsigned long size)
{
	free_bootmem_core(NODE_DATA(0)->bdata, addr, size);
}

/*将第一个结点的空闲内存全部释放*/
unsigned long __init free_all_bootmem(void)
{
	return free_all_bootmem_core(NODE_DATA(0));
}
/*从自举分配器goal位置开始，分配size=ALIGN(size, align)字节长度的内存。分配成功则返回
分配地址，否则返回NULL*/
void * __init __alloc_bootmem_nopanic(unsigned long size, unsigned long align, unsigned long goal)
{
	bootmem_data_t *bdata;
	void *ptr;

	list_for_each_entry(bdata, &bdata_list, list)
	{
		ptr = __alloc_bootmem_core(bdata, size, align, goal, 0);
		if (ptr)
			return ptr;
	}
	return NULL;
}
/*从自举分配器goal位置开始，分配size=ALIGN(size, align)字节长度的内存。分配成功时返
回分配地址，失败时打印错误信息，并返回NULL*/
void * __init __alloc_bootmem(unsigned long size, unsigned long align, unsigned long goal)
{
	void *mem = __alloc_bootmem_nopanic(size,align,goal);

	if (mem)
		return mem;
	/*
	 * Whoops, we cannot satisfy the allocation request.
	 */
	printk(KERN_ALERT "bootmem alloc of %lu bytes failed!\n", size);
	panic("Out of memory");
	return NULL;
}

/*在[goal, ARCH_LOW_ADDRESS_LIMIT]区间中分配size=ALIGN(size, align)字节长度的内存。
分配成功时返回分配的地址。失败后尝试该结点上的所有自举分配器分配，成功时返回分配
地址，失败时打印失败信息并返回NULL*/
void * __init __alloc_bootmem_node(pg_data_t *pgdat, unsigned long size, unsigned long align,
										unsigned long goal)
{
	void *ptr;

	ptr = __alloc_bootmem_core(pgdat->bdata, size, align, goal, 0);
	if (ptr)
		return ptr;

	return __alloc_bootmem(size, align, goal);
}

#ifndef ARCH_LOW_ADDRESS_LIMIT
/*定义32位地址空间的最顶端结束地址*/
#define ARCH_LOW_ADDRESS_LIMIT	0xffffffffUL
#endif

/*在[goal, ARCH_LOW_ADDRESS_LIMIT]区间中分配size=ALIGN(size, align)字节长度的内存，
失败时返回NULL，成功时返回已分配的内存地址*/
void * __init __alloc_bootmem_low(unsigned long size, unsigned long align, unsigned long goal)
{
	bootmem_data_t *bdata;
	void *ptr;
	/*遍历自举分配器，在[goal, ARCH_LOW_ADDRESS_LIMIT]区间中分配size=ALIGN(size, align)
	字节长度的内存*/
	list_for_each_entry(bdata, &bdata_list, list)
	{
		/*在[goal, ARCH_LOW_ADDRESS_LIMIT]区间中分配size=ALIGN(size, align)字节长度的
		内存，分配成功字节返回分配地址*/
		ptr = __alloc_bootmem_core(bdata, size, align, goal,	ARCH_LOW_ADDRESS_LIMIT);
		if (ptr)
			return ptr;
	}

	/*不能满足分配请求，则打印内存不足信息，并返回NULL*/
	printk(KERN_ALERT "low bootmem alloc of %lu bytes failed!\n", size);
	panic("Out of low memory");
	return NULL;
}

/*从[goal, ARCH_LOW_ADDRESS_LIMIT]区间，分配size=ALIGN(size, align)字节长度的内存*/
void * __init __alloc_bootmem_low_node(pg_data_t *pgdat, unsigned long size,
				       							unsigned long align, unsigned long goal)
{
	return __alloc_bootmem_core(pgdat->bdata, size, align, goal,	ARCH_LOW_ADDRESS_LIMIT);
}
