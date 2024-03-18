#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

#include <linux/types.h>

#ifdef __KERNEL__
#include <linux/cache.h>
#include <linux/seqlock.h>
#endif

#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec
{
	/*秒*/
	time_t	tv_sec;
	/*纳秒*/
	long	tv_nsec;
};
#endif

struct timeval
{
	/*秒*/
	time_t		tv_sec;
	/*微秒*/
	suseconds_t	tv_usec;
};

struct timezone
{
	/*与格林威治时间相差的分钟*/
	int	tz_minuteswest;
	/*夏令时*/
	int	tz_dsttime;
};

#ifdef __KERNEL__
/*秒、毫秒、微秒、纳秒、皮秒、飞秒、光秒相互转换*/
/*每秒中毫秒数目*/
#define MSEC_PER_SEC	1000L
/*每毫秒中微秒数目*/
#define USEC_PER_MSEC	1000L
/*每微秒中纳秒数目*/
#define NSEC_PER_USEC	1000L
/*每微秒中纳秒数目*/
#define NSEC_PER_MSEC	1000000L
/*每秒中微秒数目*/
#define USEC_PER_SEC	1000000L
/*每秒中纳秒数目*/
#define NSEC_PER_SEC	1000000000L
/*每秒中飞秒数目*/
#define FSEC_PER_SEC	1000000000000000L

/*测试两时间是否相等*/
static inline int timespec_equal(const struct timespec *a,
                                 const struct timespec *b)
{
	return (a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec);
}

/*比较两个struct time_spec类型的时间大小*/
static inline int timespec_compare(const struct timespec *lhs, const struct timespec *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_nsec - rhs->tv_nsec;
}

/*比较两struct time_val类型的时间大小*/
static inline int timeval_compare(const struct timeval *lhs, const struct timeval *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_usec - rhs->tv_usec;
}

extern unsigned long mktime(const unsigned int year, const unsigned int mon, const unsigned int day,
							const unsigned int hour, const unsigned int min, const unsigned int sec);
extern void set_normalized_timespec(struct timespec *ts, time_t sec, long nsec);


/*计算两struct timespec变量的时间差值，并将其组合成常规的同类型变量*/
static inline struct timespec timespec_sub(struct timespec lhs,	struct timespec rhs)
{
	struct timespec ts_delta;
	/*计算输入两参数的时间差，然后格式化为常规值赋值给已同类型变量，返回*/
	set_normalized_timespec(&ts_delta, lhs.tv_sec - rhs.tv_sec, 				lhs.tv_nsec - rhs.tv_nsec);
	return ts_delta;
}

 /*检测struct timespec类型变量的时间取值是否是正常的取值范围，是则返回true，否则返回false*/
#define timespec_valid(ts) ((ts)->tv_sec >= 0) && (((unsigned long) (ts)->tv_nsec) < NSEC_PER_SEC))

extern struct timespec xtime;
extern struct timespec wall_to_monotonic;
extern seqlock_t xtime_lock;

extern unsigned long read_persistent_clock(void);
extern int update_persistent_clock(struct timespec now);
extern int no_sync_cmos_clock __read_mostly;
void timekeeping_init(void);

unsigned long get_seconds(void);
struct timespec current_kernel_time(void);

/*获取当前时间*/
#define CURRENT_TIME		(current_kernel_time())
/*获取当前时间，只取秒*/
#define CURRENT_TIME_SEC	((struct timespec) { get_seconds(), 0 })

extern void do_gettimeofday(struct timeval *tv);
extern int do_settimeofday(struct timespec *tv);
extern int do_sys_settimeofday(struct timespec *tv, struct timezone *tz);
#define do_posix_clock_monotonic_gettime(ts) ktime_get_ts(ts)
extern long do_utimes(int dfd, char __user *filename, struct timespec *times, int flags);
						struct itimerval;
extern int do_setitimer(int which, struct itimerval *value,	struct itimerval *ovalue);
extern unsigned int alarm_setitimer(unsigned int seconds);
extern int do_getitimer(int which, struct itimerval *value);
extern void getnstimeofday(struct timespec *tv);
extern void getboottime(struct timespec *ts);
extern void monotonic_to_bootbased(struct timespec *ts);

extern struct timespec timespec_trunc(struct timespec t, unsigned gran);
extern int timekeeping_is_continuous(void);
extern void update_wall_time(void);


/*将struct timespec时间类型转换成纳秒形式*/
static inline s64 timespec_to_ns(const struct timespec *ts)
{
	return ((s64) ts->tv_sec * NSEC_PER_SEC) + ts->tv_nsec;
}

/*将struct timeval类型的时间转换为纳秒形式*/
static inline s64 timeval_to_ns(const struct timeval *tv)
{
	return ((s64) tv->tv_sec * NSEC_PER_SEC) + tv->tv_usec * NSEC_PER_USEC;
}

extern struct timespec ns_to_timespec(const s64 nsec);
extern struct timeval ns_to_timeval(const s64 nsec);

/*计算*/
static inline void timespec_add_ns(struct timespec *a, u64 ns)
{
	/*先累计纳秒*/
	ns += a->tv_nsec;
	/*如果纳秒值上溢，则转换为秒和纳秒的正常表示形式*/
	while(unlikely(ns >= NSEC_PER_SEC)) {
		ns -= NSEC_PER_SEC;
		a->tv_sec++;
	}
	/*最终计算的后纳秒值*/
	a->tv_nsec = ns;
}
#endif /* __KERNEL__ */

/*一个字长可以表示的文件描述符个数*/
#define NFDBITS			__NFDBITS
/*每个进程可打开文件的上限*/
#define FD_SETSIZE		__FD_SETSIZE
/*设置已打开文件位图*/
#define FD_SET(fd,fdsetp)	__FD_SET(fd,fdsetp)
/*文件指针中对应的已打开文件位图清0*/
#define FD_CLR(fd,fdsetp)	__FD_CLR(fd,fdsetp)
/*文件指针中已打开文件是否被置位*/
#define FD_ISSET(fd,fdsetp)	__FD_ISSET(fd,fdsetp)
/*文件描述符表清0*/
#define FD_ZERO(fdsetp)		__FD_ZERO(fdsetp)

/*内部定时器名称和结构体定义一个定时器设置*/
#define	ITIMER_REAL			0
/*进程的ITIER_VIRTUAL定时器*/
#define	ITIMER_VIRTUAL		1
/*进程的额ITIMER_PROF定时器*/
#define	ITIMER_PROF			2

/**/
struct itimerspec
{
	/**/
	struct timespec it_interval;	/* timer period */
	/**/
	struct timespec it_value;	/* timer expiration */
};

/*定时器属性*/
struct itimerval
{
	/*定时器时间间隔*/
	struct timeval it_interval;
	/*定时器当前值*/
	struct timeval it_value;
};

/*The IDs of the various system clocks (for POSIX.1b interval timers)*/
#define CLOCK_REALTIME			0
#define CLOCK_MONOTONIC			1
#define CLOCK_PROCESS_CPUTIME_ID	2
#define CLOCK_THREAD_CPUTIME_ID		3

/*The IDs of various hardware clocks:*/
#define CLOCK_SGI_CYCLE			10
#define MAX_CLOCKS			16
#define CLOCKS_MASK			(CLOCK_REALTIME | CLOCK_MONOTONIC)
#define CLOCKS_MONO			CLOCK_MONOTONIC

/*The various flags for setting POSIX.1b interval timers:*/
#define TIMER_ABSTIME			0x01

#endif
