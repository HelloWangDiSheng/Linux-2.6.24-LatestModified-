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
/*按毫秒计算的最大cpu时间*/
#define RLIMIT_CPU				0
/*允许最大的文件长度*/
#define RLIMIT_FSIZE			1
/*数据段的最大长度*/
#define RLIMIT_DATA				2
/*用户态栈的最大长度*/
#define RLIMIT_STACK			3
/*内存转储文件的最大长度*/
#define RLIMIT_CORE				4
#ifndef RLIMIT_RSS
/*常驻内存的最大长度，就是进程使用页帧的最大数目*/
#define RLIMIT_RSS				5
#endif

#ifndef RLIMIT_NPROC
/*与进程真正UID关联的用户可用拥有的进程的最大数目*/
#define RLIMIT_NPROC			6
#endif

#ifndef RLIMIT_NOFILE
/*打开文件的最大数目*/
#define RLIMIT_NOFILE			7
#endif
/*不可换出页的最大数目*/
#ifndef RLIMIT_MEMLOCK
/*不可换出页的最大数目*/
#define RLIMIT_MEMLOCK			8
#endif

#ifndef RLIMIT_AS
/*进程占用虚拟地址空间的最大长度*/
#define RLIMIT_AS				9
#endif
/*文件锁的最大数目*/
#define RLIMIT_LOCKS			10
/*待决信号的最大数目*/
#define RLIMIT_SIGPENDING		11
/*POSIX消息队列的最大字节长度*/
#define RLIMIT_MSGQUEUE			12
/* max nice prio allowed to raise to 0-39 for nice level 19 .. -20 */
/*非实时进程的优先级*/
#define RLIMIT_NICE		13	
/*最大的实时优先级*/
#define RLIMIT_RTPRIO		14	/* maximum realtime priority */
/*资源最大编号*/
#define RLIM_NLIMITS		15

/*出于兼容性原因有些体系结构覆盖资源的最大容许值RLIM_INFINITY*/
#ifndef RLIM_INFINITY
#define RLIM_INFINITY		(~0UL)
#endif

/*用户栈的默认最大值，有些体系结构重定义该值*/
#ifndef _STK_LIM_MAX
#define _STK_LIM_MAX		RLIM_INFINITY
#endif

#ifdef __KERNEL__

/*初始化进程启动时默认的资源限制*/
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
