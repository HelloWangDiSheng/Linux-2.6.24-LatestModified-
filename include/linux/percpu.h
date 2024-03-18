#ifndef __LINUX_PERCPU_H
#define __LINUX_PERCPU_H

#include <linux/preempt.h>
#include <linux/slab.h> /* For kmalloc() */
#include <linux/smp.h>
#include <linux/string.h> /* For memset() */
#include <linux/cpumask.h>

#include <asm/percpu.h>

/*���Ը����ں��а���ģ�������Ԥ�����percpu����*/
#ifndef PERCPU_ENOUGH_ROOM
#ifdef CONFIG_MODULES
#define PERCPU_MODULE_RESERVE	8192
#else
#define PERCPU_MODULE_RESERVE	0
#endif

#define PERCPU_ENOUGH_ROOM	(__per_cpu_end - __per_cpu_start + PERCPU_MODULE_RESERVE)
#endif	/* PERCPU_ENOUGH_ROOM */


/*��ȡ����varֵ��var��һ���򵥵ı�ʶ��������ǿ���﷨����ȡֵ�ڼ������ռ��
�����ڴ��Ż�����*/
#define get_cpu_var(var) (*({					\
	extern int simple_identifier_##var(void);	\
	preempt_disable();							\
	&__get_cpu_var(var); }))
	
#define put_cpu_var(var) preempt_enable()

#ifdef CONFIG_SMP
/*percpu���ݽṹ*/
struct percpu_data
{
	/*percpu����ָ��*/
	void *ptrs[NR_CPUS];
};

/*�ú�����ݽ���ȡ�����Ρ��ں���Ϊ��ֹ�û����������ݣ�����һЩ�ؼ�ָ�룩��ֱ�����ã�ͨ��
����á����Ρ��ķ��������ڷ����������ʱ���Ͷ���������Σ�Ȼ��������ʱ���ɶ�����л�ԭ����
���Ļ����û��Ͳ���ֱ��������Ӧ�����ݣ��ᵼ�·Ƿ����ʣ���������ʹ���ں��ṩ�Ľӿ�������*/
#define __percpu_disguise(pdata) (struct percpu_data *)~(unsigned long)(pdata)
/* 
 * Use this to get to a cpu's version of the per-cpu object dynamically
 * allocated. Non-atomic access to the current CPU's version should
 * probably be combined with get_cpu()/put_cpu().
 */ 
/*��ȡstruct percpu_data������ptrs[cpu]����*/
#define percpu_ptr(ptr, cpu)                              				\
({                                                        				\
        struct percpu_data *__p = __percpu_disguise(ptr); 				\
        (__typeof__(ptr))__p->ptrs[(cpu)];	          					\
})

extern void *percpu_populate(void *__pdata, size_t size, gfp_t gfp, int cpu);
extern void percpu_depopulate(void *__pdata, int cpu);
extern int __percpu_populate_mask(void *__pdata, size_t size, gfp_t gfp,
				  cpumask_t *mask);
extern void __percpu_depopulate_mask(void *__pdata, cpumask_t *mask);
extern void *__percpu_alloc_mask(size_t size, gfp_t gfp, cpumask_t *mask);
extern void percpu_free(void *__pdata);

#else /* CONFIG_SMP */

#define percpu_ptr(ptr, cpu) ({ (void)(cpu); (ptr); })

static inline void percpu_depopulate(void *__pdata, int cpu)
{
}

static inline void __percpu_depopulate_mask(void *__pdata, cpumask_t *mask)
{
}

static inline void *percpu_populate(void *__pdata, size_t size, gfp_t gfp,
				    int cpu)
{
	return percpu_ptr(__pdata, cpu);
}

static inline int __percpu_populate_mask(void *__pdata, size_t size, gfp_t gfp,
					 cpumask_t *mask)
{
	return 0;
}

static __always_inline void *__percpu_alloc_mask(size_t size, gfp_t gfp, cpumask_t *mask)
{
	return kzalloc(size, gfp);
}

static inline void percpu_free(void *__pdata)
{
	kfree(__pdata);
}

#endif /* CONFIG_SMP */

/*����maskλͼ������λ��cpu�������Ӧ��㣨��㲻������Ŵӵ�ǰ��㣩�������ҳ
��ʶgfp������Ϊsize��percpu�����ռ䣬������ɹ��ĵ�ַ�ռ��׵�ַ����ڽ������
__pdata->ptrs[cpu]��*/
#define percpu_populate_mask(__pdata, size, gfp, mask) \
	__percpu_populate_mask((__pdata), (size), (gfp), &(mask))
/*�ͷ�cpuλͼ������λ��cpu��Ӧ��struct percpu_data->ptrs[cpu]�ڴ�*/
#define percpu_depopulate_mask(__pdata, mask) __percpu_depopulate_mask((__pdata), &(mask))
/*��������ʼ��struct percpu_data������Ϊ��������CPUλͼ������λ��cpu��Ӧ����ָ������ڴ�*/
#define percpu_alloc_mask(size, gfp, mask) 	__percpu_alloc_mask((size), (gfp), &(mask))
/*��������ʼ��struct percpu_data������Ϊ��ָ��������cpu�±������ں��ѹ����cpu��Ӧ��
percpu�����ڴ�*/
#define percpu_alloc(size, gfp) percpu_alloc_mask((size), (gfp), cpu_online_map)


/*û��cpu�Ȳ�ι��ܵĴ���ӿ�*/
/**/
#define __alloc_percpu(size)	percpu_alloc_mask((size), GFP_KERNEL, cpu_possible_map)
/*����type���͵�percpu�����������percpu_counter�н���ֵcounters������*/
#define alloc_percpu(type)	(type *)__alloc_percpu(sizeof(type))
/**/
#define free_percpu(ptr)	percpu_free((ptr))
/*��ȡptr->ptrs[cpu]����ָ��*/
#define per_cpu_ptr(ptr, cpu)	percpu_ptr((ptr), (cpu))

#endif /* __LINUX_PERCPU_H */
