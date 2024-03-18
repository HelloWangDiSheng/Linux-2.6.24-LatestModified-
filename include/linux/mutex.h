#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include <linux/list.h>
#include <linux/spinlock_types.h>
#include <linux/linkage.h>
#include <linux/lockdep.h>

#include <asm/atomic.h>

/*
 * Simple, straightforward mutexes with strict semantics:
 *
 * - only one task can hold the mutex at a time
 * - only the owner can unlock the mutex
 * - multiple unlocks are not permitted
 * - recursive locking is not permitted
 * - a mutex object must be initialized via the API
 * - a mutex object must not be initialized via memset or copying
 * - task may not exit with mutex held
 * - memory areas where held locks reside must not be freed
 * - held mutexes must not be reinitialized
 * - mutexes may not be used in hardware or software interrupt
 *   contexts such as tasklets and timers
 *
 * These semantics are fully enforced when DEBUG_MUTEXES is
 * enabled. Furthermore, besides enforcing the above rules, the mutex
 * debugging code also implements a number of additional features
 * that make lock debugging easier and faster:
 *
 * - uses symbolic names of mutexes, whenever they are printed in debug output
 * - point-of-acquire tracking, symbolic lookup of function names
 * - list of all locks held in the system, printout of them
 * - owner tracking
 * - detects self-recursing locks and prints out all relevant info
 * - detects multi-task circular deadlocks and prints out all affected
 *   locks and tasks (and only those tasks)
 */
/*经典互斥量的基本数据结构。互斥量的概念相当简单：如果互斥量未锁定，则count为1，锁定分为
两种情况：如果只有一个进程在使用互斥量，则count设置为0，如果互斥量被锁定，而且进程在等待
互斥量解锁（解锁时需要唤醒等待进程），则count的值为负值。这种特殊处理有助于加快执行的速度
，通常情况下，不会有进程在互斥量上等待。有两种方法定义新的互斥量：（1）静态互斥量可以在编
译时通过使用DEFINE_MUTEX产生（不要与DECLARE_MUTEX混淆，后者时基于信号量的互斥量）。（2）
mutex_init在运行时动态初始化一个新的互斥量*/
struct mutex
{
	/*1未锁定，0已锁定，负值时可能有等待进程*/
	atomic_t		count;
	spinlock_t		wait_lock;
	struct list_head	wait_list;
#ifdef CONFIG_DEBUG_MUTEXES
	struct thread_info	*owner;
	const char 		*name;
	void			*magic;
#endif
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

/*
 * This is the control structure for tasks blocked on mutex,
 * which resides on the blocked task's kernel stack:
 */
/*进程在互斥量上被阻塞的控制结构*/
struct mutex_waiter {
	struct list_head	list;
	struct task_struct	*task;
#ifdef CONFIG_DEBUG_MUTEXES
	struct mutex		*lock;
	void			*magic;
#endif
};

#ifdef CONFIG_DEBUG_MUTEXES
#include <linux/mutex-debug.h>
#else
#define __DEBUG_MUTEX_INITIALIZER(lockname)
/*初始化一个互斥量*/
#define mutex_init(mutex) 						\
do {											\
	static struct lock_class_key __key;			\
	__mutex_init((mutex), #mutex, &__key);		\
} while (0)
#define mutex_destroy(mutex)				do { } while (0)
#endif

#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define __DEP_MAP_MUTEX_INITIALIZER(lockname) \
		, .dep_map = { .name = #lockname }
#else
#define __DEP_MAP_MUTEX_INITIALIZER(lockname)
#endif
/*初始化名为lockname的互斥量*/
#define __MUTEX_INITIALIZER(lockname) 								\
{																	\
	.count = ATOMIC_INIT(1) 										\
	, .wait_lock = __SPIN_LOCK_UNLOCKED(lockname.wait_lock)\
	, .wait_list = LIST_HEAD_INIT(lockname.wait_list) 				\
	__DEBUG_MUTEX_INITIALIZER(lockname) 							\
	__DEP_MAP_MUTEX_INITIALIZER(lockname)							\
}
/*定义并初始化一个互斥量*/
#define DEFINE_MUTEX(mutexname) struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

extern void __mutex_init(struct mutex *lock, const char *name, struct lock_class_key *key);

/**
 * mutex_is_locked - is the mutex locked
 * @lock: the mutex to be queried
 *
 * Returns 1 if the mutex is locked, 0 if unlocked.
 */
 /*互斥量是否未被锁定*/
static inline int fastcall mutex_is_locked(struct mutex *lock)
{
	return atomic_read(&lock->count) != 1;
}

/*
 * See kernel/mutex.c for detailed documentation of these APIs.
 * Also see Documentation/mutex-design.txt.
 */
#ifdef CONFIG_DEBUG_LOCK_ALLOC
extern void mutex_lock_nested(struct mutex *lock, unsigned int subclass);
extern int __must_check mutex_lock_interruptible_nested(struct mutex *lock,
					unsigned int subclass);

#define mutex_lock(lock) mutex_lock_nested(lock, 0)
#define mutex_lock_interruptible(lock) mutex_lock_interruptible_nested(lock, 0)
#else
extern void fastcall mutex_lock(struct mutex *lock);
extern int __must_check fastcall mutex_lock_interruptible(struct mutex *lock);

# define mutex_lock_nested(lock, subclass) mutex_lock(lock)
# define mutex_lock_interruptible_nested(lock, subclass) mutex_lock_interruptible(lock)
#endif

/*
 * NOTE: mutex_trylock() follows the spin_trylock() convention,
 *       not the down_trylock() convention!
 */
extern int fastcall mutex_trylock(struct mutex *lock);
extern void fastcall mutex_unlock(struct mutex *lock);

#endif
