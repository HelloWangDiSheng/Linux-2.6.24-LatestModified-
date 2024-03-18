#ifndef __LINUX_PERCPU_H
#define __LINUX_PERCPU_H

#include <linux/preempt.h>
#include <linux/slab.h> /* For kmalloc() */
#include <linux/smp.h>
#include <linux/string.h> /* For memset() */
#include <linux/cpumask.h>

#include <asm/percpu.h>

/*足以覆盖内核中包括模块的所有预定义的percpu变量*/
#ifndef PERCPU_ENOUGH_ROOM
#ifdef CONFIG_MODULES
#define PERCPU_MODULE_RESERVE	8192
#else
#define PERCPU_MODULE_RESERVE	0
#endif

#define PERCPU_ENOUGH_ROOM	(__per_cpu_end - __per_cpu_start + PERCPU_MODULE_RESERVE)
#endif	/* PERCPU_ENOUGH_ROOM */


/*获取变量var值，var是一个简单的标识符，否则，强制语法错误，取值期间禁用抢占，
启用内存优化屏障*/
#define get_cpu_var(var) (*({					\
	extern int simple_identifier_##var(void);	\
	preempt_disable();							\
	&__get_cpu_var(var); }))
	
#define put_cpu_var(var) preempt_enable()

#ifdef CONFIG_SMP
/*percpu数据结构*/
struct percpu_data
{
	/*percpu数据指针*/
	void *ptrs[NR_CPUS];
};

/*该宏对数据进行取非掩饰。内核中为防止用户对敏感数据（比如一些关键指针）的直接引用，通常
会采用“掩饰”的方法，即在分配相关数据时，就对其进行掩饰，然后再引用时，由对其进行还原，这
样的话，用户就不能直接引用相应的数据（会导致非法访问），而必须使用内核提供的接口来引用*/
#define __percpu_disguise(pdata) (struct percpu_data *)~(unsigned long)(pdata)
/* 
 * Use this to get to a cpu's version of the per-cpu object dynamically
 * allocated. Non-atomic access to the current CPU's version should
 * probably be combined with get_cpu()/put_cpu().
 */ 
/*获取struct percpu_data变量的ptrs[cpu]数据*/
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

/*根据mask位图上已置位的cpu，在其对应结点（结点不存在则才从当前结点）申请空闲页
标识gfp，长度为size的percpu变量空间，将申请成功的地址空间首地址存放在解析后的
__pdata->ptrs[cpu]中*/
#define percpu_populate_mask(__pdata, size, gfp, mask) \
	__percpu_populate_mask((__pdata), (size), (gfp), &(mask))
/*释放cpu位图中已置位的cpu对应的struct percpu_data->ptrs[cpu]内存*/
#define percpu_depopulate_mask(__pdata, mask) __percpu_depopulate_mask((__pdata), &(mask))
/*建立并初始化struct percpu_data变量，为变量中与CPU位图中已置位的cpu对应数组指针分配内存*/
#define percpu_alloc_mask(size, gfp, mask) 	__percpu_alloc_mask((size), (gfp), &(mask))
/*创建并初始化struct percpu_data变量，为该指针数组中cpu下标项与内核已管理的cpu对应的
percpu分配内存*/
#define percpu_alloc(size, gfp) percpu_alloc_mask((size), (gfp), cpu_online_map)


/*没有cpu热插拔功能的处理接口*/
/**/
#define __alloc_percpu(size)	percpu_alloc_mask((size), GFP_KERNEL, cpu_possible_map)
/*分配type类型的percpu变量，如分配percpu_counter中近似值counters数组项*/
#define alloc_percpu(type)	(type *)__alloc_percpu(sizeof(type))
/**/
#define free_percpu(ptr)	percpu_free((ptr))
/*获取ptr->ptrs[cpu]数组指针*/
#define per_cpu_ptr(ptr, cpu)	percpu_ptr((ptr), (cpu))

#endif /* __LINUX_PERCPU_H */
