#include <linux/module.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/swap.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/pfn.h>
#include <linux/poison.h>
#include <linux/bootmem.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/efi.h>
#include <linux/memory_hotplug.h>
#include <linux/initrd.h>
#include <linux/cpumask.h>
#include <asm/processor.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/dma.h>
#include <asm/fixmap.h>
#include <asm/e820.h>
#include <asm/apic.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <asm/sections.h>
#include <asm/paravirt.h>

/*VMALLOC���򳤶�*/
unsigned int __VMALLOC_RESERVE = 128 << 20;

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);
/*��������������Ϊ�߶��ڴ���ֹҳ��*/
unsigned long highstart_pfn, highend_pfn;

static int noinline do_test_wp_bit(void);

/*
 * Creates a middle page table and puts a pointer to it in the
 * given global directory entry. This only returns the gd entry
 * in non-PAE compilation mode, since the middle layer is folded.
 */
/*����һ�����ܼ�ҳ���ҷ���ҳĿ¼���ָ��*/
static pmd_t * __init one_md_table_init(pgd_t *pgd)
{
	pud_t *pud;
	pmd_t *pmd_table;

#ifdef CONFIG_X86_PAE
	if (!(pgd_val(*pgd) & _PAGE_PRESENT)) {
		pmd_table = (pmd_t *) alloc_bootmem_low_pages(PAGE_SIZE);

		paravirt_alloc_pd(__pa(pmd_table) >> PAGE_SHIFT);
		set_pgd(pgd, __pgd(__pa(pmd_table) | _PAGE_PRESENT));
		pud = pud_offset(pgd, 0);
		if (pmd_table != pmd_offset(pud, 0))
			BUG();
	}
#endif
	pud = pud_offset(pgd, 0);
	pmd_table = pmd_offset(pud, 0);
	return pmd_table;
}

/*
 * Create a page table and place a pointer to it in a middle page
 * directory entry.
 */
static pte_t * __init one_page_table_init(pmd_t *pmd)
{
	if (!(pmd_val(*pmd) & _PAGE_PRESENT)) {
		pte_t *page_table = NULL;

#ifdef CONFIG_DEBUG_PAGEALLOC
		page_table = (pte_t *) alloc_bootmem_pages(PAGE_SIZE);
#endif
		if (!page_table)
			page_table =
				(pte_t *)alloc_bootmem_low_pages(PAGE_SIZE);

		paravirt_alloc_pt(&init_mm, __pa(page_table) >> PAGE_SHIFT);
		set_pmd(pmd, __pmd(__pa(page_table) | _PAGE_TABLE));
		BUG_ON(page_table != pte_offset_kernel(pmd, 0));
	}

	return pte_offset_kernel(pmd, 0);
}

/*
 * This function initializes a certain range of kernel virtual memory
 * with new bootmem page tables, everywhere page tables are missing in
 * the given range.
 */

/*
 * NOTE: The pagetables are allocated contiguous on the physical space
 * so we can cache the place of the first one and move around without
 * checking the pgd every time.
 */
static void __init page_table_range_init (unsigned long start, unsigned long end, pgd_t *pgd_base)
{
	pgd_t *pgd;
	pmd_t *pmd;
	int pgd_idx, pmd_idx;
	unsigned long vaddr;

	vaddr = start;
	pgd_idx = pgd_index(vaddr);
	pmd_idx = pmd_index(vaddr);
	pgd = pgd_base + pgd_idx;

	for ( ; (pgd_idx < PTRS_PER_PGD) && (vaddr != end); pgd++, pgd_idx++) {
		pmd = one_md_table_init(pgd);
		pmd = pmd + pmd_index(vaddr);
		for (; (pmd_idx < PTRS_PER_PMD) && (vaddr != end); pmd++, pmd_idx++) {
			one_page_table_init(pmd);

			vaddr += PMD_SIZE;
		}
		pmd_idx = 0;
	}
}

/*�ж�ָ����ַ�Ƿ����ں˴�����С���[PAGE_OFFSET, __init_end]�򷵻�1������0*/
static inline int is_kernel_text(unsigned long addr)
{
	if (addr >= PAGE_OFFSET && addr <= (unsigned long)__init_end)
		return 1;
	return 0;
}

/*
 * This maps the physical memory to kernel virtual address space, a total
 * of max_low_pfn pages, by creating page tables starting from address
 * PAGE_OFFSET.
 */
/**/
static void __init kernel_physical_mapping_init(pgd_t *pgd_base)
{
	unsigned long pfn;
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	int pgd_idx, pmd_idx, pte_ofs;
	/*��ȡҳĿ¼������*/
	pgd_idx = pgd_index(PAGE_OFFSET);
	/*��ȡ��Ӧ��ҳĿ¼����*/
	pgd = pgd_base + pgd_idx;
	pfn = 0;
	/*�ӵ�ǰҳĿ¼�ʼ�������ѯֱ��ҳĿ¼�������*/
	for (; pgd_idx < PTRS_PER_PGD; pgd++, pgd_idx++)
	{
		pmd = one_md_table_init(pgd);
		if (pfn >= max_low_pfn)
			continue;
		for (pmd_idx = 0; pmd_idx < PTRS_PER_PMD && pfn < max_low_pfn; pmd++, pmd_idx++) {
			unsigned int address = pfn * PAGE_SIZE + PAGE_OFFSET;

			/* Map with big pages if possible, otherwise create normal page tables. */
			if (cpu_has_pse) {
				unsigned int address2 = (pfn + PTRS_PER_PTE - 1) * PAGE_SIZE + PAGE_OFFSET + PAGE_SIZE-1;
				if (is_kernel_text(address) || is_kernel_text(address2))
					set_pmd(pmd, pfn_pmd(pfn, PAGE_KERNEL_LARGE_EXEC));
				else
					set_pmd(pmd, pfn_pmd(pfn, PAGE_KERNEL_LARGE));

				pfn += PTRS_PER_PTE;
			} else {
				pte = one_page_table_init(pmd);

				for (pte_ofs = 0;
				     pte_ofs < PTRS_PER_PTE && pfn < max_low_pfn;
				     pte++, pfn++, pte_ofs++, address += PAGE_SIZE) {
					if (is_kernel_text(address))
						set_pte(pte, pfn_pte(pfn, PAGE_KERNEL_EXEC));
					else
						set_pte(pte, pfn_pte(pfn, PAGE_KERNEL));
				}
			}
		}
	}
}

/*ҳ�����[0x70000, 0x7003F]��*/
static inline int page_kills_ppro(unsigned long pagenr)
{
	if (pagenr >= 0x70000 && pagenr <= 0x7003F)
		return 1;
	return 0;
}

int page_is_ram(unsigned long pagenr)
{
	int i;
	unsigned long addr, end;

	if (efi_enabled)
	{
		efi_memory_desc_t *md;
		void *p;
		/**/
		for (p = memmap.map; p < memmap.map_end; p += memmap.desc_size)
		{
			md = p;
			if (!is_available_memory(md))
				continue;
			addr = (md->phys_addr+PAGE_SIZE-1) >> PAGE_SHIFT;
			end = (md->phys_addr + (md->num_pages << EFI_PAGE_SHIFT)) >> PAGE_SHIFT;

			if ((pagenr >= addr) && (pagenr < end))
				return 1;
		}
		return 0;
	}

	for (i = 0; i < e820.nr_map; i++) {

		if (e820.map[i].type != E820_RAM)	/* not usable memory */
			continue;
		/*
		 *	!!!FIXME!!! Some BIOSen report areas as RAM that
		 *	are not. Notably the 640->1Mb area. We need a sanity
		 *	check here.
		 */
		addr = (e820.map[i].addr+PAGE_SIZE-1) >> PAGE_SHIFT;
		end = (e820.map[i].addr+e820.map[i].size) >> PAGE_SHIFT;
		if  ((pagenr >= addr) && (pagenr < end))
			return 1;
	}
	return 0;
}

#ifdef CONFIG_HIGHMEM
pte_t *kmap_pte;
pgprot_t kmap_prot;

#define kmap_get_fixmap_pte(vaddr)					\
	pte_offset_kernel(pmd_offset(pud_offset(pgd_offset_k(vaddr), vaddr), (vaddr)), (vaddr))

static void __init kmap_init(void)
{
	unsigned long kmap_vstart;

	/* cache the first kmap pte */
	kmap_vstart = __fix_to_virt(FIX_KMAP_BEGIN);
	kmap_pte = kmap_get_fixmap_pte(kmap_vstart);

	kmap_prot = PAGE_KERNEL;
}

static void __init permanent_kmaps_init(pgd_t *pgd_base)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long vaddr;

	vaddr = PKMAP_BASE;
	page_table_range_init(vaddr, vaddr + PAGE_SIZE*LAST_PKMAP, pgd_base);

	pgd = swapper_pg_dir + pgd_index(vaddr);
	pud = pud_offset(pgd, vaddr);
	pmd = pmd_offset(pud, vaddr);
	pte = pte_offset_kernel(pmd, vaddr);
	pkmap_page_table = pte;
}

/*�ͷŸ߶��ڴ�ҳ*/
static void __meminit free_new_highpage(struct page *page)
{
	/*��ʼ��ҳ����Ϊ1*/
	init_page_count(page);
	/*�ͷ�ҳ*/
	__free_page(page);
	/*���¸߶��ڴ�ҳ��Ŀ*/
	totalhigh_pages++;
}
/**/
void __init add_one_highpage_init(struct page *page, int pfn, int bad_ppro)
{
	/**/
	if (page_is_ram(pfn) && !(bad_ppro && page_kills_ppro(pfn)))
	{
		/*���ҳ�ı�����ʶλ*/
		ClearPageReserved(page);
		/*�ͷŸ�ҳ����ҳ����per-CPU���ٻ���*/
		free_new_highpage(page);
	}
	else
		/*����Ϊ����ҳ*/
		SetPageReserved(page);
}

static int __meminit add_one_highpage_hotplug(struct page *page, unsigned long pfn)
{
	free_new_highpage(page);
	totalram_pages++;
#ifdef CONFIG_FLATMEM
	max_mapnr = max(pfn, max_mapnr);
#endif
	num_physpages++;
	return 0;
}

/*
 * Not currently handling the NUMA case.
 * Assuming single node and all memory that
 * has been added dynamically that would be
 * onlined here is in HIGHMEM
 */
void __meminit online_page(struct page *page)
{
	ClearPageReserved(page);
	add_one_highpage_hotplug(page, page_to_pfn(page));
}


#ifdef CONFIG_NUMA
extern void set_highmem_pages_init(int);
#else
/**/
static void __init set_highmem_pages_init(int bad_ppro)
{
	int pfn;
	/*�Ӹ߶��ڴ�ҳ����ʼҳ��ŵ�����ҳ���*/
	for (pfn = highstart_pfn; pfn < highend_pfn; pfn++) {
		/*SPARSEMEMģ���пն���Ӧ��ҳ����mem_map�����б��档���ҳ�����Ч����ҳ*/
		if (pfn_valid(pfn))
			add_one_highpage_init(pfn_to_page(pfn), pfn, bad_ppro);
	}
	totalram_pages += totalhigh_pages;
}
#endif /* CONFIG_FLATMEM */

#else
#define kmap_init() do { } while (0)
#define permanent_kmaps_init(pgd_base) do { } while (0)
#define set_highmem_pages_init(bad_ppro) do { } while (0)
#endif /* CONFIG_HIGHMEM */

unsigned long long __PAGE_KERNEL = _PAGE_KERNEL;
EXPORT_SYMBOL(__PAGE_KERNEL);
unsigned long long __PAGE_KERNEL_EXEC = _PAGE_KERNEL_EXEC;

#ifdef CONFIG_NUMA
extern void __init remap_numa_kva(void);
#else
#define remap_numa_kva() do {} while (0)
#endif

void __init native_pagetable_setup_start(pgd_t *base)
{
#ifdef CONFIG_X86_PAE
	int i;

	/*
	 * Init entries of the first-level page table to the
	 * zero page, if they haven't already been set up.
	 *
	 * In a normal native boot, we'll be running on a
	 * pagetable rooted in swapper_pg_dir, but not in PAE
	 * mode, so this will end up clobbering the mappings
	 * for the lower 24Mbytes of the address space,
	 * without affecting the kernel address space.
	 */
	for (i = 0; i < USER_PTRS_PER_PGD; i++)
		set_pgd(&base[i],
			__pgd(__pa(empty_zero_page) | _PAGE_PRESENT));

	/* Make sure kernel address space is empty so that a pagetable
	   will be allocated for it. */
	memset(&base[USER_PTRS_PER_PGD], 0,
	       KERNEL_PGD_PTRS * sizeof(pgd_t));
#else
	paravirt_alloc_pd(__pa(swapper_pg_dir) >> PAGE_SHIFT);
#endif
}

void __init native_pagetable_setup_done(pgd_t *base)
{
#ifdef CONFIG_X86_PAE
	/*
	 * Add low memory identity-mappings - SMP needs it when
	 * starting up on an AP from real-mode. In the non-PAE
	 * case we already have these mappings through head.S.
	 * All user-space mappings are explicitly cleared after
	 * SMP startup.
	 */
	set_pgd(&base[0], base[USER_PTRS_PER_PGD]);
#endif
}

/*
 * Build a proper pagetable for the kernel mappings.  Up until this
 * point, we've been running on some set of pagetables constructed by
 * the boot process.
 *
 * If we're booting on native hardware, this will be a pagetable
 * constructed in arch/i386/kernel/head.S, and not running in PAE mode
 * (even if we'll end up running in PAE).  The root of the pagetable
 * will be swapper_pg_dir.
 *
 * If we're booting paravirtualized under a hypervisor, then there are
 * more options: we may already be running PAE, and the pagetable may
 * or may not be based in swapper_pg_dir.  In any case,
 * paravirt_pagetable_setup_start() will set up swapper_pg_dir
 * appropriately for the rest of the initialization to work.
 *
 * In general, pagetable_init() assumes that the pagetable may already
 * be partially populated, and so it avoids stomping on any existing
 * mappings.
 */
static void __init pagetable_init (void)
{
	unsigned long vaddr, end;
	/*ҳ��ĸ�Ŀ¼��swapper_pg_dir*/
	pgd_t *pgd_base = swapper_pg_dir;

	paravirt_pagetable_setup_start(pgd_base);

	/*���PSE����������PSE���ܡ��Գ����ڴ�ҳ��Page Size Extensions����֧�֡���Щ�ر�
	��ǵ�ҳ���䳤��Ϊ4MB����������ͨ��4KB����ѡ�����ڲ��ỻ�����ں�ҳ������ҳ�Ĵ�С��
	��ζ����Ҫ��ҳ������٣���Ե�ַת���󱸻�������TLB����Ӱ��ʱ����ģ����Լ�������
	�����ں˵Ļ���ҳ*/
	if (cpu_has_pse)
		set_in_cr4(X86_CR4_PSE);

	/*���PGE���ã�������PGE��Page Global Enable�����ܣ���Ҳ��__PAGE_KERNEL��
	__PAGE_KERNEL_EXEC������__PAGE_GLOBAL����λ�Ѿ���λ��ԭ����Щ����ָ���ں�����
	����ҳ֡ʱ�ı�־���������Щ���û��Զ���Ӧ�õ��ں�ҳ�����������л��ڼ䣬������
	__PAGE_GLOBALΪ��ҳ����Ӧ��TLB�������TLB��ˢ���������ں����ǳ����������ַ�ռ�
	��ͬ����λ�ã��������ϵͳ���ܣ����ڱ���ʹ�ں����ݾ�����ã�����Ч��Ҳ�Ǻ��ܻ�ӭ��*/
	if (cpu_has_pge)
	{
		set_in_cr4(X86_CR4_PGE);
		__PAGE_KERNEL |= _PAGE_GLOBAL;
		__PAGE_KERNEL_EXEC |= _PAGE_GLOBAL;
	}
	/**/
	kernel_physical_mapping_init(pgd_base);
	/**/
	remap_numa_kva();

	/*
	 * Fixed mappings, only the page table structure has to be
	 * created - mappings will be set by set_fixmap():
	 */
	vaddr = __fix_to_virt(__end_of_fixed_addresses - 1) & PMD_MASK;
	end = (FIXADDR_TOP + PMD_SIZE - 1) & PMD_MASK;
	page_table_range_init(vaddr, end, pgd_base);

	permanent_kmaps_init(pgd_base);

	paravirt_pagetable_setup_done(pgd_base);
}

#if defined(CONFIG_HIBERNATION) || defined(CONFIG_ACPI)
/*
 * Swap suspend & friends need this for resume because things like the intel-agp
 * driver might have split up a kernel 4MB mapping.
 */
char __nosavedata swsusp_pg_dir[PAGE_SIZE]
	__attribute__ ((aligned (PAGE_SIZE)));

static inline void save_pg_dir(void)
{
	memcpy(swsusp_pg_dir, swapper_pg_dir, PAGE_SIZE);
}
#else
static inline void save_pg_dir(void)
{
}
#endif

void zap_low_mappings (void)
{
	int i;

	save_pg_dir();

	/*
	 * Zap initial low-memory mappings.
	 *
	 * Note that "pgd_clear()" doesn't do it for
	 * us, because pgd_clear() is a no-op on i386.
	 */
	for (i = 0; i < USER_PTRS_PER_PGD; i++)
#ifdef CONFIG_X86_PAE
		set_pgd(swapper_pg_dir+i, __pgd(1 + __pa(empty_zero_page)));
#else
		set_pgd(swapper_pg_dir+i, __pgd(0));
#endif
	flush_tlb_all();
}

int nx_enabled = 0;

#ifdef CONFIG_X86_PAE

static int disable_nx __initdata = 0;
u64 __supported_pte_mask __read_mostly = ~_PAGE_NX;
EXPORT_SYMBOL_GPL(__supported_pte_mask);

/*
 * noexec = on|off
 *
 * Control non executable mappings.
 *
 * on      Enable
 * off     Disable
 */
static int __init noexec_setup(char *str)
{
	if (!str || !strcmp(str, "on")) {
		if (cpu_has_nx) {
			__supported_pte_mask |= _PAGE_NX;
			disable_nx = 0;
		}
	} else if (!strcmp(str,"off")) {
		disable_nx = 1;
		__supported_pte_mask &= ~_PAGE_NX;
	} else
		return -EINVAL;

	return 0;
}
early_param("noexec", noexec_setup);

static void __init set_nx(void)
{
	unsigned int v[4], l, h;

	if (cpu_has_pae && (cpuid_eax(0x80000000) > 0x80000001)) {
		cpuid(0x80000001, &v[0], &v[1], &v[2], &v[3]);
		if ((v[3] & (1 << 20)) && !disable_nx) {
			rdmsr(MSR_EFER, l, h);
			l |= EFER_NX;
			wrmsr(MSR_EFER, l, h);
			nx_enabled = 1;
			__supported_pte_mask |= _PAGE_NX;
		}
	}
}

/*
 * Enables/disables executability of a given kernel page and
 * returns the previous setting.
 */
int __init set_kernel_exec(unsigned long vaddr, int enable)
{
	pte_t *pte;
	int ret = 1;

	if (!nx_enabled)
		goto out;

	pte = lookup_address(vaddr);
	BUG_ON(!pte);

	if (!pte_exec_kernel(*pte))
		ret = 0;

	if (enable)
		pte->pte_high &= ~(1 << (_PAGE_BIT_NX - 32));
	else
		pte->pte_high |= 1 << (_PAGE_BIT_NX - 32);
	pte_update_defer(&init_mm, vaddr, pte);
	__flush_tlb_all();
out:
	return ret;
}

#endif

/*
 * paging_init() sets up the page tables - note that the first 8MB are
 * already mapped by head.S.
 *
 * This routines also unmaps the page at virtual kernel address 0, so
 * that we can trap those pesky NULL-reference errors in the kernel.
 */
/*����ҳ��ע�⣡��һ��8M�Ѿ���head.sӳ�䣬�ú���Ҳȡ�������ں˵�ַ0���ڵ�ҳ��ӳ��
����ˣ����Բ����ں��еĿ�ָ�����*/
void __init paging_init(void)
{
#ifdef CONFIG_X86_PAE
	set_nx();
	if (nx_enabled)
		printk("NX (Execute Disable) protection: active\n");
#endif
	/**/
	pagetable_init();
	/**/
	load_cr3(swapper_pg_dir);

#ifdef CONFIG_X86_PAE
	/*
	 * We will bail out later - printk doesn't work right now so
	 * the user would just see a hanging kernel.
	 */
	if (cpu_has_pae)
		set_in_cr4(X86_CR4_PAE);
#endif
	__flush_tlb_all();

	kmap_init();
}

/*
 * Test if the WP bit works in supervisor mode. It isn't supported on 386's
 * and also on some strange 486's (NexGen etc.). All 586+'s are OK. This
 * used to involve black magic jumps to work around some nasty CPU bugs,
 * but fortunately the switch to using exceptions got rid of all that.
 */

static void __init test_wp_bit(void)
{
	printk("Checking if this processor honours the WP bit even in supervisor mode... ");

	/* Any page-aligned address will do, the test is non-destructive */
	__set_fixmap(FIX_WP_TEST, __pa(&swapper_pg_dir), PAGE_READONLY);
	boot_cpu_data.wp_works_ok = do_test_wp_bit();
	clear_fixmap(FIX_WP_TEST);

	if (!boot_cpu_data.wp_works_ok) {
		printk("No.\n");
#ifdef CONFIG_X86_WP_WORKS_OK
		panic("This kernel doesn't support CPU's with broken WP. Recompile it for a 386!");
#endif
	} else {
		printk("Ok.\n");
	}
}

/*�ں�ת������*/
static struct kcore_list kcore_mem, kcore_vmalloc;

/*�ڴ��ʼ��*/
void __init mem_init(void)
{
	extern int ppro_with_ram_bug(void);
	int codesize, reservedpages, datasize, initsize;
	int tmp;
	int bad_ppro;

#ifdef CONFIG_FLATMEM
	/*FLATMEM�ڴ�ģ���У�����ȫ��ҳ���鲻��Ϊ�գ�Ҳ���Ǳ������ڴ�*/
	BUG_ON(!mem_map);
#endif
	/**/
	bad_ppro = ppro_with_ram_bug();

#ifdef CONFIG_HIGHMEM
	/*����и߶��ڴ棬����־�ӳ�������и߶��ڴ�ʱ������Ŵ��ڣ��͹̶�ӳ�����Ƿ��ص�
	���ص�ʱ��ӡ������Ϣ��崻�*/
	if (PKMAP_BASE+LAST_PKMAP*PAGE_SIZE >= FIXADDR_START)
	{
		printk(KERN_ERR "fixmap and kmap areas overlap - this will crash\n");
		printk(KERN_ERR "pkstart: %lxh pkend: %lxh fixstart %lxh\n", PKMAP_BASE,
				PKMAP_BASE+LAST_PKMAP*PAGE_SIZE, FIXADDR_START);
		BUG();
	}
#endif
	/*����һ������ֱ��ӳ�����п����ڴ�ҳȫ��������ϵͳ����ͳ��ֱ��ӳ������ҳ֡��Ŀ*/
	totalram_pages += free_all_bootmem();
	/*��ͳ�Ʊ�����RAMҳ��Ŀ*/
	reservedpages = 0;
	for (tmp = 0; tmp < max_low_pfn; tmp++)
		if (page_is_ram(tmp) && PageReserved(pfn_to_page(tmp)))
			reservedpages++;
	/*��ʼ��ZONE_HIGHMEM�ڴ���*/
	set_highmem_pages_init(bad_ppro);
	/*���ô���γ���*/
	codesize =  (unsigned long) &_etext - (unsigned long) &_text;
	/*�����ѳ�ʼ�����ݶγ���*/
	datasize =  (unsigned long) &_edata - (unsigned long) &_etext;
	/*���ó�ʼ�����ݶγ���*/
	initsize =  (unsigned long) &__init_end - (unsigned long) &__init_begin;
	/*��ֱ��ӳ�����ں�ת��������kclist����*/
	kclist_add(&kcore_mem, __va(0), max_low_pfn << PAGE_SHIFT);
	/*��VMALLOC�����ں�ת��������kclist����*/
	kclist_add(&kcore_vmalloc, (void *)VMALLOC_START, VMALLOC_END-VMALLOC_START);

	/*��KBΪ��λ��ʾ�����ڴ桢����γ��ȡ�����ҳ��Ŀ�����ݶγ��ȡ���ʼ�����ݶγ��ȡ�
	�ܹ��߶��ڴ泤����Ϣ*/
	printk(KERN_INFO "Memory: %luk/%luk available (%dk kernel code, %dk reserved, %dk data, %dk init, %ldk highmem)\n",
		(unsigned long) nr_free_pages() << (PAGE_SHIFT-10),	num_physpages << (PAGE_SHIFT-10),
		codesize >> 10,	reservedpages << (PAGE_SHIFT-10),	datasize >> 10,	initsize >> 10,
		(unsigned long) (totalhigh_pages << (PAGE_SHIFT-10))       );

	/*��ʾ�ں������ڴ沼����Ϣ*/
#if 1 /* double-sanity-check paranoia */
	printk("virtual kernel memory layout:\n" "    fixmap  : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#ifdef CONFIG_HIGHMEM
			"    pkmap   : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#endif
			"    vmalloc : 0x%08lx - 0x%08lx   (%4ld MB)\n"
		    "    lowmem  : 0x%08lx - 0x%08lx   (%4ld MB)\n"
		    "      .init : 0x%08lx - 0x%08lx   (%4ld kB)\n"
		    "      .data : 0x%08lx - 0x%08lx   (%4ld kB)\n"
	    	"      .text : 0x%08lx - 0x%08lx   (%4ld kB)\n",
	    	/*�̶�ӳ������ֹ��ַ����KBΪ��λ�ĳ���*/
	    	FIXADDR_START, FIXADDR_TOP, (FIXADDR_TOP - FIXADDR_START) >> 10,

#ifdef CONFIG_HIGHMEM
			/*�־�ӳ������ֹ��ַ����KBΪ��λ�ĳ���*/
	    	PKMAP_BASE, PKMAP_BASE+LAST_PKMAP*PAGE_SIZE, 	       (LAST_PKMAP*PAGE_SIZE) >> 10,
#endif
			/*VMALLOC�ڴ��������ֹ��ַ����MBΪ��λ�ĳ���*/
			VMALLOC_START, VMALLOC_END, (VMALLOC_END - VMALLOC_START) >> 20,
			/*ϵͳ������8M�ڴ�������ֱ��ӳ������VMALLOC����֮�䣩��ֹ��ַ�ͳ���*/
			(unsigned long)__va(0), (unsigned long)high_memory,
			((unsigned long)high_memory - (unsigned long)__va(0)) >> 20,
			/*��ʼ��������ֹ��ַ����KBΪ��λ�ĳ���*/
			(unsigned long)&__init_begin, (unsigned long)&__init_end,
			((unsigned long)&__init_end - (unsigned long)&__init_begin) >> 10,
			/*�ں��ѳ�ʼ�����ݵ���ֹ��ַ����KBΪ��λ�ĳ���*/
			(unsigned long)&_etext, (unsigned long)&_edata,
			((unsigned long)&_edata - (unsigned long)&_etext) >> 10,
			/*�ں˴������ֹ��ַ����KBΪ��λ�ĳ���*/
			(unsigned long)&_text, (unsigned long)&_etext,
			((unsigned long)&_etext - (unsigned long)&_text) >> 10);

#ifdef CONFIG_HIGHMEM
	/*�־�ӳ�����͹̶�ӳ�����䲻���ص�*/
	BUG_ON(PKMAP_BASE+LAST_PKMAP*PAGE_SIZE > FIXADDR_START);
	/*VMALLOC�����ܺͳ־�ӳ�����ص�*/
	BUG_ON(VMALLOC_END > PKMAP_BASE);
#endif
	/*VMALLOC�������ʼ��ֹ��ַ������˳�����*/
	BUG_ON(VMALLOC_START > VMALLOC_END);
	/*�߶��ڴ������ʼ��ַ���ܴ���VMALLOC�������ʼ��ַ����Ϊ�м仹��Ĭ��8M�ı�������*/
	BUG_ON((unsigned long)high_memory      > VMALLOC_START);
#endif /* double-sanity-check paranoia */

#ifdef CONFIG_X86_PAE
	if (!cpu_has_pae)
		panic("cannot execute a PAE-enabled kernel on a PAE-less CPU!");
#endif
	/**/
	if (boot_cpu_data.wp_works_ok < 0)
		test_wp_bit();

	/*
	 * Subtle. SMP is doing it's boot stuff late (because it has to
	 * fork idle threads) - but it also needs low mappings for the
	 * protected-mode entry to work. We zap these entries only after
	 * the WP-bit has been tested.
	 */
#ifndef CONFIG_SMP
	zap_low_mappings();
#endif
}

#ifdef CONFIG_MEMORY_HOTPLUG
int arch_add_memory(int nid, u64 start, u64 size)
{
	struct pglist_data *pgdata = NODE_DATA(nid);
	struct zone *zone = pgdata->node_zones + ZONE_HIGHMEM;
	unsigned long start_pfn = start >> PAGE_SHIFT;
	unsigned long nr_pages = size >> PAGE_SHIFT;

	return __add_pages(zone, start_pfn, nr_pages);
}

#endif

struct kmem_cache *pmd_cache;

void __init pgtable_cache_init(void)
{
	if (PTRS_PER_PMD > 1)
		pmd_cache = kmem_cache_create("pmd", PTRS_PER_PMD*sizeof(pmd_t),
					      PTRS_PER_PMD*sizeof(pmd_t), SLAB_PANIC, pmd_ctor);
}

/*�ú������ܱ��__init����Ϊ�ý׶��쳣��������û�������ã����Ҳ��������������ʶ*/
static int noinline do_test_wp_bit(void)
{
	char tmp_reg;
	int flag;

	__asm__ __volatile__(
		"	movb %0,%1	\n"
		"1:	movb %1,%0	\n"
		"	xorl %2,%2	\n"
		"2:			\n"
		".section __ex_table,\"a\"\n"
		"	.align 4	\n"
		"	.long 1b,2b	\n"
		".previous		\n"
		:"=m" (*(char *)fix_to_virt(FIX_WP_TEST)),
		 "=q" (tmp_reg),
		 "=r" (flag)
		:"2" (1)
		:"memory");

	return flag;
}

#ifdef CONFIG_DEBUG_RODATA

/*���ֻ������*/
void mark_rodata_ro(void)
{
	/*�ں˴���ε���ʼλ��*/
	unsigned long start = PFN_ALIGN(_text);
	/*�ں˴���εĳ���*/
	unsigned long size = PFN_ALIGN(_etext) - start;

#ifndef CONFIG_KPROBES
#ifdef CONFIG_HOTPLUG_CPU
	/* It must still be possible to apply SMP alternatives. */
	/**/
	if (num_possible_cpus() <= 1)
#endif
	{
		/*���ں˴���ε�ҳ��������д����Ȩ��*/
		change_page_attr(virt_to_page(start), size >> PAGE_SHIFT, PAGE_KERNEL_RX);
		printk("Write protecting the kernel text: %luk\n", size >> 10);
	}
#endif
	/*ָ��Ų���ѳ�ʼ��������ʼ��ַ*/
	start += size;
	/*�ѳ�ʼ��ֻ�����ݶγ���*/
	size = (unsigned long)__end_rodata - start;
	/*�����ѳ�ʼ���ں����ݶε�ֻ������*/
	change_page_attr(virt_to_page(start), size >> PAGE_SHIFT, PAGE_KERNEL_RO);
	printk("Write protecting the kernel read-only data: %luk\n", size >> 10);
	/*change_page_attr()ִ�к���Ҫ����global_flush_tlb()������֮ǰ����Ҫ����printk��
	���Ƿ��޸�ҳ����ʱ�������ٳ���ʱ���ṩ�����Ϣ*/
	global_flush_tlb();
}
#endif
/**/
void free_init_pages(char *what, unsigned long begin, unsigned long end)
{
	unsigned long addr;
	/*�ͷ���ֹ��ַ�е�ҳ*/
	for (addr = begin; addr < end; addr += PAGE_SIZE)
	{
		/*���ҳ������ʶ���˴�Ϊɶ������ʱ��������virt_to_page(addr)�ý���������ּ���
		һ�Σ��˷���Դ*/
		ClearPageReserved(virt_to_page(addr));
		/*��ҳ�����ü�������Ϊ1*/
		init_page_count(virt_to_page(addr));
		/*�ͷų�ʼ���ڼ�ʹ�õ����ݺʹ�����ռ��ҳ�棬����ҳ����*/
		memset((void *)addr, POISON_FREE_INITMEM, PAGE_SIZE);
		/*�ͷ�ҳ����ҳ����per-CPU���ٻ������ҳ������*/
		free_page(addr);
		totalram_pages++;
	}
	/*PLKA���ᵽ��Freeing unused kernel memory: 308K freed*/
	printk(KERN_INFO "Freeing %s: %luk freed\n", what, (end - begin) >> 10);
}
/*�ͷų�ʼ�����ڴ�����������صķ��������ϵͳ�����������̸պý���ʱ�ص��øú�������
�����init��Ϊϵͳ�е�һ������������������־������һ����Ϣ��ָ���ͷ��˶����ڴ�*/
void free_initmem(void)
{
	free_init_pages("unused kernel memory",	(unsigned long)(&__init_begin),
						(unsigned long)(&__init_end));
}

#ifdef CONFIG_BLK_DEV_INITRD
void free_initrd_mem(unsigned long start, unsigned long end)
{
	free_init_pages("initrd memory", start, end);
}
#endif

