#ifndef _ASM_GENERIC_RESOURCE_H
#define _ASM_GENERIC_RESOURCE_H

/*
 * Resource limit IDs
 *
 * ( Compatibility detail: there are architectures that have
 *   a different rlimit ID order in the 5-9 range and want
 *   to keep that order for binary compatibility. The reasons
 *   are historic and all new rlimits are identical across all
 *   arches. If an arch has such special order for some rlimits
 *   then it defines them prior including asm-generic/resource.h. )
 */
/*�������������cpuʱ��*/
#define RLIMIT_CPU				0
/*���������ļ�����*/
#define RLIMIT_FSIZE			1
/*���ݶε���󳤶�*/
#define RLIMIT_DATA				2
/*�û�̬ջ����󳤶�*/
#define RLIMIT_STACK			3
/*�ڴ�ת���ļ�����󳤶�*/
#define RLIMIT_CORE				4
#ifndef RLIMIT_RSS
/*��פ�ڴ����󳤶ȣ����ǽ���ʹ��ҳ֡�������Ŀ*/
#define RLIMIT_RSS				5
#endif

#ifndef RLIMIT_NPROC
/*���������UID�������û�����ӵ�еĽ��̵������Ŀ*/
#define RLIMIT_NPROC			6
#endif

#ifndef RLIMIT_NOFILE
/*���ļ��������Ŀ*/
#define RLIMIT_NOFILE			7
#endif
/*���ɻ���ҳ�������Ŀ*/
#ifndef RLIMIT_MEMLOCK
/*���ɻ���ҳ�������Ŀ*/
#define RLIMIT_MEMLOCK			8
#endif

#ifndef RLIMIT_AS
/*����ռ�������ַ�ռ����󳤶�*/
#define RLIMIT_AS				9
#endif
/*�ļ����������Ŀ*/
#define RLIMIT_LOCKS			10
/*�����źŵ������Ŀ*/
#define RLIMIT_SIGPENDING		11
/*POSIX��Ϣ���е�����ֽڳ���*/
#define RLIMIT_MSGQUEUE			12
/* max nice prio allowed to raise to 0-39 for nice level 19 .. -20 */
/*��ʵʱ���̵����ȼ�*/
#define RLIMIT_NICE		13	
/*����ʵʱ���ȼ�*/
#define RLIMIT_RTPRIO		14	/* maximum realtime priority */
/*��Դ�����*/
#define RLIM_NLIMITS		15

/*���ڼ�����ԭ����Щ��ϵ�ṹ������Դ���������ֵRLIM_INFINITY*/
#ifndef RLIM_INFINITY
#define RLIM_INFINITY		(~0UL)
#endif

/*�û�ջ��Ĭ�����ֵ����Щ��ϵ�ṹ�ض����ֵ*/
#ifndef _STK_LIM_MAX
#define _STK_LIM_MAX		RLIM_INFINITY
#endif

#ifdef __KERNEL__

/*��ʼ����������ʱĬ�ϵ���Դ����*/
#define INIT_RLIMITS							\
{									\
	[RLIMIT_CPU]		= {  RLIM_INFINITY,  RLIM_INFINITY },	\
	[RLIMIT_FSIZE]		= {  RLIM_INFINITY,  RLIM_INFINITY },	\
	[RLIMIT_DATA]		= {  RLIM_INFINITY,  RLIM_INFINITY },	\
	[RLIMIT_STACK]		= {       _STK_LIM,   _STK_LIM_MAX },	\
	[RLIMIT_CORE]		= {              0,  RLIM_INFINITY },	\
	[RLIMIT_RSS]		= {  RLIM_INFINITY,  RLIM_INFINITY },	\
	[RLIMIT_NPROC]		= {              0,              0 },	\
	[RLIMIT_NOFILE]		= {       INR_OPEN,       INR_OPEN },	\
	[RLIMIT_MEMLOCK]	= {    MLOCK_LIMIT,    MLOCK_LIMIT },	\
	[RLIMIT_AS]		= {  RLIM_INFINITY,  RLIM_INFINITY },	\
	[RLIMIT_LOCKS]		= {  RLIM_INFINITY,  RLIM_INFINITY },	\
	[RLIMIT_SIGPENDING]	= { 		0,	       0 },	\
	[RLIMIT_MSGQUEUE]	= {   MQ_BYTES_MAX,   MQ_BYTES_MAX },	\
	[RLIMIT_NICE]		= { 0, 0 },				\
	[RLIMIT_RTPRIO]		= { 0, 0 },				\
}

#endif	/* __KERNEL__ */

#endif
