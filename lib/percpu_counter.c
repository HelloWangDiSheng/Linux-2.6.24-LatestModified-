/*
 * Fast batching percpu counters.
 */

#include <linux/percpu_counter.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/module.h>

/*����CPU�Ȳ��ʱ��Ҫ�ڻ�����percpu_counters_lock�ı���������*/
#ifdef CONFIG_HOTPLUG_CPU
static LIST_HEAD(percpu_counters);
static DEFINE_MUTEX(percpu_counters_lock);
#endif

/*����û�б��ں˹����cpu��Ӧ��percpu_counter�н���ֵ���㣬ʵ��ֵ����Ϊamount*/
void percpu_counter_set(struct percpu_counter *fbc, s64 amount)
{
	int cpu;
	/*����percpu_counter������*/
	spin_lock(&fbc->lock);
	/*�������л�δ���ں˹����cpu*/
	for_each_possible_cpu(cpu)
	{
		/*��percpu_counter�б����cpu��Ӧ��counter����������Ϊ0*/
		s32 *pcount = per_cpu_ptr(fbc->counters, cpu);
		*pcount = 0;
	}
	/*����percpu_count��ʵ��ֵΪamount*/
	fbc->count = amount;
	spin_unlock(&fbc->lock);
}
EXPORT_SYMBOL(percpu_counter_set);

/*��ȡ��ǰcpu��Ӧ��percpu_counters�����еĽ���ֵ��Ȼ����amount���ܣ�����ֵ�����
[-batch, batch]��Χ�ڣ��������ֵ��ֵΪ����ֵ�����򣬽�ʵ��ֵ�ۼƼ���ֵ��������ֵ��0*/
void __percpu_counter_add(struct percpu_counter *fbc, s64 amount, s32 batch)
{
	s64 count;
	s32 *pcount;
	
	/*ͣ����ռ������ȡ��ǰcpu*/
	int cpu = get_cpu();
	/*��ȡ��Ӧcpu��percpu_counter����ֵָ��*/
	pcount = per_cpu_ptr(fbc->counters, cpu);
	/*�����cpu��ŵĽ���ֵ��amount�ĺ�*/
	count = *pcount + amount;
	if (count >= batch || count <= -batch)
	{
		/*������ܺ��ֵ����[-batch, batch]�ڣ��򽫸�ֵ���ܵ�ʵ��ֵ���������Ӧ��
	percpu_counters�Ľ���ֵ����*/
		spin_lock(&fbc->lock);
		fbc->count += count;
		*pcount = 0;
		spin_unlock(&fbc->lock);
	}
	else
	{
		/*������ܺ��ʵ��ֵ��[-batch, batch]�����У��򽫸�cpu��Ŷ�Ӧ�Ľ���ֵ����
		Ϊ����ֵ*/
		*pcount = count;
	}
	/*������ռ*/
	put_cpu();
}
EXPORT_SYMBOL(__percpu_counter_add);

/*
 * Add up all the per-cpu counts, return the result.  This is a more accurate
 * but much slower version of percpu_counter_read_positive()
 */
/*����percpu_counters�е�ʵ��ֵ���ۼƵ����н���ֵ֮��*/
s64 __percpu_counter_sum(struct percpu_counter *fbc)
{
	s64 ret;
	int cpu;

	spin_lock(&fbc->lock);
	/*��ȡʵ��ֵ*/
	ret = fbc->count;
	/*��ȡʵ��ֵ���ۼ����н���ֵ�ĺ�*/
	for_each_online_cpu(cpu) {
		s32 *pcount = per_cpu_ptr(fbc->counters, cpu);
		ret += *pcount;
	}
	spin_unlock(&fbc->lock);
	return ret;
}
EXPORT_SYMBOL(__percpu_counter_sum);

static struct lock_class_key percpu_counter_irqsafe;

/*percpu_counter��ʼ������ʵ��ֵ��ֵΪamount��Ϊ��δ�����ں˹����cpu����
percpu_data->ptrs[cpu]������ɹ�����0��ʧ���򷵻��ڴ治��*/
int percpu_counter_init(struct percpu_counter *fbc, s64 amount)
{
	spin_lock_init(&fbc->lock);
	/*��percpu_counter��ʵ��ֵ��ֵΪamount*/
	fbc->count = amount;
	/*Ϊpercpu_counter���������������ں˹����cpu�Ľ���ֵ����ռ�*/
	fbc->counters = alloc_percpu(s32);
	if (!fbc->counters)
		return -ENOMEM;
#ifdef CONFIG_HOTPLUG_CPU
	/*�������cpu�Ȳ�ι��ܣ�����percpu_counters_lock�������ı����£�������
	percpu_counters������percpu_counters˫������*/
	mutex_lock(&percpu_counters_lock);
	list_add(&fbc->list, &percpu_counters);
	mutex_unlock(&percpu_counters_lock);
#endif
	return 0;
}
EXPORT_SYMBOL(percpu_counter_init);

/*���ñ���cpu�ж϶�percpu_counter��ʼ��*/
int percpu_counter_init_irq(struct percpu_counter *fbc, s64 amount)
{
	int err;

	err = percpu_counter_init(fbc, amount);
	if (!err)
		lockdep_set_class(&fbc->lock, &percpu_counter_irqsafe);
	return err;
}

/*�ͷ�percpu_counter�н���ֵ������ռ�ռ�*/
void percpu_counter_destroy(struct percpu_counter *fbc)
{
	/*percpu_counter�����б������ֵ��ָ������Ϊ�գ�Ҳ����û�и�����ֵ����ռ䣬
	ֱ�ӷ���*/
	if (!fbc->counters)
		return;
	/*�ͷŻ�δ���ں���������cpu��Ӧ�Ľ���ֵ��ռ�ռ�*/
	free_percpu(fbc->counters);
	/*���cpu�����Ȳ�ι��ܣ������Ӧ��percpu_counter����ɾ��*/
#ifdef CONFIG_HOTPLUG_CPU
	mutex_lock(&percpu_counters_lock);
	list_del(&fbc->list);
	mutex_unlock(&percpu_counters_lock);
#endif
}
EXPORT_SYMBOL(percpu_counter_destroy);

/*����cpu�Ȳ��ʱ�����ڼ�*/
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
