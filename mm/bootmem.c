/*�����ڼ�򵥵������ڴ���������ͷ��������������ڴ���ϵͳ�����ڴ��ϵͳ�ն�*/

#include <linux/init.h>
#include <linux/pfn.h>
#include <linux/bootmem.h>
#include <linux/module.h>

#include <asm/bug.h>
#include <asm/io.h>
#include <asm/processor.h>

#include "internal.h"

/*�ⲿ��������������ʱ�����ģ���Ҫ���з��ʸ���ϵͳ*/
/*ֱ��ӳ���������ҳ֡��*/
unsigned long max_low_pfn;
/*ֱ��ӳ��������Сҳ֡��*/
unsigned long min_low_pfn;
/*���ҳ֡��*/
unsigned long max_pfn;
/*�Ծٷ�����˫�����ͷ*/
static LIST_HEAD(bdata_list);
#ifdef CONFIG_CRASH_DUMP

/*�����Ϊ������Ҫ������max_pfn������һ����С��ֵ����Ҫֱ���ں�֮ǰʹ�õ��ڴ�����*/
unsigned long saved_max_pfn;
#endif

/*���ؽ�ҪΪ����λͼ�����ҳ��Ŀ����IA-32ϵͳ��4G�ڴ棬ֱ��ӳ����896M��Ӧ���ڴ棬
��Ҫ896M/4K=224k=224*1024=229376��ҳ����ô�������е�mapsize=(229376+7)/8=28672��
mapsize = (mapsize + ~PAGE_MASK) & PAGE_MASK=(28672 + 4095) & (~4095)=0X7FFF�ĵ�
12λȫ�����㣬��mapsize >>= PAGE_SHIFT���Ϊ7��ֱ�۵Ľ����(224K+4096)/(4K*8)=7��
���߼�����һ��*/
unsigned long __init bootmem_bootmap_pages(unsigned long pages)
{
	unsigned long mapsize;
	/*ҳ��Ŀ���뵽�ֽڵ���Ŀ��������100��ҳ���Ǿ���Ҫ13��unsigned char*/
	mapsize = (pages+7)/8;
	/*��Ҫ��ҳ��Ŀ*/
	mapsize = (mapsize + ~PAGE_MASK) & PAGE_MASK;
	/*��Ҫҳ����Ŀ*/
	mapsize >>= PAGE_SHIFT;

	return mapsize;
}

/*���Ծٷ���������ʼҳ�����С�����˳����������������*/
static void __init link_bootmem(bootmem_data_t *bdata)
{
	bootmem_data_t *ent;
	/*���ȫ���Ծٷ�������˫����Ϊ�գ�ͷ�巨����ָ�����Ծٷ�����*/
	if (list_empty(&bdata_list))
	{
		list_add(&bdata->list, &bdata_list);
		return;
	}
	/*��������ʼҳ�����С�������в����Ծٷ�����*/
	list_for_each_entry(ent, &bdata_list, list)
	{
		/*���Ը��Ծٷ���������������˳��������Ծٷ���������ʼҳ��С��ϵ�����С�ڣ�
		������Ѳ��ҵ��Ľ��֮ǰ*/
		if (bdata->node_boot_start < ent->node_boot_start) {
			list_add_tail(&bdata->list, &ent->list);
			return;
		}
	}
	/*ԭ�����е������Ծٷ���������ʼҳ��Ŷ�С�ڸ��Ծٷ���������ʼҳ��ţ���˲����
	λ�������*/
	list_add_tail(&bdata->list, &bdata_list);
}

/*����һ���Ѿ���ʼ�����Ծٷ���������������Ҫ����long���ͱ�����ʾ��λͼ�ռ���Ŀ����
�磬IA-32ϵͳ��4G�ڴ棬��Ӧ896M��ֱ��ӳ�������Ծٷ���������ʼҳ�����0�����һ����
ֱ�ӹ����ҳ�����896M/4K=224k=224*1024=229376�������м����mapsize��28672����
size(long)��������28672������Ľ�����ǻ�ȡ�Ծٷ�����������ҳ��Ӧλͼ��ռ�ֽ���Ŀ*/
static unsigned long __init get_mapsize(bootmem_data_t *bdata)
{
	unsigned long mapsize;
	/*��ȡ�Ծٷ���������ʼҳ��Ŷ�Ӧ��ҳ�ı��*/
	unsigned long start = PFN_DOWN(bdata->node_boot_start);
	/*��ȡ�Ծٷ����������һ��ҳ�ı��*/
	unsigned long end = bdata->node_low_pfn;
	/*��ȡ�Ծٷ���������Ч��ŷ�Χ������ֽ���Ŀ*/
	mapsize = ((end - start) + 7) / 8;
	/*��ȡλͼ����Ч�ֽ���Ŀ��long���Ͷ�������Ŀ*/
	return ALIGN(mapsize, sizeof(long));
}

/*ֻ����һ�Σ�����ָ�������Ծٷ�����*/
static unsigned long __init init_bootmem_core(pg_data_t *pgdat, unsigned long mapstart,
													unsigned long start, unsigned long end)
{
	/*��ȡ��������ģ���һ�����Ծٷ����������������н϶಻�����Ŀն�����ý��Ӧ��
	��Ӧ����Ծٷ�����ʵ����*/
	bootmem_data_t *bdata = pgdat->bdata;
	unsigned long mapsize;
	/*��ȡ�Ծٷ�������λͼ����Ϊϵͳ�����ڼ�������ֱ��ӳ��������ˣ�����ֱ�Ӹ���ҳ
	�Ż�ȡ�������ַ��Ȼ��ת���������ַ*/
	bdata->node_bootmem_map = phys_to_virt(PFN_PHYS(mapstart));
	/*��ȡ��ʼҳ��Ӧ��ӳ�����е������ַ*/
	bdata->node_boot_start = PFN_PHYS(start);
	bdata->node_low_pfn = end;
	/*���Ծٷ��������뵽�������ȫ���Ծٷ�������������*/
	link_bootmem(bdata);

	/*�ڳ�������ҳ������Ϊ����ҳ��setup_arch()������ʽע�����RAM��*/
	/*��ȡ���Ծٷ�����������Чҳ��Χ����Ҫ��long���ͱ�������Ŀ*/
	mapsize = get_mapsize(bdata);
	/*��λͼ��ǰ�ķ�֮һλ��λ*/
	memset(bdata->node_bootmem_map, 0xff, mapsize);

	return mapsize;
}

/*���һ���ر������ڴ����ǲ��ɷ���ġ����õ�RAM�����Ѿ��������ڼ��ѱ�����ʹ�ã�����
�Ժ���ӵ�����ҳ�أ����������ڼ�_end��������struct *node_mem_mapҳ���飩��*/
static void __init reserve_bootmem_core(bootmem_data_t *bdata, unsigned long addr,
											unsigned long size)
{
	unsigned long sidx, eidx;
	unsigned long i;

	/*�����ϣ����ֱ�������ҳ�汻��Ϊ����ȫ������*/
	 /*ָ���ĳ��Ȳ���Ϊ��*/
	BUG_ON(!size);
	/*ָ���ĵ�ַ��Ӧ��ҳ��Ų��ܳ����Ծٷ����������ҳ���*/
	BUG_ON(PFN_DOWN(addr) >= bdata->node_low_pfn);
	/*ָ���ĵ�ַ��������ҳ����һ��ҳ��Ų��ܳ����Ծٷ����������һ��ҳ���*/
	BUG_ON(PFN_UP(addr + size) > bdata->node_low_pfn);
	/*��ȡָ����ַ��ǰƫ���Ծٷ�������ʼҳ��ŵ�ҳ���*/
	sidx = PFN_DOWN(addr - bdata->node_boot_start);
	/*��ȡָ����ַ�ռ�����ַ������Ծٷ�������ʼҳ���ƫ�Ƶĺ�һ��ҳ���*/
	eidx = PFN_UP(addr + size - bdata->node_boot_start);
	/*��ָ����ַ��������Ӧ��ҳ����Ϊ��λ״̬*/
	for (i = sidx; i < eidx; i++)
		if (test_and_set_bit(i, bdata->node_bootmem_map))
		{
#ifdef CONFIG_DEBUG_BOOTMEM
			printk("hm, page %08lx reserved twice.\n", i*PAGE_SIZE);
#endif
		}
}

/*�����Ծٷ�������ָ���ĵ�ַ�����Ӧ��ҳΪδʹ��״̬*/
static void __init free_bootmem_core(bootmem_data_t *bdata, unsigned long addr,
				     					unsigned long size)
{
	unsigned long sidx, eidx;
	unsigned long i;

	/*ȡ�����ڴ��ǰһҳ�����ֿ���ҳ����Ϊ�Ǳ�����*/
	/*ָ���ĳ��Ȳ���Ϊ��*/
	BUG_ON(!size);
	/*ָ����ַ�����������ҳ��Ų��ܴ����������Ծٷ��������ҳ���*/
	BUG_ON(PFN_DOWN(addr + size) > bdata->node_low_pfn);
	/*���ָ����ַС���ϴη���ɹ��ĵ�ַ�������������ϴγɹ������ַΪָ����ַ*/
	if (addr < bdata->last_success)
		bdata->last_success = addr;

	/*
	 * Round up the beginning of the address.
	 */
	/*��ȡָ����ַ��һ��ҳ��ź��Ծٷ�������ʼҳ��ŵ�ƫ��*/
	sidx = PFN_UP(addr) - PFN_DOWN(bdata->node_boot_start);
	/*��ȡָ����ַ������������Ծٷ�������ʼ����ƫ�ƶ�Ӧ��ҳ��*/
	eidx = PFN_DOWN(addr + size - bdata->node_boot_start);
	/*����ָ����ַ�����Ӧ��ҳΪ����״̬*/
	for (i = sidx; i < eidx; i++)
	{
		/*���֮ǰ�����ڵ�ҳ֮ǰΪδʹ��״̬������BUG*/
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
/*��bdata�Ծٷ�������[goal, limit]�����з���size=ALIGN(size, align)�ֽڳ��ȵ��ڴ�*/
void * __init __alloc_bootmem_core(struct bootmem_data *bdata, unsigned long size,
	      								unsigned long align, unsigned long goal, unsigned long limit)
{
	unsigned long offset, remaining_size, areasize, preferred;
	unsigned long i, start = 0, incr, eidx, end_pfn;
	void *ret;

	/*ָ�����䳤�Ȳ���Ϊ��*/
	if (!size)
	{
		printk("__alloc_bootmem_core(): zero-sized request\n");
		BUG();
	}
	/*���볤�ȱ�����2���ݴη�*/
	BUG_ON(align & (align-1));
	/*���ָ����������ֵ��û�г����Ծٷ���������ʼҳ��ţ���ֱ�ӷ��ؿ�*/
	if (limit && bdata->node_boot_start >= limit)
		return NULL;

	/*ָ�����Ծٷ��������ڴ棨bootmem_mapΪ�գ����򷵻ؿ�*/
	if (!bdata->node_bootmem_map)
		return NULL;

	/*��ȡ���ξ߷������Ľ���ҳ*/
	end_pfn = bdata->node_low_pfn;
	/*��ȡ���������ַ��Ӧ��ҳ���*/
	limit = PFN_DOWN(limit);
	/*����Ծٷ������Ľ���ҳ���ڷ��������ַ�����������÷��������ַ����С������Χ��*/
	if (limit && end_pfn > limit)
		end_pfn = limit;
	/*��ȡ���������ҳ��������Ծٷ�������ʼҳ�ŵ�ƫ��*/
	eidx = end_pfn - PFN_DOWN(bdata->node_boot_start);
	/*��������ƫ��ֵ������ƫ��ֵ���뵽ҳ���*/
	offset = 0;
	if (align && (bdata->node_boot_start & (align - 1UL)) != 0)
		offset = align - (bdata->node_boot_start & (align - 1UL));
	offset = PFN_DOWN(offset);

	/*���ȳ�����ָ��������֮�Ϸ���ҳ����ʧ�ܺ��ٷ���͵�ַ��ҳ*/
	if (goal && goal >= bdata->node_boot_start && PFN_DOWN(goal) < end_pfn)
	{
		/*����Ŀ���������������ʼ����ƫ��λ��*/
		preferred = goal - bdata->node_boot_start;
		/*����ϴη���ɹ���λ�ò�С��Ŀ����������ƫ��λ*/
		if (bdata->last_success >= preferred)
			/*���û��������������λ�û�����������λ�����ϴη���ɹ�λ��֮�����ϴ�
			�����ɹ�λ������Ϊ������ʼλ��*/
			if (!limit || (limit && limit > bdata->last_success))
				preferred = bdata->last_success;
	} 
	else
		/*���ÿ�ʼλ��Ϊ������ʼλ��*/
		preferred = 0;
	/*�������ÿ�ʼ����λ��Ϊƫ�ƺ����ָ�����볤�ȶ�����ҳ���*/
	preferred = PFN_DOWN(ALIGN(preferred, align)) + offset;
	/*�������¶���ҳ���ҳ��Ŀ*/
	areasize = (size + PAGE_SIZE-1) / PAGE_SIZE;
	/*���õ������ӣ����볤�ȴ���ҳʱ���뵽ҳ�ı���������ҳ����*/
	incr = align >> PAGE_SHIFT ? : 1;

restart_scan:
	/*����λͼ��*/
	for (i = preferred; i < eidx; i += incr)
	{
		unsigned long j;
		/*����λͼ[i, i+eidx]������δ��λ��λ*/
		i = find_next_zero_bit(bdata->node_bootmem_map, eidx, i);
		/*�ҵ����ȡ�������Ӻ��ֵ*/
		i = ALIGN(i, incr);
		/*����������ϣ�û���ҵ���Ӧ�Ŀ���λ���˳�ѭ��*/
		if (i >= eidx)
			break;
		/*�����ҵ�λ�ѱ���ʶΪ���ã����������*/
		if (test_bit(i, bdata->node_bootmem_map))
			continue;
		/*�ҵ���λ��ʾ��ҳ���У���ѭ���ҽ�����areasize-1��ҳ��������ǿ��еģ���ɹ�
		*/
		for (j = i + 1; j < i + areasize; ++j)
		{
			/*���ҵ�����λ����Ȼû��ָ�������Ŀ���ҳ��ʧ��*/
			if (j >= eidx)
				goto fail_block;
			/*���ҵ���λ����ʾ��Ӧ��ҳ��ʹ�ã���Ӧ�����¶�λ���п�ʼ������λ������������*/
			if (test_bit(j, bdata->node_bootmem_map))
				goto fail_block;
		}
		/*�Ѳ鵽ָ���������������ȣ������ҵ�����ʼҳ��ţ�����ת���ҵ����*/
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
	/*���ҳɹ������������ϴβ��ҳɹ��������ַ*/
	bdata->last_success = PFN_PHYS(start);
	/*������ҵ���λ�ò�С��ָ���Ĳ��ҽ���λ�ã�ʱBUG*/
	BUG_ON(start >= eidx);

	/*֮ǰ�������λ�õ���һҳ�ǵ�ǰ���仺�����Ŀ�ʼλ������������ܺϲ�֮ǰ����ҳ
	��Ϊ���η���*/
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
	/*������ɹ���ҳ���Ϊ��ʹ��*/
	for (i = start; i < start + areasize; i++)
		/*�������ɹ���ҳλͼ��ʾ��Ӧҳ��ʹ�ã�����BUG*/
		if (unlikely(test_and_set_bit(i, bdata->node_bootmem_map)))
			BUG();
	/*������ɹ����ڴ泤������*/
	memset(ret, 0, size);
	return ret;
}

/*�����ʼ�����ַҳ�����32����ģ�����gofastΪ1�����gofast����1�����Ҵ�page��ʼ��
32��ҳ��δʹ�ã���page��ʼ��32��ҳ����յ�buddyϵͳ�У����ͨ�����ú���
__free_pages(page, order)��һ�λ���32��ҳ�������page��ʼ��32��ҳ���д���δʹ�õģ�
������32��ҳ���пյ�ҳ�򣬲����յ�buddyϵͳ�У�һ�λ���һ��ҳ�����32��ҳ��ʹ����
����ʲô��������Ȼ�������һ��32��ҳ����գ�ֱ��������bootmem���������������ҳ��*/
static unsigned long __init free_all_bootmem_core(pg_data_t *pgdat)
{
	struct page *page;
	unsigned long pfn;
	/*��ȡָ������Ӧ���Ծٷ�����*/
	bootmem_data_t *bdata = pgdat->bdata;
	unsigned long i, count, total = 0;
	unsigned long idx;
	unsigned long *map; 
	int gofast = 0;
	/*ָ���Ծٷ��������ڴ治��Ϊ��*/
	BUG_ON(!bdata->node_bootmem_map);

	count = 0;
	/* first extant page of the node */
	/*��ȡ�Ծٷ������ϵ�һ��ҳ���*/
	pfn = PFN_DOWN(bdata->node_boot_start);
	/*��ȡ�Ծٷ������ϵ�ҳ��Ŀ*/
	idx = bdata->node_low_pfn - pfn;
	/*��ȡ�Ծٷ������ϵ�ҳλͼ��ַ*/
	map = bdata->node_bootmem_map;
	/*�����ʼ�����ַ�ǲ���O(LOG2(BITS_PER_LONG))ҳ���룿*/
	/*��ʼҳ��ַΪ0��ҳ֡���ǲ���32λ��64λҳ֡�Ŷ��룬ffs(BITS_PER_LONG)��6��7����
	�ˣ�ffs(bdata->node_boot_start)Ҫ����18��19��Ҳ��node_boot_start��ʼλ����512K��
	1M��ʱif�ڶ��������ų���*/
	if (bdata->node_boot_start == 0 ||
	    ffs(bdata->node_boot_start) - PAGE_SHIFT > ffs(BITS_PER_LONG))
		gofast = 1;
	/*ѭ����������λͼ*/
	for (i = 0; i < idx; )
	{
		/*ֱ�ӽ�λͼ�����޷��ų��������鴦����λȡ���������ֵΪ0����˵����
		BITS_PER_LONG��λ����Ӧ��ҳ����ʹ���У����������в���ҳû��ʹ��*/
		unsigned long v = ~map[i / BITS_PER_LONG];
		/*ѡ����λͼ��������ҳ��û��ʹ�ã�����BITS_PER_LONG��ҳ��Ŀ��ٷ���*/
		if (gofast && v == ~0UL)
		{
			int order;
			/*����ҳ��Ż�ȡ��Ӧ��ҳ*/
			page = pfn_to_page(pfn);
			/*�ۼƿ���ҳ��Ŀ*/
			count += BITS_PER_LONG;
			/*IA-32ϵͳ��4G�ڴ棬order����5*/
			order = ffs(BITS_PER_LONG) - 1;
			/*�ͷ�pageΪ�׵�32��ҳ����ҳ������ϵͳ��*/
			__free_pages_bootmem(page, order);
			/*�������ͷŵ�ҳ��Ŀ*/
			i += BITS_PER_LONG;
			/*����ҳʵ��ָ��*/
			page += BITS_PER_LONG;
		}
		/*ѡ��BITS_PER_LONG��ҳ�������в���û��ʹ��*/
		else if (v)
		{
			unsigned long m;
			/*��ȡָ��ҳ��ŵ�ҳʵ��*/
			page = pfn_to_page(pfn);
			/*m��ֵΪ1��ÿ������һλ������ҳδռ��ʱ�Ż��ͷ�ҳ��ѭ�����������ѭ��
			BITS_PER_LONG�Σ�С��ʱ˵���Ѿ���λͼĩβ��*/
			for (m = 1; m && i < idx; m<<=1, page++, i++)
			{
				/*���ͷ�δʹ�õ�ҳ*/
				if (v & m)
				{
					/*�������ͷŵ�ҳ��Ŀ*/
					count++;
					/*�ͷ�ҳ*/
					__free_pages_bootmem(page, 0);
				}
			}
		}
		/*ָ��ҳ��������ҳ������ʹ���У�ֱ������*/
		else
		{
			i += BITS_PER_LONG;
		}
		/*���µ�ǰ�Դ������ҳ���*/
		pfn += BITS_PER_LONG;
	}
	/*�ۼ��Ѵ����ҳ��Ŀ*/
	total += count;
	
	/*������Ҫ�ͷ��Ծٷ�������λͼ����λͼ�Ժ�����Ҫ��*/
	/*��ȡλͼ���������ַ��ҳʵ��*/
	page = virt_to_page(bdata->node_bootmem_map);
	count = 0;
	/*��ȡλͼ��ռ��ҳ��Ŀ*/
	idx = (get_mapsize(bdata) + PAGE_SIZE-1) >> PAGE_SHIFT;
	/*��ͷ��ʼѭ���ͷ�λͼ��ռ�õ�ҳ*/
	for (i = 0; i < idx; i++, page++)
	{
		__free_pages_bootmem(page, 0);
		count++;
	}
	/*�����ܹ��ͷŵ�ҳ��Ŀ*/
	total += count;
	/*�����Ծٷ�������ҳλͼΪ��*/
	bdata->node_bootmem_map = NULL;
	/*�����ܹ��ͷŵ�ҳ��Ŀ*/
	return total;
}

unsigned long __init init_bootmem_node(pg_data_t *pgdat, unsigned long freepfn,
						unsigned long startpfn, unsigned long endpfn)
{
	return init_bootmem_core(pgdat, freepfn, startpfn, endpfn);
}

/*���ξ߷�������ָ����ַ�����Ӧ��ҳ���Ϊ��ʹ��״̬�����ñ���ҳ��*/
void __init reserve_bootmem_node(pg_data_t *pgdat, unsigned long physaddr, unsigned long size)
{
	reserve_bootmem_core(pgdat->bdata, physaddr, size);
}
/*�����Ծٷ�������ָ���ĵ�ַ�����Ӧ��ҳΪδʹ��״̬*/
void __init free_bootmem_node(pg_data_t *pgdat, unsigned long physaddr, unsigned long size)
{
	free_bootmem_core(pgdat->bdata, physaddr, size);
}

/*�ͷ�ָ������Ӧ�������Ծٷ�����*/
unsigned long __init free_all_bootmem_node(pg_data_t *pgdat)
{
	return free_all_bootmem_core(pgdat);
}

/*����0�����Ծٷ���������ʼ���ý���ֱ��ӳ��������Ӧ��λͼ*/
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

/*��0�����Ծٷ�������ָ����ַ�ռ�����Ӧ��ҳ��ҳλͼ�б��Ϊ����״̬*/
void __init free_bootmem(unsigned long addr, unsigned long size)
{
	free_bootmem_core(NODE_DATA(0)->bdata, addr, size);
}

/*����һ�����Ŀ����ڴ�ȫ���ͷ�*/
unsigned long __init free_all_bootmem(void)
{
	return free_all_bootmem_core(NODE_DATA(0));
}
/*���Ծٷ�����goalλ�ÿ�ʼ������size=ALIGN(size, align)�ֽڳ��ȵ��ڴ档����ɹ��򷵻�
�����ַ�����򷵻�NULL*/
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
/*���Ծٷ�����goalλ�ÿ�ʼ������size=ALIGN(size, align)�ֽڳ��ȵ��ڴ档����ɹ�ʱ��
�ط����ַ��ʧ��ʱ��ӡ������Ϣ��������NULL*/
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

/*��[goal, ARCH_LOW_ADDRESS_LIMIT]�����з���size=ALIGN(size, align)�ֽڳ��ȵ��ڴ档
����ɹ�ʱ���ط���ĵ�ַ��ʧ�ܺ��Ըý���ϵ������Ծٷ��������䣬�ɹ�ʱ���ط���
��ַ��ʧ��ʱ��ӡʧ����Ϣ������NULL*/
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
/*����32λ��ַ�ռ����˽�����ַ*/
#define ARCH_LOW_ADDRESS_LIMIT	0xffffffffUL
#endif

/*��[goal, ARCH_LOW_ADDRESS_LIMIT]�����з���size=ALIGN(size, align)�ֽڳ��ȵ��ڴ棬
ʧ��ʱ����NULL���ɹ�ʱ�����ѷ�����ڴ��ַ*/
void * __init __alloc_bootmem_low(unsigned long size, unsigned long align, unsigned long goal)
{
	bootmem_data_t *bdata;
	void *ptr;
	/*�����Ծٷ���������[goal, ARCH_LOW_ADDRESS_LIMIT]�����з���size=ALIGN(size, align)
	�ֽڳ��ȵ��ڴ�*/
	list_for_each_entry(bdata, &bdata_list, list)
	{
		/*��[goal, ARCH_LOW_ADDRESS_LIMIT]�����з���size=ALIGN(size, align)�ֽڳ��ȵ�
		�ڴ棬����ɹ��ֽڷ��ط����ַ*/
		ptr = __alloc_bootmem_core(bdata, size, align, goal,	ARCH_LOW_ADDRESS_LIMIT);
		if (ptr)
			return ptr;
	}

	/*������������������ӡ�ڴ治����Ϣ��������NULL*/
	printk(KERN_ALERT "low bootmem alloc of %lu bytes failed!\n", size);
	panic("Out of low memory");
	return NULL;
}

/*��[goal, ARCH_LOW_ADDRESS_LIMIT]���䣬����size=ALIGN(size, align)�ֽڳ��ȵ��ڴ�*/
void * __init __alloc_bootmem_low_node(pg_data_t *pgdat, unsigned long size,
				       							unsigned long align, unsigned long goal)
{
	return __alloc_bootmem_core(pgdat->bdata, size, align, goal,	ARCH_LOW_ADDRESS_LIMIT);
}
