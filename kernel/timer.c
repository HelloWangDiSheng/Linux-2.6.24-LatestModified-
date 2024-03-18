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

/*�ں���Ҫ���ݽṹ������ϵͳ��ע������ж�ʱ������Щ��ʱ�����ܷ����ĳ�����̻��ں˱�
�����ýṹ����������ٶ���Ч�ؼ�鵽�ڵĶ�ʱ������������̫��CPUʱ�䡣�Ͼ���ÿ��ʱ��
�ж϶�������������ļ�顣
���ݽṹ�в��������������ʱ�������ȫ����Ϣ�������������ܹ������׵ؽ��������Ե�ɨ
�裬�Ա�ִ�е��ڵĶ�ʱ����ɾ������Ҫ����������ɨ�輴�����ں͸ոյ��ڵĶ�ʱ��������
Ϊֻ�ǽ�����timer_listʵ���򵥵ش�����һ���ǲ����ģ��ں˴����˲�ͬ���飬���ݶ�ʱ����
����ʱ����з��ࡣ����Ļ�����һ�������飬��5��������������顣�������5��λ�ø���
����ʱ��Զ�ʱ�����д��Եķ��ࡣ��һ���ǵ���ʱ����0��255����2^8-1����ʱ������֮�����
�ж�ʱ�����ڶ�������˵���ʱ����256��2^(8+6)-1 = 2^14-1��ʱ������֮������ж�ʱ����
�������ж�ʱ���ĵ���ʱ�䷶Χ�Ǵ�2^14��2^(8+2��6)-1��ʱ�����ڣ��������ơ������еĸ����Ϊ�飨group�� ����ʱ�ֳ�ΪͰ��bucket�� ����15-1�г��˸�����ʱ����
��ʱ��������������ͨϵͳ��Ͱ�Ĵ�С��Ϊ����Ļ��������ڴ���ٵ�С��ϵͳ�ϣ�ʱ������
����ͬ����ʱ����ʱ����
	�� 				ʱ����
	tv1 			0					2^8-1
	tv2 			2^8  				2^14-1
	tv3 			2^14  				2^20-1
	tv4 			2^20  				2^26-1
	tv5 			2^26  				2^32-1

ÿ���鱾����һ��������ɣ���ʱ���������ٴ����򡣵�һ�����������256�������ÿ��λ��
��ʾ0��255��ʱ������֮��һ�����ܵĵ���ʱ�䡣���ϵͳ���м�����ʱ���ĵ���ʱ����ͬ������
ͨ��һ����׼��˫������������������Ԫ��Ϊtimer_list��entry��Ա�����������Ҳ��������ɣ�
����������Ŀ���٣���64�����������������timer_list��˫������ÿ�������������
timer_list��expiresֵ����ֻ��һ��������һ��ʱ����������ĳ�����������صġ��Եڶ���
��˵��ÿ��������������ʱ����Ϊ256=2^8��ʱ�����ڣ����Ե�������˵��2^14��ʱ�����ڣ�
�Ե�������˵��2^20���Ե�������˵��2^26�������ǿ��Ƕ�ʱ�����������ʱ������ƶ�����ִ��
�Լ���ص����ݽṹ��θı��ʱ��������Щʱ����������ͺ�����ˡ���ʱ�������ִ�е�
�أ��ں���Ҫ�����ע��һ��Ķ�ʱ������Ϊ��Щ��ʱ���������Ժ��ڡ�Ϊ����������Ǽٶ�
ÿ�鶼��һ�����������洢��ĳ������λ�õı�ţ�ʵ�ʵ��ں�ʵ���ڹ������ǵ�Ч�ģ����ṹ��
�������̶�Ҫ��ö࣬�����Ժ�ῴ������
��һ���е�������ָ�������Ԫ�أ��������Ժ󼴽�ִ�еĸ���ʱ����timer_listʵ����ÿ������
һ��ʱ���ж�ʱ���ں˶�ɨ�������ִ�����ж�ʱ����������������λ�ü�1����ִ�й��Ķ�ʱ��
������ݽṹ�Ƴ�����һ�η���ʱ���ж�ʱ����ִ���µ�����λ���ϵĶ�ʱ��������������ݽṹ
�Ƴ���ͬ����������1���������ơ������������֮������ֵΪ255����Ϊ����ļӷ���ģ256��
������������ָ�����ʼλ�ã�λ��0������Ϊ��һ������������256��ʱ������֮��ͻ�ľ���
���뽫��������Ķ�ʱ������ǰ�ƣ����²����һ�顣�ڵ�һ�������λ�ûָ�����ʼλ��0֮��
�Ὣ�ڶ�����һ������������ж�ʱ�����䵽��һ�顣����������������Ϊʲô����ѡ���˲�ͬ��
ʱ��������Ϊ��һ��ĸ������������256����ͬ�ĵ���ʱ�䣬���ڶ�����һ������������ݾ���
������һ����������顣�õ���ͬ�������ں������顣�������һ�������������ͬ���������
�����ڶ��飬�������һ��������Ҳ����������������飬���������һ��������Ҳ�����������
�����顣�������������λ�ò������ѡ��ģ����е���������Ȼ���������á����������ֵ����
��ÿ��ʱ�����ڼ�1������ÿ256^(i-1)��ʱ�����ڼ�1������i����ı�š����Ǹ���һ����������
������������Ϊģʽ���ӵ�һ��Ĵ���ʼ�Ѿ�����256��jiffies����ʱ��������Ϊ0��ͬʱ���ڶ�
��ĵ�һ������������ݽ����䵽��һ���С����Ǽٶ��ڵ�һ����������ʱ��jiffiesϵͳ��ʱ����
ֵΪ10000���ڵڶ���ĵ�һ���������У���һ����ʱ����������ʱ���ֱ���ʱ������10001��
10015��10015��10254���ڡ���Щ��ʱ���ֱ�ᶨλ����һ���1��15��254����λ��15�ᴴ��һ����
����������ָ�룬��Ϊ��������ʱ��ͬʱ���ڡ��ڸ�����ɺ󣬽��ڶ��������λ�ü�1��ѭ����
�������¿�ʼ����ÿ��ʱ�����ڻ���һ�����һ��ĸ�������λ���ϵĶ�ʱ����ֱ����������λ��
255�����������õڶ���ĵڶ�������Ԫ���е����ж�ʱ�����������һ�顣�ڵڶ��������λ�õ�
��63ʱ���ӵڶ��鿪ʼ��ÿ��ֻ����64�����������ʹ�õ������һ�������������������ڶ�
�顣����ڵ����������λ�õ������ֵʱ���ӵ�����ȡ���µ����ݣ�ͬ����ԭ��Ҳ�����ڵ�
���鵽����������ݴ��䡣Ϊȷ����Щ��ʱ���Ѿ����ڣ��ں�����ɨ��һ���޴�Ķ�ʱ��������
��Χ�����ڵ�һ���е�һ���������Ϊ��λ��ͨ���ǿյĻ������һ����ʱ���������Ժܿ�
���С�ż���Ӻ����ĸ�����ǰ������������Ҳ����Ҫ����CPUʱ�䣬��Ϊ���ƿ���ͨ��ָ�������Ч
���У��ں����븴���ڴ�飬��ֻ�轫ָ������Ϊ��ֵ����ͬ��׼��������������
*/

/*����64λjiffies*/
u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
EXPORT_SYMBOL(jiffies_64);

/*per-CPU��ʱ����������*/
/*��̬��ʱ����һ��������λƫ�ƣ����������Ӧ��������Ŀ*/
#define TVN_BITS (CONFIG_BASE_SMALL ? 4 : 6)
/*��̬��ʱ����һ��λƫ�ƣ����������һ����������Ŀ*/
#define TVR_BITS (CONFIG_BASE_SMALL ? 6 : 8)
/*��һ���Ժ������������Ŀ*/
#define TVN_SIZE (1 << TVN_BITS)
/*��һ����������Ŀ*/
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)
/*tvec_root_t��Ӧ��һ�飬��tvec_t��ʾ�������顣�����ṹ�Ĳ�ֻͬ����������ĸ������Ե�
һ�飬TVR_SIZE����Ϊ256������������ʹ�õ����鳤��ΪTVN_SIZE��Ĭ��ֵ64��ȱ���ڴ��ϵͳ��
��������ѡ��BASE_SMALL������������£���һ����64������������������������Ϊ16��*/
typedef struct tvec_root_s
{
	struct list_head vec[TVR_SIZE];
} tvec_root_t;

typedef struct tvec_s
{
	struct list_head vec[TVN_SIZE];
} tvec_t;

/*ϵͳ�е�ÿ��������������������ݽṹ�����������������ϵĶ�ʱ�����������ݽṹ��һ����
CPUʵ����������������*/
struct tvec_t_base_s
{
	spinlock_t lock;
	struct timer_list *running_timer;
	/*��¼��һ��ʱ��㣨��λΪjiffies�����ýṹ�д�ǰ���ڵĶ�ʱ�����Ѿ�ִ�С����磬��
���ñ�����ֵΪ10 500����ô�ں˾�֪����jiffiesֵ10 499��֮ǰ���ڵĶ�ʱ�����Ѿ�ִ�й��ˡ�
ͨ����timer_jiffies����jiffies���jiffiesС1������ں���һ��ʱ���޷�ִ�ж�ʱ����ϵͳ��
�ɷǳ��ߣ������ߵĲ�ֵ���ܻ��Դ�һ��*/
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

/*��ʼ��һ����ʱ������ʱ�������ڵ���������ʱ������֮ǰ��ʼ��*/
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
/*������N������ֵ*/
#define INDEX(N) ((base->timer_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)

/**
 * __run_timers - run all expired timers (if any) on this CPU.
 * @base: the timer vector to be processed.
 *
 * This function cascades all vectors and executes all expired timer
 * vectors.
 */
/*�����ж�ʱ���Ĵ�����update_process_timesby�����������run_local_timers������
�ú�����ʹ��raise_softirq(TIMER_SOFTIRQ)�����ʱ���������жϣ�����һ�����ܵ�ʱ��ִ
�С�run_timer_softirq���������жϵĴ��������������ѡ���ض���CPU��struct
tvec_t_base_sʵ����������__run_timers��__run_timersʵ���������������㷨��������������
���������ݽṹ�У���û�з����㷨�������ᵽ������λ�ã��ں˲�����Ҫһ����ʽ�ı�������¼
����Ϣ�����б�Ҫ����Ϣ���Ѿ�������base��timer_jiffies��Ա�Тڡ�Ϊ�˶��������к꣺ */
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
 /*��ȡTASK_RUNNING״̬�����񣨽��̺��̣߳���Ŀ*/
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
 Linux��ϵͳ����ָ���ж��е�ƽ�����ȣ�Ҳ���ǵȴ�CPU��ƽ������������ΪLinux�ڽ�ֹ����
 ���㣬���ϵͳ�ĸ���ֻ��ͨ������仯�Ĵ�����һ�� ��ֵ�����㡣Linux�ں˶���һ������Ϊ
 3��˫������avenrun��˫�ֵĵ�11λ���ڴ�Ÿ��ص�С�����֣���21λ���ڴ���������֡�����
 �����ĵ� CPUʱ��Ƭ������CPU��5�����ܹ��ṩ��ʱ��Ƭ��ʱ���ں˼����������������ء�����
 ��ʼ��Ϊ0���������1��5��15�����ڵ�ƽ�����طֱ�Ϊ load1��load5��load15����ô��һ����
 ��ʱ�̵���ʱ���ں�ͨ���������ʽ���㸺�أ�
 load1 -= load1 -* exp(-5 / 60) -+ n * (1 - exp(-5 / 60 ))
 load5 -= load5 -* exp(-5 / 300) + n * (1 - exp(-5 / 300))
 load15 = load15 * exp(-5 / 900) + n * (1 - exp(-5 / 900))
 ���У�exp(x)Ϊe��x���ݣ�nΪ��ǰ���ж��еĳ��ȡ�Linux�ں���Ϊ���̵�����ʱ����Ӳ���Ϊ
 1��ָ���ֲ���ָ���ֲ��ĸ����ܶ�Ϊ�����ں˼��� ����load1Ϊ������������������ʱ��֮��ϵ
 ͳ��Ľ��̼���ΪS0����1����ǰ����ǰ����ʱ�����ʱ��������load1�����̣������ǵ�
 ������ S1���ں���Ϊ�ĸ����ܶ���:��e-��x�����ڵ�ǰʱ�̻��n�����̣������ǵļ�����Sn��
 ����Ϊ�ĸ����ܶ���1-��e-��x������x = 5 / 60����Ϊ������������ʱ��֮��������ĵ�CPUʱ��
 Ϊ5�룬�����ǵ�ʱ�����1����(60��)����ô����������1����ϵͳ���ж��еĳ��ȣ�
 load1 = |S1| -* ��e-��x + |Sn| * (1-��e-��x) = load1 * ��e-��x + n * (1-��e-��x)
 ���Ц� = 1�� x = 5 / 60�� |S1|��|Sn|�Ǽ���Ԫ�صĸ����������Linux�ں�Դ�ļ�shed.c�ĺ�
 ��calc_load()���㸺�ص���ѧ���ݡ�
 ���ԡ�Loadֵ=CPU�������������������״̬��û���κξ�����һ���������һ���ˡ�
 ����������ÿ��5���Ӽ��һ�λ�Ծ�Ľ�������Ȼ����������ֵ������ġ�������������CPU
 �ĺ������������5��ʱ��ͱ���ϵͳ�ڳ�������ת��
 */
/*����tick�������¹��Ƶ�1/5/15���ӵ�ƽ�����ж��г��ȣ���xtime_lock����
һ��write_lockʱ���øú���*/
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
 /*��ȡ��ǰ���̵���ʵ������*/
asmlinkage long sys_getppid(void)
{
	int pid;

	rcu_read_lock();
	pid = task_tgid_nr_ns(current->real_parent, current->nsproxy->pid_ns);
	rcu_read_unlock();

	return pid;
}

/*��ȡ��ǰ���̵�uid*/
asmlinkage long sys_getuid(void)
{
	/* Only we change this so SMP safe */
	return current->uid;
}

/*��õ�ǰ���̵�ueid*/
asmlinkage long sys_geteuid(void)
{
	/* Only we change this so SMP safe */
	return current->euid;
}
/*��ȡ��ǰ���̵�gid*/
asmlinkage long sys_getgid(void)
{
	/* Only we change this so SMP safe */
	return current->gid;
}
/*��ȡ��ǰ���̵�egid*/
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

/*˯�ߵȴ��ź�*/
unsigned long msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs) + 1;

	while (timeout && !signal_pending(current))
		timeout = schedule_timeout_interruptible(timeout);
	return jiffies_to_msecs(timeout);
}

EXPORT_SYMBOL(msleep_interruptible);
