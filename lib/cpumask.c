#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/cpumask.h>
#include <linux/module.h>

/*获取cpu位图中第一个已置位的位编号*/
int __first_cpu(const cpumask_t *srcp)
{
	return min_t(int, NR_CPUS, find_first_bit(srcp->bits, NR_CPUS));
}
EXPORT_SYMBOL(__first_cpu);

/*获取cpu位图中第n+1开始的比特位之后第一个置位的位编号*/
int __next_cpu(int n, const cpumask_t *srcp)
{
	return min_t(int, NR_CPUS, find_next_bit(srcp->bits, NR_CPUS, n+1));
}
EXPORT_SYMBOL(__next_cpu);

/*获取cpu位图中第一个已置位且在cpu_online_map位图中已置位的cpu编号*/
int __any_online_cpu(const cpumask_t *mask)
{
	int cpu;
	/*遍历cpu位图中的所有位*/
	for_each_cpu_mask(cpu, *mask)
	{
		/*如果当前位的cpu可供调度，这跳出循环，返回对应的cpu编号*/
		if (cpu_online(cpu))
			break;
	}
	return cpu;
}
EXPORT_SYMBOL(__any_online_cpu);
