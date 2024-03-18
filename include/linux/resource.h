#ifndef _LINUX_RESOURCE_H
#define _LINUX_RESOURCE_H

#include <linux/time.h>

struct task_struct;
/*��ȡ���̵���Դʹ��ͳ�ƣ��᷵�ظý����������̵߳���Դʹ��ͳ�ƺ�*/
#define	RUSAGE_SELF	0
/*��ȡ������������ֹ�ұ������ӽ��̵���Դ����ͳ��*/
#define	RUSAGE_CHILDREN	(-1)
/*��ȡ���̼����������ߣ�����ֹ�ұ����յ������ӽ��̵�ͳ����Ϣ��*/
#define RUSAGE_BOTH	(-2)		/* sys_wait4() uses this */
/*�������̵���Դʹ�����*/
struct	rusage
{
	/*�û�̬������ʱ��*/
	struct timeval ru_utime;
	/*�ں�̬������ʱ��*/
	struct timeval ru_stime;
	/*��פ�ڴ漯����󳤶�*/
	long	ru_maxrss;
	/*�����ڴ泤��*/
	long	ru_ixrss;
	/*�ǹ������ݳ���*/
	long	ru_idrss;
	/*�ǹ���ջ����*/
	long	ru_isrss;
	/*ҳ����մ���*/
	long	ru_minflt;
	/*ȱҳ�жϴ���*/
	long	ru_majflt;
	/*����������Ŀ*/
	long	ru_nswap;
	/*������Ĳ�������*/
	long	ru_inblock;
	/*������Ĳ�������*/
	long	ru_oublock;
	/*������Ϣ�Ĵ���*/
	long	ru_msgsnd;
	/*������Ϣ�Ĵ���*/
	long	ru_msgrcv;
	/*�����źŵĴ���*/
	long	ru_nsignals;
	/*�ȴ���Դ��ԭ����Ը����cpu�Ĵ���*/
	long	ru_nvcsw;
	/*ʱ��Ƭ����򱻵��ȵȷ���Ը�л�����*/
	long	ru_nivcsw;
};
/*linux�ṩ����Դ���ƣ�resource limit��rlimit�����ƣ��Խ���ʹ��ϵͳ��Դʩ��ĳЩ���ơ�
ϵͳ����setrlimit��������ǰ���ƣ������ܳ���rlim_maxָ����ֵ��getrlimit���ڼ�鵱ǰ����*/
struct rlimit
{
	/*��ǰ����Դ���ƣ�Ҳ��Ϊ������*/
	unsigned long	rlim_cur;
	/*�����Ƶ����ֵ��Ҳ��ΪӲ����*/
	unsigned long	rlim_max;
};

#define	PRIO_MIN	(-20)
#define	PRIO_MAX	20

#define	PRIO_PROCESS	0
#define	PRIO_PGRP	1
#define	PRIO_USER	2

/*Ĭ��ջ����󳤶ȣ������Ҫ�Ļ�root�û��������Ӹ�ֵ*/
#define _STK_LIM	(8*1024*1024)

/*���ɻ���ҳ����󳤶�*/
#define MLOCK_LIMIT	(8 * PAGE_SIZE)

/*��Ϊ�����Ƶļ����ԣ�ʵ�ʵ���Դ���ڲ�ͬ��linux�汾�п��ܲ�ͬ*/
#include <asm/resource.h>

int getrusage(struct task_struct *p, int who, struct rusage __user *ru);

#endif
