#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/pid_namespace.h>
#include <linux/notifier.h>
#include <linux/thread_info.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/posix-timers.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/tick.h>
#include <linux/kallsyms.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/div64.h>
#include <asm/timex.h>
#include <asm/io.h>

/*内核需要数据结构来管理系统中注册的所有定时器（这些定时器可能分配给某个进程或内核本
身）。该结构必须容许快速而高效地检查到期的定时器，以免消耗太多CPU时间。毕竟，每个时钟
中断都必须进行这样的检查。
数据结构中不仅必须包含管理定时器所需的全部信息，而且它必须能够很容易地进行周期性的扫
描，以便执行到期的定时器并删除。主要的困难在于扫描即将到期和刚刚到期的定时器链表。因
为只是将所有timer_list实例简单地串联在一起是不够的，内核创建了不同的组，根据定时器的
到期时间进行分类。分类的基础是一个主数组，有5个数组项，都是数组。主数组的5个位置根据
到期时间对定时器进行粗略的分类。第一组是到期时间在0到255（或2^8-1）个时钟周期之间的所
有定时器。第二组包含了到期时间在256和2^(8+6)-1 = 2^14-1个时钟周期之间的所有定时器。
第三组中定时器的到期时间范围是从2^14到2^(8+2×6)-1个时钟周期，依次类推。主表中的各项，称为组（group） ，有时又称为桶（bucket） 。表15-1列出了各个定时器组
的时间间隔。这里以普通系统上桶的大小作为计算的基础。在内存较少的小型系统上，时间间隔有
所不同。定时器的时间间隔
	组 				时间间隔
	tv1 			0					2^8-1
	tv2 			2^8  				2^14-1
	tv3 			2^14  				2^20-1
	tv4 			2^20  				2^26-1
	tv5 			2^26  				2^32-1

每个组本身由一个数组组成，定时器在其中再次排序。第一个组的数组有256个数组项，每个位置
表示0到255个时钟周期之间一个可能的到期时间。如果系统中有几个定时器的到期时间相同，它们
通过一个标准的双链表连接起来（链表元素为timer_list的entry成员）。其余的组也由数组组成，
但数组项数目较少，是64个。数组项包含的是timer_list的双链表。但每个数组项包含的
timer_list的expires值不再只有一个，而是一个时间间隔。间隔的长度与组是相关的。对第二组
来说，每个数组项可容许的时间间隔为256=2^8个时钟周期，而对第三组来说是2^14个时钟周期，
对第四组来说是2^20，对第五组来说是2^26。在我们考虑定时器是如何随着时间的推移而最终执行
以及相关的数据结构如何改变的时候，上述这些时间间隔的意义就很清楚了。定时器是如何执行的
呢？内核主要负责关注第一组的定时器，因为这些定时器都将在稍后到期。为简单起见，我们假定
每组都有一个计数器，存储了某个数组位置的编号（实际的内核实现在功能上是等效的，但结构上
的清晰程度要差得多，读者稍后会看到）。
第一组中的索引项指向的数组元素，保存了稍后即将执行的各定时器的timer_list实例。每当遇到
一个时钟中断时，内核都扫描该链表，执行所有定时器函数，并将索引位置加1。刚执行过的定时器
则从数据结构移除。下一次发生时钟中断时，将执行新的数组位置上的定时器，并将其从数据结构
移除，同样将索引加1，依次类推。在所有项都处理之后，索引值为255。因为这里的加法是模256的
，因而索引将恢复到初始位置（位置0）。因为第一组的内容在最多256个时钟周期之后就会耗尽，
必须将后续各组的定时器依次前推，重新补足第一组。在第一组的索引位置恢复到初始位置0之后，
会将第二组中一个数组项的所有定时器补充到第一组。这种做法，解释了为什么各组选择了不同的
时间间隔。因为第一组的各数组项可能有256个不同的到期时间，而第二组中一个数组项的数据就足
以填充第一组的整个数组。该道理同样适用于后续各组。第三组的一个数组项的数据同样足以填充
整个第二组，第四组的一个数组项也足以填充整个第三组，而第五组的一个数组项也足以填充整个
第四组。后续各组的数组位置并非随机选择的，其中的索引项仍然发挥了作用。但索引项的值不再
是每个时钟周期加1，而是每256^(i-1)个时钟周期加1，其中i是组的编号。我们根据一个具体例子
来考察这种行为模式：从第一组的处理开始已经过了256个jiffies，此时索引重置为0。同时，第二
组的第一个数组项的内容将补充到第一组中。我们假定在第一组索引重置时，jiffies系统计时器的
值为10000。在第二组的第一个数组项中，有一个定时器链表，各定时器分别在时钟周期10001、
10015、10015、10254到期。这些定时器分别会定位到第一组的1、15和254，在位置15会创建一个链
表，包括两个指针，因为这两个定时器同时到期。在复制完成后，将第二组的索引位置加1。循环接
下来重新开始。在每个时钟周期会逐一处理第一组的各个索引位置上的定时器，直至到达索引位置
255。接下来，用第二组的第二个数组元素中的所有定时器，来补充第一组。在第二组的索引位置到
达63时（从第二组开始，每组只包含64个数组项），则使用第三组第一个数组项的内容来补充第二
组。最后，在第三组的索引位置到达最大值时，从第四组取得新的数据；同样的原则，也适用于第
五组到第四组的数据传输。为确定哪些定时器已经到期，内核无须扫描一个巨大的定时器链表，处
理范围仅限于第一组中的一个数组项。因为该位置通常是空的或仅包含一个定时器，检查可以很快
进行。偶尔从后续的各组向前复制数据甚至也不需要多少CPU时间，因为复制可以通过指针操作高效
进行（内核无须复制内存块，而只需将指针设置为新值，如同标准链表函数那样）。
*/

/*定义64位jiffies*/
u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
EXPORT_SYMBOL(jiffies_64);

/*per-CPU定时器向量定义*/
/*动态定时器第一组后面个组位偏移，用来计算对应数组项数目*/
#define TVN_BITS (CONFIG_BASE_SMALL ? 4 : 6)
/*动态定时器第一组位偏移，用来计算第一组数组项数目*/
#define TVR_BITS (CONFIG_BASE_SMALL ? 6 : 8)
/*第一组以后各组数组项数目*/
#define TVN_SIZE (1 << TVN_BITS)
/*第一组数组项数目*/
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)
/*tvec_root_t对应第一组，而tvec_t表示后续各组。两个结构的不同只在于数组项的个数。对第
一组，TVR_SIZE定义为256。所有其他组使用的数组长度为TVN_SIZE，默认值64。缺乏内存的系统可
设置配置选项BASE_SMALL。在这种情况下，第一组有64个数组项，而其他各组的数组项为16个*/
typedef struct tvec_root_s
{
	struct list_head vec[TVR_SIZE];
} tvec_root_t;

typedef struct tvec_s
{
	struct list_head vec[TVN_SIZE];
} tvec_t;

/*系统中的每个处理器都有自身的数据结构，来管理运行于其上的定时器。下列数据结构的一个各
CPU实例，用作根数据项*/
struct tvec_t_base_s
{
	spinlock_t lock;
	struct timer_list *running_timer;
	/*记录了一个时间点（单位为jiffies），该结构中此前到期的定时器都已经执行。例如，如
果该变量的值为10 500，那么内核就知道，jiffies值10 499及之前到期的定时器都已经执行过了。
通常，timer_jiffies等于jiffies或比jiffies小1。如果内核有一段时间无法执行定时器（系统负
荷非常高），二者的差值可能会稍大一点*/
	unsigned long timer_jiffies;
	tvec_root_t tv1;
	tvec_t tv2;
	tvec_t tv3;
	tvec_t tv4;
	tvec_t tv5;
} ____cacheline_aligned;
typedef struct tvec_t_base_s tvec_base_t;
/**/
tvec_base_t boot_tvec_bases;
EXPORT_SYMBOL(boot_tvec_bases);
static DEFINE_PER_CPU(tvec_base_t *, tvec_bases) = &boot_tvec_bases;

/*
 * Note that all tvec_bases is 2 byte aligned and lower bit of
 * base in timer_list is guaranteed to be zero. Use the LSB for
 * the new flag to indicate whether the timer is deferrable
 */
#define TBASE_DEFERRABLE_FLAG		(0x1)

/* Functions below help us manage 'deferrable' flag */
static inline unsigned int tbase_get_deferrable(tvec_base_t *base)
{
	return ((unsigned int)(unsigned long)base & TBASE_DEFERRABLE_FLAG);
}

static inline tvec_base_t *tbase_get_base(tvec_base_t *base)
{
	return ((tvec_base_t *)((unsigned long)base & ~TBASE_DEFERRABLE_FLAG));
}

static inline void timer_set_deferrable(struct timer_list *timer)
{
	timer->base = ((tvec_base_t *)((unsigned long)(timer->base) |
				       TBASE_DEFERRABLE_FLAG));
}

static inline void
timer_set_base(struct timer_list *timer, tvec_base_t *new_base)
{
	timer->base = (tvec_base_t *)((unsigned long)(new_base) |
				      tbase_get_deferrable(timer->base));
}

/**
 * __round_jiffies - function to round jiffies to a full second
 * @j: the time in (absolute) jiffies that should be rounded
 * @cpu: the processor number on which the timeout will happen
 *
 * __round_jiffies() rounds an absolute time in the future (in jiffies)
 * up or down to (approximately) full seconds. This is useful for timers
 * for which the exact time they fire does not matter too much, as long as
 * they fire approximately every X seconds.
 *
 * By rounding these timers to whole seconds, all such timers will fire
 * at the same time, rather than at various times spread out. The goal
 * of this is to have the CPU wake up less, which saves power.
 *
 * The exact rounding is skewed for each processor to avoid all
 * processors firing at the exact same time, which could lead
 * to lock contention or spurious cache line bouncing.
 *
 * The return value is the rounded version of the @j parameter.
 */
unsigned long __round_jiffies(unsigned long j, int cpu)
{
	int rem;
	unsigned long original = j;

	/*
	 * We don't want all cpus firing their timers at once hitting the
	 * same lock or cachelines, so we skew each extra cpu with an extra
	 * 3 jiffies. This 3 jiffies came originally from the mm/ code which
	 * already did this.
	 * The skew is done by adding 3*cpunr, then round, then subtract this
	 * extra offset again.
	 */
	j += cpu * 3;

	rem = j % HZ;

	/*
	 * If the target jiffie is just after a whole second (which can happen
	 * due to delays of the timer irq, long irq off times etc etc) then
	 * we should round down to the whole second, not up. Use 1/4th second
	 * as cutoff for this rounding as an extreme upper bound for this.
	 */
	if (rem < HZ/4) /* round down */
		j = j - rem;
	else /* round up */
		j = j - rem + HZ;

	/* now that we have rounded, subtract the extra skew again */
	j -= cpu * 3;

	if (j <= jiffies) /* rounding ate our timeout entirely; */
		return original;
	return j;
}
EXPORT_SYMBOL_GPL(__round_jiffies);

/**
 * __round_jiffies_relative - function to round jiffies to a full second
 * @j: the time in (relative) jiffies that should be rounded
 * @cpu: the processor number on which the timeout will happen
 *
 * __round_jiffies_relative() rounds a time delta  in the future (in jiffies)
 * up or down to (approximately) full seconds. This is useful for timers
 * for which the exact time they fire does not matter too much, as long as
 * they fire approximately every X seconds.
 *
 * By rounding these timers to whole seconds, all such timers will fire
 * at the same time, rather than at various times spread out. The goal
 * of this is to have the CPU wake up less, which saves power.
 *
 * The exact rounding is skewed for each processor to avoid all
 * processors firing at the exact same time, which could lead
 * to lock contention or spurious cache line bouncing.
 *
 * The return value is the rounded version of the @j parameter.
 */
unsigned long __round_jiffies_relative(unsigned long j, int cpu)
{
	/*
	 * In theory the following code can skip a jiffy in case jiffies
	 * increments right between the addition and the later subtraction.
	 * However since the entire point of this function is to use approximate
	 * timeouts, it's entirely ok to not handle that.
	 */
	return  __round_jiffies(j + jiffies, cpu) - jiffies;
}
EXPORT_SYMBOL_GPL(__round_jiffies_relative);

/**
 * round_jiffies - function to round jiffies to a full second
 * @j: the time in (absolute) jiffies that should be rounded
 *
 * round_jiffies() rounds an absolute time in the future (in jiffies)
 * up or down to (approximately) full seconds. This is useful for timers
 * for which the exact time they fire does not matter too much, as long as
 * they fire approximately every X seconds.
 *
 * By rounding these timers to whole seconds, all such timers will fire
 * at the same time, rather than at various times spread out. The goal
 * of this is to have the CPU wake up less, which saves power.
 *
 * The return value is the rounded version of the @j parameter.
 */
unsigned long round_jiffies(unsigned long j)
{
	return __round_jiffies(j, raw_smp_processor_id());
}
EXPORT_SYMBOL_GPL(round_jiffies);

/**
 * round_jiffies_relative - function to round jiffies to a full second
 * @j: the time in (relative) jiffies that should be rounded
 *
 * round_jiffies_relative() rounds a time delta  in the future (in jiffies)
 * up or down to (approximately) full seconds. This is useful for timers
 * for which the exact time they fire does not matter too much, as long as
 * they fire approximately every X seconds.
 *
 * By rounding these timers to whole seconds, all such timers will fire
 * at the same time, rather than at various times spread out. The goal
 * of this is to have the CPU wake up less, which saves power.
 *
 * The return value is the rounded version of the @j parameter.
 */
unsigned long round_jiffies_relative(unsigned long j)
{
	return __round_jiffies_relative(j, raw_smp_processor_id());
}
EXPORT_SYMBOL_GPL(round_jiffies_relative);


static inline void set_running_timer(tvec_base_t *base,
					struct timer_list *timer)
{
#ifdef CONFIG_SMP
	base->running_timer = timer;
#endif
}

static void internal_add_timer(tvec_base_t *base, struct timer_list *timer)
{
	unsigned long expires = timer->expires;
	unsigned long idx = expires - base->timer_jiffies;
	struct list_head *vec;

	if (idx < TVR_SIZE) {
		int i = expires & TVR_MASK;
		vec = base->tv1.vec + i;
	} else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
		int i = (expires >> TVR_BITS) & TVN_MASK;
		vec = base->tv2.vec + i;
	} else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		vec = base->tv3.vec + i;
	} else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		vec = base->tv4.vec + i;
	} else if ((signed long) idx < 0) {
		/*
		 * Can happen if you add a timer with expires == jiffies,
		 * or you set a timer to go off in the past
		 */
		vec = base->tv1.vec + (base->timer_jiffies & TVR_MASK);
	} else {
		int i;
		/* If the timeout is larger than 0xffffffff on 64-bit
		 * architectures then we use the maximum timeout:
		 */
		if (idx > 0xffffffffUL) {
			idx = 0xffffffffUL;
			expires = idx + base->timer_jiffies;
		}
		i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		vec = base->tv5.vec + i;
	}
	/*
	 * Timers are FIFO:
	 */
	list_add_tail(&timer->entry, vec);
}

#ifdef CONFIG_TIMER_STATS
void __timer_stats_timer_set_start_info(struct timer_list *timer, void *addr)
{
	if (timer->start_site)
		return;

	timer->start_site = addr;
	memcpy(timer->start_comm, current->comm, TASK_COMM_LEN);
	timer->start_pid = current->pid;
}

static void timer_stats_account_timer(struct timer_list *timer)
{
	unsigned int flag = 0;

	if (unlikely(tbase_get_deferrable(timer->base)))
		flag |= TIMER_STATS_FLAG_DEFERRABLE;

	timer_stats_update_stats(timer, timer->start_pid, timer->start_site,
				 timer->function, timer->start_comm, flag);
}

#else
static void timer_stats_account_timer(struct timer_list *timer) {}
#endif

/*初始化一个定时器，定时器必须在调用其它定时器函数之前初始化*/
void fastcall init_timer(struct timer_list *timer)
{
	timer->entry.next = NULL;
	timer->base = __raw_get_cpu_var(tvec_bases);
#ifdef CONFIG_TIMER_STATS
	timer->start_site = NULL;
	timer->start_pid = -1;
	memset(timer->start_comm, 0, TASK_COMM_LEN);
#endif
}
EXPORT_SYMBOL(init_timer);

void fastcall init_timer_deferrable(struct timer_list *timer)
{
	init_timer(timer);
	timer_set_deferrable(timer);
}
EXPORT_SYMBOL(init_timer_deferrable);

static inline void detach_timer(struct timer_list *timer,
				int clear_pending)
{
	struct list_head *entry = &timer->entry;

	__list_del(entry->prev, entry->next);
	if (clear_pending)
		entry->next = NULL;
	entry->prev = LIST_POISON2;
}

/*
 * We are using hashed locking: holding per_cpu(tvec_bases).lock
 * means that all timers which are tied to this base via timer->base are
 * locked, and the base itself is locked too.
 *
 * So __run_timers/migrate_timers can safely modify all timers which could
 * be found on ->tvX lists.
 *
 * When the timer's base is locked, and the timer removed from list, it is
 * possible to set timer->base = NULL and drop the lock: the timer remains
 * locked.
 */
static tvec_base_t *lock_timer_base(struct timer_list *timer,
					unsigned long *flags)
	__acquires(timer->base->lock)
{
	tvec_base_t *base;

	for (;;) {
		tvec_base_t *prelock_base = timer->base;
		base = tbase_get_base(prelock_base);
		if (likely(base != NULL)) {
			spin_lock_irqsave(&base->lock, *flags);
			if (likely(prelock_base == timer->base))
				return base;
			/* The timer has migrated to another CPU */
			spin_unlock_irqrestore(&base->lock, *flags);
		}
		cpu_relax();
	}
}

int __mod_timer(struct timer_list *timer, unsigned long expires)
{
	tvec_base_t *base, *new_base;
	unsigned long flags;
	int ret = 0;

	timer_stats_timer_set_start_info(timer);
	BUG_ON(!timer->function);

	base = lock_timer_base(timer, &flags);

	if (timer_pending(timer)) {
		detach_timer(timer, 0);
		ret = 1;
	}

	new_base = __get_cpu_var(tvec_bases);

	if (base != new_base) {
		/*
		 * We are trying to schedule the timer on the local CPU.
		 * However we can't change timer's base while it is running,
		 * otherwise del_timer_sync() can't detect that the timer's
		 * handler yet has not finished. This also guarantees that
		 * the timer is serialized wrt itself.
		 */
		if (likely(base->running_timer != timer)) {
			/* See the comment in lock_timer_base() */
			timer_set_base(timer, NULL);
			spin_unlock(&base->lock);
			base = new_base;
			spin_lock(&base->lock);
			timer_set_base(timer, base);
		}
	}

	timer->expires = expires;
	internal_add_timer(base, timer);
	spin_unlock_irqrestore(&base->lock, flags);

	return ret;
}

EXPORT_SYMBOL(__mod_timer);

/**
 * add_timer_on - start a timer on a particular CPU
 * @timer: the timer to be added
 * @cpu: the CPU to start it on
 *
 * This is not very scalable on SMP. Double adds are not possible.
 */
void add_timer_on(struct timer_list *timer, int cpu)
{
	tvec_base_t *base = per_cpu(tvec_bases, cpu);
	unsigned long flags;

	timer_stats_timer_set_start_info(timer);
	BUG_ON(timer_pending(timer) || !timer->function);
	spin_lock_irqsave(&base->lock, flags);
	timer_set_base(timer, base);
	internal_add_timer(base, timer);
	spin_unlock_irqrestore(&base->lock, flags);
}


/**
 * mod_timer - modify a timer's timeout
 * @timer: the timer to be modified
 * @expires: new timeout in jiffies
 *
 * mod_timer() is a more efficient way to update the expire field of an
 * active timer (if the timer is inactive it will be activated)
 *
 * mod_timer(timer, expires) is equivalent to:
 *
 *     del_timer(timer); timer->expires = expires; add_timer(timer);
 *
 * Note that if there are multiple unserialized concurrent users of the
 * same timer, then mod_timer() is the only safe way to modify the timeout,
 * since add_timer() cannot modify an already running timer.
 *
 * The function returns whether it has modified a pending timer or not.
 * (ie. mod_timer() of an inactive timer returns 0, mod_timer() of an
 * active timer returns 1.)
 */
int mod_timer(struct timer_list *timer, unsigned long expires)
{
	BUG_ON(!timer->function);

	timer_stats_timer_set_start_info(timer);
	/*
	 * This is a common optimization triggered by the
	 * networking code - if the timer is re-modified
	 * to be the same thing then just return:
	 */
	if (timer->expires == expires && timer_pending(timer))
		return 1;

	return __mod_timer(timer, expires);
}

EXPORT_SYMBOL(mod_timer);

/**
 * del_timer - deactive a timer.
 * @timer: the timer to be deactivated
 *
 * del_timer() deactivates a timer - this works on both active and inactive
 * timers.
 *
 * The function returns whether it has deactivated a pending timer or not.
 * (ie. del_timer() of an inactive timer returns 0, del_timer() of an
 * active timer returns 1.)
 */
int del_timer(struct timer_list *timer)
{
	tvec_base_t *base;
	unsigned long flags;
	int ret = 0;

	timer_stats_timer_clear_start_info(timer);
	if (timer_pending(timer)) {
		base = lock_timer_base(timer, &flags);
		if (timer_pending(timer)) {
			detach_timer(timer, 1);
			ret = 1;
		}
		spin_unlock_irqrestore(&base->lock, flags);
	}

	return ret;
}

EXPORT_SYMBOL(del_timer);

#ifdef CONFIG_SMP
/**
 * try_to_del_timer_sync - Try to deactivate a timer
 * @timer: timer do del
 *
 * This function tries to deactivate a timer. Upon successful (ret >= 0)
 * exit the timer is not queued and the handler is not running on any CPU.
 *
 * It must not be called from interrupt contexts.
 */
int try_to_del_timer_sync(struct timer_list *timer)
{
	tvec_base_t *base;
	unsigned long flags;
	int ret = -1;

	base = lock_timer_base(timer, &flags);

	if (base->running_timer == timer)
		goto out;

	ret = 0;
	if (timer_pending(timer)) {
		detach_timer(timer, 1);
		ret = 1;
	}
out:
	spin_unlock_irqrestore(&base->lock, flags);

	return ret;
}

EXPORT_SYMBOL(try_to_del_timer_sync);

/**
 * del_timer_sync - deactivate a timer and wait for the handler to finish.
 * @timer: the timer to be deactivated
 *
 * This function only differs from del_timer() on SMP: besides deactivating
 * the timer it also makes sure the handler has finished executing on other
 * CPUs.
 *
 * Synchronization rules: Callers must prevent restarting of the timer,
 * otherwise this function is meaningless. It must not be called from
 * interrupt contexts. The caller must not hold locks which would prevent
 * completion of the timer's handler. The timer's handler must not call
 * add_timer_on(). Upon exit the timer is not queued and the handler is
 * not running on any CPU.
 *
 * The function returns whether it has deactivated a pending timer or not.
 */
int del_timer_sync(struct timer_list *timer)
{
	for (;;) {
		int ret = try_to_del_timer_sync(timer);
		if (ret >= 0)
			return ret;
		cpu_relax();
	}
}

EXPORT_SYMBOL(del_timer_sync);
#endif

static int cascade(tvec_base_t *base, tvec_t *tv, int index)
{
	/* cascade all the timers from tv up one level */
	struct timer_list *timer, *tmp;
	struct list_head tv_list;

	list_replace_init(tv->vec + index, &tv_list);

	/*
	 * We are removing _all_ timers from the list, so we
	 * don't have to detach them individually.
	 */
	list_for_each_entry_safe(timer, tmp, &tv_list, entry) {
		BUG_ON(tbase_get_base(timer->base) != base);
		internal_add_timer(base, timer);
	}

	return index;
}
/*计算组N的索引值*/
#define INDEX(N) ((base->timer_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)

/**
 * __run_timers - run all expired timers (if any) on this CPU.
 * @base: the timer vector to be processed.
 *
 * This function cascades all vectors and executes all expired timer
 * vectors.
 */
/*对所有定时器的处理都由update_process_timesby发起，它会调用run_local_timers函数。
该函数将使用raise_softirq(TIMER_SOFTIRQ)来激活定时器管理软中断，在下一个可能的时机执
行。run_timer_softirq用作该软中断的处理程序函数，它会选择特定于CPU的struct
tvec_t_base_s实例，并调用__run_timers。__run_timers实现了上面描述的算法。但我们在上面
给出的数据结构中，并没有发现算法描述中提到的索引位置！内核并不需要一个显式的变量来记录
该信息，所有必要的信息都已经包含在base的timer_jiffies成员中②。为此定义了下列宏： */
static inline void __run_timers(tvec_base_t *base)
{
	struct timer_list *timer;

	spin_lock_irq(&base->lock);
	while (time_after_eq(jiffies, base->timer_jiffies))
	{
		struct list_head work_list;
		struct list_head *head = &work_list;
		int index = base->timer_jiffies & TVR_MASK;

		/*
		 * Cascade timers:
		 */
		if (!index &&	(!cascade(base, &base->tv2, INDEX(0))) &&
				(!cascade(base, &base->tv3, INDEX(1))) &&
					!cascade(base, &base->tv4, INDEX(2)))
			cascade(base, &base->tv5, INDEX(3));
		++base->timer_jiffies;
		list_replace_init(base->tv1.vec + index, &work_list);
		while (!list_empty(head))
		{
			void (*fn)(unsigned long);
			unsigned long data;

			timer = list_first_entry(head, struct timer_list,entry);
			fn = timer->function;
			data = timer->data;

			timer_stats_account_timer(timer);

			set_running_timer(base, timer);
			detach_timer(timer, 1);
			spin_unlock_irq(&base->lock);
			{
				int preempt_count = preempt_count();
				fn(data);
				if (preempt_count != preempt_count())
				{
					printk(KERN_WARNING "huh, entered %p " "with preempt_count %08x, exited"
						" with %08x?\n", fn, preempt_count, preempt_count());
					BUG();
				}
			}
			spin_lock_irq(&base->lock);
		}
	}
	set_running_timer(base, NULL);
	spin_unlock_irq(&base->lock);
}

#if defined(CONFIG_NO_IDLE_HZ) || defined(CONFIG_NO_HZ)
/*
 * Find out when the next timer event is due to happen. This
 * is used on S/390 to stop all activity when a cpus is idle.
 * This functions needs to be called disabled.
 */
static unsigned long __next_timer_interrupt(tvec_base_t *base)
{
	unsigned long timer_jiffies = base->timer_jiffies;
	unsigned long expires = timer_jiffies + NEXT_TIMER_MAX_DELTA;
	int index, slot, array, found = 0;
	struct timer_list *nte;
	tvec_t *varray[4];

	/* Look for timer events in tv1. */
	index = slot = timer_jiffies & TVR_MASK;
	do {
		list_for_each_entry(nte, base->tv1.vec + slot, entry) {
			if (tbase_get_deferrable(nte->base))
				continue;

			found = 1;
			expires = nte->expires;
			/* Look at the cascade bucket(s)? */
			if (!index || slot < index)
				goto cascade;
			return expires;
		}
		slot = (slot + 1) & TVR_MASK;
	} while (slot != index);

cascade:
	/* Calculate the next cascade event */
	if (index)
		timer_jiffies += TVR_SIZE - index;
	timer_jiffies >>= TVR_BITS;

	/* Check tv2-tv5. */
	varray[0] = &base->tv2;
	varray[1] = &base->tv3;
	varray[2] = &base->tv4;
	varray[3] = &base->tv5;

	for (array = 0; array < 4; array++) {
		tvec_t *varp = varray[array];

		index = slot = timer_jiffies & TVN_MASK;
		do {
			list_for_each_entry(nte, varp->vec + slot, entry) {
				found = 1;
				if (time_before(nte->expires, expires))
					expires = nte->expires;
			}
			/*
			 * Do we still search for the first timer or are
			 * we looking up the cascade buckets ?
			 */
			if (found) {
				/* Look at the cascade bucket(s)? */
				if (!index || slot < index)
					break;
				return expires;
			}
			slot = (slot + 1) & TVN_MASK;
		} while (slot != index);

		if (index)
			timer_jiffies += TVN_SIZE - index;
		timer_jiffies >>= TVN_BITS;
	}
	return expires;
}

/*
 * Check, if the next hrtimer event is before the next timer wheel
 * event:
 */
static unsigned long cmp_next_hrtimer_event(unsigned long now,
					    unsigned long expires)
{
	ktime_t hr_delta = hrtimer_get_next_event();
	struct timespec tsdelta;
	unsigned long delta;

	if (hr_delta.tv64 == KTIME_MAX)
		return expires;

	/*
	 * Expired timer available, let it expire in the next tick
	 */
	if (hr_delta.tv64 <= 0)
		return now + 1;

	tsdelta = ktime_to_timespec(hr_delta);
	delta = timespec_to_jiffies(&tsdelta);

	/*
	 * Limit the delta to the max value, which is checked in
	 * tick_nohz_stop_sched_tick():
	 */
	if (delta > NEXT_TIMER_MAX_DELTA)
		delta = NEXT_TIMER_MAX_DELTA;

	/*
	 * Take rounding errors in to account and make sure, that it
	 * expires in the next tick. Otherwise we go into an endless
	 * ping pong due to tick_nohz_stop_sched_tick() retriggering
	 * the timer softirq
	 */
	if (delta < 1)
		delta = 1;
	now += delta;
	if (time_before(now, expires))
		return now;
	return expires;
}

/**
 * get_next_timer_interrupt - return the jiffy of the next pending timer
 * @now: current time (in jiffies)
 */
unsigned long get_next_timer_interrupt(unsigned long now)
{
	tvec_base_t *base = __get_cpu_var(tvec_bases);
	unsigned long expires;

	spin_lock(&base->lock);
	expires = __next_timer_interrupt(base);
	spin_unlock(&base->lock);

	if (time_before_eq(expires, now))
		return now;

	return cmp_next_hrtimer_event(now, expires);
}

#ifdef CONFIG_NO_IDLE_HZ
unsigned long next_timer_interrupt(void)
{
	return get_next_timer_interrupt(jiffies);
}
#endif

#endif

#ifndef CONFIG_VIRT_CPU_ACCOUNTING
void account_process_tick(struct task_struct *p, int user_tick)
{
	if (user_tick) {
		account_user_time(p, jiffies_to_cputime(1));
		account_user_time_scaled(p, jiffies_to_cputime(1));
	} else {
		account_system_time(p, HARDIRQ_OFFSET, jiffies_to_cputime(1));
		account_system_time_scaled(p, jiffies_to_cputime(1));
	}
}
#endif

/*
 * Called from the timer interrupt handler to charge one tick to the current
 * process.  user_tick is 1 if the tick is user time, 0 for system.
 */
void update_process_times(int user_tick)
{
	struct task_struct *p = current;
	int cpu = smp_processor_id();

	/* Note: this timer irq context must be accounted for as well. */
	account_process_tick(p, user_tick);
	run_local_timers();
	if (rcu_pending(cpu))
		rcu_check_callbacks(cpu, user_tick);
	scheduler_tick();
	run_posix_cpu_timers(p);
}

/*
 * Nr of active tasks - counted in fixed-point numbers
 */
 /*获取TASK_RUNNING状态的任务（进程和线程）数目*/
static unsigned long count_active_tasks(void)
{
	return nr_active() * FIXED_1;
}

/*
 * Hmm.. Changed this, as the GNU make sources (load.c) seems to
 * imply that avenrun[] is the standard name for this kind of thing.
 * Nothing else seems to be standardized: the fractional size etc
 * all seem to differ on different machines.
 *
 * Requires xtime_lock to access.
 */
unsigned long avenrun[3];

EXPORT_SYMBOL(avenrun);

/*
 Linux的系统负载指运行队列的平均长度，也就是等待CPU的平均进程数。因为Linux内禁止浮点
 运算，因此系统的负载只能通过计算变化的次数这一修 正值来计算。Linux内核定义一个长度为
 3的双字数组avenrun，双字的低11位用于存放负载的小数部分，高21位用于存放整数部分。当进
 程所耗的 CPU时间片数超过CPU在5秒内能够提供的时间片数时，内核计算上述的三个负载。负载
 初始化为0，假设最近1、5、15分钟内的平均负载分别为 load1、load5和load15，那么下一个计
 算时刻到来时，内核通过下面的算式计算负载：
 load1 -= load1 -* exp(-5 / 60) -+ n * (1 - exp(-5 / 60 ))
 load5 -= load5 -* exp(-5 / 300) + n * (1 - exp(-5 / 300))
 load15 = load15 * exp(-5 / 900) + n * (1 - exp(-5 / 900))
 其中，exp(x)为e的x次幂，n为当前运行队列的长度。Linux内核认为进程的生存时间服从参数为
 1的指数分布，指数分布的概率密度为：以内核计算 负载load1为例，设相邻两个计算时刻之间系
 统活动的进程集合为S0。从1分钟前到当前计算时刻这段时间里面活动的load1个进程，设他们的
 集合是 S1，内核认为的概率密度是:λe-λx，而在当前时刻活动的n个进程，设他们的集合是Sn内
 核认为的概率密度是1-λe-λx。其中x = 5 / 60，因为相邻两个计算时刻之间进程所耗的CPU时间
 为5秒，而考虑的时间段是1分钟(60秒)。那么可以求出最近1分钟系统运行队列的长度：
 load1 = |S1| -* λe-λx + |Sn| * (1-λe-λx) = load1 * λe-λx + n * (1-λe-λx)
 其中λ = 1， x = 5 / 60， |S1|和|Sn|是集合元素的个数，这就是Linux内核源文件shed.c的函
 数calc_load()计算负载的数学依据。
 所以“Load值=CPU核数”，这是最理想的状态，没有任何竞争，一个任务分配一个核。
 由于数据是每隔5秒钟检查一次活跃的进程数，然后根据这个数值算出来的。如果这个数除以CPU
 的核数，结果高于5的时候就表明系统在超负荷运转了
 */
/*给定tick数，更新估计的1/5/15分钟的平均运行队列长度，当xtime_lock持有
一个write_lock时调用该函数*/
static inline void calc_load(unsigned long ticks)
{
	unsigned long active_tasks; /* fixed-point */
	static int count = LOAD_FREQ;

	count -= ticks;
	if (unlikely(count < 0))
	{
		active_tasks = count_active_tasks();
		do
		{
			CALC_LOAD(avenrun[0], EXP_1, active_tasks);
			CALC_LOAD(avenrun[1], EXP_5, active_tasks);
			CALC_LOAD(avenrun[2], EXP_15, active_tasks);
			count += LOAD_FREQ;
		} while (count < 0);
	}
}

/*
 * This function runs timers and the timer-tq in bottom half context.
 */
static void run_timer_softirq(struct softirq_action *h)
{
	tvec_base_t *base = __get_cpu_var(tvec_bases);

	hrtimer_run_queues();

	if (time_after_eq(jiffies, base->timer_jiffies))
		__run_timers(base);
}

/*
 * Called by the local, per-CPU timer interrupt on SMP.
 */
void run_local_timers(void)
{
	raise_softirq(TIMER_SOFTIRQ);
	softlockup_tick();
}

/*
 * Called by the timer interrupt. xtime_lock must already be taken
 * by the timer IRQ!
 */
static inline void update_times(unsigned long ticks)
{
	update_wall_time();
	calc_load(ticks);
}

/*
 * The 64-bit jiffies value is not atomic - you MUST NOT read it
 * without sampling the sequence number in xtime_lock.
 * jiffies is defined in the linker script...
 */

void do_timer(unsigned long ticks)
{
	jiffies_64 += ticks;
	update_times(ticks);
}

#ifdef __ARCH_WANT_SYS_ALARM

/*
 * For backwards compatibility?  This can be done in libc so Alpha
 * and all newer ports shouldn't need it.
 */
asmlinkage unsigned long sys_alarm(unsigned int seconds)
{
	return alarm_setitimer(seconds);
}

#endif

#ifndef __alpha__

/*
 * The Alpha uses getxpid, getxuid, and getxgid instead.  Maybe this
 * should be moved into arch/i386 instead?
 */

/**
 * sys_getpid - return the thread group id of the current process
 *
 * Note, despite the name, this returns the tgid not the pid.  The tgid and
 * the pid are identical unless CLONE_THREAD was specified on clone() in
 * which case the tgid is the same in all threads of the same group.
 *
 * This is SMP safe as current->tgid does not change.
 */
asmlinkage long sys_getpid(void)
{
	return task_tgid_vnr(current);
}

/*
 * Accessing ->real_parent is not SMP-safe, it could
 * change from under us. However, we can use a stale
 * value of ->real_parent under rcu_read_lock(), see
 * release_task()->call_rcu(delayed_put_task_struct).
 */
 /*获取当前进程的真实父进程*/
asmlinkage long sys_getppid(void)
{
	int pid;

	rcu_read_lock();
	pid = task_tgid_nr_ns(current->real_parent, current->nsproxy->pid_ns);
	rcu_read_unlock();

	return pid;
}

/*获取当前进程的uid*/
asmlinkage long sys_getuid(void)
{
	/* Only we change this so SMP safe */
	return current->uid;
}

/*获得当前进程的ueid*/
asmlinkage long sys_geteuid(void)
{
	/* Only we change this so SMP safe */
	return current->euid;
}
/*获取当前进程的gid*/
asmlinkage long sys_getgid(void)
{
	/* Only we change this so SMP safe */
	return current->gid;
}
/*获取当前进程的egid*/
asmlinkage long sys_getegid(void)
{
	/* Only we change this so SMP safe */
	return current->egid;
}

#endif

static void process_timeout(unsigned long __data)
{
	wake_up_process((struct task_struct *)__data);
}

/**
 * schedule_timeout - sleep until timeout
 * @timeout: timeout value in jiffies
 *
 * Make the current task sleep until @timeout jiffies have
 * elapsed. The routine will return immediately unless
 * the current task state has been set (see set_current_state()).
 *
 * You can set the task state as follows -
 *
 * %TASK_UNINTERRUPTIBLE - at least @timeout jiffies are guaranteed to
 * pass before the routine returns. The routine will return 0
 *
 * %TASK_INTERRUPTIBLE - the routine may return early if a signal is
 * delivered to the current task. In this case the remaining time
 * in jiffies will be returned, or 0 if the timer expired in time
 *
 * The current task state is guaranteed to be TASK_RUNNING when this
 * routine returns.
 *
 * Specifying a @timeout value of %MAX_SCHEDULE_TIMEOUT will schedule
 * the CPU away without a bound on the timeout. In this case the return
 * value will be %MAX_SCHEDULE_TIMEOUT.
 *
 * In all cases the return value is guaranteed to be non-negative.
 */
fastcall signed long __sched schedule_timeout(signed long timeout)
{
	struct timer_list timer;
	unsigned long expire;

	switch (timeout)
	{
	case MAX_SCHEDULE_TIMEOUT:
		/*
		 * These two special cases are useful to be comfortable
		 * in the caller. Nothing more. We could take
		 * MAX_SCHEDULE_TIMEOUT from one of the negative value
		 * but I' d like to return a valid offset (>=0) to allow
		 * the caller to do everything it want with the retval.
		 */
		schedule();
		goto out;
	default:
		/*
		 * Another bit of PARANOID. Note that the retval will be
		 * 0 since no piece of kernel is supposed to do a check
		 * for a negative retval of schedule_timeout() (since it
		 * should never happens anyway). You just have the printk()
		 * that will tell you if something is gone wrong and where.
		 */
		if (timeout < 0) {
			printk(KERN_ERR "schedule_timeout: wrong timeout "
				"value %lx\n", timeout);
			dump_stack();
			current->state = TASK_RUNNING;
			goto out;
		}
	}

	expire = timeout + jiffies;

	setup_timer(&timer, process_timeout, (unsigned long)current);
	__mod_timer(&timer, expire);
	schedule();
	del_singleshot_timer_sync(&timer);

	timeout = expire - jiffies;

 out:
	return timeout < 0 ? 0 : timeout;
}
EXPORT_SYMBOL(schedule_timeout);

/*
 * We can use __set_current_state() here because schedule_timeout() calls
 * schedule() unconditionally.
 */
signed long __sched schedule_timeout_interruptible(signed long timeout)
{
	__set_current_state(TASK_INTERRUPTIBLE);
	return schedule_timeout(timeout);
}
EXPORT_SYMBOL(schedule_timeout_interruptible);

signed long __sched schedule_timeout_uninterruptible(signed long timeout)
{
	__set_current_state(TASK_UNINTERRUPTIBLE);
	return schedule_timeout(timeout);
}
EXPORT_SYMBOL(schedule_timeout_uninterruptible);

/* Thread ID - the internal kernel "pid" */
asmlinkage long sys_gettid(void)
{
	return task_pid_vnr(current);
}

/**
 * do_sysinfo - fill in sysinfo struct
 * @info: pointer to buffer to fill
 */
int do_sysinfo(struct sysinfo *info)
{
	unsigned long mem_total, sav_total;
	unsigned int mem_unit, bitcount;
	unsigned long seq;

	memset(info, 0, sizeof(struct sysinfo));

	do {
		struct timespec tp;
		seq = read_seqbegin(&xtime_lock);

		/*
		 * This is annoying.  The below is the same thing
		 * posix_get_clock_monotonic() does, but it wants to
		 * take the lock which we want to cover the loads stuff
		 * too.
		 */

		getnstimeofday(&tp);
		tp.tv_sec += wall_to_monotonic.tv_sec;
		tp.tv_nsec += wall_to_monotonic.tv_nsec;
		monotonic_to_bootbased(&tp);
		if (tp.tv_nsec - NSEC_PER_SEC >= 0) {
			tp.tv_nsec = tp.tv_nsec - NSEC_PER_SEC;
			tp.tv_sec++;
		}
		info->uptime = tp.tv_sec + (tp.tv_nsec ? 1 : 0);

		info->loads[0] = avenrun[0] << (SI_LOAD_SHIFT - FSHIFT);
		info->loads[1] = avenrun[1] << (SI_LOAD_SHIFT - FSHIFT);
		info->loads[2] = avenrun[2] << (SI_LOAD_SHIFT - FSHIFT);

		info->procs = nr_threads;
	} while (read_seqretry(&xtime_lock, seq));

	si_meminfo(info);
	si_swapinfo(info);

	/*
	 * If the sum of all the available memory (i.e. ram + swap)
	 * is less than can be stored in a 32 bit unsigned long then
	 * we can be binary compatible with 2.2.x kernels.  If not,
	 * well, in that case 2.2.x was broken anyways...
	 *
	 *  -Erik Andersen <andersee@debian.org>
	 */

	mem_total = info->totalram + info->totalswap;
	if (mem_total < info->totalram || mem_total < info->totalswap)
		goto out;
	bitcount = 0;
	mem_unit = info->mem_unit;
	while (mem_unit > 1) {
		bitcount++;
		mem_unit >>= 1;
		sav_total = mem_total;
		mem_total <<= 1;
		if (mem_total < sav_total)
			goto out;
	}

	/*
	 * If mem_total did not overflow, multiply all memory values by
	 * info->mem_unit and set it to 1.  This leaves things compatible
	 * with 2.2.x, and also retains compatibility with earlier 2.4.x
	 * kernels...
	 */

	info->mem_unit = 1;
	info->totalram <<= bitcount;
	info->freeram <<= bitcount;
	info->sharedram <<= bitcount;
	info->bufferram <<= bitcount;
	info->totalswap <<= bitcount;
	info->freeswap <<= bitcount;
	info->totalhigh <<= bitcount;
	info->freehigh <<= bitcount;

out:
	return 0;
}

asmlinkage long sys_sysinfo(struct sysinfo __user *info)
{
	struct sysinfo val;

	do_sysinfo(&val);

	if (copy_to_user(info, &val, sizeof(struct sysinfo)))
		return -EFAULT;

	return 0;
}

/*
 * lockdep: we want to track each per-CPU base as a separate lock-class,
 * but timer-bases are kmalloc()-ed, so we need to attach separate
 * keys to them:
 */
static struct lock_class_key base_lock_keys[NR_CPUS];

static int __cpuinit init_timers_cpu(int cpu)
{
	int j;
	tvec_base_t *base;
	static char __cpuinitdata tvec_base_done[NR_CPUS];

	if (!tvec_base_done[cpu]) {
		static char boot_done;

		if (boot_done) {
			/*
			 * The APs use this path later in boot
			 */
			base = kmalloc_node(sizeof(*base),
						GFP_KERNEL | __GFP_ZERO,
						cpu_to_node(cpu));
			if (!base)
				return -ENOMEM;

			/* Make sure that tvec_base is 2 byte aligned */
			if (tbase_get_deferrable(base)) {
				WARN_ON(1);
				kfree(base);
				return -ENOMEM;
			}
			per_cpu(tvec_bases, cpu) = base;
		} else {
			/*
			 * This is for the boot CPU - we use compile-time
			 * static initialisation because per-cpu memory isn't
			 * ready yet and because the memory allocators are not
			 * initialised either.
			 */
			boot_done = 1;
			base = &boot_tvec_bases;
		}
		tvec_base_done[cpu] = 1;
	} else {
		base = per_cpu(tvec_bases, cpu);
	}

	spin_lock_init(&base->lock);
	lockdep_set_class(&base->lock, base_lock_keys + cpu);

	for (j = 0; j < TVN_SIZE; j++) {
		INIT_LIST_HEAD(base->tv5.vec + j);
		INIT_LIST_HEAD(base->tv4.vec + j);
		INIT_LIST_HEAD(base->tv3.vec + j);
		INIT_LIST_HEAD(base->tv2.vec + j);
	}
	for (j = 0; j < TVR_SIZE; j++)
		INIT_LIST_HEAD(base->tv1.vec + j);

	base->timer_jiffies = jiffies;
	return 0;
}

#ifdef CONFIG_HOTPLUG_CPU
static void migrate_timer_list(tvec_base_t *new_base, struct list_head *head)
{
	struct timer_list *timer;

	while (!list_empty(head)) {
		timer = list_first_entry(head, struct timer_list, entry);
		detach_timer(timer, 0);
		timer_set_base(timer, new_base);
		internal_add_timer(new_base, timer);
	}
}

static void __cpuinit migrate_timers(int cpu)
{
	tvec_base_t *old_base;
	tvec_base_t *new_base;
	int i;

	BUG_ON(cpu_online(cpu));
	old_base = per_cpu(tvec_bases, cpu);
	new_base = get_cpu_var(tvec_bases);

	local_irq_disable();
	double_spin_lock(&new_base->lock, &old_base->lock,
			 smp_processor_id() < cpu);

	BUG_ON(old_base->running_timer);

	for (i = 0; i < TVR_SIZE; i++)
		migrate_timer_list(new_base, old_base->tv1.vec + i);
	for (i = 0; i < TVN_SIZE; i++) {
		migrate_timer_list(new_base, old_base->tv2.vec + i);
		migrate_timer_list(new_base, old_base->tv3.vec + i);
		migrate_timer_list(new_base, old_base->tv4.vec + i);
		migrate_timer_list(new_base, old_base->tv5.vec + i);
	}

	double_spin_unlock(&new_base->lock, &old_base->lock,
			   smp_processor_id() < cpu);
	local_irq_enable();
	put_cpu_var(tvec_bases);
}
#endif /* CONFIG_HOTPLUG_CPU */

static int __cpuinit timer_cpu_notify(struct notifier_block *self,
				unsigned long action, void *hcpu)
{
	long cpu = (long)hcpu;
	switch(action) {
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		if (init_timers_cpu(cpu) < 0)
			return NOTIFY_BAD;
		break;
#ifdef CONFIG_HOTPLUG_CPU
	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
		migrate_timers(cpu);
		break;
#endif
	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block __cpuinitdata timers_nb = {
	.notifier_call	= timer_cpu_notify,
};


void __init init_timers(void)
{
	int err = timer_cpu_notify(&timers_nb, (unsigned long)CPU_UP_PREPARE,
				(void *)(long)smp_processor_id());

	init_timer_stats();

	BUG_ON(err == NOTIFY_BAD);
	register_cpu_notifier(&timers_nb);
	open_softirq(TIMER_SOFTIRQ, run_timer_softirq, NULL);
}

/**
 * msleep - sleep safely even with waitqueue interruptions
 * @msecs: Time in milliseconds to sleep for
 */
void msleep(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs) + 1;

	while (timeout)
		timeout = schedule_timeout_uninterruptible(timeout);
}

EXPORT_SYMBOL(msleep);

/*睡眠等待信号*/
unsigned long msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs) + 1;

	while (timeout && !signal_pending(current))
		timeout = schedule_timeout_interruptible(timeout);
	return jiffies_to_msecs(timeout);
}

EXPORT_SYMBOL(msleep_interruptible);
