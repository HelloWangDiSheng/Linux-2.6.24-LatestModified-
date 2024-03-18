/* rwsem-spinlock.h: fallback C implementation
 *
 * Copyright (c) 2001   David Howells (dhowells@redhat.com).
 * - Derived partially from ideas by Andrea Arcangeli <andrea@suse.de>
 * - Derived also from comments by Linus
 */

#ifndef _LINUX_RWSEM_SPINLOCK_H
#define _LINUX_RWSEM_SPINLOCK_H

#ifndef _LINUX_RWSEM_H
#error "please don't include linux/rwsem-spinlock.h directly, use linux/rwsem.h instead"
#endif

#include <linux/spinlock.h>
#include <linux/list.h>

#ifdef __KERNEL__

#include <linux/types.h>

struct rwsem_waiter;

/*��д�ź�������ṹ�塣��ԱactivityֵΪ0˵��û�л�Ķ���д���̣�ֵ����0��ʾ�
�����̵���Ŀ��ֵΪ-1˵����һ�����д���̣������Աwait_list�ǿգ�˵���н����ڵȴ�
���ź���*/
struct rw_semaphore
{
	/*��Ķ���д���̵���Ŀ*/
	__s32			activity;
	/*�����ź����ı�����*/
	spinlock_t		wait_lock;
	/*�ȴ���������*/
	struct list_head	wait_list;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
};

#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define __RWSEM_DEP_MAP_INIT(lockname) , .dep_map = { .name = #lockname }
#else
#define __RWSEM_DEP_MAP_INIT(lockname)
#endif

/*��ʼ��һ��ָ�����ƵĶ�д�ź���*/
#define __RWSEM_INITIALIZER(name) 				\
{												\
	0,											\
	__SPIN_LOCK_UNLOCKED(name.wait_lock),\
	LIST_HEAD_INIT((name).wait_list) 			\
	__RWSEM_DEP_MAP_INIT(name) 					\
}

/*���岢��ʼ��һ��ָ�����ƵĶ�д�ź���*/
#define DECLARE_RWSEM(name)		struct rw_semaphore name = __RWSEM_INITIALIZER(name)

extern void __init_rwsem(struct rw_semaphore *sem, const char *name,
		struct lock_class_key *key);
/**/
#define init_rwsem(sem)							\
do 												\
{												\
	static struct lock_class_key __key;			\
	__init_rwsem((sem), #sem, &__key);			\
} while (0)

extern void FASTCALL(__down_read(struct rw_semaphore *sem));
extern int FASTCALL(__down_read_trylock(struct rw_semaphore *sem));
extern void FASTCALL(__down_write(struct rw_semaphore *sem));
extern void FASTCALL(__down_write_nested(struct rw_semaphore *sem, int subclass));
extern int FASTCALL(__down_write_trylock(struct rw_semaphore *sem));
extern void FASTCALL(__up_read(struct rw_semaphore *sem));
extern void FASTCALL(__up_write(struct rw_semaphore *sem));
extern void FASTCALL(__downgrade_write(struct rw_semaphore *sem));

/*����ָ����д�ź����Ƿ�������ʹ����*/
static inline int rwsem_is_locked(struct rw_semaphore *sem)
{
	return (sem->activity != 0);
}

#endif /* __KERNEL__ */
#endif /* _LINUX_RWSEM_SPINLOCK_H */
