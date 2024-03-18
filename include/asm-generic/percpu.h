#ifndef _ASM_GENERIC_PERCPU_H_
#define _ASM_GENERIC_PERCPU_H_
#include <linux/compiler.h>
#include <linux/threads.h>
/*��ͨ���û��ռ������Ʋ����漰��һ���������������ν��per_cpu������������ͨ��
DEFINE_PER_CPU(name, type)���������У�name�Ǳ�������type�����������ͣ��磬int[3],
struct hash�ȣ����ڵ�������ϵͳ�ϣ����볣��ı�������û�в�ͬ����������cpu��SMPϵ
ͳ�ϣ���Ϊÿ��cpu�ֱ𴴽�������һ��ʵ��������ĳ���ض�cpu��ʵ������ͨ��get_cpu(name
, cpu)��ã�����cmp_processor_id()���Է��ص�ǰ���cpu id������ǰ����cpu������
����per_cpu���������ºô����������ݺܿ��ܴ����ڴ������Ļ����У����Ը����ٵķ��ʣ�
����ڶദ����ϵͳ���Ǹ�ʹ�ÿ��ܱ�����cpuͬʱ���ʵı��������ܻ�����һЩͨ�ŷ����
���⣬����per_cpu����պ��ƹ���Щ����*/

#define __GENERIC_PER_CPU
#ifdef CONFIG_SMP

/*����cpu���Բ��Ҹö�*/
extern unsigned long __per_cpu_offset[NR_CPUS];

/*����һ��percpuƫ�����������__per_cpu_offset[0]λ�õ�ƫ��*/
#define per_cpu_offset(x) (__per_cpu_offset[x])

/* Separate out the type, so (int[3], foo) works. */
/*�ں�ʹ��DEFINE_PER_CPUΪϵͳ��ÿ��cpu������һ��ʵ�����Ľ���cpu���ٻ����������*/
/*����һ��type���͵�percpu����name���ñ���������".data.percpu"���ݶ���*/
#define DEFINE_PER_CPU(type, name) \
    __attribute__((__section__(".data.percpu"))) __typeof__(type) per_cpu__##name

/*����һ��SMPϵͳ�Ϲ����L1�����ж����type���͵�percpu����name���ñ���������
".data.percpu.shared_aligned"���ݶ���*/
#define DEFINE_PER_CPU_SHARED_ALIGNED(type, name)					\
    __attribute__((__section__(".data.percpu.shared_aligned"))) 	\
    __typeof__(type) per_cpu__##name								\
    ____cacheline_aligned_in_smp

/* var is in discarded region: offset to particular copy we want */
/*��ȡָ��cpu�ϵ�var����ֵ*/
#define per_cpu(var, cpu) (*({					\
	extern int simple_identifier_##var(void);	\
	RELOC_HIDE(&per_cpu__##var, __per_cpu_offset[cpu]); }))
/*��ȡ��ǰcpu�ϵ�var����ֵ*/
#define __get_cpu_var(var) per_cpu(var, smp_processor_id())
/**/
#define __raw_get_cpu_var(var) per_cpu(var, raw_smp_processor_id())

/* A macro to avoid #include hell... */
/*����src��ַ��ʼλsize���ֽ����ݵ�ָ����per_cpu����*/
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

/*����һ��type���͵�per_cpu����name*/
#define DECLARE_PER_CPU(type, name) extern __typeof__(type) per_cpu__##name
/*����per_cpu����var*/
#define EXPORT_PER_CPU_SYMBOL(var) EXPORT_SYMBOL(per_cpu__##var)
/*����GPL֧�ֵ�per_cpu����var*/
#define EXPORT_PER_CPU_SYMBOL_GPL(var) EXPORT_SYMBOL_GPL(per_cpu__##var)

#endif /* _ASM_GENERIC_PERCPU_H_ */
