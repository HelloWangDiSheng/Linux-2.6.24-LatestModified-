#ifndef LINUX_HARDIRQ_H
#define LINUX_HARDIRQ_H

#include <linux/preempt.h>
#include <linux/smp_lock.h>
#include <linux/lockdep.h>
#include <asm/hardirq.h>
#include <asm/system.h>

/*Ӳ�жϼ����������жϼ������ŵ���ռ�������С��������£�0��7����λ������ռ����������
����ռ���256����8��15����λ�����жϼ������������Ŀ��256����Ӳ�жϼ�������ÿ����ϵ��
������д��Ĭ���ǣ�16��27λ����Ӳ�жϼ������������Ŀ��4096������28λ��PREEMPT_ACTIVE
��ʶ��PREEMPT_MASK: 0x000000ff��SOFTIRQ_MASK: 0x0000ff00��HARDIRQ_MASK: 0x0fff0000*/
/*��ͨ��ռ��ռ����λ��Ŀ��0��7��8λ��*/
#define PREEMPT_BITS		8
/*���ж���ռ����λ��Ŀ��8��15λ��8λ��*/
#define SOFTIRQ_BITS		8
/*Ӳ�ж���ռ����λ��Ŀ��Ĭ����16��27��12λ��*/
#ifndef HARDIRQ_BITS
#define HARDIRQ_BITS		12

/*����ÿ��cpu����Ӳ�ж���Ŀ*/
#ifndef MAX_HARDIRQS_PER_CPU
#define MAX_HARDIRQS_PER_CPU NR_IRQS
#endif

/*
 * The hardirq mask has to be large enough to have space for potentially
 * all IRQ sources in the system nesting on a single CPU.
 */
#if (1 << HARDIRQ_BITS) < MAX_HARDIRQS_PER_CPU
#error HARDIRQ_BITS is too low!
#endif
#endif

/*��ͨ��ռƫ��λ��ռ��0��7�ĵ�8λ������������λ0��ʼ*/
#define PREEMPT_SHIFT		0
/*���ж�ƫ��λ������ͨ��ռ����λ��ʼ������λ8ֵ15λ��*/
#define SOFTIRQ_SHIFT		(PREEMPT_SHIFT + PREEMPT_BITS)
/*Ӳ�ж�ƫ��λ�����жϽ���λ��ʼ*/
#define HARDIRQ_SHIFT		(SOFTIRQ_SHIFT + SOFTIRQ_BITS)
/*�ж���������*/
#define __IRQ_MASK(x)		((1UL << (x))-1)
/*��ͨ��ռ����0x000000FF*/
#define PREEMPT_MASK		(__IRQ_MASK(PREEMPT_BITS) << PREEMPT_SHIFT)
/*���ж�����0x0000FF00*/
#define SOFTIRQ_MASK		(__IRQ_MASK(SOFTIRQ_BITS) << SOFTIRQ_SHIFT)
/*Ӳ�ж�����0x0FFF0000*/
#define HARDIRQ_MASK		(__IRQ_MASK(HARDIRQ_BITS) << HARDIRQ_SHIFT)

/*��ͨ��ռƫ��*/
#define PREEMPT_OFFSET		(1UL << PREEMPT_SHIFT)
/*���ж�ƫ��*/
#define SOFTIRQ_OFFSET		(1UL << SOFTIRQ_SHIFT)
/*Ӳ�ж�ƫ��*/
#define HARDIRQ_OFFSET		(1UL << HARDIRQ_SHIFT)

/*�����ں���ռ����Ч��*/
#if PREEMPT_ACTIVE < (1 << (HARDIRQ_SHIFT + HARDIRQ_BITS))
#error PREEMPT_ACTIVE is too low!
#endif

/*��ȡӲ�жϱ��*/
#define hardirq_count()		(preempt_count() & HARDIRQ_MASK)
/*��ȡ���жϱ��*/
#define softirq_count()		(preempt_count() & SOFTIRQ_MASK)
/*��ȡ�жϱ��*/
#define irq_count()			(preempt_count() & (HARDIRQ_MASK | SOFTIRQ_MASK))

/*���Դ���Ӳ�жϻ������жϣ��������ж��������л���Ӳ�ж���������*/
/*����Ӳ�ж���*/
#define in_irq()			(hardirq_count())
/*�������ж���*/
#define in_softirq()		(softirq_count())
/*�����ж���*/
#define in_interrupt()		(irq_count())


#if defined(CONFIG_PREEMPT) && !defined(CONFIG_PREEMPT_BKL)
/*ϵͳ�����жϻ���ռ��*/
#define in_atomic()			((preempt_count() & ~PREEMPT_ACTIVE) != kernel_locked())
#else
#define in_atomic()			((preempt_count() & ~PREEMPT_ACTIVE) != 0)
#endif

#ifdef CONFIG_PREEMPT
#define PREEMPT_CHECK_OFFSET 1
#else
#define PREEMPT_CHECK_OFFSET 0
#endif

/*
 * Check whether we were atomic before we did preempt_disable():
 * (used by the scheduler)
 */
#define in_atomic_preempt_off() \
		((preempt_count() & ~PREEMPT_ACTIVE) != PREEMPT_CHECK_OFFSET)

#ifdef CONFIG_PREEMPT
#define preemptible()		(preempt_count() == 0 && !irqs_disabled())
#define IRQ_EXIT_OFFSET (HARDIRQ_OFFSET-1)
#else
#define preemptible()	0
#define IRQ_EXIT_OFFSET HARDIRQ_OFFSET
#endif

#ifdef CONFIG_SMP
extern void synchronize_irq(unsigned int irq);
#else
#define synchronize_irq(irq)	barrier()
#endif

struct task_struct;

#ifndef CONFIG_VIRT_CPU_ACCOUNTING
static inline void account_system_vtime(struct task_struct *tsk)
{
}
#endif

/*
 * It is safe to do non-atomic ops on ->hardirq_context,
 * because NMI handlers may not preempt and the ops are
 * always balanced, so the interrupted value of ->hardirq_context
 * will always be restored.
 */
#define __irq_enter()					\
	do {						\
		account_system_vtime(current);		\
		add_preempt_count(HARDIRQ_OFFSET);	\
		trace_hardirq_enter();			\
	} while (0)

/*
 * Enter irq context (on NO_HZ, update jiffies):
 */
extern void irq_enter(void);

/*
 * Exit irq context without processing softirqs:
 */
#define __irq_exit()					\
	do {						\
		trace_hardirq_exit();			\
		account_system_vtime(current);		\
		sub_preempt_count(HARDIRQ_OFFSET);	\
	} while (0)

/*
 * Exit irq context and process softirqs if needed:
 */
extern void irq_exit(void);

#define nmi_enter()		do { lockdep_off(); __irq_enter(); } while (0)
#define nmi_exit()		do { __irq_exit(); lockdep_on(); } while (0)

#endif /* LINUX_HARDIRQ_H */
