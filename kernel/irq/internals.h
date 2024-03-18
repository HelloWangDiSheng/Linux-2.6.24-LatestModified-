/*中断请求子系统内部函数和变量*/

extern int noirqdebug;

/*irq_chip数据结构的默认设置函数*/
extern void irq_chip_set_defaults(struct irq_chip *chip);

/*设置默认处理函数*/
extern void compat_irq_chip_set_default_handler(struct irq_desc *desc);

#ifdef CONFIG_PROC_FS
extern void register_irq_proc(unsigned int irq);
extern void register_handler_proc(unsigned int irq, struct irqaction *action);
extern void unregister_handler_proc(unsigned int irq, struct irqaction *action);
#else
static inline void register_irq_proc(unsigned int irq) { }
static inline void register_handler_proc(unsigned int irq, struct irqaction *action) { }
static inline void unregister_handler_proc(unsigned int irq, struct irqaction *action) { }
#endif

/*调试输出信息*/

#include <linux/kallsyms.h>

/*如果中断请求标识被设置，则打印该提示信息*/
#define P(f) if (desc->status & f) printk("%14s set\n", #f)

/*打印中断描述符相关信息*/
static inline void print_irq_desc(unsigned int irq, struct irq_desc *desc)
{
	printk("irq %d, desc: %p, depth: %d, count: %d, unhandled: %d\n", irq, desc, desc->depth,
			desc->irq_count, desc->irqs_unhandled);
	printk("->handle_irq():  %p, ", desc->handle_irq);
	print_symbol("%s\n", (unsigned long)desc->handle_irq);
	printk("->chip(): %p, ", desc->chip);
	print_symbol("%s\n", (unsigned long)desc->chip);
	printk("->action(): %p\n", desc->action);
	if (desc->action)
	{
		printk("->action->handler(): %p, ", desc->action->handler);
		print_symbol("%s\n", (unsigned long)desc->action->handler);
	}

	P(IRQ_INPROGRESS);
	P(IRQ_DISABLED);
	P(IRQ_PENDING);
	P(IRQ_REPLAY);
	P(IRQ_AUTODETECT);
	P(IRQ_WAITING);
	P(IRQ_LEVEL);
	P(IRQ_MASKED);
#ifdef CONFIG_IRQ_PER_CPU
	P(IRQ_PER_CPU);
#endif
	P(IRQ_NOPROBE);
	P(IRQ_NOREQUEST);
	P(IRQ_NOAUTOEN);
}

#undef P

