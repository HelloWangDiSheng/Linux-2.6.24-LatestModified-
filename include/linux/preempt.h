#ifndef __LINUX_PREEMPT_H
#define __LINUX_PREEMPT_H

/*���ʺͲ�����ռ�������꣬�����������ں���ռ���жϼ�����*/
#include <linux/thread_info.h>
#include <linux/linkage.h>
#include <linux/list.h>

#ifdef CONFIG_DEBUG_PREEMPT
  extern void fastcall add_preempt_count(int val);
  extern void fastcall sub_preempt_count(int val);
#else
/*������ռ��������ֵ*/
#define add_preempt_count(val)	do { preempt_count() += (val); } while (0)
/*������ռ��������ֵ*/
#define sub_preempt_count(val)	do { preempt_count() -= (val); } while (0)
#endif
/*��ռ����������1*/
#define inc_preempt_count() add_preempt_count(1)
/*��ռ�������Լ�1*/
#define dec_preempt_count() sub_preempt_count(1)
/*��ȡ��ǰ������ռ��������ֵ*/
#define preempt_count()	(current_thread_info()->preempt_count)

#ifdef CONFIG_PREEMPT

asmlinkage void preempt_schedule(void);
/*ͨ������inc_preempt_countͣ����ռ�����⣬��ָʾ����������ĳЩ�ڴ��Ż������⵼��ĳЩ
����ռ������ص�����*/
#define preempt_disable() 		\
do								\
{ 								\
	inc_preempt_count(); 		\/*������ռ*/\
	barrier(); 					\/*�����ڴ��Ż�����*/\
} while (0)
/*�����ں���ռ�����������ص���*/
#define preempt_enable_no_resched() 		\
do											\
{ 											\
	barrier(); 								\/*�����ڴ�����*/\
	dec_preempt_count(); 					\/*������ռ*/\
} while (0)
/*����Ƿ��б�Ҫ���е��ȣ����б�Ҫ�����*/
#define preempt_check_resched() 									\
do																	\
{ 																	\
	if (unlikely(test_thread_flag(TIF_NEED_RESCHED))) 				\/*��ǰ�����ڵȴ��ص���*/\
		preempt_schedule(); 										\/**/\
} while (0)
/*�����ں���ռ*/
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


/*���̱���ռ�����µ���ʱ���õ�֪ͨ����*/
struct preempt_ops
{
	/*���̱����µ��ȵ�ָ��������������*/
	void (*sched_in)(struct preempt_notifier *notifier, int cpu);
	/*���̱��ĸ�������ռ*/
	void (*sched_out)(struct preempt_notifier *notifier, struct task_struct *next);
};

/*������ռ֪ͨ�ؼ���Ϣ��������container_ofһ��ʹ��*/
struct preempt_notifier
{
	/*�ڲ�ʹ��*/
	struct hlist_node link;
	/*֪ͨ����*/
	struct preempt_ops *ops;
};

void preempt_notifier_register(struct preempt_notifier *notifier);
void preempt_notifier_unregister(struct preempt_notifier *notifier);
/*��ʼ����ռ֪ͨ����*/
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
