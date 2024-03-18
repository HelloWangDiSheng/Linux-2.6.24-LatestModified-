#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <linux/time.h>

struct task_struct;
/*获取进程的资源使用统计，会返回该进程下所有线程的资源使用统计和*/
#define	RUSAGE_SELF	0
/*获取进程所有已终止且被回收子进程的资源用量统计*/
#define	RUSAGE_CHILDREN	(-1)
/*获取进程及其所有在线，已终止且被回收的所有子进程的统计信息和*/
#define RUSAGE_BOTH	(-2)		/* sys_wait4() uses this */
/*描述进程的资源使用情况*/
struct	rusage
{
	/*用户态下运行时间*/
	struct timeval ru_utime;
	/*内核态下运行时间*/
	struct timeval ru_stime;
	/*常驻内存集的最大长度*/
	long	ru_maxrss;
	/*共享内存长度*/
	long	ru_ixrss;
	/*非共享数据长度*/
	long	ru_idrss;
	/*非共享栈长度*/
	long	ru_isrss;
	/*页面回收次数*/
	long	ru_minflt;
	/*缺页中断次数*/
	long	ru_majflt;
	/*交换缓存数目*/
	long	ru_nswap;
	/*块输入的操作次数*/
	long	ru_inblock;
	/*块输出的操作次数*/
	long	ru_oublock;
	/*发送消息的次数*/
	long	ru_msgsnd;
	/*接受消息的次数*/
	long	ru_msgrcv;
	/*接收信号的次数*/
	long	ru_nsignals;
	/*等待资源等原因自愿放弃cpu的次数*/
	long	ru_nvcsw;
	/*时间片用完或被调度等非自愿切换次数*/
	long	ru_nivcsw;
};
/*linux提供了资源限制（resource limit，rlimit）机制，对进程使用系统资源施加某些限制。
系统调用setrlimit来增减当前限制，但不能超过rlim_max指定的值，getrlimit用于检查当前限制*/
struct rlimit
{
	/*当前的资源限制，也称为软限制*/
	unsigned long	rlim_cur;
	/*该限制的最大值，也称为硬限制*/
	unsigned long	rlim_max;
};

#define	PRIO_MIN	(-20)
#define	PRIO_MAX	20

#define	PRIO_PROCESS	0
#define	PRIO_PGRP	1
#define	PRIO_USER	2

/*默认栈的最大长度，如果需要的话root用户可以增加该值*/
#define _STK_LIM	(8*1024*1024)

/*不可换出页的最大长度*/
#define MLOCK_LIMIT	(8 * PAGE_SIZE)

/*因为二进制的兼容性，实际的资源数在不同的linux版本中可能不同*/
#include <asm/resource.h>

int getrusage(struct task_struct *p, int who, struct rusage __user *ru);

#endif
