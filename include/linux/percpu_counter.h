#ifndef _LINUX_PERCPU_COUNTER_H
#define _LINUX_PERCPU_COUNTER_H

/*ext2和ext3超级块使用的简单的大约数计数器。警告：these things are HUGE. 
4 kbytes per counter on 32-way P4*/

#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/list.h>
#include <linux/threads.h>
#include <linux/percpu.h>
#include <linux/types.h>

#ifdef CONFIG_SMP
/*如果系统安装有大量cpu，计数器可能成为瓶颈：每次只有一个cpu可以修改其值；所有其它cpu
都必须等待操作结束，才能再次访问计数器，如果计数器频繁访问，则会严重影响系统性能，对
某些计数器，没有必要实时了解其准确的数值。这种计数器的近似值与准确值，作用上没什么差别
可以利用这种情况，引入所谓的per_CPU计数器，来加速SMP系统上个计数器的操作。基本思想是：
计数器的准确值存储在内存中某处，准确值所在内存位置之后是一个数组，每个数组项对应于系统
中的一个cpu。如果某个处理器想要修改计数器的值，它不会直接修改计数器的值，因为这需要防止
其它的访问cpu计数器（这是一个费时的操作），相反，所需的修改将保存到与计数器相关的数组中
特定与当前cpu的数组项，如果某个特定于cpu的数组元素修改后的绝对值超出了某个阈值，则认为
这种修改有问题，将随之修改计数器的值，在这种情况下，内核需要确保通过释放的锁机制来保护这
次访问。由于这种改变很少发生，因此操作的代价将不那么重要。只要计数器改变适度，这种方案中
读操作得到的平均值会相当接近于计数器的准确值*/
struct percpu_counter
{
	/*lock自旋锁用于在需要准确值时保护计数器*/
	spinlock_t lock;
	/*count是计数器的准确值*/
	s64 count;
#ifdef CONFIG_HOTPLUG_CPU
	/*配置cpu热插拔时将percpu_counters放在一个链表上*/
	struct list_head list;	/* All percpu_counters are on a list */
#endif
	/*数组中个数组项是特定于cpu的，该数组缓存了对计数器的操作*/
	s32 *counters;
};

/*触发计数器修改值的阈值范围[-FBC_BATCH, FBC_BATCH]依赖于系统中cpu的数目*/
#if NR_CPUS >= 16
#define FBC_BATCH		(NR_CPUS*2)
#else
#define FBC_BATCH		(NR_CPUS*4)
#endif

int percpu_counter_init(struct percpu_counter *fbc, s64 amount);
int percpu_counter_init_irq(struct percpu_counter *fbc, s64 amount);
void percpu_counter_destroy(struct percpu_counter *fbc);
void percpu_counter_set(struct percpu_counter *fbc, s64 amount);
void __percpu_counter_add(struct percpu_counter *fbc, s64 amount, s32 batch);
s64 __percpu_counter_sum(struct percpu_counter *fbc);

/*将percpu_counter变量fbc中当前cpu对应的近似值添加amount*/
static inline void percpu_counter_add(struct percpu_counter *fbc, s64 amount)
{
	__percpu_counter_add(fbc, amount, FBC_BATCH);
}

/*计算并返回percpu_counter变量fbc中累计的所有近似值与实际值之和，结果为负则返回0*/
static inline s64 percpu_counter_sum_positive(struct percpu_counter *fbc)
{
	s64 ret = __percpu_counter_sum(fbc);
	return ret < 0 ? 0 : ret;
}

/*获取percpu_counter变量fbc中实际值和累计所有近似值之和*/
static inline s64 percpu_counter_sum(struct percpu_counter *fbc)
{
	return __percpu_counter_sum(fbc);
}

/*读取percpu_counter变量fbc的实际值*/
static inline s64 percpu_counter_read(struct percpu_counter *fbc)
{
	return fbc->count;
}

/*获取percpu_counter变量fbc的实际值，实际值为负时返回1。percpu_counter_read函数
可能因为有些不应该为负数的近似值而返回一个小的负数*/
static inline s64 percpu_counter_read_positive(struct percpu_counter *fbc)
{
	/*获取fbc实际值*/
	s64 ret = fbc->count;
	/*禁止重载fbc的实际值*/
	barrier();	
	if (ret >= 0)
		return ret;
	return 1;
}

#else

struct percpu_counter {
	s64 count;
};

static inline int percpu_counter_init(struct percpu_counter *fbc, s64 amount)
{
	fbc->count = amount;
	return 0;
}

#define percpu_counter_init_irq percpu_counter_init

static inline void percpu_counter_destroy(struct percpu_counter *fbc)
{
}

static inline void percpu_counter_set(struct percpu_counter *fbc, s64 amount)
{
	fbc->count = amount;
}

#define __percpu_counter_add(fbc, amount, batch) \
	percpu_counter_add(fbc, amount)

static inline void
percpu_counter_add(struct percpu_counter *fbc, s64 amount)
{
	preempt_disable();
	fbc->count += amount;
	preempt_enable();
}

static inline s64 percpu_counter_read(struct percpu_counter *fbc)
{
	return fbc->count;
}

static inline s64 percpu_counter_read_positive(struct percpu_counter *fbc)
{
	return fbc->count;
}

static inline s64 percpu_counter_sum_positive(struct percpu_counter *fbc)
{
	return percpu_counter_read_positive(fbc);
}

static inline s64 percpu_counter_sum(struct percpu_counter *fbc)
{
	return percpu_counter_read(fbc);
}

#endif	/* CONFIG_SMP */

/*将percpu_counter变量fbc中当前cpu对应的近似值加1*/
static inline void percpu_counter_inc(struct percpu_counter *fbc)
{
	percpu_counter_add(fbc, 1);
}

/*将percpu_counter变量fbc中当前cpu对应的近似值减1*/
static inline void percpu_counter_dec(struct percpu_counter *fbc)
{
	percpu_counter_add(fbc, -1);
}

/*将percpu_counter变量fbc中当前cpu对应的近似值减amount*/
static inline void percpu_counter_sub(struct percpu_counter *fbc, s64 amount)
{
	percpu_counter_add(fbc, -amount);
}

#endif /* _LINUX_PERCPU_COUNTER_H */
