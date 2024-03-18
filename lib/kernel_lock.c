 /*��ͳ�Ĵ��ں��������������ڷǹؼ��������ϵͳ*/
#include <linux/smp_lock.h>
#include <linux/module.h>
#include <linux/kallsyms.h>

/*�����ں˿���ռ*/
#ifdef CONFIG_PREEMPT_BKL
 /*���ں����ź��������ź������Ա�lock_kernel������unlock_kernel���������ݹ��������ͷţ�
�����ź��������Ĵ���ֻ����ͬһ�����»��⴮��ִ�У����뱣֤������Ȼ��ͬһ��cpu�����У���
Ҫ���´�����ʹ�ô��ں���*/
static DECLARE_MUTEX(kernel_sem);

/*
 * Re-acquire the kernel semaphore.
 *
 * This function is called with preemption off.
 *
 * We are executing in schedule() so the code must be extremely careful
 * about recursion, both due to the down() and due to the enabling of
 * preemption. schedule() will re-check the preemption flag after
 * reacquiring the semaphore.
 */
 /*����������ں����ź���������ʱҪ�ر���ռ
*/
int __lockfunc __reacquire_kernel_lock(void)
{
	/*��ȡ��ǰ����*/
	struct task_struct *task = current;
	/*���浱ǰ���̵������*/
	int saved_lock_depth = task->lock_depth;
	/**/
	BUG_ON(saved_lock_depth < 0);

	task->lock_depth = -1;
	preempt_enable_no_resched();
	/*������ں����ź���*/
	down(&kernel_sem);

	preempt_disable();
	task->lock_depth = saved_lock_depth;

	return 0;
}
/*�ͷŴ��ں������ͷŶ�Ӧ��ȫ���ź���*/
void __lockfunc __release_kernel_lock(void)
{
	up(&kernel_sem);
}

/*
 * Getting the big kernel semaphore.
 */
/*��ô��ں����ź����������ں�*/
void __lockfunc lock_kernel(void)
{
	/*��õ�ǰ����*/
	struct task_struct *task = current;
	/*��ǰ���̼���*/
	int depth = task->lock_depth + 1;
	/*��ǰ����ʱ��һ������������*/
	if (likely(!depth))
		/*
		 * No recursion worries - we set up lock_depth _after_
		 */
		 /*������ں����ź���*/
		down(&kernel_sem);
	/*���õ�ǰ������������*/
	task->lock_depth = depth;
}

/*�������ں���*/
void __lockfunc unlock_kernel(void)
{
	/*��ȡ��ǰ����*/
	struct task_struct *task = current;
	/*��ǰ����Ŀǰû�г��д��ں��� BUG*/
	BUG_ON(task->lock_depth < 0);
	/*�������ں��������û�г��д��ں��������ͷŴ��ں����ź���*/
	if (likely(--task->lock_depth < 0))
		up(&kernel_sem);
}

#else

/*
 * The 'big kernel lock'
 *
 * This spinlock is taken and released recursively by lock_kernel()
 * and unlock_kernel().  It is transparently dropped and reacquired
 * over schedule().  It is used to protect legacy code that hasn't
 * been migrated to a proper locking design yet.
 *
 * Don't use in new code.
 */
 /*������ں�����������������L1�����ж��룩*/
static  __cacheline_aligned_in_smp DEFINE_SPINLOCK(kernel_flag);


/*
 * Acquire/release the underlying lock from the scheduler.
 *
 * This is called with preemption disabled, and should
 * return an error value if it cannot get the lock and
 * TIF_NEED_RESCHED gets set.
 *
 * If it successfully gets the lock, it should increment
 * the preemption count like any spinlock does.
 *
 * (This works on UP too - _raw_spin_trylock will never
 * return false in that case)
 */
int __lockfunc __reacquire_kernel_lock(void)
{
	while (!_raw_spin_trylock(&kernel_flag)) {
		if (test_thread_flag(TIF_NEED_RESCHED))
			return -EAGAIN;
		cpu_relax();
	}
	preempt_disable();
	return 0;
}

void __lockfunc __release_kernel_lock(void)
{
	_raw_spin_unlock(&kernel_flag);
	preempt_enable_no_resched();
}

/*
 * These are the BKL spinlocks - we try to be polite about preemption. 
 * If SMP is not on (ie UP preemption), this all goes away because the
 * _raw_spin_trylock() will always succeed.
 */
#ifdef CONFIG_PREEMPT
static inline void __lock_kernel(void)
{
	preempt_disable();
	if (unlikely(!_raw_spin_trylock(&kernel_flag))) {
		/*
		 * If preemption was disabled even before this
		 * was called, there's nothing we can be polite
		 * about - just spin.
		 */
		if (preempt_count() > 1) {
			_raw_spin_lock(&kernel_flag);
			return;
		}

		/*
		 * Otherwise, let's wait for the kernel lock
		 * with preemption enabled..
		 */
		do {
			preempt_enable();
			while (spin_is_locked(&kernel_flag))
				cpu_relax();
			preempt_disable();
		} while (!_raw_spin_trylock(&kernel_flag));
	}
}

#else

/*
 * Non-preemption case - just get the spinlock
 */
static inline void __lock_kernel(void)
{
	_raw_spin_lock(&kernel_flag);
}
#endif

static inline void __unlock_kernel(void)
{
	/*
	 * the BKL is not covered by lockdep, so we open-code the
	 * unlocking sequence (and thus avoid the dep-chain ops):
	 */
	_raw_spin_unlock(&kernel_flag);
	preempt_enable();
}

/*
 * Getting the big kernel lock.
 *
 * This cannot happen asynchronously, so we only need to
 * worry about other CPU's.
 */
void __lockfunc lock_kernel(void)
{
	int depth = current->lock_depth+1;
	if (likely(!depth))
		__lock_kernel();
	current->lock_depth = depth;
}
/*������ǰ����*/
void __lockfunc unlock_kernel(void)
{
	BUG_ON(current->lock_depth < 0);
	if (likely(--current->lock_depth < 0))
		__unlock_kernel();
}

#endif

EXPORT_SYMBOL(lock_kernel);
EXPORT_SYMBOL(unlock_kernel);
