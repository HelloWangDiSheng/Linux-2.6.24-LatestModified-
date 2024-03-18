#ifndef __LINUX_RT_MUTEX_H
#define __LINUX_RT_MUTEX_H

#include <linux/linkage.h>
#include <linux/plist.h>
#include <linux/spinlock_types.h>

/*实时互斥量的定义非常接近于普通互斥量，与普通互斥量相比，决定性的改变时等待列表
中的进程按优先级排序，在等待列表改变时，内核可相应地矫正锁持有者的优先级，这需要
调度器的一个接口，可由rt_mutex_setprio提供，该函数更新进程动态优先级，而普通优先
级不变，此外，内核提供了几个标准函数rt_mutex_init、rt_mutex_lock、rt_mutex_unlock
、rt_mutex_trylock，工作方式与普通互斥量完全相同*/
struct rt_mutex
{
	/*实际保护该结构的自旋锁*/
	spinlock_t		wait_lock;
	/*所有等待的进程按优先级都在wait_list中排队*/
	struct plist_head	wait_list;
	/*互斥量所有者*/
	struct task_struct	*owner;
#ifdef CONFIG_DEBUG_RT_MUTEXES
	int			save_state;
	const char 		*name, *file;
	int			line;
	void			*magic;
#endif
};

struct rt_mutex_waiter;
struct hrtimer_sleeper;

#ifdef CONFIG_DEBUG_RT_MUTEXES
 extern int rt_mutex_debug_check_no_locks_freed(const void *from,
						unsigned long len);
 extern void rt_mutex_debug_check_no_locks_held(struct task_struct *task);
#else
 static inline int rt_mutex_debug_check_no_locks_freed(const void *from,
						       unsigned long len)
 {
	return 0;
 }
#define rt_mutex_debug_check_no_locks_held(task)	do { } while (0)
#endif

#ifdef CONFIG_DEBUG_RT_MUTEXES
#define __DEBUG_RT_MUTEX_INITIALIZER(mutexname) \
	, .name = #mutexname, .file = __FILE__, .line = __LINE__
#define rt_mutex_init(mutex)			__rt_mutex_init(mutex, __FUNCTION__)
 extern void rt_mutex_debug_task_free(struct task_struct *tsk);
#else
#define __DEBUG_RT_MUTEX_INITIALIZER(mutexname)
#define rt_mutex_init(mutex)			__rt_mutex_init(mutex, NULL)
#define rt_mutex_debug_task_free(t)			do { } while (0)
#endif

/*实时互斥量mutex初始化*/
#define __RT_MUTEX_INITIALIZER(mutexname) \
	{ .wait_lock = __SPIN_LOCK_UNLOCKED(mutexname.wait_lock) \
	, .wait_list = PLIST_HEAD_INIT(mutexname.wait_list, mutexname.wait_lock) \
	, .owner = NULL \
	__DEBUG_RT_MUTEX_INITIALIZER(mutexname)}

/*定义并初始化一个实时互斥量mutexname*/
#define DEFINE_RT_MUTEX(mutexname) \
	struct rt_mutex mutexname = __RT_MUTEX_INITIALIZER(mutexname)


/*互斥量是否已被锁定，也即当前互斥量的所有者是否存在，存在即锁定，返回1，否则返回0*/
static inline int rt_mutex_is_locked(struct rt_mutex *lock)
{
	return lock->owner != NULL;
}

extern void __rt_mutex_init(struct rt_mutex *lock, const char *name);
extern void rt_mutex_destroy(struct rt_mutex *lock);

extern void rt_mutex_lock(struct rt_mutex *lock);
extern int rt_mutex_lock_interruptible(struct rt_mutex *lock,
						int detect_deadlock);
extern int rt_mutex_timed_lock(struct rt_mutex *lock,
					struct hrtimer_sleeper *timeout,
					int detect_deadlock);

extern int rt_mutex_trylock(struct rt_mutex *lock);

extern void rt_mutex_unlock(struct rt_mutex *lock);

#ifdef CONFIG_RT_MUTEXES
#define INIT_RT_MUTEXES(tsk)						\
	.pi_waiters	= PLIST_HEAD_INIT(tsk.pi_waiters, tsk.pi_lock),	\
	INIT_RT_MUTEX_DEBUG(tsk)
#else
#define INIT_RT_MUTEXES(tsk)
#endif

#endif
