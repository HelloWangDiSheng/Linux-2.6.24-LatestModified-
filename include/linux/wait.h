#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

/*�ȴ�����wait_queue����ʹ���̵ȴ�ĳһ�ض��¼��ķ�����������Ƶ����ѯ�������ڵȴ��ڼ�
˯�ߣ����¼�����ʱ���ں��Զ����ѡ������completion���ƻ��ڵȴ����У��ں����øû��Ƶ�
��ĳһ���������������ֻ���ʹ�õĶ��Ƚ�Ƶ������Ҫ�����豸�������򡣵ȴ����е�ʹ�÷�Ϊ
�����֣���1��Ϊʹ��ǰ������һ���ȴ�������˯�ߣ���Ҫ����wait_event������Ĭ�ϵȼۺ�����
�����̽���˯�ߣ�������Ȩ�ͷŸ����������ں�ͨ����������豸������������󣬵��øú���
����Ϊ���䲻���������������ڴ��ڼ���û������������������Խ��̿���˯�ߣ���cpuʱ���ø�
ϵͳ���������̣�2�����ں˵���һ���������ǵ����Ӷ��ԣ������Կ��豸�����ݵ���֮�󣬱���
����wake_up��������ĳ���ȼ۵ĺ������������ѵȴ������е�˯�߽��̡�*/

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
/*����һ�����ѵȴ�����ָ��*/
typedef int (*wait_queue_func_t)(wait_queue_t *wait, unsigned mode, int sync, void *key);
int default_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key);
/*�����еĳ�Ա�ṹ*/
struct __wait_queue
{
	/*�ñ�ʶֵΪ0����WQ_FLAGS_EXCLUSIVE����ǰû�ж���������־*/
	unsigned int flags;
	/*��ʾ�ȴ�������Ҫ����ռ�ػ���*/
#define WQ_FLAG_EXCLUSIVE	0x01
	/*ָ����̵�struct task_structʵ�����ñ��������Ͽ���ָ�������˽�����ݣ����ں�ֻ
	�к�������²���ô��*/
	void *private;
	/*���ѵȴ����̵ĺ���ָ��*/
	wait_queue_func_t func;
	/*˫�������ڽ�wait_queue_tʵ�����õ��ȴ�������*/
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
/*ÿ���ȴ����ж���һ������ͷ*/
struct __wait_queue_head
{
	/*�ȴ�����Ҳ�������ж�ʱ�޸ģ��ڲ�������֮ǰ�����ȡ������*/
	spinlock_t lock;
	/*˫�������ӵȴ�����*/
	struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

struct task_struct;

/*��ʼ���ȴ������еȴ���Աname����Ϣ*/
#define __WAITQUEUE_INITIALIZER(name, tsk) {\
	.private	= tsk,								\
	.func		= default_wake_function,			\
	.task_list	= { NULL, NULL } }
/*���岢��ʼ���ȴ������еȴ���Աname����Ϣ*/
#define DECLARE_WAITQUEUE(name, tsk)				\
	wait_queue_t name = __WAITQUEUE_INITIALIZER(name, tsk)
/*��ʼ���ȴ�����ͷname����Ϣ*/
#define __WAIT_QUEUE_HEAD_INITIALIZER(name) {				\
	.lock		= __SPIN_LOCK_UNLOCKED(name.lock),			\
	.task_list	= { &(name).task_list, &(name).task_list } }
/*���岢��ʼ���ȴ�����ͷname����Ϣ*/
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

/*�ɵȴ����̹����ʼ���ȴ����г�Ա*/
static inline void init_waitqueue_entry(wait_queue_t *q, struct task_struct *p)
{
	/*�Ƕ�ռʽ����*/
	q->flags = 0;
	/*�ȴ�����*/
	q->private = p;
	/*Ĭ�ϵĻ��ѵȴ����̺���ָ��*/
	q->func = default_wake_function;
}

/*�ɵȴ����ѽ��̺���ָ�빹��ȴ����г�Ա*/
static inline void init_waitqueue_func_entry(wait_queue_t *q,
					wait_queue_func_t func)
{
	q->flags = 0;
	q->private = NULL;
	q->func = func;
}

/*���Եȴ��������Ƿ��еȴ�����*/
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
 /*��������������ͬ��IO���첽IO�ȴ�*/
#define is_sync_wait(wait)	(!(wait) || ((wait)->private))

extern void FASTCALL(add_wait_queue(wait_queue_head_t *q, wait_queue_t * wait));
extern void FASTCALL(add_wait_queue_exclusive(wait_queue_head_t *q, wait_queue_t * wait));
extern void FASTCALL(remove_wait_queue(wait_queue_head_t *q, wait_queue_t * wait));

/*ͷ�巨��ȴ����������һ���ȴ���Ա*/
static inline void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new)
{
	list_add(&new->task_list, &head->task_list);
}

/*β�巨��ȴ������в���һ���ȴ���Ա*/
static inline void __add_wait_queue_tail(wait_queue_head_t *head,
						wait_queue_t *new)
{
	list_add_tail(&new->task_list, &head->task_list);
}

/*��ָ���ȴ���Ա�ӵȴ�������ɾ��*/
static inline void __remove_wait_queue(wait_queue_head_t *head,
							wait_queue_t *old)
{
	list_del(&old->task_list);
}

/*���ѵȴ������еĽ���*/
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
/*�ʹ�õ�wake_up������nr_exclusive����Ϊ1��ȷ��ֻ����һ����ռ���ʵĽ���*/
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

/*����DEFINE_WAIT�����ȴ����г�Ա֮������������һ������ѭ����ʹ��prepare_to_wait
ʹ�����ڵȴ�������˯�ߡ�ÿ�ν��̱�����ʱ���ں˶�����ָ���������Ƿ����㣬���������
�˳�����ѭ�������򣬽�����Ȩ�����������������ٴ�˯��*/
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

/*���̽��벻���ж�˯�ߣ�ÿ�λ��ѵȴ����ж�����������ֱ���ȴ�����Ϊ��ʱ���ѡ�
���κε��µȴ������ı����ı�ʱwake_up�ᱻ����*/
#define wait_event(wq, condition) 					\
do {									\
	if (condition)	 						\
		break;							\
	__wait_event(wq, condition);					\
} while (0)

/*�ȴ�����ָ����������������ȴ��¼�������ָ���ĳ�ʱ���ƣ���jiffiesָ������ֹͣ��
	���ֹ�˽�����Զ˯��*/
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

/*���̽���˯��״̬��ֱ���ȴ��������������յ��źţ��ȴ�����ÿ�α�����ʱ������
���ȴ������ȴ����̱��ж�ʱ�ú����᷵��-ERESTARTSYS������Ϊ��ʱ����0*/
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

/*����˯��ֱ�������������յ��źŻ��߳�ʱʱ������*/
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
/*�����Ա����ľɽӿڣ���Ҫ�����´�����*/
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

/*����һ��wait_queue_t�ľ�̬ʵ�����������Զ���ʼ����*/
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

/*����ǰ�������Ϊ�ȴ���Ա*/
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
