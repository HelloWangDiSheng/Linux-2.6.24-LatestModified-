/*
 * Fast batching percpu counters.
 */

#include <linux/percpu_counter.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/module.h>

/*配置CPU热插拔时需要在互斥量percpu_counters_lock的保护，创建*/
#ifdef CONFIG_HOTPLUG_CPU
static LIST_HEAD(percpu_counters);
static DEFINE_MUTEX(percpu_counters_lock);
#endif

/*将还没有被内核管理的cpu对应的percpu_counter中近似值清零，实际值设置为amount*/
void percpu_counter_set(struct percpu_counter *fbc, s64 amount)
{
	int cpu;
	/*申请percpu_counter保护锁*/
	spin_lock(&fbc->lock);
	/*遍历所有还未被内核管理的cpu*/
	for_each_possible_cpu(cpu)
	{
		/*将percpu_counter中保存该cpu对应的counter数组项设置为0*/
		s32 *pcount = per_cpu_ptr(fbc->counters, cpu);
		*pcount = 0;
	}
	/*设置percpu_count的实际值为amount*/
	fbc->count = amount;
	spin_unlock(&fbc->lock);
}
EXPORT_SYMBOL(percpu_counter_set);

/*获取当前cpu对应的percpu_counters变量中的近似值，然后与amount加总，加总值如果在
[-batch, batch]范围内，则将其近视值赋值为加总值，否则，将实际值累计加总值并将近似值清0*/
void __percpu_counter_add(struct percpu_counter *fbc, s64 amount, s32 batch)
{
	s64 count;
	s32 *pcount;
	
	/*停用抢占，并获取当前cpu*/
	int cpu = get_cpu();
	/*获取对应cpu的percpu_counter近似值指针*/
	pcount = per_cpu_ptr(fbc->counters, cpu);
	/*计算该cpu编号的近似值和amount的和*/
	count = *pcount + amount;
	if (count >= batch || count <= -batch)
	{
		/*如果加总后的值不在[-batch, batch]在，则将该值加总到实际值，并将其对应的
	percpu_counters的近似值清零*/
		spin_lock(&fbc->lock);
		fbc->count += count;
		*pcount = 0;
		spin_unlock(&fbc->lock);
	}
	else
	{
		/*如果加总后的实际值在[-batch, batch]区间中，则将该cpu编号对应的近似值设置
		为加总值*/
		*pcount = count;
	}
	/*启用抢占*/
	put_cpu();
}
EXPORT_SYMBOL(__percpu_counter_add);

/*
 * Add up all the per-cpu counts, return the result.  This is a more accurate
 * but much slower version of percpu_counter_read_positive()
 */
/*计算percpu_counters中的实际值与累计的所有近似值之和*/
s64 __percpu_counter_sum(struct percpu_counter *fbc)
{
	s64 ret;
	int cpu;

	spin_lock(&fbc->lock);
	/*获取实际值*/
	ret = fbc->count;
	/*获取实际值与累计所有近似值的和*/
	for_each_online_cpu(cpu) {
		s32 *pcount = per_cpu_ptr(fbc->counters, cpu);
		ret += *pcount;
	}
	spin_unlock(&fbc->lock);
	return ret;
}
EXPORT_SYMBOL(__percpu_counter_sum);

static struct lock_class_key percpu_counter_irqsafe;

/*percpu_counter初始化，将实际值赋值为amount，为还未纳入内核管理的cpu分配
percpu_data->ptrs[cpu]，分配成功返回0，失败则返回内存不足*/
int percpu_counter_init(struct percpu_counter *fbc, s64 amount)
{
	spin_lock_init(&fbc->lock);
	/*将percpu_counter的实际值赋值为amount*/
	fbc->count = amount;
	/*为percpu_counter中所有内有纳入内核管理的cpu的近似值分配空间*/
	fbc->counters = alloc_percpu(s32);
	if (!fbc->counters)
		return -ENOMEM;
#ifdef CONFIG_HOTPLUG_CPU
	/*如果启用cpu热插拔功能，则在percpu_counters_lock自旋锁的保护下，将所有
	percpu_counters保存在percpu_counters双链表上*/
	mutex_lock(&percpu_counters_lock);
	list_add(&fbc->list, &percpu_counters);
	mutex_unlock(&percpu_counters_lock);
#endif
	return 0;
}
EXPORT_SYMBOL(percpu_counter_init);

/*禁用本地cpu中断对percpu_counter初始化*/
int percpu_counter_init_irq(struct percpu_counter *fbc, s64 amount)
{
	int err;

	err = percpu_counter_init(fbc, amount);
	if (!err)
		lockdep_set_class(&fbc->lock, &percpu_counter_irqsafe);
	return err;
}

/*释放percpu_counter中近似值数组所占空间*/
void percpu_counter_destroy(struct percpu_counter *fbc)
{
	/*percpu_counter变量中保存近似值的指针数组为空，也就是没有给近似值分配空间，
	直接返回*/
	if (!fbc->counters)
		return;
	/*释放还未被内核纳入管理的cpu对应的近似值所占空间*/
	free_percpu(fbc->counters);
	/*如果cpu启用热插拔功能，则将其对应的percpu_counter链表删除*/
#ifdef CONFIG_HOTPLUG_CPU
	mutex_lock(&percpu_counters_lock);
	list_del(&fbc->list);
	mutex_unlock(&percpu_counters_lock);
#endif
}
EXPORT_SYMBOL(percpu_counter_destroy);

/*启用cpu热插拔时启动期间*/
#ifdef CONFIG_HOTPLUG_CPU
static int __cpuinit percpu_counter_hotcpu_callback(struct notifier_block *nb,
					unsigned long action, void *hcpu)
{
	unsigned int cpu;
	struct percpu_counter *fbc;

	if (action != CPU_DEAD)
		return NOTIFY_OK;

	cpu = (unsigned long)hcpu;
	mutex_lock(&percpu_counters_lock);
	list_for_each_entry(fbc, &percpu_counters, list)
	{
		s32 *pcount;
		unsigned long flags;

		spin_lock_irqsave(&fbc->lock, flags);
		pcount = per_cpu_ptr(fbc->counters, cpu);
		fbc->count += *pcount;
		*pcount = 0;
		spin_unlock_irqrestore(&fbc->lock, flags);
	}
	mutex_unlock(&percpu_counters_lock);
	return NOTIFY_OK;
}

static int __init percpu_counter_startup(void)
{
	hotcpu_notifier(percpu_counter_hotcpu_callback, 0);
	return 0;
}
module_init(percpu_counter_startup);
#endif
