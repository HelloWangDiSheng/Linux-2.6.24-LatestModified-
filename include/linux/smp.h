#ifndef __LINUX_SMP_H
#define __LINUX_SMP_H

/*通用SMP支持。SMP（Symmetrical Multi-Processing）即对称多处理技术，是指将多CPU汇集在
同一总线上，各CPU间进行内存和总线共享的技术。将同一个工作平衡地(run in parallel)分布到
多个CPU上运行，该相同任务在不同CPU上共享着相同的物理内存。在现行的SMP架构中，发展出三
种模型：UMA、NUMA和COMA。
UMA（Uniform Memory Access）即均匀存储器存取模型，所有处理器有着平等的访存时间
NUMA（Non-uniform Memory Access）即非均匀存储器存取模型。与UMA不同的是，NUMA中每个处理
器有属于自己的本地物理内存(local memory)，对于其他CPU来说是远程物理内存(remote memory)
。一般而言，访问本地物理内存由于路径更短，其访存时间要更短。在UMA模型中，随着处理器的数
量增加，访存会成为整个系统的性能瓶颈。当前NUMA更多用于服务器场景*/
/*在UMA架构下，多核服务器中的多个CPU位于总线的一侧，所有内存条组成一大片内存位于总线的
另一侧，所有CPU访问内存都要过总线，而且距离都是一样的，由于所有CPU对内存的访问距离都是
一样的，所以在UMA架构下所有CPU访问内存的速度都是一样的。这种访问模式称为SMP（Symmetric
multiprocessing），即对称多处理器。这里的一致性是指同一个CPU对所有内存的访问的速度是一
样的。即一致性内存访问UMA（Uniform Memory Access）。但是随着多核技术的发展，服务器上的
CPU个数会越来越多，而UMA架构下所有CPU都是需要通过总线来访问内存的，这样总线很快就会成为
性能瓶颈，主要体现在以下两个方面：（1）总线的带宽压力会越来越大，随着CPU个数的增多导致
每个CPU可用带宽会减少（2）总线的长度也会因此而增加，进而增加访问延迟。UMA架构的优点很明
显就是结构简单，所有的CPU访问内存速度都是一致的，都必须经过总线。然而它的缺点笔者刚刚也
提到了，就是随着处理器核数的增多，总线的带宽压力会越来越大。解决办法就只能扩宽总线，然
而成本十分高昂，未来可能仍然面临带宽压力。为了解决以上问题，提高CPU访问内存的性能和扩展
性，于是引入了一种新的架构：非一致性内存访问NUMA（Non-uniformmemoryaccess）。
在NUMA架构下，内存就不是一整片的了，而是被划分成了一个一个的内存节点（NUMA节点），每个
CPU都有属于自己的本地内存节点，CPU访问自己的本地内存不需要经过总线，因此访问速度是最快
的。当CPU自己的本地内存不足时，CPU就需要跨节点去访问其他内存节点，这种情况下CPU访问内存
就会慢很多。在NUMA架构下，任意一个CPU都可以访问全部的内存节点，访问自己的本地内存节点是
最快的，但访问其他内存节点就会慢很多，这就导致了CPU访问内存的速度不一致，所以叫做非一致
性内存访问架构。CPU和它的本地内存组成了NUMA节点，CPU与CPU之间通过QPI（Intel QuickPath
Interconnect）点对点完成互联，在CPU的本地内存不足的情况下，CPU需要通过QPI访问远程NUMA节
点上的内存控制器从而在远程内存节点上分配内存，这就导致了远程访问比本地访问多了额外的延
迟开销（需要通过QPI遍历远程NUMA节点）。在NUMA架构下，只有DISCONTIGMEM非连续内存模型和
SPARSEMEM稀疏内存模型是可用的。而UMA架构下，前面介绍的三种内存模型都可以配置使用。*/

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

/*停用当前cpu以外的所有cpu*/
extern void smp_send_stop(void);

/*向另一个cpu发送重调度事件*/
extern void smp_send_reschedule(int cpu);

/*准备启动其它cpu*/
extern void smp_prepare_cpus(unsigned int max_cpus);

/*唤醒一个cpu*/
extern int __cpu_up(unsigned int cpunum);

/*
 * Final polishing of CPUs
 */

extern void smp_cpus_done(unsigned int max_cpus);

/*在所有其它处理器调用一个函数*/
int smp_call_function(void(*func)(void *info), void *info, int retry, int wait);
int smp_call_function_single(int cpuid, void (*func) (void *info), void *info,	int retry,
									int wait);
/*在所有cpu上执行一个函数*/
int on_each_cpu(void (*func) (void *info), void *info, int retry, int wait);

/*假设小于32768个cpu*/
#define MSG_ALL_BUT_SELF		0x8000
#define MSG_ALL					0x8001
/*远程处理器TLB无效*/
#define MSG_INVALIDATE_TLB		0x0001
/*启动时启动cpu以外cpu发送停止消息*/
#define MSG_STOP_CPU			0x0002
/*主cpu请求重调度*/
#define MSG_RESCHEDULE			0x0003
/*在其它cpu上调用函数*/
#define MSG_CALL_FUNCTION       0x0004
/*标记启动cpu可调度，因此，它可以在printk（）中调用控制台驱动并访问它的per-cpu存储区*/
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
/*获得当前cpu编号*/
#define smp_processor_id() debug_smp_processor_id()
#else
#define smp_processor_id() raw_smp_processor_id()
#endif

/*停用抢占并获得当前cpu*/
#define get_cpu()		({ preempt_disable(); smp_processor_id(); })
/*启用抢占*/
#define put_cpu()		preempt_enable()
/*启用内核抢占，但不进行重调度*/
#define put_cpu_no_resched()	preempt_enable_no_resched()

void smp_setup_processor_id(void);

#endif /* __LINUX_SMP_H */
