#ifndef __LINUX_SMP_H
#define __LINUX_SMP_H

/*ͨ��SMP֧�֡�SMP��Symmetrical Multi-Processing�����Գƶദ��������ָ����CPU�㼯��
ͬһ�����ϣ���CPU������ڴ�����߹���ļ�������ͬһ������ƽ���(run in parallel)�ֲ���
���CPU�����У�����ͬ�����ڲ�ͬCPU�Ϲ�������ͬ�������ڴ档�����е�SMP�ܹ��У���չ����
��ģ�ͣ�UMA��NUMA��COMA��
UMA��Uniform Memory Access�������ȴ洢����ȡģ�ͣ����д���������ƽ�ȵķô�ʱ��
NUMA��Non-uniform Memory Access�����Ǿ��ȴ洢����ȡģ�͡���UMA��ͬ���ǣ�NUMA��ÿ������
���������Լ��ı��������ڴ�(local memory)����������CPU��˵��Զ�������ڴ�(remote memory)
��һ����ԣ����ʱ��������ڴ�����·�����̣���ô�ʱ��Ҫ���̡���UMAģ���У����Ŵ���������
�����ӣ��ô���Ϊ����ϵͳ������ƿ������ǰNUMA�������ڷ���������*/
/*��UMA�ܹ��£���˷������еĶ��CPUλ�����ߵ�һ�࣬�����ڴ������һ��Ƭ�ڴ�λ�����ߵ�
��һ�࣬����CPU�����ڴ涼Ҫ�����ߣ����Ҿ��붼��һ���ģ���������CPU���ڴ�ķ��ʾ��붼��
һ���ģ�������UMA�ܹ�������CPU�����ڴ���ٶȶ���һ���ġ����ַ���ģʽ��ΪSMP��Symmetric
multiprocessing�������Գƶദ�����������һ������ָͬһ��CPU�������ڴ�ķ��ʵ��ٶ���һ
���ġ���һ�����ڴ����UMA��Uniform Memory Access�����������Ŷ�˼����ķ�չ���������ϵ�
CPU������Խ��Խ�࣬��UMA�ܹ�������CPU������Ҫͨ�������������ڴ�ģ��������ߺܿ�ͻ��Ϊ
����ƿ������Ҫ�����������������棺��1�����ߵĴ���ѹ����Խ��Խ������CPU���������ർ��
ÿ��CPU���ô������٣�2�����ߵĳ���Ҳ����˶����ӣ��������ӷ����ӳ١�UMA�ܹ����ŵ����
�Ծ��ǽṹ�򵥣����е�CPU�����ڴ��ٶȶ���һ�µģ������뾭�����ߡ�Ȼ������ȱ����߸ո�Ҳ
�ᵽ�ˣ��������Ŵ��������������࣬���ߵĴ���ѹ����Խ��Խ�󡣽���취��ֻ���������ߣ�Ȼ
���ɱ�ʮ�ָ߰���δ��������Ȼ���ٴ���ѹ����Ϊ�˽���������⣬���CPU�����ڴ�����ܺ���չ
�ԣ�����������һ���µļܹ�����һ�����ڴ����NUMA��Non-uniformmemoryaccess����
��NUMA�ܹ��£��ڴ�Ͳ���һ��Ƭ���ˣ����Ǳ����ֳ���һ��һ�����ڴ�ڵ㣨NUMA�ڵ㣩��ÿ��
CPU���������Լ��ı����ڴ�ڵ㣬CPU�����Լ��ı����ڴ治��Ҫ�������ߣ���˷����ٶ������
�ġ���CPU�Լ��ı����ڴ治��ʱ��CPU����Ҫ��ڵ�ȥ���������ڴ�ڵ㣬���������CPU�����ڴ�
�ͻ����ܶࡣ��NUMA�ܹ��£�����һ��CPU�����Է���ȫ�����ڴ�ڵ㣬�����Լ��ı����ڴ�ڵ���
���ģ������������ڴ�ڵ�ͻ����ܶ࣬��͵�����CPU�����ڴ���ٶȲ�һ�£����Խ�����һ��
���ڴ���ʼܹ���CPU�����ı����ڴ������NUMA�ڵ㣬CPU��CPU֮��ͨ��QPI��Intel QuickPath
Interconnect����Ե���ɻ�������CPU�ı����ڴ治�������£�CPU��Ҫͨ��QPI����Զ��NUMA��
���ϵ��ڴ�������Ӷ���Զ���ڴ�ڵ��Ϸ����ڴ棬��͵�����Զ�̷��ʱȱ��ط��ʶ��˶������
�ٿ�������Ҫͨ��QPI����Զ��NUMA�ڵ㣩����NUMA�ܹ��£�ֻ��DISCONTIGMEM�������ڴ�ģ�ͺ�
SPARSEMEMϡ���ڴ�ģ���ǿ��õġ���UMA�ܹ��£�ǰ����ܵ������ڴ�ģ�Ͷ���������ʹ�á�*/

#include <linux/errno.h>

extern void cpu_idle(void);

#ifdef CONFIG_SMP

#include <linux/preempt.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/thread_info.h>
#include <asm/smp.h>

/*
 * main cross-CPU interfaces, handles INIT, TLB flush, STOP, etc.
 * (defined in asm header):
 */

/*ͣ�õ�ǰcpu���������cpu*/
extern void smp_send_stop(void);

/*����һ��cpu�����ص����¼�*/
extern void smp_send_reschedule(int cpu);

/*׼����������cpu*/
extern void smp_prepare_cpus(unsigned int max_cpus);

/*����һ��cpu*/
extern int __cpu_up(unsigned int cpunum);

/*
 * Final polishing of CPUs
 */

extern void smp_cpus_done(unsigned int max_cpus);

/*��������������������һ������*/
int smp_call_function(void(*func)(void *info), void *info, int retry, int wait);
int smp_call_function_single(int cpuid, void (*func) (void *info), void *info,	int retry,
									int wait);
/*������cpu��ִ��һ������*/
int on_each_cpu(void (*func) (void *info), void *info, int retry, int wait);

/*����С��32768��cpu*/
#define MSG_ALL_BUT_SELF		0x8000
#define MSG_ALL					0x8001
/*Զ�̴�����TLB��Ч*/
#define MSG_INVALIDATE_TLB		0x0001
/*����ʱ����cpu����cpu����ֹͣ��Ϣ*/
#define MSG_STOP_CPU			0x0002
/*��cpu�����ص���*/
#define MSG_RESCHEDULE			0x0003
/*������cpu�ϵ��ú���*/
#define MSG_CALL_FUNCTION       0x0004
/*�������cpu�ɵ��ȣ���ˣ���������printk�����е��ÿ���̨��������������per-cpu�洢��*/
void smp_prepare_boot_cpu(void);

#else /* !SMP */

/*
 *	These macros fold the SMP functionality into a single CPU system
 */
#define raw_smp_processor_id()			0
static inline int up_smp_call_function(void (*func)(void *), void *info)
{
	return 0;
}
#define smp_call_function(func, info, retry, wait) \
			(up_smp_call_function(func, info))
#define on_each_cpu(func,info,retry,wait)	\
	({					\
		local_irq_disable();		\
		func(info);			\
		local_irq_enable();		\
		0;				\
	})
static inline void smp_send_reschedule(int cpu) { }
#define num_booting_cpus()			1
#define smp_prepare_boot_cpu()			do {} while (0)
#define smp_call_function_single(cpuid, func, info, retry, wait) \
({ \
	WARN_ON(cpuid != 0);	\
	local_irq_disable();	\
	(func)(info);		\
	local_irq_enable();	\
	0;			\
})
#define smp_call_function_mask(mask, func, info, wait) \
			(up_smp_call_function(func, info))

#endif /* !SMP */

/*
 * smp_processor_id(): get the current CPU ID.
 *
 * if DEBUG_PREEMPT is enabled the we check whether it is
 * used in a preemption-safe way. (smp_processor_id() is safe
 * if it's used in a preemption-off critical section, or in
 * a thread that is bound to the current CPU.)
 *
 * NOTE: raw_smp_processor_id() is for internal use only
 * (smp_processor_id() is the preferred variant), but in rare
 * instances it might also be used to turn off false positives
 * (i.e. smp_processor_id() use that the debugging code reports but
 * which use for some reason is legal). Don't use this to hack around
 * the warning message, as your code might not work under PREEMPT.
 */
#ifdef CONFIG_DEBUG_PREEMPT
  extern unsigned int debug_smp_processor_id(void);
/*��õ�ǰcpu���*/
#define smp_processor_id() debug_smp_processor_id()
#else
#define smp_processor_id() raw_smp_processor_id()
#endif

/*ͣ����ռ����õ�ǰcpu*/
#define get_cpu()		({ preempt_disable(); smp_processor_id(); })
/*������ռ*/
#define put_cpu()		preempt_enable()
/*�����ں���ռ�����������ص���*/
#define put_cpu_no_resched()	preempt_enable_no_resched()

void smp_setup_processor_id(void);

#endif /* __LINUX_SMP_H */
