/*
 * linux/mm/allocpercpu.c
 *
 * Separated from slab.c August 11, 2006 Christoph Lameter <clameter@sgi.com>
 */
#include <linux/mm.h>
#include <linux/module.h>

/*释放指定cpu的percpu_data中ptrs->[cpu]的值。为一个即将离线的cpu释放percpu_data数据
是一种很常见的情况，在此之前，需要先为该cpu注册一个可热插拔处理函数*/
void percpu_depopulate(void *__pdata, int cpu)
{
	/*创建与现在对其地址两次取反操作，还原为原来的数值*/
	struct percpu_data *pdata = __percpu_disguise(__pdata);
	kfree(pdata->ptrs[cpu]);
	pdata->ptrs[cpu] = NULL;
}
EXPORT_SYMBOL_GPL(percpu_depopulate);

/**
 * percpu_depopulate_mask - depopulate per-cpu data for some cpu's
 * @__pdata: per-cpu data to depopulate
 * @mask: depopulate per-cpu data for cpu's selected through mask bits
 */
/*释放指定CPU位图中已置位的之前已申请的struct percpu_data->ptrs[cpu]内存*/
void __percpu_depopulate_mask(void *__pdata, cpumask_t *mask)
{
	int cpu;
	/*遍历所有指定位图上的cpu，释放其ptrs[cpu]空间*/
	for_each_cpu_mask(cpu, *mask)
		percpu_depopulate(__pdata, cpu);
}
EXPORT_SYMBOL_GPL(__percpu_depopulate_mask);

/**
 * percpu_populate - populate per-cpu data for given cpu
 * @__pdata: per-cpu data to populate further
 * @size: size of per-cpu object
 * @gfp: may sleep or not etc.
 * @cpu: populate per-data for this cpu
 *
 * Populating per-cpu data for a cpu coming online would be a typical
 * use case. You need to register a cpu hotplug handler for that purpose.
 * Per-cpu object is populated with zeroed buffer.
 */
 /*为一个在即将成为内核管理下的cpu添加一个percpu数据是一种很典型的使用例子，需要在
 为此目的，需要先注册一个cpu热插拔的处理函数，per_cpu对象被添加到一个初始化为0的缓
 冲区中*/
 /*从cpu对应的（否则为当前）结点，创建pdata->ptrs[cpu]，失败时返回NULL*/
void *percpu_populate(void *__pdata, size_t size, gfp_t gfp, int cpu)
{
	/*获取percpu_data原值*/
	struct percpu_data *pdata = __percpu_disguise(__pdata);
	/*获取该cpu对应的结点*/
	int node = cpu_to_node(cpu);
	/*预创建的percpu变量不能为空*/
	BUG_ON(pdata->ptrs[cpu]);

	/*如果cpu对应的结点在线，则从该结点分配一个已清零的size长（对象大小）空间，否则，
	从当前结点分配*/	
	if (node_online(node))
		pdata->ptrs[cpu] = kmalloc_node(size, gfp|__GFP_ZERO, node);
	else
		pdata->ptrs[cpu] = kzalloc(size, gfp);
	return pdata->ptrs[cpu];
}
EXPORT_SYMBOL_GPL(percpu_populate);

/**
 * percpu_populate_mask - populate per-cpu data for more cpu's
 * @__pdata: per-cpu data to populate further
 * @size: size of per-cpu object
 * @gfp: may sleep or not etc.
 * @mask: populate per-cpu data for cpu's selected through mask bits
 *
 * Per-cpu objects are populated with zeroed buffers.
 */
/*给CPU位图上已置位的cpu申请对应结点空闲页标识gfp大小空间为size的percpu对象*/
int __percpu_populate_mask(void *__pdata, size_t size, gfp_t gfp, cpumask_t *mask)
{
	/*该cpu位图为记录已分配成功的percpu数据对应的CPU编号，分配失败时，将其上已经
置位的cpu对应的percpu数据清空*/
	cpumask_t populated = CPU_MASK_NONE;
	int cpu;
	/*遍历指定CPU位图中的所有cpu*/
	for_each_cpu_mask(cpu, *mask)
		if (unlikely(!percpu_populate(__pdata, size, gfp, cpu)))
		{
			/*内存不足，分配失败，释放已经分配的percpu数据*/
			__percpu_depopulate_mask(__pdata, &populated);
			return -ENOMEM;
		} 
		else
			/*分配成功时保存对应的cpu集合，失败时使用*/
			cpu_set(cpu, populated);
	return 0;
}
EXPORT_SYMBOL_GPL(__percpu_populate_mask);

/**
 * percpu_alloc_mask - initial setup of per-cpu data
 * @size: size of per-cpu object
 * @gfp: may sleep or not etc.
 * @mask: populate per-data for cpu's selected through mask bits
 *
 * Populating per-cpu data for all online cpu's would be a typical use case,
 * which is simplified by the percpu_alloc() wrapper.
 * Per-cpu objects are populated with zeroed buffers.
 */
/*建立并初始化struct percpu_data变量数据。根据cpu位图中已置位的cpu，在其对应的结点
（否则为当前结点）分配大小为size（对象长度），分配空闲页标识为gfp的内存作为percpu变
量的内存，分配成功后返回已清零的内存首地址，否则为空*/
void *__percpu_alloc_mask(size_t size, gfp_t gfp, cpumask_t *mask)
{
	/*申请NR_CPUS个数组指针*/
	void *pdata = kzalloc(sizeof(struct percpu_data), gfp);
	/*将该指针掩饰，按值取反*/
	void *__pdata = __percpu_disguise(pdata);
	/*内存申请失败，返回NULL*/
	if (unlikely(!pdata))
		return NULL;
	/*根据cpu位图中已置位cpu，分配pdata->ptrs[cpu]内存*/
	if (likely(!__percpu_populate_mask(__pdata, size, gfp, mask)))
		return __pdata;
	kfree(pdata);
	return NULL;
}
EXPORT_SYMBOL_GPL(__percpu_alloc_mask);

/**
 * percpu_free - final cleanup of per-cpu data
 * @__pdata: object to clean up
 *
 * We simply clean up any per-cpu object left. No need for the client to
 * track and specify through a bis mask which per-cpu objects are to free.
 */
/*最终释放per_cpu数据*/
void percpu_free(void *__pdata)
{
	/*无效对象直接退出*/
	if (unlikely(!__pdata))
		return;
	/*释放还没有被内核管理的所有cpu对应的struct percpu_data数据*/
	__percpu_depopulate_mask(__pdata, &cpu_possible_map);
	kfree(__percpu_disguise(__pdata));
}
EXPORT_SYMBOL_GPL(percpu_free);
