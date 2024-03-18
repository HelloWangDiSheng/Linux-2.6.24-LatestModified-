#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <linux/highmem.h>
#include <linux/initrd.h>
#include <linux/nodemask.h>
#include <linux/module.h>
#include <linux/kexec.h>
#include <linux/pfn.h>
#include <linux/swap.h>

#include <asm/e820.h>
#include <asm/setup.h>
#include <asm/mmzone.h>
#include <bios_ebda.h>

/*保存系统中所有NUMA结点的全局NUMA结点指针数组*/
struct pglist_data *node_data[MAX_NUMNODES] __read_mostly;
EXPORT_SYMBOL(node_data);
static bootmem_data_t node0_bdata;

/*node_online_map   		位图中所有结点在系统中都已配置（在线）*/
/*结点中起始页帧编号，大部分时候是读操作*/
unsigned long node_start_pfn[MAX_NUMNODES] __read_mostly;
/*结点中结束页帧编号，大部分时候是读操作*/
unsigned long node_end_pfn[MAX_NUMNODES] __read_mostly;


#ifdef CONFIG_DISCONTIGMEM
/*
 * 4) physnode_map     - the mapping between a pfn and owning node
 * physnode_map keeps track of the physical memory layout of a generic
 * numa node on a 256Mb break (each element of the array will
 * represent 256Mb of memory and will be marked by the node id.  so,
 * if the first gig is on node 0, and the second gig is on node 1
 * physnode_map will contain:
 *
 *     physnode_map[0-3] = 0;
 *     physnode_map[4-7] = 1;
 *     physnode_map[8- ] = -1;
 */
s8 physnode_map[MAX_ELEMENTS] __read_mostly = { [0 ... (MAX_ELEMENTS - 1)] = -1};
EXPORT_SYMBOL(physnode_map);

/*将start和end之间的页都标记为nid结点所有*/
void memory_present(int nid, unsigned long start, unsigned long end)
{
	unsigned long pfn;

	printk(KERN_INFO "Node: %d, start_pfn: %ld, end_pfn: %ld\n", nid, start, end);
	printk(KERN_DEBUG "  Setting physnode_map array to node %d for pfns:\n", nid);
	printk(KERN_DEBUG "  ");
	/*设置指定起始和结束页之间的所属结点*/
	for (pfn = start; pfn < end; pfn += PAGES_PER_ELEMENT)
	{
		physnode_map[pfn / PAGES_PER_ELEMENT] = nid;
		printk("%ld ", pfn);
	}
	printk("\n");
}

/*获取结点中为保存起始页和结束页的页数目所需的页管理信息所占空间*/
unsigned long node_memmap_size_bytes(int nid, unsigned long start_pfn, unsigned long end_pfn)
{
	/*获取结点中起始页和结束页之间的页数目*/
	unsigned long nr_pages = end_pfn - start_pfn;
	/*如果页数目为零，则返回零*/
	if (!nr_pages)
		return 0;
	/*获取需要为保存页管理信息所需的空间*/
	return (nr_pages + 1) * sizeof(struct page);
}
#endif

extern unsigned long find_max_low_pfn(void);
extern void add_one_highpage_init(struct page *, int, int);
extern unsigned long highend_pfn, highstart_pfn;

#define LARGE_PAGE_BYTES (PTRS_PER_PTE * PAGE_SIZE)

static unsigned long node_remap_start_pfn[MAX_NUMNODES];
unsigned long node_remap_size[MAX_NUMNODES];
static unsigned long node_remap_offset[MAX_NUMNODES];
static void *node_remap_start_vaddr[MAX_NUMNODES];
void set_pmd_pfn(unsigned long vaddr, unsigned long pfn, pgprot_t flags);

static void *node_remap_end_vaddr[MAX_NUMNODES];
static void *node_remap_alloc_vaddr[MAX_NUMNODES];
static unsigned long kva_start_pfn;
static unsigned long kva_pages;
/*
 * FLAT - support for basic PC memory model with discontig enabled, essentially
 *        a single node with all available processors in it with a flat
 *        memory map.
 */
int __init get_memcfg_numa_flat(void)
{
	printk("NUMA - single node, flat memory mode\n");

	/* Run the memory configuration and find the top of memory. */
	/*运行内存配置并查找内存顶端。e820获取内存配置信息*/
	find_max_pfn();
	/*设置结点0的起始地址是0*/
	node_start_pfn[0] = 0;
	/*设置结点0的虽大页帧编号*/
	node_end_pfn[0] = max_pfn;
	/*设置结点0的起始页帧编号*/
	memory_present(0, 0, max_pfn);
	/*将在线结点位图清零*/
	nodes_clear(node_online_map);
	/*设置结点0在线，表明只有一个结点可用*/
	node_set_online(0);
	return 1;
}

/*获取结点内可以使用的最大页帧号*/
static void __init find_max_pfn_node(int nid)
{
	/*系统启动期间，设置结点内最大页帧号为直接映射区的最大页帧号*/
	if (node_end_pfn[nid] > max_pfn)
		node_end_pfn[nid] = max_pfn;
	/*
	 * if a user has given mem=XXXX, then we need to make sure 
	 * that the node _starts_ before that, too, not just ends
	 */
	 /*如果用户已经设置了mem=XXX，需要确保结点结点的起始页帧号在???*/
	if (node_start_pfn[nid] > max_pfn)
		node_start_pfn[nid] = max_pfn;
	/*结点内起始页帧号不能大于结点内最大页帧号*/
	BUG_ON(node_start_pfn[nid] > node_end_pfn[nid]);
}

/* 
 * Allocate memory for the pg_data_t for this node via a crude pre-bootmem
 * method.  For node zero take this from the bottom of memory, for
 * subsequent nodes place them at node_remap_start_vaddr which contains
 * node local data in physically node local memory.  See setup_memory()
 * for details.
 */
/**/
static void __init allocate_pgdat(int nid)
{
	/*不是第一个结点编号并且结点在线状态，直接从*/
	if (nid && node_has_online_mem(nid))
		NODE_DATA(nid) = (pg_data_t *)node_remap_start_vaddr[nid];
	else
	{
		NODE_DATA(nid) = (pg_data_t *)(pfn_to_kaddr(min_low_pfn));
		min_low_pfn += PFN_UP(sizeof(pg_data_t));
	}
}

void *alloc_remap(int nid, unsigned long size)
{
	void *allocation = node_remap_alloc_vaddr[nid];

	size = ALIGN(size, L1_CACHE_BYTES);

	if (!allocation || (allocation + size) >= node_remap_end_vaddr[nid])
		return 0;

	node_remap_alloc_vaddr[nid] += size;
	memset(allocation, 0, size);

	return allocation;
}

void __init remap_numa_kva(void)
{
	void *vaddr;
	unsigned long pfn;
	int node;

	for_each_online_node(node) {
		for (pfn=0; pfn < node_remap_size[node]; pfn += PTRS_PER_PTE) {
			vaddr = node_remap_start_vaddr[node]+(pfn<<PAGE_SHIFT);
			set_pmd_pfn((ulong) vaddr, 
				node_remap_start_pfn[node] + pfn, 
				PAGE_KERNEL_LARGE);
		}
	}
}

static unsigned long calculate_numa_remap_pages(void)
{
	int nid;
	unsigned long size, reserve_pages = 0;
	unsigned long pfn;
	/*遍历所有在线结点*/
	for_each_online_node(nid)
	{
		/*获取结点的最大页帧编号*/
		unsigned old_end_pfn = node_end_pfn[nid];

		/*
		 * The acpi/srat node info can show hot-add memroy zones
		 * where memory could be added but not currently present.
		 */
		if (node_start_pfn[nid] > max_pfn)
			continue;
		if (node_end_pfn[nid] > max_pfn)
			node_end_pfn[nid] = max_pfn;

		/* ensure the remap includes space for the pgdat. */
		size = node_remap_size[nid] + sizeof(pg_data_t);

		/* convert size to large (pmd size) pages, rounding up */
		size = (size + LARGE_PAGE_BYTES - 1) / LARGE_PAGE_BYTES;
		/* now the roundup is correct, convert to PAGE_SIZE pages */
		size = size * PTRS_PER_PTE;

		/*
		 * Validate the region we are allocating only contains valid
		 * pages.
		 */
		for (pfn = node_end_pfn[nid] - size;
		     pfn < node_end_pfn[nid]; pfn++)
			if (!page_is_ram(pfn))
				break;

		if (pfn != node_end_pfn[nid])
			size = 0;

		printk("Reserving %ld pages of KVA for lmem_map of node %d\n",
				size, nid);
		node_remap_size[nid] = size;
		node_remap_offset[nid] = reserve_pages;
		reserve_pages += size;
		printk("Shrinking node %d from %ld pages to %ld pages\n",
			nid, node_end_pfn[nid], node_end_pfn[nid] - size);

		if (node_end_pfn[nid] & (PTRS_PER_PTE-1)) {
			/*
			 * Align node_end_pfn[] and node_remap_start_pfn[] to
			 * pmd boundary. remap_numa_kva will barf otherwise.
			 */
			printk("Shrinking node %d further by %ld pages for proper alignment\n",
				nid, node_end_pfn[nid] & (PTRS_PER_PTE-1));
			size +=  node_end_pfn[nid] & (PTRS_PER_PTE-1);
		}

		node_end_pfn[nid] -= size;
		node_remap_start_pfn[nid] = node_end_pfn[nid];
		shrink_active_range(nid, old_end_pfn, node_end_pfn[nid]);
	}
	printk("Reserving total of %ld pages for numa KVA remap\n",
			reserve_pages);
	return reserve_pages;
}

/*返回直接映射区总最后一页的编号*/
extern void setup_bootmem_allocator(void);
unsigned long __init setup_memory(void)
{
	int nid;
	unsigned long system_start_pfn, system_max_low_pfn;

	/*
	 * When mapping a NUMA machine we allocate the node_mem_map arrays
	 * from node local memory.  They are then mapped directly into KVA
	 * between zone normal and vmalloc space.  Calculate the size of
	 * this space and use it to adjust the boundary between ZONE_NORMAL
	 * and ZONE_HIGHMEM.
	 */
	/*当映射一个NUMA机器时，从结点本地内存分配node_mem_map数组。它们直接映射在内核的
	普通内存域和vmalloc空间之间的虚拟区域。计算该空间大小并用它来调整ZONE_NORMAL和
	ZONE_HIGHMEN之间的边界*/
	find_max_pfn();
	get_memcfg_numa();

	kva_pages = calculate_numa_remap_pages();

	/* partially used pages are not usable - thus round upwards */
	system_start_pfn = min_low_pfn = PFN_UP(init_pg_tables_end);

	kva_start_pfn = find_max_low_pfn() - kva_pages;

#ifdef CONFIG_BLK_DEV_INITRD
	/* Numa kva area is below the initrd */
	if (boot_params.hdr.type_of_loader && boot_params.hdr.ramdisk_image)
		kva_start_pfn = PFN_DOWN(boot_params.hdr.ramdisk_image)
			- kva_pages;
#endif
	kva_start_pfn -= kva_start_pfn & (PTRS_PER_PTE-1);

	system_max_low_pfn = max_low_pfn = find_max_low_pfn();
	printk("kva_start_pfn ~ %ld find_max_low_pfn() ~ %ld\n",
		kva_start_pfn, max_low_pfn);
	printk("max_pfn = %ld\n", max_pfn);
#ifdef CONFIG_HIGHMEM
	highstart_pfn = highend_pfn = max_pfn;
	if (max_pfn > system_max_low_pfn)
		highstart_pfn = system_max_low_pfn;
	printk(KERN_NOTICE "%ldMB HIGHMEM available.\n",
	       pages_to_mb(highend_pfn - highstart_pfn));
	num_physpages = highend_pfn;
	high_memory = (void *) __va(highstart_pfn * PAGE_SIZE - 1) + 1;
#else
	num_physpages = system_max_low_pfn;
	high_memory = (void *) __va(system_max_low_pfn * PAGE_SIZE - 1) + 1;
#endif
	printk(KERN_NOTICE "%ldMB LOWMEM available.\n",
			pages_to_mb(system_max_low_pfn));
	printk("min_low_pfn = %ld, max_low_pfn = %ld, highstart_pfn = %ld\n", 
			min_low_pfn, max_low_pfn, highstart_pfn);

	printk("Low memory ends at vaddr %08lx\n",
			(ulong) pfn_to_kaddr(max_low_pfn));
	for_each_online_node(nid) {
		node_remap_start_vaddr[nid] = pfn_to_kaddr(
				kva_start_pfn + node_remap_offset[nid]);
		/* Init the node remap allocator */
		node_remap_end_vaddr[nid] = node_remap_start_vaddr[nid] +
			(node_remap_size[nid] * PAGE_SIZE);
		node_remap_alloc_vaddr[nid] = node_remap_start_vaddr[nid] +
			ALIGN(sizeof(pg_data_t), PAGE_SIZE);

		allocate_pgdat(nid);
		printk ("node %d will remap to vaddr %08lx - %08lx\n", nid,
			(ulong) node_remap_start_vaddr[nid],
			(ulong) pfn_to_kaddr(highstart_pfn
			   + node_remap_offset[nid] + node_remap_size[nid]));
	}
	printk("High memory starts at vaddr %08lx\n",
			(ulong) pfn_to_kaddr(highstart_pfn));
	for_each_online_node(nid)
		find_max_pfn_node(nid);

	memset(NODE_DATA(0), 0, sizeof(struct pglist_data));
	NODE_DATA(0)->bdata = &node0_bdata;
	setup_bootmem_allocator();
	return max_low_pfn;
}

void __init numa_kva_reserve(void)
{
	reserve_bootmem(PFN_PHYS(kva_start_pfn),PFN_PHYS(kva_pages));
}

/*内存域初始化*/
void __init zone_sizes_init(void)
{
	int nid;
	unsigned long max_zone_pfns[MAX_NR_ZONES];
	memset(max_zone_pfns, 0, sizeof(max_zone_pfns));
	/*保存ZONE_DMA内存域的页帧编号*/
	max_zone_pfns[ZONE_DMA] = virt_to_phys((char *)MAX_DMA_ADDRESS) >> PAGE_SHIFT;
	/*保存ZONE_NORMAL内存域中的最大页帧编号*/
	max_zone_pfns[ZONE_NORMAL] = max_low_pfn;
#ifdef CONFIG_HIGHMEM
	/*保存ZONE_HIGHMEM内存域最大页帧编号*/
	max_zone_pfns[ZONE_HIGHMEM] = highend_pfn;
#endif

	/* If SRAT has not registered memory, register it now */
	/*如果内存域没有注册，则现在注册它*/
	if (find_max_pfn_with_active_regions() == 0)
	{
		/*遍历所有在线结点*/
		for_each_online_node(nid)
		{
			/*注册结点及对应的起止页帧号*/
			if (node_has_online_mem(nid))
				add_active_range(nid, node_start_pfn[nid],	node_end_pfn[nid]);
		}
	}

	free_area_init_nodes(max_zone_pfns);
	return;
}

void __init set_highmem_pages_init(int bad_ppro) 
{
#ifdef CONFIG_HIGHMEM
	struct zone *zone;
	struct page *page;

	for_each_zone(zone)
	{
		unsigned long node_pfn, zone_start_pfn, zone_end_pfn;
		/*非高端内存域直接跳过*/
		if (!is_highmem(zone))
			continue;
		/*获取高端内存域的起始页帧号*/
		zone_start_pfn = zone->zone_start_pfn;
		/*获得高端内存域的结束页帧号*/
		zone_end_pfn = zone_start_pfn + zone->spanned_pages;
		/*打印高端内存名称、结点号。起始页帧号、终止页帧号信息*/
		printk("Initializing %s for node %d (%08lx:%08lx)\n", zone->name, zone_to_nid(zone),
				zone_start_pfn, zone_end_pfn);
		/*遍历所有高端内存页，将*/
		for (node_pfn = zone_start_pfn; node_pfn < zone_end_pfn; node_pfn++)
		{
			/*无效页帧号直接跳过*/
			if (!pfn_valid(node_pfn))
				continue;
			/*获取页帧号对应的页实例*/
			page = pfn_to_page(node_pfn);
			/*将非保留页放入伙伴系统*/
			add_one_highpage_init(page, node_pfn, bad_ppro);
		}
	}
	/*更新结点有效页总数*/
	totalram_pages += totalhigh_pages;
#endif
}

#ifdef CONFIG_MEMORY_HOTPLUG
static int paddr_to_nid(u64 addr)
{
	int nid;
	unsigned long pfn = PFN_DOWN(addr);

	for_each_node(nid)
		if (node_start_pfn[nid] <= pfn &&
		    pfn < node_end_pfn[nid])
			return nid;

	return -1;
}

/*
 * This function is used to ask node id BEFORE memmap and mem_section's
 * initialization (pfn_to_nid() can't be used yet).
 * If _PXM is not defined on ACPI's DSDT, node id must be found by this.
 */
int memory_add_physaddr_to_nid(u64 addr)
{
	int nid = paddr_to_nid(addr);
	return (nid >= 0) ? nid : 0;
}

EXPORT_SYMBOL_GPL(memory_add_physaddr_to_nid);
#endif
