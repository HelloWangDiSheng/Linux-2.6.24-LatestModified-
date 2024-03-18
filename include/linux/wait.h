#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

/*等待队列wait_queue用于使进程等待某一特定事件的发生，而无须频繁轮询，进程在等待期间
睡眠，在事件发生时由内核自动唤醒。完成量completion机制基于等待队列，内核利用该机制等
待某一操作结束。这两种机制使用的都比较频繁，主要用于设备驱动程序。等待队列的使用分为
两部分：（1）为使当前进程在一个等待队列中睡眠，需要调用wait_event（或者默认等价函数）
，进程进入睡眠，将控制权释放给调度器。内核通常会在向块设备发出传输请求后，调用该函数
，因为传输不会立即发生，而在此期间有没有其它事情可做，所以进程可以睡眠，将cpu时间让给
系统中其它进程（2）在内核的另一处，就我们的例子而言，是来自块设备的数据到达之后，必须
调用wake_up函数（或某个等价的函数），来唤醒等待队列中的睡眠进程。*/

#define WNOHANG		0x00000001
#define WUNTRACED	0x00000002
#define WSTOPPED	WUNTRACED
#define WEXITED		0x00000004
#define WCONTINUED	0x00000008
#define WNOWAIT		0x01000000	/* Don't reap, just poll status.  */

#define __WNOTHREAD	0x20000000	/* Don't wait on children of other threads in this group */
#define __WALL		0x40000000	/* Wait on all children, regardless of type */
#define __WCLONE	0x80000000	/* Wait only on non-SIGCHLD children */

/* First argument to waitid: */
#define P_ALL		0
#define P_PID		1
#define P_PGID		2

#ifdef __KERNEL__

#include <linux/list.h>
#include <linux/stddef.h>
#include <linux/spinlock.h>
#include <asm/system.h>
#include <asm/current.h>

typedef struct __wait_queue wait_queue_t;
/*定义一个唤醒等待函数指针*/
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int sync, void *key);
int default_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key);
/*队列中的成员结构*/
struct __wait_queue
{
	/*该标识值为0或者WQ_FLAGS_EXCLUSIVE，当前没有定义其它标志*/
	unsigned int flags;
	/*表示等待进程想要被独占地唤醒*/
#define WQ_FLAG_EXCLUSIVE	0x01
	/*指向进程的struct task_struct实例，该变量本质上可以指向任意的私有数据，但内核只
	有很少情况下才这么用*/
	void *private;
	/*唤醒等待进程的函数指针*/
	wait_queue_func_t func;
	/*双链表用于将wait_queue_t实例放置到等待队列中*/
	struct list_head task_list;
};

struct wait_bit_key
{
	void *flags;
	int bit_nr;
};

struct wait_bit_queue
{
	struct wait_bit_key key;
	wait_queue_t wait;
};
/*每个等待队列都有一个队列头*/
struct __wait_queue_head
{
	/*等待队列也可以在中断时修改，在操作队列之前必须获取自旋锁*/
	spinlock_t lock;
	/*双链表连接等待队列*/
	struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

struct task_struct;

/*初始化等待队列中等待成员name的信息*/
#define __WAITQUEUE_INITIALIZER(name, tsk) {\
	.private	= tsk,								\
	.func		= default_wake_function,			\
	.task_list	= { NULL, NULL } }
/*定义并初始化等待队列中等待成员name的信息*/
#define DECLARE_WAITQUEUE(name, tsk)				\
	wait_queue_t name = __WAITQUEUE_INITIALIZER(name, tsk)
/*初始化等待队列头name的信息*/
#define __WAIT_QUEUE_HEAD_INITIALIZER(name) {				\
	.lock		= __SPIN_LOCK_UNLOCKED(name.lock),			\
	.task_list	= { &(name).task_list, &(name).task_list } }
/*定义并初始化等待队列头name的信息*/
#define DECLARE_WAIT_QUEUE_HEAD(name) \
	wait_queue_head_t name = __WAIT_QUEUE_HEAD_INITIALIZER(name)
/**/
#define __WAIT_BIT_KEY_INITIALIZER(word, bit)		\
	{ .flags = word, .bit_nr = bit, }

extern void init_waitqueue_head(wait_queue_head_t *q);
/**/
#ifdef CONFIG_LOCKDEP
#define __WAIT_QUEUE_HEAD_INIT_ONSTACK(name) ({ init_waitqueue_head(&name); name; })
#define DECLARE_WAIT_QUEUE_HEAD_ONSTACK(name) \
	wait_queue_head_t name = __WAIT_QUEUE_HEAD_INIT_ONSTACK(name)
#else
#define DECLARE_WAIT_QUEUE_HEAD_ONSTACK(name) DECLARE_WAIT_QUEUE_HEAD(name)
#endif

/*由等待进程构造初始化等待队列成员*/
static inline void init_waitqueue_entry(wait_queue_t *q, struct task_struct *p)
{
	/*非独占式唤醒*/
	q->flags = 0;
	/*等待进程*/
	q->private = p;
	/*默认的唤醒等待进程函数指针*/
	q->func = default_wake_function;
}

/*由等待唤醒进程函数指针构造等待队列成员*/
static inline void init_waitqueue_func_entry(wait_queue_t *q,
					wait_queue_func_t func)
{
	q->flags = 0;
	q->private = NULL;
	q->func = func;
}

/*测试等待队列中是否有等待进程*/
static inline int waitqueue_active(wait_queue_head_t *q)
{
	return !list_empty(&q->task_list);
}

/*
 * Used to distinguish between sync and async io wait context:
 * sync i/o typically specifies a NULL wait queue entry or a wait
 * queue entry bound to a task (current task) to wake up.
 * aio specifies a wait queue entry with an async notification
 * callback routine, not associated with any task.
 */
 /*经常被用来区别同步IO和异步IO等待*/
#define is_sync_wait(wait)	(!(wait) || ((wait)->private))

extern void FASTCALL(add_wait_queue(wait_queue_head_t *q, wait_queue_t * wait));
extern void FASTCALL(add_wait_queue_exclusive(wait_queue_head_t *q, wait_queue_t * wait));
extern void FASTCALL(remove_wait_queue(wait_queue_head_t *q, wait_queue_t * wait));

/*头插法向等待队列中添加一个等待成员*/
static inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
	list_add(&new->task_list, &head->task_list);
}

/*尾插法向等待队列中插入一个等待成员*/
static inline void __add_wait_queue_tail(wait_queue_head_t *head,
						wait_queue_t *new)
{
	list_add_tail(&new->task_list, &head->task_list);
}

/*将指定等待成员从等待队列中删除*/
static inline void __remove_wait_queue(wait_queue_head_t *head,
							wait_queue_t *old)
{
	list_del(&old->task_list);
}

/*唤醒等待队列中的进程*/
void FASTCALL(__wake_up(wait_queue_head_t *q, unsigned int mode, int nr, void *key));
extern void FASTCALL(__wake_up_locked(wait_queue_head_t *q, unsigned int mode));
extern void FASTCALL(__wake_up_sync(wait_queue_head_t *q, unsigned int mode, int nr));
void FASTCALL(__wake_up_bit(wait_queue_head_t *, void *, int));
int FASTCALL(__wait_on_bit(wait_queue_head_t *, struct wait_bit_queue *, int (*)(void *), unsigned));
int FASTCALL(__wait_on_bit_lock(wait_queue_head_t *, struct wait_bit_queue *, int (*)(void *), unsigned));
void FASTCALL(wake_up_bit(void *, int));
int FASTCALL(out_of_line_wait_on_bit(void *, int, int (*)(void *), unsigned));
int FASTCALL(out_of_line_wait_on_bit_lock(void *, int, int (*)(void *), unsigned));
wait_queue_head_t *FASTCALL(bit_waitqueue(void *, int));
/*最常使用的wake_up函数将nr_exclusive设置为1，确保只唤醒一个独占访问的进程*/
#define wake_up(x)			__wake_up(x, TASK_UNINTERRUPTIBLE | TASK_INTERRUPTIBLE, 1, NULL)
/**/
#define wake_up_nr(x, nr)		__wake_up(x, TASK_UNINTERRUPTIBLE | TASK_INTERRUPTIBLE, nr, NULL)
/**/
#define wake_up_all(x)			__wake_up(x, TASK_UNINTERRUPTIBLE | TASK_INTERRUPTIBLE, 0, NULL)
/**/
#define wake_up_interruptible(x)	__wake_up(x, TASK_INTERRUPTIBLE, 1, NULL)
/**/
#define wake_up_interruptible_nr(x, nr)	__wake_up(x, TASK_INTERRUPTIBLE, nr, NULL)
/**/
#define wake_up_interruptible_all(x)	__wake_up(x, TASK_INTERRUPTIBLE, 0, NULL)
/**/
#define	wake_up_locked(x)		__wake_up_locked((x), TASK_UNINTERRUPTIBLE | TASK_INTERRUPTIBLE)
/**/
#define wake_up_interruptible_sync(x)   __wake_up_sync((x),TASK_INTERRUPTIBLE, 1)

/*在用DEFINE_WAIT建立等待队列成员之后，这个宏产生了一个无限循环，使用prepare_to_wait
使进程在等待队列上睡眠。每次进程被唤醒时，内核都会检查指定的条件是否满足，如果满足则
退出无限循环，否则，将控制权交给调度器，进程再次睡眠*/
#define __wait_event(wq, condition) 					\
do {									\
	DEFINE_WAIT(__wait);						\
									\
	for (;;) {							\
		prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);	\
		if (condition)						\
			break;						\
		schedule();						\
	}								\
	finish_wait(&wq, &__wait);					\
} while (0)

/*进程进入不可中断睡眠，每次唤醒等待队列都会检查条件，直到等待条件为真时唤醒。
当任何导致等待条件的变量改变时wake_up会被调用*/
#define wait_event(wq, condition) 					\
do {									\
	if (condition)	 						\
		break;							\
	__wait_event(wq, condition);					\
} while (0)

/*等待满足指定的条件，但如果等待事件超过了指定的超时限制（按jiffies指定）则停止，
	这防止了进程永远睡眠*/
#define __wait_event_timeout(wq, condition, ret)			\
do {									\
	DEFINE_WAIT(__wait);						\
									\
	for (;;) {							\
		prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);	\
		if (condition)						\
			break;						\
		ret = schedule_timeout(ret);				\
		if (!ret)						\
			break;						\
	}								\
	finish_wait(&wq, &__wait);					\
} while (0)

/**
 * wait_event_timeout - sleep until a condition gets true or a timeout elapses
 * @wq: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 * @timeout: timeout, in jiffies
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function returns 0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 */
/**/
#define wait_event_timeout(wq, condition, timeout)			\
({									\
	long __ret = timeout;						\
	if (!(condition)) 						\
		__wait_event_timeout(wq, condition, __ret);		\
	__ret;								\
})

/**/
#define __wait_event_interruptible(wq, condition, ret)			\
do {									\
	DEFINE_WAIT(__wait);						\
									\
	for (;;) {							\
		prepare_to_wait(&wq, &__wait, TASK_INTERRUPTIBLE);	\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			schedule();					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	finish_wait(&wq, &__wait);					\
} while (0)

/*进程进入睡眠状态，直到等待的条件满足或接收到信号，等待队列每次被唤醒时，都会
检查等待条件等待进程被中断时该函数会返回-ERESTARTSYS，条件为真时返回0*/
#define wait_event_interruptible(wq, condition)				\
({									\
	int __ret = 0;							\
	if (!(condition))						\
		__wait_event_interruptible(wq, condition, __ret);	\
	__ret;								\
})
/**/
#define __wait_event_interruptible_timeout(wq, condition, ret)		\
do {									\
	DEFINE_WAIT(__wait);						\
									\
	for (;;) {							\
		prepare_to_wait(&wq, &__wait, TASK_INTERRUPTIBLE);	\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			ret = schedule_timeout(ret);			\
			if (!ret)					\
				break;					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	finish_wait(&wq, &__wait);					\
} while (0)

/*进程睡眠直到条件成立、收到信号或者超时时被唤醒*/
#define wait_event_interruptible_timeout(wq, condition, timeout)	\
({									\
	long __ret = timeout;						\
	if (!(condition))						\
		__wait_event_interruptible_timeout(wq, condition, __ret); \
	__ret;								\
})
/**/
#define __wait_event_interruptible_exclusive(wq, condition, ret)	\
do {									\
	DEFINE_WAIT(__wait);						\
									\
	for (;;) {							\
		prepare_to_wait_exclusive(&wq, &__wait,			\
					TASK_INTERRUPTIBLE);		\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			schedule();					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	finish_wait(&wq, &__wait);					\
} while (0)

/**/
#define wait_event_interruptible_exclusive(wq, condition)		\
({									\
	int __ret = 0;							\
	if (!(condition))						\
		__wait_event_interruptible_exclusive(wq, condition, __ret);\
	__ret;								\
})

/*
 * Must be called with the spinlock in the wait_queue_head_t held.
 */
/**/
static inline void add_wait_queue_exclusive_locked(wait_queue_head_t *q,
						   wait_queue_t * wait)
{
	wait->flags |= WQ_FLAG_EXCLUSIVE;
	__add_wait_queue_tail(q,  wait);
}

/*
 * Must be called with the spinlock in the wait_queue_head_t held.
 */
/**/
static inline void remove_wait_queue_locked(wait_queue_head_t *q,
					    wait_queue_t * wait)
{
	__remove_wait_queue(q,  wait);
}

/*
 * These are the old interfaces to sleep waiting for an event.
 * They are racy.  DO NOT use them, use the wait_event* interfaces above.
 * We plan to remove these interfaces.
 */
/*兼容性保留的旧接口，不要用于新代码中*/
extern void sleep_on(wait_queue_head_t *q);
extern long sleep_on_timeout(wait_queue_head_t *q,
				      signed long timeout);
extern void interruptible_sleep_on(wait_queue_head_t *q);
extern long interruptible_sleep_on_timeout(wait_queue_head_t *q,
					   signed long timeout);

/*
 * Waitqueues which are removed from the waitqueue_head at wakeup time
 */
void FASTCALL(prepare_to_wait(wait_queue_head_t *q,
				wait_queue_t *wait, int state));
void FASTCALL(prepare_to_wait_exclusive(wait_queue_head_t *q,
				wait_queue_t *wait, int state));
void FASTCALL(finish_wait(wait_queue_head_t *q, wait_queue_t *wait));
int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key);
int wake_bit_function(wait_queue_t *wait, unsigned mode, int sync, void *key);

/*创建一个wait_queue_t的静态实例，它可以自动初始化，*/
#define DEFINE_WAIT(name)										\
	wait_queue_t name = {										\
		.private	= current,									\
		.func		= autoremove_wake_function,					\
		.task_list	= LIST_HEAD_INIT((name).task_list),			\
	}
/**/
#define DEFINE_WAIT_BIT(name, word, bit)						\
	struct wait_bit_queue name = {								\
		.key = __WAIT_BIT_KEY_INITIALIZER(word, bit),			\
		.wait	= {												\
			.private	= current,								\
			.func		= wake_bit_function,					\
			.task_list	=										\
				LIST_HEAD_INIT((name).wait.task_list),			\
		},														\
	}

/*将当前进程添加为等待成员*/
#define init_wait(wait)											\
	do {														\
		(wait)->private = current;								\
		(wait)->func = autoremove_wake_function;				\
		INIT_LIST_HEAD(&(wait)->task_list);						\
	} while (0)

/**
 * wait_on_bit - wait for a bit to be cleared
 * @word: the word being waited on, a kernel virtual address
 * @bit: the bit of the word being waited on
 * @action: the function used to sleep, which may take special actions
 * @mode: the task state to sleep in
 *
 * There is a standard hashed waitqueue table for generic use. This
 * is the part of the hashtable's accessor API that waits on a bit.
 * For instance, if one were to have waiters on a bitflag, one would
 * call wait_on_bit() in threads waiting for the bit to clear.
 * One uses wait_on_bit() where one is waiting for the bit to clear,
 * but has no intention of setting it.
 */
static inline int wait_on_bit(void *word, int bit,
				int (*action)(void *), unsigned mode)
{
	if (!test_bit(bit, word))
		return 0;
	return out_of_line_wait_on_bit(word, bit, action, mode);
}

/**
 * wait_on_bit_lock - wait for a bit to be cleared, when wanting to set it
 * @word: the word being waited on, a kernel virtual address
 * @bit: the bit of the word being waited on
 * @action: the function used to sleep, which may take special actions
 * @mode: the task state to sleep in
 *
 * There is a standard hashed waitqueue table for generic use. This
 * is the part of the hashtable's accessor API that waits on a bit
 * when one intends to set it, for instance, trying to lock bitflags.
 * For instance, if one were to have waiters trying to set bitflag
 * and waiting for it to clear before setting it, one would call
 * wait_on_bit() in threads waiting to be able to set the bit.
 * One uses wait_on_bit_lock() where one is waiting for the bit to
 * clear with the intention of setting it, and when done, clearing it.
 */
static inline int wait_on_bit_lock(void *word, int bit,
				int (*action)(void *), unsigned mode)
{
	if (!test_and_set_bit(bit, word))
		return 0;
	return out_of_line_wait_on_bit_lock(word, bit, action, mode);
}

#endif /* __KERNEL__ */

#endif
