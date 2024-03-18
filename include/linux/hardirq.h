#ifndef LINUX_HARDIRQ_H
#define LINUX_HARDIRQ_H

#include <linux/preempt.h>
#include <linux/smp_lock.h>
#include <linux/lockdep.h>
#include <asm/hardirq.h>
#include <asm/system.h>

/*硬中断计数器和软中断计数器放到抢占计数器中。掩码如下：0至7比特位用作抢占计数器（最
大抢占深度256）；8至15比特位是软中断计数器（最大数目是256）；硬中断计数器（每个体系结
构可重写）默认是：16至27位用是硬中断计数器（最大数目是4096）；第28位是PREEMPT_ACTIVE
标识。PREEMPT_MASK: 0x000000ff、SOFTIRQ_MASK: 0x0000ff00、HARDIRQ_MASK: 0x0fff0000*/
/*普通抢占所占比特位数目（0至7低8位）*/
#define PREEMPT_BITS		8
/*软中断所占比特位数目（8至15位共8位）*/
#define SOFTIRQ_BITS		8
/*硬中断所占比特位数目（默认是16至27共12位）*/
#ifndef HARDIRQ_BITS
#define HARDIRQ_BITS		12

/*定义每个cpu最大的硬中断数目*/
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

/*普通抢占偏移位（占据0至7的低8位）从索引比特位0开始*/
#define PREEMPT_SHIFT		0
/*软中断偏移位，从普通抢占结束位开始（比特位8值15位）*/
#define SOFTIRQ_SHIFT		(PREEMPT_SHIFT + PREEMPT_BITS)
/*硬中断偏移位从软中断结束位开始*/
#define HARDIRQ_SHIFT		(SOFTIRQ_SHIFT + SOFTIRQ_BITS)
/*中断请求掩码*/
#define __IRQ_MASK(x)		((1UL << (x))-1)
/*普通抢占掩码0x000000FF*/
#define PREEMPT_MASK		(__IRQ_MASK(PREEMPT_BITS) << PREEMPT_SHIFT)
/*软中断掩码0x0000FF00*/
#define SOFTIRQ_MASK		(__IRQ_MASK(SOFTIRQ_BITS) << SOFTIRQ_SHIFT)
/*硬中断掩码0x0FFF0000*/
#define HARDIRQ_MASK		(__IRQ_MASK(HARDIRQ_BITS) << HARDIRQ_SHIFT)

/*普通抢占偏移*/
#define PREEMPT_OFFSET		(1UL << PREEMPT_SHIFT)
/*软中断偏移*/
#define SOFTIRQ_OFFSET		(1UL << SOFTIRQ_SHIFT)
/*硬中断偏移*/
#define HARDIRQ_OFFSET		(1UL << HARDIRQ_SHIFT)

/*测试内核抢占的有效性*/
#if PREEMPT_ACTIVE < (1 << (HARDIRQ_SHIFT + HARDIRQ_BITS))
#error PREEMPT_ACTIVE is too low!
#endif

/*获取硬中断编号*/
#define hardirq_count()		(preempt_count() & HARDIRQ_MASK)
/*获取软中断编号*/
#define softirq_count()		(preempt_count() & SOFTIRQ_MASK)
/*获取中断编号*/
#define irq_count()			(preempt_count() & (HARDIRQ_MASK | SOFTIRQ_MASK))

/*测试处于硬中断还是软中断？处于软中断上下文中还是硬中断上下文中*/
/*处于硬中断中*/
#define in_irq()			(hardirq_count())
/*处于软中断中*/
#define in_softirq()		(softirq_count())
/*处于中断中*/
#define in_interrupt()		(irq_count())


#if defined(CONFIG_PREEMPT) && !defined(CONFIG_PREEMPT_BKL)
/*系统处于中断或抢占中*/
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
