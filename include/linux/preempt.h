#ifndef __LINUX_PREEMPT_H
#define __LINUX_PREEMPT_H

/*访问和操作抢占计数器宏，经常被用于内核抢占、中断计数等*/
#include <linux/thread_info.h>
#include <linux/linkage.h>
#include <linux/list.h>

#ifdef CONFIG_DEBUG_PREEMPT
  extern void fastcall add_preempt_count(int val);
  extern void fastcall sub_preempt_count(int val);
#else
/*增加抢占计数器的值*/
#define add_preempt_count(val)	do { preempt_count() += (val); } while (0)
/*减少抢占计数器的值*/
#define sub_preempt_count(val)	do { preempt_count() -= (val); } while (0)
#endif
/*抢占计数器自增1*/
#define inc_preempt_count() add_preempt_count(1)
/*抢占计数器自减1*/
#define dec_preempt_count() sub_preempt_count(1)
/*获取当前进程抢占计数器的值*/
#define preempt_count()	(current_thread_info()->preempt_count)

#ifdef CONFIG_PREEMPT

asmlinkage void preempt_schedule(void);
/*通过调用inc_preempt_count停用抢占，此外，会指示编译器避免某些内存优化，以免导致某些
与抢占机制相关的问题*/
#define preempt_disable() 		\
do								\
{ 								\
	inc_preempt_count(); 		\/*禁用抢占*/\
	barrier(); 					\/*启用内存优化屏障*/\
} while (0)
/*启用内核抢占，但不进行重调度*/
#define preempt_enable_no_resched() 		\
do											\
{ 											\
	barrier(); 								\/*启用内存屏障*/\
	dec_preempt_count(); 					\/*启用抢占*/\
} while (0)
/*检测是否有必要进行调度，如有必要则进行*/
#define preempt_check_resched() 									\
do																	\
{ 																	\
	if (unlikely(test_thread_flag(TIF_NEED_RESCHED))) 				\/*当前进程在等待重调度*/\
		preempt_schedule(); 										\/**/\
} while (0)
/*启用内核抢占*/
#define preempt_enable() 				\
do										\
{ 										\
	preempt_enable_no_resched(); 		\/**/\
	barrier(); 							\
	preempt_check_resched(); 			\
} while (0)

#else

#define preempt_disable()		do { } while (0)
#define preempt_enable_no_resched()	do { } while (0)
#define preempt_enable()		do { } while (0)
#define preempt_check_resched()		do { } while (0)

#endif

#ifdef CONFIG_PREEMPT_NOTIFIERS

struct preempt_notifier;


/*进程被抢占或重新调度时启用的通知机制*/
struct preempt_ops
{
	/*进程被重新调度到指定处理器上运行*/
	void (*sched_in)(struct preempt_notifier *notifier, int cpu);
	/*进程被哪个进程抢占*/
	void (*sched_out)(struct preempt_notifier *notifier, struct task_struct *next);
};

/*建立抢占通知关键信息，经常和container_of一块使用*/
struct preempt_notifier
{
	/*内部使用*/
	struct hlist_node link;
	/*通知函数*/
	struct preempt_ops *ops;
};

void preempt_notifier_register(struct preempt_notifier *notifier);
void preempt_notifier_unregister(struct preempt_notifier *notifier);
/*初始化抢占通知机制*/
static inline void preempt_notifier_init(struct preempt_notifier *notifier,
				     							struct preempt_ops *ops)
{
	/**/
	INIT_HLIST_NODE(&notifier->link);
	/**/
	notifier->ops = ops;
}

#endif

#endif /* __LINUX_PREEMPT_H */
