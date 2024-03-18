#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/cpumask.h>
#include <linux/module.h>

/*��ȡcpuλͼ�е�һ������λ��λ���*/
int __first_cpu(const cpumask_t *srcp)
{
	return min_t(int, NR_CPUS, find_first_bit(srcp->bits, NR_CPUS));
}
EXPORT_SYMBOL(__first_cpu);

/*��ȡcpuλͼ�е�n+1��ʼ�ı���λ֮���һ����λ��λ���*/
int __next_cpu(int n, const cpumask_t *srcp)
{
	return min_t(int, NR_CPUS, find_next_bit(srcp->bits, NR_CPUS, n+1));
}
EXPORT_SYMBOL(__next_cpu);

/*��ȡcpuλͼ�е�һ������λ����cpu_online_mapλͼ������λ��cpu���*/
int __any_online_cpu(const cpumask_t *mask)
{
	int cpu;
	/*����cpuλͼ�е�����λ*/
	for_each_cpu_mask(cpu, *mask)
	{
		/*�����ǰλ��cpu�ɹ����ȣ�������ѭ�������ض�Ӧ��cpu���*/
		if (cpu_online(cpu))
			break;
	}
	return cpu;
}
EXPORT_SYMBOL(__any_online_cpu);
