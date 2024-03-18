/*
 * linux/mm/allocpercpu.c
 *
 * Separated from slab.c August 11, 2006 Christoph Lameter <clameter@sgi.com>
 */
#include <linux/mm.h>
#include <linux/module.h>

/*�ͷ�ָ��cpu��percpu_data��ptrs->[cpu]��ֵ��Ϊһ���������ߵ�cpu�ͷ�percpu_data����
��һ�ֺܳ�����������ڴ�֮ǰ����Ҫ��Ϊ��cpuע��һ�����Ȳ�δ�����*/
void percpu_depopulate(void *__pdata, int cpu)
{
	/*���������ڶ����ַ����ȡ����������ԭΪԭ������ֵ*/
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
/*�ͷ�ָ��CPUλͼ������λ��֮ǰ�������struct percpu_data->ptrs[cpu]�ڴ�*/
void __percpu_depopulate_mask(void *__pdata, cpumask_t *mask)
{
	int cpu;
	/*��������ָ��λͼ�ϵ�cpu���ͷ���ptrs[cpu]�ռ�*/
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
 /*Ϊһ���ڼ�����Ϊ�ں˹����µ�cpu���һ��percpu������һ�ֺܵ��͵�ʹ�����ӣ���Ҫ��
 Ϊ��Ŀ�ģ���Ҫ��ע��һ��cpu�Ȳ�εĴ�������per_cpu������ӵ�һ����ʼ��Ϊ0�Ļ�
 ������*/
 /*��cpu��Ӧ�ģ�����Ϊ��ǰ����㣬����pdata->ptrs[cpu]��ʧ��ʱ����NULL*/
void *percpu_populate(void *__pdata, size_t size, gfp_t gfp, int cpu)
{
	/*��ȡpercpu_dataԭֵ*/
	struct percpu_data *pdata = __percpu_disguise(__pdata);
	/*��ȡ��cpu��Ӧ�Ľ��*/
	int node = cpu_to_node(cpu);
	/*Ԥ������percpu��������Ϊ��*/
	BUG_ON(pdata->ptrs[cpu]);

	/*���cpu��Ӧ�Ľ�����ߣ���Ӹý�����һ���������size���������С���ռ䣬����
	�ӵ�ǰ������*/	
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
/*��CPUλͼ������λ��cpu�����Ӧ������ҳ��ʶgfp��С�ռ�Ϊsize��percpu����*/
int __percpu_populate_mask(void *__pdata, size_t size, gfp_t gfp, cpumask_t *mask)
{
	/*��cpuλͼΪ��¼�ѷ���ɹ���percpu���ݶ�Ӧ��CPU��ţ�����ʧ��ʱ���������Ѿ�
��λ��cpu��Ӧ��percpu�������*/
	cpumask_t populated = CPU_MASK_NONE;
	int cpu;
	/*����ָ��CPUλͼ�е�����cpu*/
	for_each_cpu_mask(cpu, *mask)
		if (unlikely(!percpu_populate(__pdata, size, gfp, cpu)))
		{
			/*�ڴ治�㣬����ʧ�ܣ��ͷ��Ѿ������percpu����*/
			__percpu_depopulate_mask(__pdata, &populated);
			return -ENOMEM;
		} 
		else
			/*����ɹ�ʱ�����Ӧ��cpu���ϣ�ʧ��ʱʹ��*/
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
/*��������ʼ��struct percpu_data�������ݡ�����cpuλͼ������λ��cpu�������Ӧ�Ľ��
������Ϊ��ǰ��㣩�����СΪsize�����󳤶ȣ����������ҳ��ʶΪgfp���ڴ���Ϊpercpu��
�����ڴ棬����ɹ��󷵻���������ڴ��׵�ַ������Ϊ��*/
void *__percpu_alloc_mask(size_t size, gfp_t gfp, cpumask_t *mask)
{
	/*����NR_CPUS������ָ��*/
	void *pdata = kzalloc(sizeof(struct percpu_data), gfp);
	/*����ָ�����Σ���ֵȡ��*/
	void *__pdata = __percpu_disguise(pdata);
	/*�ڴ�����ʧ�ܣ�����NULL*/
	if (unlikely(!pdata))
		return NULL;
	/*����cpuλͼ������λcpu������pdata->ptrs[cpu]�ڴ�*/
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
/*�����ͷ�per_cpu����*/
void percpu_free(void *__pdata)
{
	/*��Ч����ֱ���˳�*/
	if (unlikely(!__pdata))
		return;
	/*�ͷŻ�û�б��ں˹��������cpu��Ӧ��struct percpu_data����*/
	__percpu_depopulate_mask(__pdata, &cpu_possible_map);
	kfree(__percpu_disguise(__pdata));
}
EXPORT_SYMBOL_GPL(percpu_free);
