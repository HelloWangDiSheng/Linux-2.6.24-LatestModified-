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
	/*��*/
	time_t	tv_sec;
	/*����*/
	long	tv_nsec;
};
#endif

struct timeval
{
	/*��*/
	time_t		tv_sec;
	/*΢��*/
	suseconds_t	tv_usec;
};

struct timezone
{
	/*���������ʱ�����ķ���*/
	int	tz_minuteswest;
	/*����ʱ*/
	int	tz_dsttime;
};

#ifdef __KERNEL__
/*�롢���롢΢�롢���롢Ƥ�롢���롢�����໥ת��*/
/*ÿ���к�����Ŀ*/
#define MSEC_PER_SEC	1000L
/*ÿ������΢����Ŀ*/
#define USEC_PER_MSEC	1000L
/*ÿ΢����������Ŀ*/
#define NSEC_PER_USEC	1000L
/*ÿ΢����������Ŀ*/
#define NSEC_PER_MSEC	1000000L
/*ÿ����΢����Ŀ*/
#define USEC_PER_SEC	1000000L
/*ÿ����������Ŀ*/
#define NSEC_PER_SEC	1000000000L
/*ÿ���з�����Ŀ*/
#define FSEC_PER_SEC	1000000000000000L

/*������ʱ���Ƿ����*/
static inline int timespec_equal(const struct timespec *a,
                                 const struct timespec *b)
{
	return (a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec);
}

/*�Ƚ�����struct time_spec���͵�ʱ���С*/
static inline int timespec_compare(const struct timespec *lhs, const struct timespec *rhs)
{
	if (lhs->tv_sec < rhs->tv_sec)
		return -1;
	if (lhs->tv_sec > rhs->tv_sec)
		return 1;
	return lhs->tv_nsec - rhs->tv_nsec;
}

/*�Ƚ���struct time_val���͵�ʱ���С*/
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


/*������struct timespec������ʱ���ֵ����������ϳɳ����ͬ���ͱ���*/
static inline struct timespec timespec_sub(struct timespec lhs,	struct timespec rhs)
{
	struct timespec ts_delta;
	/*����������������ʱ��Ȼ���ʽ��Ϊ����ֵ��ֵ����ͬ���ͱ���������*/
	set_normalized_timespec(&ts_delta, lhs.tv_sec - rhs.tv_sec, 				lhs.tv_nsec - rhs.tv_nsec);
	return ts_delta;
}

 /*���struct timespec���ͱ�����ʱ��ȡֵ�Ƿ���������ȡֵ��Χ�����򷵻�true�����򷵻�false*/
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

/*��ȡ��ǰʱ��*/
#define CURRENT_TIME		(current_kernel_time())
/*��ȡ��ǰʱ�䣬ֻȡ��*/
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


/*��struct timespecʱ������ת����������ʽ*/
static inline s64 timespec_to_ns(const struct timespec *ts)
{
	return ((s64) ts->tv_sec * NSEC_PER_SEC) + ts->tv_nsec;
}

/*��struct timeval���͵�ʱ��ת��Ϊ������ʽ*/
static inline s64 timeval_to_ns(const struct timeval *tv)
{
	return ((s64) tv->tv_sec * NSEC_PER_SEC) + tv->tv_usec * NSEC_PER_USEC;
}

extern struct timespec ns_to_timespec(const s64 nsec);
extern struct timeval ns_to_timeval(const s64 nsec);

/*����*/
static inline void timespec_add_ns(struct timespec *a, u64 ns)
{
	/*���ۼ�����*/
	ns += a->tv_nsec;
	/*�������ֵ���磬��ת��Ϊ��������������ʾ��ʽ*/
	while(unlikely(ns >= NSEC_PER_SEC)) {
		ns -= NSEC_PER_SEC;
		a->tv_sec++;
	}
	/*���ռ���ĺ�����ֵ*/
	a->tv_nsec = ns;
}
#endif /* __KERNEL__ */

/*һ���ֳ����Ա�ʾ���ļ�����������*/
#define NFDBITS			__NFDBITS
/*ÿ�����̿ɴ��ļ�������*/
#define FD_SETSIZE		__FD_SETSIZE
/*�����Ѵ��ļ�λͼ*/
#define FD_SET(fd,fdsetp)	__FD_SET(fd,fdsetp)
/*�ļ�ָ���ж�Ӧ���Ѵ��ļ�λͼ��0*/
#define FD_CLR(fd,fdsetp)	__FD_CLR(fd,fdsetp)
/*�ļ�ָ�����Ѵ��ļ��Ƿ���λ*/
#define FD_ISSET(fd,fdsetp)	__FD_ISSET(fd,fdsetp)
/*�ļ�����������0*/
#define FD_ZERO(fdsetp)		__FD_ZERO(fdsetp)

/*�ڲ���ʱ�����ƺͽṹ�嶨��һ����ʱ������*/
#define	ITIMER_REAL			0
/*���̵�ITIER_VIRTUAL��ʱ��*/
#define	ITIMER_VIRTUAL		1
/*���̵Ķ�ITIMER_PROF��ʱ��*/
#define	ITIMER_PROF			2

/**/
struct itimerspec
{
	/**/
	struct timespec it_interval;	/* timer period */
	/**/
	struct timespec it_value;	/* timer expiration */
};

/*��ʱ������*/
struct itimerval
{
	/*��ʱ��ʱ����*/
	struct timeval it_interval;
	/*��ʱ����ǰֵ*/
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
