#ifndef __LINUX_COMPLETION_H
#define __LINUX_COMPLETION_H

#include <linux/wait.h>

/*�����completion���ƻ��ڵȴ����У��ں����øû��Ƶȴ�ĳһ������������Ҫ�����豸��������
��������Ľӿڳ����������������ߣ�һ���ڵȴ�ĳ������ɣ�����һ���ڲ������ʱ����������ʵ
���ϣ�������������Ŀ�Ľ��̵ȴ���������ɣ�Ϊ��ʾ���̵ȴ��ļ�����ɵ�ĳ�������ں�ʹ�ø���
�ݽṹ*/
struct completion
{
	/*������ĳЩ���̿�ʼ�ȴ�֮ǰ���¼����Ѿ���ɣ�done����������������*/
	unsigned int done;
	/*wait��һ����׼�ĵȴ����У��ȴ������ڶ�����˯��*/
	wait_queue_head_t wait;
};
/*��ʼ��һ�������*/
#define COMPLETION_INITIALIZER(work) { 0, __WAIT_QUEUE_HEAD_INITIALIZER((work).wait) }
/**/
#define COMPLETION_INITIALIZER_ONSTACK(work) ({ init_completion(&work); work; })
/*��������ʼ��һ�������*/
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

/*��ʼ��һ�������*/
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
/*����ʼ��������е�done��Ա*/
#define INIT_COMPLETION(x)	((x).done = 0)

#endif
