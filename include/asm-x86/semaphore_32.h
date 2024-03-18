#ifndef _I386_SEMAPHORE_H
#define _I386_SEMAPHORE_H

#include <linux/linkage.h>

#ifdef __KERNEL__

#include <asm/system.h>
#include <asm/atomic.h>
#include <linux/wait.h>
#include <linux/rwsem.h>
 /*SMP和中断安全的信号量，与自旋锁相比，信号量适合于保护更长的临界区，以防止并行访问，
但它们不应该用于保护较短的代码范围，因为竞争信号量时需要使进程睡眠和再次唤醒，代价很高*/
 /*虽然该结构定义在体系结构相关的头文件中，但大多数体系结构都使用该信号量结构*/
struct semaphore
{
	/*可以同时处于信号量保护的临界区中进程的数目，count=1用于大多数情况，此类信号量
	又称为互斥信号量，实现互斥*/
	atomic_t count;
	/*等待允许进入临界区的进程的数目，不同于自旋锁，等待的进程可以进入睡眠状态，直至
	信号量释放才会被唤醒，这意味着相关的处理器在同时可以执行其它任务*/
	int sleepers;
	/*等待队列，保存所有在该信号量上睡眠的进程的task_struct*/
	wait_queue_head_t wait;
};

/*初始化name信号量*/
#define __SEMAPHORE_INITIALIZER(name, n)				\
{									\
	.count		= ATOMIC_INIT(n),				\
	.sleepers	= 0,						\
	.wait		= __WAIT_QUEUE_HEAD_INITIALIZER((name).wait)	\
}
/*定义并初始化name信号量*/
#define __DECLARE_SEMAPHORE_GENERIC(name,count) \
	struct semaphore name = __SEMAPHORE_INITIALIZER(name,count)

/*基于信号量的互斥量。大多数情况下，不需要使用信号量的所用功能，只是将其用作互斥量，
这是一种二值信号量*/
#define DECLARE_MUTEX(name) __DECLARE_SEMAPHORE_GENERIC(name,1)

/*初始化信号量*/
static inline void sema_init (struct semaphore *sem, int val)
{
	atomic_set(&sem->count, val);
	sem->sleepers = 0;
	init_waitqueue_head(&sem->wait);
}
/*初始化一个二值信号量*/
static inline void init_MUTEX (struct semaphore *sem)
{
	sema_init(sem, 1);
}
/**/
static inline void init_MUTEX_LOCKED (struct semaphore *sem)
{
	sema_init(sem, 0);
}

fastcall void __down_failed(void /* special register calling convention */);
fastcall int  __down_failed_interruptible(void  /* params in registers */);
fastcall int  __down_failed_trylock(void  /* params in registers */);
fastcall void __up_wakeup(void /* special register calling convention */);

 /*申请信号量*/
static inline void down(struct semaphore * sem)
{
	might_sleep();
	__asm__ __volatile__(
		"# atomic down operation\n\t"
		LOCK_PREFIX "decl %0\n\t"     /* --sem->count */
		"jns 2f\n"
		"\tlea %0,%%eax\n\t"
		"call __down_failed\n"
		"2:"
		:"+m" (sem->count)
		:
		:"memory","ax");
}

 /*可中断状态申请信号量，申请成功返回0，如果被中断，则返回-EINTR*/
static inline int down_interruptible(struct semaphore * sem)
{
	int result;

	might_sleep();
	__asm__ __volatile__(
		"# atomic interruptible down operation\n\t"
		"xorl %0,%0\n\t"
		LOCK_PREFIX "decl %1\n\t"     /* --sem->count */
		"jns 2f\n\t"
		"lea %1,%%eax\n\t"
		"call __down_failed_interruptible\n"
		"2:"
		:"=&a" (result), "+m" (sem->count)
		:
		:"memory");
	return result;
}


 /*非阻塞式申请信号量，如果申请成功返回0*/
static inline int down_trylock(struct semaphore * sem)
{
	int result;

	__asm__ __volatile__(
		"# atomic interruptible down operation\n\t"
		"xorl %0,%0\n\t"
		LOCK_PREFIX "decl %1\n\t"     /* --sem->count */
		"jns 2f\n\t"
		"lea %1,%%eax\n\t"
		"call __down_failed_trylock\n\t"
		"2:\n"
		:"=&a" (result), "+m" (sem->count)
		:
		:"memory");
	return result;
}

 /*释放信号量，信号量为负值时唤醒等待进程*/
static inline void up(struct semaphore * sem)
{
	__asm__ __volatile__(
		"# atomic up operation\n\t"
		LOCK_PREFIX "incl %0\n\t"     /* ++sem->count */
		"jg 1f\n\t"
		"lea %0,%%eax\n\t"
		"call __up_wakeup\n"
		"1:"
		:"+m" (sem->count)
		:
		:"memory","ax");
}

#endif
#endif
