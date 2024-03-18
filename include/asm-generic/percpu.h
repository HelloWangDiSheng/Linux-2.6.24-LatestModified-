#ifndef _ASM_GENERIC_PERCPU_H_
#define _ASM_GENERIC_PERCPU_H_
#include <linux/compiler.h>
#include <linux/threads.h>
/*普通的用户空间程序设计不会涉及的一个特殊事项就是所谓的per_cpu变量。它们是通过
DEFINE_PER_CPU(name, type)声明，其中，name是变量名，type是其数据类型（如，int[3],
struct hash等）。在单处理器系统上，这与常规的变量声明没有不同，在有若干cpu的SMP系
统上，会为每个cpu分别创建变量的一个实例。用于某个特定cpu的实例可以通过get_cpu(name
, cpu)获得，其中cmp_processor_id()可以返回当前活动的cpu id，用作前述的cpu参数。
采用per_cpu变量有以下好处：所需数据很可能存在于处理器的缓存中，可以更快速的访问，
如果在多处理器系统找那个使用可能被所有cpu同时访问的变量，可能会引发一些通信方面的
问题，采用per_cpu概念刚好绕过这些问题*/

#define __GENERIC_PER_CPU
#ifdef CONFIG_SMP

/*其它cpu可以查找该段*/
extern unsigned long __per_cpu_offset[NR_CPUS];

/*定义一个percpu偏移量，相对于__per_cpu_offset[0]位置的偏移*/
#define per_cpu_offset(x) (__per_cpu_offset[x])

/* Separate out the type, so (int[3], foo) works. */
/*内核使用DEFINE_PER_CPU为系统的每个cpu都建立一个实例，改进对cpu高速缓存的利用率*/
/*定义一个type类型的percpu变量name，该变量保存在".data.percpu"数据段中*/
#define DEFINE_PER_CPU(type, name) \
    __attribute__((__section__(".data.percpu"))) __typeof__(type) per_cpu__##name

/*定义一个SMP系统上共享的L1缓存行对齐的type类型的percpu变量name，该变量保存在
".data.percpu.shared_aligned"数据段中*/
#define DEFINE_PER_CPU_SHARED_ALIGNED(type, name)					\
    __attribute__((__section__(".data.percpu.shared_aligned"))) 	\
    __typeof__(type) per_cpu__##name								\
    ____cacheline_aligned_in_smp

/* var is in discarded region: offset to particular copy we want */
/*获取指定cpu上的var变量值*/
#define per_cpu(var, cpu) (*({					\
	extern int simple_identifier_##var(void);	\
	RELOC_HIDE(&per_cpu__##var, __per_cpu_offset[cpu]); }))
/*获取当前cpu上的var变量值*/
#define __get_cpu_var(var) per_cpu(var, smp_processor_id())
/**/
#define __raw_get_cpu_var(var) per_cpu(var, raw_smp_processor_id())

/* A macro to avoid #include hell... */
/*复制src地址起始位size长字节数据到指定的per_cpu区域*/
#define percpu_modcopy(pcpudst, src, size)				\
do {													\
	unsigned int __i;									\
	for_each_possible_cpu(__i)							\
		memcpy((pcpudst)+__per_cpu_offset[__i],			\
		       (src), (size));							\
} while (0)
#else /* ! SMP */

#define DEFINE_PER_CPU(type, name) \
    __typeof__(type) per_cpu__##name

#define DEFINE_PER_CPU_SHARED_ALIGNED(type, name)	\
    DEFINE_PER_CPU(type, name)

#define per_cpu(var, cpu)			(*((void)(cpu), &per_cpu__##var))
#define __get_cpu_var(var)			per_cpu__##var
#define __raw_get_cpu_var(var)			per_cpu__##var

#endif	/* SMP */

/*声明一个type类型的per_cpu变量name*/
#define DECLARE_PER_CPU(type, name) extern __typeof__(type) per_cpu__##name
/*导出per_cpu符号var*/
#define EXPORT_PER_CPU_SYMBOL(var) EXPORT_SYMBOL(per_cpu__##var)
/*导出GPL支持的per_cpu符号var*/
#define EXPORT_PER_CPU_SYMBOL_GPL(var) EXPORT_SYMBOL_GPL(per_cpu__##var)

#endif /* _ASM_GENERIC_PERCPU_H_ */
