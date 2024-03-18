#ifndef __LINUX_COMPLETION_H
#define __LINUX_COMPLETION_H

#include <linux/wait.h>

/*完成量completion机制基于等待队列，内核利用该机制等待某一操作结束。主要用于设备驱动程序
。完成量的接口场景中有两个参与者：一个在等待某操作完成，而另一个在操作完成时发出声明。实
际上，可以有任意数目的进程等待操作的完成，为表示进程等待的即将完成的某操作，内核使用该数
据结构*/
struct completion
{
	/*可能在某些进程开始等待之前，事件就已经完成，done用来处理这种情形*/
	unsigned int done;
	/*wait是一个标准的等待队列，等待进程在队列上睡眠*/
	wait_queue_head_t wait;
};
/*初始化一个完成量*/
#define COMPLETION_INITIALIZER(work) { 0, __WAIT_QUEUE_HEAD_INITIALIZER((work).wait) }
/**/
#define COMPLETION_INITIALIZER_ONSTACK(work) ({ init_completion(&work); work; })
/*声明并初始化一个完成量*/
#define DECLARE_COMPLETION(work) struct completion work = COMPLETION_INITIALIZER(work)

/*
 * Lockdep needs to run a non-constant initializer for on-stack
 * completions - so we use the _ONSTACK() variant for those that
 * are on the kernel stack:
 */
#ifdef CONFIG_LOCKDEP
#define DECLARE_COMPLETION_ONSTACK(work) \
	struct completion work = COMPLETION_INITIALIZER_ONSTACK(work)
#else
#define DECLARE_COMPLETION_ONSTACK(work) DECLARE_COMPLETION(work)
#endif

/*初始化一个完成量*/
static inline void init_completion(struct completion *x)
{
	x->done = 0;
	init_waitqueue_head(&x->wait);
}

extern void wait_for_completion(struct completion *);
extern int wait_for_completion_interruptible(struct completion *x);
extern unsigned long wait_for_completion_timeout(struct completion *x, unsigned long timeout);
extern unsigned long wait_for_completion_interruptible_timeout(struct completion *x,
																			unsigned long timeout);
extern void complete(struct completion *);
extern void complete_all(struct completion *);
/*仅初始化完成量中的done成员*/
#define INIT_COMPLETION(x)	((x).done = 0)

#endif
