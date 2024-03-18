#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

/*进程退出时发送的信号*/
#define CSIGNAL						0x000000ff
/*进程间共享虚拟内存地址空间*/
#define CLONE_VM					0x00000100
/*进程间共享文件系统*/
#define CLONE_FS					0x00000200
/*进程间共享已打开文件*/
#define CLONE_FILES					0x00000400
/*进程间共享信号处理函数和被阻塞的信号*/
#define CLONE_SIGHAND				0x00000800
/*父进程被调试时子进程也会被调试*/
#define CLONE_PTRACE				0x00002000
/*创建后的子进程先运行，父进程被阻塞，直到调用mm_release唤醒父进程*/
#define CLONE_VFORK					0x00004000
/*创建进程和被创建进程共有一个父进程，两兄弟进程*/
#define CLONE_PARENT				0x00008000
/*创建进程和被创建进程同属一个线程组*/
#define CLONE_THREAD				0x00010000	/* Same thread group? */
/*进程间不共享命名空间，需要为被创建的进程创建新的命名空间*/
#define CLONE_NEWNS					0x00020000	/* New namespace group? */
/*共享进程间通信机制*/
#define CLONE_SYSVSEM				0x00040000
/*为新进程创建线程本地存储*/
#define CLONE_SETTLS				0x00080000
/*将生成线程的pid复制到clone调用指定的用户应用空间中的某个地址*/
#define CLONE_PARENT_SETTID			0x00100000
/**/
#define CLONE_CHILD_CLEARTID		0x00200000
/*没有使用，忽略*/
#define CLONE_DETACHED				0x00400000
/*新创建的进程不能被调试*/
#define CLONE_UNTRACED				0x00800000
/*将另一个传递到clone的用户空间指针(child_tidptr)保存在新进程的task_struct中*/
#define CLONE_CHILD_SETTID			0x01000000
/*进程被创建后处于停止状态*/
#define CLONE_STOPPED				0x02000000
/*创建新的UTS*/
#define CLONE_NEWUTS				0x04000000
/*创建新的IPC*/
#define CLONE_NEWIPC				0x08000000
/*创建新的user命名空间*/
#define CLONE_NEWUSER				0x10000000
/*创建新的pid命名空间*/
#define CLONE_NEWPID				0x20000000
/*创建新的网络命名空间*/
#define CLONE_NEWNET				0x40000000


/*完全公平调度的略中普通调度*/
#define SCHED_NORMAL		0
/*实时调度策略中的先来先服务调度，该进程一直运行至结束  */
#define SCHED_FIFO			1
/*实时进程的非经典时间片轮转调度  */
#define SCHED_RR			2
/*用于非交互、CPU使用密集的批处理进程，通过完全公平调度器来处理，调度决策对此类进程给与"冷处理"，
它们绝不会抢占CFS调度器处理的另一个进程，因此不会干扰交互式进程，如果不打算用nice降低进程的静态优
先级，同时又不希望该进程影响系统的交互性，最适合用该调度策略*/

#define SCHED_BATCH			3
/*可用于次要的进程，其相对权重总是最小的，也通过完全公平调度器来处理。要注意的是，SCHED_IDLE不负责
调度空闲进程，空闲进程由内核提供单独的机制来处理.只有root用户能通过sched_setscheduler()系统调用来改
变调度策略*/
#define SCHED_IDLE			5

#ifdef __KERNEL__

struct sched_param
{
	int sched_priority;
};

#include <asm/param.h>
#include <linux/capability.h>
#include <linux/threads.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timex.h>
#include <linux/jiffies.h>
#include <linux/rbtree.h>
#include <linux/thread_info.h>
#include <linux/cpumask.h>
#include <linux/errno.h>
#include <linux/nodemask.h>
#include <linux/mm_types.h>

#include <asm/system.h>
#include <asm/semaphore.h>
#include <asm/page.h>
#include <asm/ptrace.h>
#include <asm/cputime.h>

#include <linux/smp.h>
#include <linux/sem.h>
#include <linux/signal.h>
#include <linux/securebits.h>
#include <linux/fs_struct.h>
#include <linux/compiler.h>
#include <linux/completion.h>
#include <linux/pid.h>
#include <linux/percpu.h>
#include <linux/topology.h>
#include <linux/proportions.h>
#include <linux/seccomp.h>
#include <linux/rcupdate.h>
#include <linux/futex.h>
#include <linux/rtmutex.h>
#include <linux/time.h>
#include <linux/param.h>
#include <linux/resource.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/task_io_accounting.h>
#include <linux/kobject.h>
#include <asm/processor.h>

struct exec_domain;
struct futex_pi_state;
struct bio;

/*共享内核线程的复制标识*/
#define CLONE_KERNEL	(CLONE_FS | CLONE_FILES | CLONE_SIGHAND)

/*
 * These are the constant used to fake the fixed-point load-average
 * counting. Some notes:
 *  - 11 bit fractions expand to 22 bits by the multiplies: this gives
 *    a load-average precision of 10 bits integer + 11 bits fractional
 *  - if you want to count load-averages more often, you need more
 *    precision, or rounding will get you. With 2-second counting freq,
 *    the EXP_n values would be 1981, 2034 and 2043 if still using only
 *    11 bit fractions.
 */
extern unsigned long avenrun[];
/*精度需要的比特位数目*/
#define FSHIFT		11
#define FIXED_1		(1<<FSHIFT)	/* 1.0 as fixed-point */
#define LOAD_FREQ	(5*HZ+1)	/* 5 sec intervals */
#define EXP_1		1884		/* 1/exp(5sec/1min) as fixed-point */
#define EXP_5		2014		/* 1/exp(5sec/5min) */
#define EXP_15		2037		/* 1/exp(5sec/15min) */
/*计算系统平均负载*/
#define CALC_LOAD(load,exp,n) load *= exp; load += n*(FIXED_1-exp); load >>= FSHIFT;

extern unsigned long total_forks;
extern int nr_threads;
DECLARE_PER_CPU(unsigned long, process_counts);

extern int nr_processes(void);
extern unsigned long nr_running(void);
extern unsigned long nr_uninterruptible(void);
extern unsigned long nr_active(void);
extern unsigned long nr_iowait(void);
extern unsigned long weighted_cpuload(const int cpu);

struct seq_file;
struct cfs_rq;
struct task_group;
#ifdef CONFIG_SCHED_DEBUG
extern void proc_sched_show_task(struct task_struct *p, struct seq_file *m);
extern void proc_sched_set_task(struct task_struct *p);
extern void
print_cfs_rq(struct seq_file *m, int cpu, struct cfs_rq *cfs_rq);
#else
static inline void
proc_sched_show_task(struct task_struct *p, struct seq_file *m)
{
}
static inline void proc_sched_set_task(struct task_struct *p)
{
}
static inline void
print_cfs_rq(struct seq_file *m, int cpu, struct cfs_rq *cfs_rq)
{
}
#endif

/*进程处于可运行状态，这并不意味着已经实际分配了cpu。进程可能会一直等到调度器选中它，
该状态确保进程可以立即运行，而无需等待外部事件*/
#define TASK_RUNNING				0
/*针对等待某事件或其它资源的睡眠进程设置的。在内核发送信号给该进程，表明事件已经发生时，
进程状态变为TASK_RUNNING，它只要调度器选中该进程即可恢复执行*/
#define TASK_INTERRUPTIBLE			1
/*用于因内核指示而停止的睡眠进程，它们不能有外部信号唤醒，只能有内核亲自唤醒*/
#define TASK_UNINTERRUPTIBLE		2
/*进程特意停止运行。如由调试器暂停*/
#define TASK_STOPPED				4
/*本来不是进程状态，用于从停止的进程中，将当前被调试的那些（使用ptrace机制）与常规的
进程区分开来*/
#define TASK_TRACED					8
/* in tsk->exit_state */
/*僵尸状态*/
#define EXIT_ZOMBIE					16
/*wait系统调用已经发出，而进程完全从系统移除之前的状态，只有多个线程对同一进程发出
wait调用时，该状态才有意义*/
#define EXIT_DEAD					32
/* in tsk->state again */
/**/
#define TASK_DEAD					64

/*设置进程的状态*/
#define __set_task_state(tsk, state_value)	do { (tsk)->state = (state_value); } while (0)

/*包含内存屏障，不论该进程是否处于睡眠状态都能够正确修改*/
#define set_task_state(tsk, state_value)		set_mb((tsk)->state, (state_value))

/*非内存屏障式的设置当前进程的状态*/
#define __set_current_state(state_value) do { current->state = (state_value); } while (0)
/*包含内存屏障，不论当前进程是否处于睡眠状态都能够正确修改*/
#define set_current_state(state_value)	set_mb(current->state, (state_value))

/*进程名称长度*/
#define TASK_COMM_LEN 16

#include <linux/spinlock.h>

/*串行化schedule()和就绪队列的删除与修改操作的保护锁（就绪队列添加操作需要另一个单独锁）*/
extern rwlock_t tasklist_lock;
extern spinlock_t mmlist_lock;

struct task_struct;

extern void sched_init(void);
extern void sched_init_smp(void);
extern void init_idle(struct task_struct *idle, int cpu);
extern void init_idle_bootup_task(struct task_struct *idle);

extern cpumask_t nohz_cpu_mask;
#if defined(CONFIG_SMP) && defined(CONFIG_NO_HZ)
extern int select_nohz_load_balancer(int cpu);
#else
static inline int select_nohz_load_balancer(int cpu)
{
	return 0;
}
#endif

/*进程的内存转储。输入参数0则转储所有进程*/
extern void show_state_filter(unsigned long state_filter);
static inline void show_state(void)
{
	show_state_filter(0);
}
extern void show_regs(struct pt_regs *);

/*栈回溯信息，task指针指向欲回溯的进程，为空时显示当前进程信息，sp是第一个应该显示
栈回溯的栈帧指针，上一个活动记录的栈帧指针*/
extern void show_stack(struct task_struct *task, unsigned long *sp);

void io_schedule(void);
long io_schedule_timeout(long timeout);

extern void cpu_init (void);
extern void trap_init(void);
extern void account_process_tick(struct task_struct *task, int user);
extern void update_process_times(int user);
extern void scheduler_tick(void);

#ifdef CONFIG_DETECT_SOFTLOCKUP
extern void softlockup_tick(void);
extern void spawn_softlockup_task(void);
extern void touch_softlockup_watchdog(void);
extern void touch_all_softlockup_watchdogs(void);
extern int softlockup_thresh;
#else
static inline void softlockup_tick(void)
{
}
static inline void spawn_softlockup_task(void)
{
}
static inline void touch_softlockup_watchdog(void)
{
}
static inline void touch_all_softlockup_watchdogs(void)
{
}
#endif

/*附接到任何在wchan输出中应该被忽略函数*/
/*被该标识标记的函数存储在二进制文件中的".sched.text"代码段中*/
#define __sched		__attribute__((__section__(".sched.text")))

/*保存起始和终止的__sched函数地址*/
extern char __sched_text_start[], __sched_text_end[];

/*测试地址是否在__sched 函数范围内*/
extern int in_sched_functions(unsigned long addr);
/*设置最大调度超时时间*/
#define	MAX_SCHEDULE_TIMEOUT	LONG_MAX
extern signed long FASTCALL(schedule_timeout(signed long timeout));
extern signed long schedule_timeout_interruptible(signed long timeout);
extern signed long schedule_timeout_uninterruptible(signed long timeout);
asmlinkage void schedule(void);

struct nsproxy;
struct user_namespace;

/*活动位图区域的最大数目，是一个大的随机数*/
#define DEFAULT_MAX_MAP_COUNT	65536
extern int sysctl_max_map_count;

#include <linux/aio.h>

extern unsigned long arch_get_unmapped_area(struct file *, unsigned long, unsigned long,
		       										unsigned long, unsigned long);
extern unsigned long arch_get_unmapped_area_topdown(struct file *filp, unsigned long addr,
			  							unsigned long len, unsigned long pgoff, unsigned long flags);
extern void arch_unmap_area(struct mm_struct *, unsigned long);
extern void arch_unmap_area_topdown(struct mm_struct *, unsigned long);

#if NR_CPUS >= CONFIG_SPLIT_PTLOCK_CPUS
/*内存计数器没有受到page_table_lock保护，因此必须自动地自增*/
/*设置/获取/增加/自增/自减mm计数器的值*/
#define set_mm_counter(mm, member, value) atomic_long_set(&(mm)->_##member, value)
#define get_mm_counter(mm, member) ((unsigned long)atomic_long_read(&(mm)->_##member))
#define add_mm_counter(mm, member, value) atomic_long_add(value, &(mm)->_##member)
#define inc_mm_counter(mm, member) atomic_long_inc(&(mm)->_##member)
#define dec_mm_counter(mm, member) atomic_long_dec(&(mm)->_##member)

#else  /* NR_CPUS < CONFIG_SPLIT_PTLOCK_CPUS */
/*内存计数器受到page_table_lock的保护，可以直接自增*/
#define set_mm_counter(mm, member, value) (mm)->_##member = (value)
#define get_mm_counter(mm, member) ((mm)->_##member)
#define add_mm_counter(mm, member, value) (mm)->_##member += (value)
#define inc_mm_counter(mm, member) (mm)->_##member++
#define dec_mm_counter(mm, member) (mm)->_##member--

#endif /* NR_CPUS < CONFIG_SPLIT_PTLOCK_CPUS */
/*获取文件映射和匿名映射常驻内存集大小之和*/
#define get_mm_rss(mm)					\
	(get_mm_counter(mm, file_rss) + get_mm_counter(mm, anon_rss))
/*获取文件和匿名映射常驻内存集之和，如果该值大于高水印时的常驻内存集大小，则赋值给常驻内存集*/
#define update_hiwater_rss(mm)	do {			\
	unsigned long _rss = get_mm_rss(mm);		\
	if ((mm)->hiwater_rss < _rss)				\
		(mm)->hiwater_rss = _rss;				\
} while (0)
/*更新高水印虚拟内存页数目，*/
#define update_hiwater_vm(mm)	do {			\
	if ((mm)->hiwater_vm < (mm)->total_vm)		\
		(mm)->hiwater_vm = (mm)->total_vm;	\
} while (0)

extern void set_dumpable(struct mm_struct *mm, int value);
extern int get_dumpable(struct mm_struct *mm);

/* mm flags */
/*允许内核转储*/
#define MMF_DUMPABLE      				0
/*只有root可读内核转储文件*/
#define MMF_DUMP_SECURELY 				1
#define MMF_DUMPABLE_BITS 				2

/*内核转储过滤位*/
#define MMF_DUMP_ANON_PRIVATE			2
#define MMF_DUMP_ANON_SHARED			3
#define MMF_DUMP_MAPPED_PRIVATE			4
#define MMF_DUMP_MAPPED_SHARED			5
#define MMF_DUMP_ELF_HEADERS			6
#define MMF_DUMP_FILTER_SHIFT			MMF_DUMPABLE_BITS
#define MMF_DUMP_FILTER_BITS			5
#define MMF_DUMP_FILTER_MASK (((1 << MMF_DUMP_FILTER_BITS) - 1) << MMF_DUMP_FILTER_SHIFT)
#define MMF_DUMP_FILTER_DEFAULT	((1 << MMF_DUMP_ANON_PRIVATE) |	(1 << MMF_DUMP_ANON_SHARED))
/*管理设置的信号处理程序的信息*/
struct sighand_struct
{
	/*共享该结构的进程数目*/
	atomic_t		count;
	/*信号处理程序*/
	struct k_sigaction	action[_NSIG];
	/*保护锁*/
	spinlock_t		siglock;
	/*等待队列*/
	wait_queue_head_t	signalfd_wqh;
};

struct pacct_struct
{
	/**/
	int			ac_flag;
	/**/
	long			ac_exitcode;
	/**/
	unsigned long		ac_mem;
	/**/
	cputime_t		ac_utime, ac_stime;
	/**/
	unsigned long		ac_minflt, ac_majflt;
};

/*
 * NOTE! "signal_struct" does not have it's own
 * locking, because a shared signal_struct always
 * implies a shared sighand_struct, so locking
 * sighand_struct is always a proper superset of
 * the locking of signal_struct.
 */
/*信号描述符自己没有锁，因为一个共享的信号描述符经常implies一个共享的信号处理描述符。
因此锁定信号处理描述符被认为是锁定信号描述符的超集*/
struct signal_struct
{
	/**/
	atomic_t		count;
	/**/
	atomic_t		live;
	/**/
	wait_queue_head_t	wait_chldexit;	/* for wait4() */
	/* current thread group signal load-balancing target: */
	/**/
	struct task_struct	*curr_target;

	/* shared signal handling: */
	/**/
	struct sigpending	shared_pending;

	/* thread group exit support */
	/**/
	int			group_exit_code;
	/* overloaded:
	 * - notify group_exit_task when ->count is equal to notify_count
	 * - everyone except group_exit_task is stopped during signal delivery
	 *   of fatal signals, group_exit_task processes the signal.
	 */
	/**/
	struct task_struct	*group_exit_task;
	/**/
	int notify_count;

	/* thread group stop support, overloads group_exit_code too */
	/**/
	int			group_stop_count;
	/**/
	unsigned int		flags; /* see SIGNAL_* flags below */

	/* POSIX.1b Interval Timers */
	/**/
	struct list_head posix_timers;

	/* ITIMER_REAL timer for the process */
	/*进程的ITIMER_REAL定时器*/
	struct hrtimer real_timer;
	/**/
	struct task_struct *tsk;
	/**/
	ktime_t it_real_incr;

	/* ITIMER_PROF and ITIMER_VIRTUAL timers for the process */
	/*进程的ITIMER_PROF和ITIMER_VIRTUAL定时器中下一次定时器到期的时间*/
	cputime_t it_prof_expires, it_virt_expires;
	/*定时器在多长时间之后调用*/
	cputime_t it_prof_incr, it_virt_incr;

	/* job control IDs */

	/*
	 * pgrp and session fields are deprecated.
	 * use the task_session_Xnr and task_pgrp_Xnr routines below
	 */

	union
	{
		/**/
		pid_t pgrp __deprecated;
		/*进程组ID*/
		pid_t __pgrp;
	};
	/**/
	struct pid *tty_old_pgrp;

	union
	{
		/**/
		pid_t session __deprecated;
		/*会话ID*/
		pid_t __session;
	};

	/* boolean value for session group leader */
	/**/
	int leader;
	/**/
	struct tty_struct *tty; /* NULL if no tty */

	/*
	 * Cumulative resource counters for dead threads in the group,
	 * and for reaped dead child processes forked by this group.
	 * Live threads maintain their own counters and add to these
	 * in __exit_signal, except for the group leader.
	 */
	/*组内已死亡的线程和该组内创建的已死亡的孤儿线程累计资源计数器*/
	/*本次在用户态运行的时间*/
	cputime_t utime;
	/*本次在内核态运行的时间*/
	cputime_t stime;
	/*总共在用户态运行的时间*/
	cputime_t cutime;
	/*总共在内核态运行的时间*/
	cputime_t cstime;
	/**/
	cputime_t gtime;
	/**/
	cputime_t cgtime;
	/*因为等待资源等原因放弃cpu的次数*/
	unsigned long nvcsw;
	/*因为时间片用尽被调度或被抢占而切换的次数*/
	unsigned long nivcsw;
	/*自愿上下文切换总数*/
	unsigned long cnvcsw;
	/*非自愿上下文切换总数*/
	unsigned long cnivcsw;
	/**/
	unsigned long min_flt;
	/*缺页中断的次数*/
	unsigned long maj_flt;
	/**/
	unsigned long cmin_flt;
	/**/
	unsigned long cmaj_flt;
	/**/
	unsigned long inblock;
	/**/
	unsigned long oublock;
	/**/
	unsigned long cinblock;
	/**/
	unsigned long coublock;

	/*
	 * Cumulative ns of scheduled CPU time for dead threads in the
	 * group, not including a zombie group leader.  (This only differs
	 * from jiffies_to_ns(utime + stime) if sched_clock uses something
	 * other than jiffies.)
	 */
	/**/
	unsigned long long sum_sched_runtime;

	/*
	 * We don't bother to synchronize most readers of this at all,
	 * because there is no reader checking a limit that actually needs
	 * to get both rlim_cur and rlim_max atomically, and either one
	 * alone is a single word that can safely be read normally.
	 * getrlimit/setrlimit use task_lock(current->group_leader) to
	 * protect this instead of the siglock, because they really
	 * have no need to disable irqs.
	 */
	/*进程资源限制*/
	struct rlimit rlim[RLIM_NLIMITS];

	/**/
	struct list_head cpu_timers[3];

	/* keep the process-shared keyrings here so that they do the right
	 * thing in threads created with CLONE_THREAD */
#ifdef CONFIG_KEYS
	/**/
	struct key *session_keyring;	/* keyring inherited over fork */
	/**/
	struct key *process_keyring;	/* keyring private to this process */
#endif
#ifdef CONFIG_BSD_PROCESS_ACCT
	/**/
	struct pacct_struct pacct;	/* per-process accounting information */
#endif
#ifdef CONFIG_TASKSTATS
	/**/
	struct taskstats *stats;
#endif
#ifdef CONFIG_AUDIT
	/**/
	unsigned audit_tty;
	/**/
	struct tty_audit_buf *tty_audit_buf;
#endif
};

/* Context switch must be unlocked if interrupts are to be enabled */
#ifdef __ARCH_WANT_INTERRUPTS_ON_CTXSW
# define __ARCH_WANT_UNLOCKED_CTXSW
#endif

/*
 * Bits in flags field of signal_struct.
 */
/**/
#define SIGNAL_STOP_STOPPED	0x00000001 /* job control stop in effect */
/**/
#define SIGNAL_STOP_DEQUEUED	0x00000002 /* stop signal dequeued */
/**/
#define SIGNAL_STOP_CONTINUED	0x00000004 /* SIGCONT since WCONTINUED reap */
/**/
#define SIGNAL_GROUP_EXIT	0x00000008 /* group exit in progress */

/*
 * Some day this will be a full-fledged user tracking system..
 */
 /*该结构威武了一些统计数据，每个用户命名空间对其用户资源使用的统计，与其它命名空间
完全无关，对root用户的统计也是如此，这是因为，在克隆一个用户命名空间时，为当前用户和
root用户都创建了新的user_struct实例*/
struct user_struct
{
	/*该结构被引用的次数*/
	atomic_t __count;
	/*用户拥有的进程数目*/
	atomic_t processes;
	/*该用户已打开文件的数目*/
	atomic_t files;
	/*该用户还有待处理信号的数目*/
	atomic_t sigpending;
#ifdef CONFIG_INOTIFY_USER
	/**/
	atomic_t inotify_watches; /* How many inotify watches does this user have? */
	/**/
	atomic_t inotify_devs;	/* How many inotify devs does this user have opened? */
#endif
#ifdef CONFIG_POSIX_MQUEUE
	/* protected by mq_lock	*/
	/**/
	unsigned long mq_bytes;	/* How many bytes can be allocated to mqueue? */
#endif
	/**/
	unsigned long locked_shm; /* How many pages of mlocked shm ? */

#ifdef CONFIG_KEYS
	/**/
	struct key *uid_keyring;	/* UID specific keyring */
	/**/
	struct key *session_keyring;	/* UID's default session keyring */
#endif

	/* Hash table maintenance information */
	/*将用户添加到用户命名空间中散列表*/
	struct hlist_node uidhash_node;
	/*用户id*/
	uid_t uid;

#ifdef CONFIG_FAIR_USER_SCHED
	struct task_group *tg;
#ifdef CONFIG_SYSFS
	struct kset kset;
	struct subsys_attribute user_attr;
	struct work_struct work;
#endif
#endif
};

#ifdef CONFIG_FAIR_USER_SCHED
extern int uids_kobject_init(void);
#else
static inline int uids_kobject_init(void) { return 0; }
#endif

extern struct user_struct *find_user(uid_t);
extern struct user_struct root_user;
#define INIT_USER (&root_user)

struct backing_dev_info;
struct reclaim_state;

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
struct sched_info {
	/* cumulative counters */
	unsigned long pcount;	      /* # of times run on this cpu */
	unsigned long long cpu_time,  /* time spent on the cpu */
			   run_delay; /* time spent waiting on a runqueue */

	/* timestamps */
	unsigned long long last_arrival,/* when we last ran on a cpu */
			   last_queued;	/* when we were last queued to run */
#ifdef CONFIG_SCHEDSTATS
	/* BKL stats */
	unsigned int bkl_count;
#endif
};
#endif /* defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT) */

#ifdef CONFIG_SCHEDSTATS
extern const struct file_operations proc_schedstat_operations;
#endif /* CONFIG_SCHEDSTATS */

#ifdef CONFIG_TASK_DELAY_ACCT
struct task_delay_info {
	spinlock_t	lock;
	unsigned int	flags;	/* Private per-task flags */

	/* For each stat XXX, add following, aligned appropriately
	 *
	 * struct timespec XXX_start, XXX_end;
	 * u64 XXX_delay;
	 * u32 XXX_count;
	 *
	 * Atomicity of updates to XXX_delay, XXX_count protected by
	 * single lock above (split into XXX_lock if contention is an issue).
	 */

	/*
	 * XXX_count is incremented on every XXX operation, the delay
	 * associated with the operation is added to XXX_delay.
	 * XXX_delay contains the accumulated delay time in nanoseconds.
	 */
	struct timespec blkio_start, blkio_end;	/* Shared by blkio, swapin */
	u64 blkio_delay;	/* wait for sync block io completion */
	u64 swapin_delay;	/* wait for swapin block io completion */
	u32 blkio_count;	/* total count of the number of sync block */
				/* io operations performed */
	u32 swapin_count;	/* total count of the number of swapin block */
				/* io operations performed */
};
#endif	/* CONFIG_TASK_DELAY_ACCT */

static inline int sched_info_on(void)
{
#ifdef CONFIG_SCHEDSTATS
	return 1;
#elif defined(CONFIG_TASK_DELAY_ACCT)
	extern int delayacct_on;
	return delayacct_on;
#else
	return 0;
#endif
}

enum cpu_idle_type
{
	/**/
	CPU_IDLE,
	/**/
	CPU_NOT_IDLE,
	/**/
	CPU_NEWLY_IDLE,
	/**/
	CPU_MAX_IDLE_TYPES
};

/*
 * sched-domains (multiprocessor balancing) declarations:
 */

/*
 * Increase resolution of nice-level calculations:
 */
#define SCHED_LOAD_SHIFT	10
#define SCHED_LOAD_SCALE	(1L << SCHED_LOAD_SHIFT)

#define SCHED_LOAD_SCALE_FUZZ	SCHED_LOAD_SCALE

#ifdef CONFIG_SMP
/*在当前作用域负载均衡*/
#define SD_LOAD_BALANCE		1	/* Do load balancing on this domain. */
/*空闲时负载均衡*/
#define SD_BALANCE_NEWIDLE	2	/* Balance when about to become idle */
/*exec时负载均衡*/
#define SD_BALANCE_EXEC		4	/* Balance on exec */
/*fork或clone时负载均衡*/
#define SD_BALANCE_FORK		8	/* Balance on fork, clone */
/*进程唤醒时唤醒空闲cpu*/
#define SD_WAKE_IDLE		16	/* Wake to idle CPU on task wakeup */
/*唤醒函数唤醒cpu*/
#define SD_WAKE_AFFINE		32	/* Wake task to waking CPU */
/*进程唤醒时执行负载均衡*/
#define SD_WAKE_BALANCE		64	/* Perform balancing at task wakeup */
/**/
#define SD_SHARE_CPUPOWER	128	/* Domain members share cpu power */
/*为节约电源而均衡*/
#define SD_POWERSAVINGS_BALANCE	256	/* Balance for power savings */
/**/
#define SD_SHARE_PKG_RESOURCES	512	/* Domain members share cpu pkg resources */
/*仅有一个负载均衡实例*/
#define SD_SERIALIZE		1024	/* Only a single load balancing instance */
/**/
#define BALANCE_FOR_MC_POWER	(sched_smt_power_savings ? SD_POWERSAVINGS_BALANCE : 0)
/**/
#define BALANCE_FOR_PKG_POWER	((sched_mc_power_savings || sched_smt_power_savings) ?	\
	 									SD_POWERSAVINGS_BALANCE : 0)
/**/
#define test_sd_parent(sd, flag)	((sd->parent && (sd->parent->flags & flag)) ? 1 : 0)

/**/
struct sched_group
{
	/**/
	struct sched_group *next;	/* Must be a circular list */
	/**/
	cpumask_t cpumask;

	/*
	 * CPU power of this group, SCHED_LOAD_SCALE being max power for a
	 * single CPU. This is read only (except for setup, hotplug CPU).
	 * Note : Never change cpu_power without recompute its reciprocal
	 */
	 /**/
	unsigned int __cpu_power;
	/*
	 * reciprocal value of cpu_power to avoid expensive divides
	 * (see include/linux/reciprocal_div.h)
	 */
	 /**/
	u32 reciprocal_cpu_power;
};
/**/
struct sched_domain
{
	/* These fields must be setup */
	/*调度域父节点，最高调度域必须以NULL终结*/
	struct sched_domain *parent;
	/*调度域子节点，最低调度域必须以NULL终结*/
	struct sched_domain *child;
	/*调度域的组均衡*/
	struct sched_group *groups;
	/**/
	cpumask_t span;			/* span of all CPUs in this domain */
	/*最小间隔均衡时间，ms单位*/
	unsigned long min_interval;	/* Minimum balance interval ms */
	/*最大均衡间隔时间，ms单位*/
	unsigned long max_interval;	/* Maximum balance interval ms */
	/**/
	unsigned int busy_factor;	/* less balancing by factor if busy */
	/**/
	unsigned int imbalance_pct;	/* No balance until over watermark */
	/**/
	unsigned int cache_nice_tries;	/* Leave cache hot tasks for # tries */
	/**/
	unsigned int busy_idx;
	/**/
	unsigned int idle_idx;
	/**/
	unsigned int newidle_idx;
	/**/
	unsigned int wake_idx;
	/**/
	unsigned int forkexec_idx;
	/**/
	int flags;			/* See SD_* */

	/* Runtime fields. */
	/**/
	unsigned long last_balance;	/* init to jiffies. units in jiffies */
	/**/
	unsigned int balance_interval;	/* initialise to 1. units in ms. */
	/**/
	unsigned int nr_balance_failed; /* initialise to 0 */

#ifdef CONFIG_SCHEDSTATS
	/* load_balance() stats */
	unsigned int lb_count[CPU_MAX_IDLE_TYPES];
	unsigned int lb_failed[CPU_MAX_IDLE_TYPES];
	unsigned int lb_balanced[CPU_MAX_IDLE_TYPES];
	unsigned int lb_imbalance[CPU_MAX_IDLE_TYPES];
	unsigned int lb_gained[CPU_MAX_IDLE_TYPES];
	unsigned int lb_hot_gained[CPU_MAX_IDLE_TYPES];
	unsigned int lb_nobusyg[CPU_MAX_IDLE_TYPES];
	unsigned int lb_nobusyq[CPU_MAX_IDLE_TYPES];

	/* Active load balancing */
	unsigned int alb_count;
	unsigned int alb_failed;
	unsigned int alb_pushed;

	/* SD_BALANCE_EXEC stats */
	unsigned int sbe_count;
	unsigned int sbe_balanced;
	unsigned int sbe_pushed;

	/* SD_BALANCE_FORK stats */
	unsigned int sbf_count;
	unsigned int sbf_balanced;
	unsigned int sbf_pushed;

	/* try_to_wake_up() stats */
	unsigned int ttwu_wake_remote;
	unsigned int ttwu_move_affine;
	unsigned int ttwu_move_balance;
#endif
};

extern void partition_sched_domains(int ndoms_new, cpumask_t *doms_new);

#endif	/* CONFIG_SMP */

/*
 * A runqueue laden with a single nice 0 task scores a weighted_cpuload of
 * SCHED_LOAD_SCALE. This function returns 1 if any cpu is laden with a
 * task of nice 0 or enough lower priority tasks to bring up the
 * weighted_cpuload
 */
static inline int above_background_load(void)
{
	unsigned long cpu;

	for_each_online_cpu(cpu)
	{
		if (weighted_cpuload(cpu) >= SCHED_LOAD_SCALE)
			return 1;
	}
	return 0;
}

struct io_context;			/* See blkdev.h */
#define NGROUPS_SMALL		32
#define NGROUPS_PER_BLOCK	((int)(PAGE_SIZE / sizeof(gid_t)))
/**/
struct group_info
{
	/**/
	int ngroups;
	/**/
	atomic_t usage;
	/**/
	gid_t small_block[NGROUPS_SMALL];
	/**/
	int nblocks;
	/**/
	gid_t *blocks[0];
};

/*
 * get_group_info() must be called with the owning task locked (via task_lock())
 * when task != current.  The reason being that the vast majority of callers are
 * looking at current->group_info, which can not be changed except by the
 * current task.  Changing current->group_info requires the task lock, too.
 */
 /*引用组信息*/
 #define get_group_info(group_info) do { \
	atomic_inc(&(group_info)->usage); 			\
} while (0)
/*解除对组信息的引用，组信息没有被引用时则释放该组信息*/
#define put_group_info(group_info) do { 						\
	if (atomic_dec_and_test(&(group_info)->usage)) 				\
		groups_free(group_info); 								\
} while (0)

extern struct group_info *groups_alloc(int gidsetsize);
extern void groups_free(struct group_info *group_info);
extern int set_current_groups(struct group_info *group_info);
extern int groups_search(struct group_info *group_info, gid_t grp);
/* access the groups "array" with this macro */
#define GROUP_AT(gi, i)    ((gi)->blocks[(i)/NGROUPS_PER_BLOCK][(i)%NGROUPS_PER_BLOCK])

#ifdef ARCH_HAS_PREFETCH_SWITCH_STACK
extern void prefetch_stack(struct task_struct *t);
#else
static inline void prefetch_stack(struct task_struct *t) { }
#endif

struct audit_context;		/* See audit.c */
struct mempolicy;
struct pipe_inode_info;
struct uts_namespace;

struct rq;
struct sched_domain;

/*调度类*/
struct sched_class
{
	/*下一个调度类*/
	const struct sched_class *next;
	/*将进程插入就绪队列*/
	void (*enqueue_task) (struct rq *rq, struct task_struct *p, int wakeup);
	/*将进程从就绪队列上删除*/
	void (*dequeue_task) (struct rq *rq, struct task_struct *p, int sleep);
	/*就绪队列当前运行进程放弃cpu，重新在就绪队列上排队*/
	void (*yield_task) (struct rq *rq);
	/*查看进程是否可以抢占就绪队列当前正在运行的进程*/
	void (*check_preempt_curr) (struct rq *rq, struct task_struct *p);
	/*设置下一个将要运行的进程*/
	struct task_struct * (*pick_next_task) (struct rq *rq);
	/**/
	void (*put_prev_task) (struct rq *rq, struct task_struct *p);

#ifdef CONFIG_SMP
	/*多处理器系统上，内核必须考虑几个额外的问题，以确保良好的调度（1）CPU负荷必须尽
	可能公平地在所有的处理器上共享。如果一个处理器负责3个并发的应用程序，而另一个只能
	处理空闲进程，那是没有意义的（2）进程与系统中某些处理器的亲合性（affinity）必须是
	可设置的。例如在4个CPU系统中，可以将计算密集型应用程序绑定到前3个CPU，而剩余的（交
	互式）进程则在第4个CPU上运行（3）内核必须能够将进程从一个CPU迁移到另一个。但该选项
	必须谨慎使用，因为它会严重危害性能。在小型SMP系统上CPU高速缓存是最大的问题。对于真
	正大型系统，CPU与迁移进程此前使用的物理内存距离可能有若干米，因此对该进程内存的访问
	代价高昂。进程对特定CPU的亲合性，定义在task_struct的cpus_allowed成员中。Linux提供了
	sched_setaffinity系统调用，可修改进程与CPU的现有分配关系。在SMP系统上，每个调度器类
	的调度方法必须增加两个额外的函数：load_balance和move_one_task。这些函数并不直接负责
	处理负载均衡。每当内核认为有必要重新均衡时，核心调度器代码都会调用这些函数。特定于
	调度器类的函数接下来建立一个迭代器，使得核心调度器能够遍历所有可能迁移到另一个队列
	的备选进程，但各个调度器类的内部结构不能因为迭代器而暴露给核心调度器。load_balance
	函数指针采用了一般性的函数load_balance，而move_one_task则使用了iter_move_one_task。
	这些函数用于不同的目的。iter_move_one_task从最忙碌的就绪队列移出一个进程，迁移到当
	前CPU的就绪队列。load_balance则允许从最忙的就绪队列分配多个进程到当前CPU，但移动的
	负荷不能比max_load_move更多。
	负载均衡处理过程是如何发起的？在SMP系统上，周期性调度器函数scheduler_tick按上文所
	述完成所有系统都需要的任务之后，会调用trigger_load_balance函数。这会引发SCHEDULE_
	SOFTIRQ软中断softIRQ（硬件中断的软件模拟），该中断确保会在适当的时机执行
	run_rebalance_domains。该函数最终对当前CPU调用rebalance_domains，实现负载均衡。
	*/

	/**/
	unsigned long (*load_balance) (struct rq *this_rq, int this_cpu,
			struct rq *busiest, unsigned long max_load_move,
			struct sched_domain *sd, enum cpu_idle_type idle,
			int *all_pinned, int *this_best_prio);
	/**/
	int (*move_one_task) (struct rq *this_rq, int this_cpu,
			      struct rq *busiest, struct sched_domain *sd,
			      enum cpu_idle_type idle);
#endif
	/**/
	void (*set_curr_task) (struct rq *rq);
	/**/
	void (*task_tick) (struct rq *rq, struct task_struct *p);
	/**/
	void (*task_new) (struct rq *rq, struct task_struct *p);
};
/*进程的重要性不仅由优先级指定。而且还需要考虑保存在task_struct->se.load的负荷权重
，set_load_weight负责根据进程类型及其静态优先级计算负荷权重*/
struct load_weight
{
	/*负荷权重*/
	unsigned long weight;
	/*保存负荷权重的倒数。由于使用了普通的long类型，内核无法直接存储1/weight，必须借助于乘法和移位来执行
	除法*/
	unsigned long inv_weight;
};

/*
 * CFS stats for a schedulable entity (task, task-group etc)
 *
 * Current field usage histogram:
 *
 *     4 se->block_start
 *     4 se->run_node
 *     4 se->sleep_start
 *     6 se->load.weight
 */
 /*调度器可以操作比进程更一般的实体，因此需要一个适当的数据结构来描述此类实体，定义如下*/
struct sched_entity
{
	/*负荷权重，用于负载均衡。权重决定了各个实体占队列总负荷的比例。计算负荷权重是调度器
	的一项重任，因为CFS所需的虚拟时钟的速度最终依赖与负荷*/
	struct load_weight	load;
	/*标准树结点，使得实体可以在就绪队列上接受调度*/
	struct rb_node		run_node;
	/*该实体当前是否已在就绪队列上接受调度*/
	unsigned int		on_rq;
	/*实体开始执行的时间*/
	u64			exec_start;
	/*记录使用的CPU时间，已用于完全公平调度器*/
	u64			sum_exec_runtime;
	/*进程执行期间虚拟时钟上流失的时间数量*/
	u64			vruntime;
	/*在进程被撤销cpu时，其当前的sum_exec_runtime值累计和，该值持续单调增长，也就是进程
	之前总计运行的时间*/
	u64			prev_sum_exec_runtime;
/*编译内核时启用了调度器统计时包含的统计成员*/
#ifdef CONFIG_SCHEDSTATS
	u64			wait_start;
	u64			wait_max;

	u64			sleep_start;
	u64			sleep_max;
	s64			sum_sleep_runtime;

	u64			block_start;
	u64			block_max;
	u64			exec_max;
	u64			slice_max;

	u64			nr_migrations;
	u64			nr_migrations_cold;
	u64			nr_failed_migrations_affine;
	u64			nr_failed_migrations_running;
	u64			nr_failed_migrations_hot;
	u64			nr_forced_migrations;
	u64			nr_forced2_migrations;

	u64			nr_wakeups;
	u64			nr_wakeups_sync;
	u64			nr_wakeups_migrate;
	u64			nr_wakeups_local;
	u64			nr_wakeups_remote;
	u64			nr_wakeups_affine;
	u64			nr_wakeups_affine_attempts;
	u64			nr_wakeups_passive;
	u64			nr_wakeups_idle;
#endif
/*启用组调度时包含的的统计成员*/
#ifdef CONFIG_FAIR_GROUP_SCHED
	struct sched_entity	*parent;
	/* rq on which this entity is (to be) queued: */
	struct cfs_rq		*cfs_rq;
	/* rq "owned" by this entity/group: */
	struct cfs_rq		*my_q;
#endif
};

struct task_struct
{
	/*进程当前的状态，-1不可运行 0可运行 >0停止*/
	volatile long state;
	/*进程的内核栈。通过系统调用进入内核空间时，保存进程信息，主要是寄存器信息，退出内核态时
	，从内核栈读取数据，继续执行。通过alloc_thread_info申请2（默认）页，通过free_thread_info释放*/
	void *stack;
	/*进程描述符使用计数，2表示进程正在被使用*/
	atomic_t usage;
	/*PF_开头的进程标识*/
	unsigned int flags;
	/*是否启用调试功能，0表示不启用*/
	unsigned int ptrace;
	/*获取大内核锁的次数，-1表示进程未获得锁*/
	int lock_depth;

#ifdef CONFIG_SMP
#ifdef __ARCH_WANT_UNLOCKED_CTXSW
	/*体系结构无锁进程切换时确定进程运行所在的cpu*/
	int oncpu;
#endif
#endif
	/*调度器使用的进程的动态优先级，有些情况下需要暂时提高进程的优先级（解决优先级反转情况）
	，因为不是持久的，静态和普通优先级不变*/
	int prio;
	/*静态优先级是启动时分配的优先级，可用nice和sched_setscheduler修改，否则执行期间不变*/
	int static_prio;
	/*根据进程的静态优先级和调度策略确定，即使普通进程和实时进程具有相同的静态优先级，其
	普通优先级也是不同的，进程被创建（fork）时，继承创建进程的普通优先级*/
	int normal_prio;
	/**/
	struct list_head run_list;
	/*进程所属的调度类{stop/rt/fair/idle}_sched_class*/
	const struct sched_class *sched_class;
	/* 用于普通进程的调度实体。调度器不限于调度进程，还可以处理更大的实体。可实现组调度
	（可用cpu时间首先在一般的进程组之间分配，组得到的cpu时间再在组内再次分配）。这种一般
	要求调度器不直接操作进程，而是处理可调度实体，一个实体有sched_entity实例，调度器可据
	此操作各个task_struct*/
	struct sched_entity se;

#ifdef CONFIG_PREEMPT_NOTIFIERS
	/*当前进程被抢占或重调度时保存相关信息，散列表头*/
	struct hlist_head preempt_notifiers;
#endif
	/*进程输入输出的优先级*/
	unsigned short ioprio;
	/*
	 * fpu_counter contains the number of consecutive context switches
	 * that the FPU is used. If this is over a threshold, the lazy fpu
	 * saving becomes unlazy to save the trap. This is an unsigned char
	 * so that after 256 times the counter wraps and the behavior turns
	 * lazy again; this to deal with bursty apps that only use FPU for
	 * a short time
	 */
	 /*fpu使用时上下文切换次数*/
	unsigned char fpu_counter;
	/**/
	s8 oomkilladj; /* OOM kill score adjustment (bit shift). */
#ifdef CONFIG_BLK_DEV_IO_TRACE
	/*块设备io层跟踪计数*/
	unsigned int btrace_seq;
#endif
	/*调度策略*/
	unsigned int policy;
	/*cpu位图，在多处理器上使用，用于控制进程可以在哪个CPU上运行*/
	cpumask_t cpus_allowed;
	/*SCHED_RR调度时的时间片*/
	unsigned int time_slice;

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	/*调度器统计进程运行时相关信息*/
	struct sched_info sched_info;
#endif
	/*将当前进程的task_struct串联到内核进程列表中，构建进程链表*/
	struct list_head tasks;
	/*
	 * ptrace_list/ptrace_children forms the list of my children
	 * that were stolen by a ptracer.
	 */
	 /*所有被调试的子进程链表*/
	struct list_head ptrace_children;
	/*调试进程链表*/
	struct list_head ptrace_list;
	/*mm指向进程所拥有的内存描述符，对于普通进程而言，这两个指针变量的值相同，内核线程不拥
	有任何内存描述符，所以，它的mm成员总是空，当内核线程运行时,它的active_mm成员被初始化为
	前一个运行进程的active_mm值*/
	struct mm_struct *mm;
	/*active_mm指向进程运行时所使用的内存描述符*/
	struct mm_struct *active_mm;

	/*Linux支持的可执行二进制文件文件*/
	struct linux_binfmt *binfmt;
	/*进程的退出状态码*/
	int exit_state;
	/*用于设置进程的终止代码，这个值要么是_exit()或exit_group系统调用（正常终止），要么是
	由内核提供的一个错误代号（异常终止）*/
	int exit_code;
	/*-1表示是某个线程组中的一员，只有当线程组的最后一个成员终止时，才会产生一个信号，以
	通知线程组组长的父进程*/
	int exit_signal;
	/*父进程终止时发送的信号*/
	int pdeath_signal;
	/*用于处理不同ABI*/
	unsigned int personality;
	/*进程创建后是否已执行exec函数*/
	unsigned did_exec:1;
	/*进程的全局PID*/
	pid_t pid;
	/*进程的全局线程组组长PID，在Linux系统中，一个线程组中的所有线程使用和该线程组的组长
	(该组中的第一个轻量级进程)相同的PID，并被存放在tgid成员中。只有线程组组长的pid成员才会
	被设置为与tgid相同的值。注意，getpid()系统调用返回的是当前进程的tgid值而不是pid值*/
	pid_t tgid;

#ifdef CONFIG_CC_STACKPROTECTOR
	/* Canary value for the -fstack-protector gcc feature */
	/*防止利用内核栈溢出的0day攻击。进程启动时，在每个缓冲区的后面放置该值，可以理解为哨兵，
	当缓冲区溢出时，肯定会破坏该值，内核监测到该错误时就当机，内核编译时	-fstack-protector
	编译选项的添加，将使gcc监测栈溢出*/
	unsigned long stack_canary;
#endif
	/*指向创建它的父进程，如果创建它的父进程不存在则指向初始化进程*/
	struct task_struct *real_parent; /* real parent process (when being debugged) */
	/*指向父进程，当它终止时，必须向它的父进程发送信号*/
	struct task_struct *parent;	/* parent process */
	/*子进程链表表头，该链表保存所有子进程*/
	struct list_head children;
	/*连接到父进程的子进程链表，新的子进程置于sibling链表的起始位置，这意味着可以重建进程
	分支的时间顺序*/
	struct list_head sibling;
	/*线程组组长*/
	struct task_struct *group_leader;
	/*pid_link可将task_struct连接到表头在struct pid中的散列表中*/
	struct pid_link pids[PIDTYPE_MAX];
	/*线程组保存该进程所拥有的线程*/
	struct list_head thread_group;
	/*在执行do_fork时，如果给定特别标识，则vfork_done会指向一个特殊地址*/
	struct completion *vfork_done;		/*for vfork()*/
	/*如果copy_process函数的clone_flags参数的值被置为CLONE_CHILD_SETTID或CLONE_CHILD_CLEARTID，
	则会把child_tidptr参数的值分别复制到set_child_tid和clear_child_tid成员。这些标志说明必须改变
	子进程用户态地址空间的child_tidptr所指向的变量的值*/
	int __user *set_child_tid;
	/*子进程退出时清零该地址*/
	int __user *clear_child_tid;
	/*实时进程的优先级，值越大优先级越高*/
	unsigned int rt_priority;
	/*进程本次被调用在用户态下运行的节拍数（处理器的频率）*/
	cputime_t utime;
	/*进程本次被调用在内核态下运行的节拍数*/
	cputime_t stime;
	/*进程在用户态下运行的时间*/
	cputime_t utimescaled;
	/*进程在内核态下运行的时间*/
	cputime_t stimescaled;
	/*以节拍计数的虚拟机运行时间*/
	cputime_t gtime;
	/*本次调用之前，进程已经在用户态下运行的时间总计*/
	cputime_t prev_utime;
	/*本次调用之前，进程已经在用户态下运行的时间总计*/
	cputime_t prev_stime;
	/*进程应为等待资源等原因放弃cpu的次数*/
	unsigned long nvcsw;
	/*进程运行时由于时间片用完或被更高优先级抢占等原因而放弃cpu的次数*/
	unsigned long nivcsw;
	/*进程创建时单调时钟时间*/
	struct timespec start_time;
	/*进程真正开始执行的时间*/
	struct timespec real_start_time;
	/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
	/*数据已经在交换区中*/
	unsigned long min_flt;
	/*缺页中断次数，数据需要会后备存储器中读取*/
	unsigned long maj_flt;
	/**/
  	cputime_t it_prof_expires;
	/**/
	cputime_t it_virt_expires;
	/**/
	unsigned long long it_sched_expires;
	/**/
	struct list_head cpu_timers[3];

	/* 进程凭证 */
	/*真实用户ID和真实组ID：这些凭证代表进程或任务的实际用户和组身份，它们定义了进程的客观
	上下文，并通常继承自启动该进程的用户*/
	uid_t uid;
	/*有效uid，确定进程在执行时具有的权限和特权，在某些情况下，有效用户ID和有效组ID可能与
	这是的不同，例如，当进程执行一个具有设置了setuid和setgid权限的程序时，其有效ID可能变成
	了该程序的所有者ID，从而使进程能够在一定时间内暂时具有该所有者的权限*/
	uid_t euid;
	/*保存的用户ID（saved user ID）和保存的组ID（saved group ID）：这些凭证用于保留进程临时
	更改有效ID时的原始用户ID和组ID，通常在特权提升操作中使用，允许进程在执行特权操作后恢复
	到其原始身份*/
	uid_t suid;
	/*文件系统用户ID和文件系统组ID：这些凭证在进程访问文件系统上的文件时使用，它们为文件相
	关操作提供给临时身份更改，与有效ID独立，这允许进程在文件操作中具有不同的权限，同时保留
	其有效ID用于其它目的*/
	uid_t fsuid;
	/*进程真实的gid*/
	gid_t gid;
	/*有效组id*/
	gid_t egid;
	/*暂时保存的真实gid*/
	gid_t sgid;
	/*操作文件时的gid*/
	gid_t fsgid;
	/*进程所属进程组的信息*/
	struct group_info *group_info;
	/*进程的有效权限*/
	kernel_cap_t cap_effective;
	/*进程继承的权限*/
	kernel_cap_t cap_inheritable;
	/*进程被允许的权限*/
	kernel_cap_t cap_permitted;
	/*是否允许进程保持特定权限*/
	unsigned keep_capabilities:1;
	/*进程所属的用户*/
	struct user_struct *user;
#ifdef CONFIG_KEYS
	/**/
	struct key *request_key_auth;	/* assumed request_key authority */
	/**/
	struct key *thread_keyring;	/* keyring private to this thread */
	/**/
	unsigned char jit_keyring;	/* default keyring to attach requested keys to */
#endif
	/*获得自旋锁后通过get/set_task_comm()函数获取的不含路径的可执行文件名*/
	char comm[TASK_COMM_LEN];
	/*用于在查找环形链表时防止无限循环，默认情况下，内核允许MAX_NESTED_LINKS（通常为
	8个）个递归*/
	int link_count;
	/*限制路径名中连接的最大数目，默认40个连续的链接，硬编码，并非通过预处理器符号定义*/
	int total_link_count;
#ifdef CONFIG_SYSVIPC
	/*ipc相关*/
	struct sysv_sem sysvsem;
#endif
/* CPU-specific state of this task */
	struct thread_struct thread;
	/*进程文件系统相关信息*/
	struct fs_struct *fs;
	/*已打开文件相关信息*/
	struct files_struct *files;
	/*命名空间信息*/
	struct nsproxy *nsproxy;
	/*信号描述符*/
	struct signal_struct *signal;
	/*管理设置的信号处理函数的信息*/
	struct sighand_struct *sighand;
	/*所有被阻塞的信号*/
	sigset_t blocked;
	/**/
	sigset_t real_blocked;
	/*用TIF_RESTORE_SIGMASK恢复*/
	sigset_t saved_sigmask;
	/*pengding建立一个链表，包含所有已经引发、仍然有待内核处理的信号*/
	struct sigpending pending;
	/*信号处理程序备用栈的地址*/
	unsigned long sas_ss_sp;
	/*信号处理栈的大小*/
	size_t sas_ss_size;
	/*设备驱动程序常用notifier指向的函数来阻塞进程的某些信号*/
	int (*notifier)(void *priv);
	/*notifier指向的函数可能使用的数据*/
	void *notifier_data;
	/*标识被阻塞信号的位掩码*/
	sigset_t *notifier_mask;
#ifdef CONFIG_SECURITY
	/*LSM（Linux Security Module）是一种允许对任务操作进行额外控制的机制，它在linux中支持
	多个LSM选项。LSM通过为系统中对象进行标记，并应用一组规则来限制具有一个标记的任务对具有
	另一个标记的对象可进行的操作。LSM的基本实现是通过在内核中引入一个可插拔的安全模块接口，
	允许不同的安全模块实现特定的安全策略。每个安全模块都可以对系统中的对象（如文件、进程、
	网络连接等）进行标记，并定义了一组规则来控制对象之间的访问和操作*/
	void *security;
#endif
	/*进程审计相关*/
	struct audit_context *audit_context;
	/*进程安全计算相关*/
	seccomp_t seccomp;

/* Thread group tracking */
   	/**/
	u32 parent_exec_id;
   	/**/
	u32 self_exec_id;
	/*分配或释放内存文件/文件系统/终端设备/令牌关键字的保护锁*/
	spinlock_t alloc_lock;
	/*优先级继承PI（priority inheritance）数据保护锁，task_rq_lock函数使用的自旋锁*/
	spinlock_t pi_lock;

#ifdef CONFIG_RT_MUTEXES
	/* PI waiters blocked on a rt_mutex held by this task */
	/**/
	struct plist_head pi_waiters;
	/*死锁检测和优先级继承处理*/
	struct rt_mutex_waiter *pi_blocked_on;
#endif

#ifdef CONFIG_DEBUG_MUTEXES
	/*死锁检测互斥量*/
	struct mutex_waiter *blocked_on;
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	/**/
	unsigned int irq_events;
	/**/
	int hardirqs_enabled;
	/**/
	unsigned long hardirq_enable_ip;
	/**/
	unsigned int hardirq_enable_event;
	/**/
	unsigned long hardirq_disable_ip;
	/**/
	unsigned int hardirq_disable_event;
	/**/
	int softirqs_enabled;
	/**/
	unsigned long softirq_disable_ip;
	/**/
	unsigned int softirq_disable_event;
	/**/
	unsigned long softirq_enable_ip;
	/**/
	unsigned int softirq_enable_event;
	/**/
	int hardirq_context;
	/**/
	int softirq_context;
#endif
#ifdef CONFIG_LOCKDEP
#define MAX_LOCK_DEPTH 30UL
	/**/
	u64 curr_chain_key;
	/**/
	int lockdep_depth;
	/**/
	struct held_lock held_locks[MAX_LOCK_DEPTH];
	/**/
	unsigned int lockdep_recursion;
#endif

	/*日志文件系统信息*/
	void *journal_info;

	/*块设备栈信息，尽管递归调用在用户空间没有问题，但内核中栈空间非常有限，因此可能会引起
	问题，因而需要确定一个合理的值，来限制递归的最大深度，下述两个struct bio指针成员用于将
	递归的最大深度限制为1，这样就不会丢失任何提交的bio*/
	struct bio *bio_list;
	struct bio **bio_tail;

	/*虚拟内存状态信息*/
	struct reclaim_state *reclaim_state;
	/**/
	struct backing_dev_info *backing_dev_info;
	/**/
	struct io_context *io_context;
	/**/
	unsigned long ptrace_message;
	/**/
	siginfo_t *last_siginfo; /* For ptrace use.  */
#ifdef CONFIG_TASK_XACCT
/* i/o counters(bytes read/written, #syscalls */
	/**/
	u64 rchar;
	/**/
	u64 wchar;
	/**/
	u64 syscr;
	/**/
	u64 syscw;
#endif
	/**/
	struct task_io_accounting ioac;
#if defined(CONFIG_TASK_XACCT)
	u64 acct_rss_mem1;	/* accumulated rss usage */
	u64 acct_vm_mem1;	/* accumulated virtual memory usage */
	cputime_t acct_stimexpd;/* stime since last update */
#endif
#ifdef CONFIG_NUMA
	/**/
  	struct mempolicy *mempolicy;
	/**/
	short il_next;
#endif
#ifdef CONFIG_CPUSETS
	/**/
	nodemask_t mems_allowed;
	/**/
	int cpuset_mems_generation;
	/**/
	int cpuset_mem_spread_rotor;
#endif
#ifdef CONFIG_CGROUPS
	/* Control Group info protected by css_set_lock */
	/**/
	struct css_set *cgroups;
	/* cg_list protected by css_set_lock and tsk->alloc_lock */
	/**/
	struct list_head cg_list;
#endif
#ifdef CONFIG_FUTEX
	/**/
	struct robust_list_head __user *robust_list;
#ifdef CONFIG_COMPAT
	/**/
	struct compat_robust_list_head __user *compat_robust_list;
#endif
	/**/
	struct list_head pi_state_list;
	/**/
	struct futex_pi_state *pi_state_cache;
#endif
	/*持有独占文件系统资源*/
	atomic_t fs_excl;
	/**/
	struct rcu_head rcu;

	/*
	 * cache last used pipe for splice
	 */
	/**/
	struct pipe_inode_info *splice_pipe;
#ifdef	CONFIG_TASK_DELAY_ACCT
	/**/
	struct task_delay_info *delays;
#endif
#ifdef CONFIG_FAULT_INJECTION
	/**/
	int make_it_fail;
#endif
	/**/
	struct prop_local_single dirties;
};

/*
 * Priority of a process goes from 0..MAX_PRIO-1, valid RT
 * priority is 0..MAX_RT_PRIO-1, and SCHED_NORMAL/SCHED_BATCH
 * tasks are in the range MAX_RT_PRIO..MAX_PRIO-1. Priority
 * values are inverted: lower p->prio value means higher priority.
 *
 * The MAX_USER_RT_PRIO value allows the actual maximum
 * RT priority to be separate from the value exported to
 * user-space.  This allows kernel threads to set their
 * priority to a value higher than any user task. Note:
 * MAX_RT_PRIO must not be smaller than MAX_USER_RT_PRIO.
 */
/*用户进程的最大优先级*/
#define MAX_USER_RT_PRIO		100
#define MAX_RT_PRIO				MAX_USER_RT_PRIO
/*用户进程的最小优先级*/
#define MAX_PRIO				(MAX_RT_PRIO + 40)
/*用户进程默认优先级*/
#define DEFAULT_PRIO			(MAX_RT_PRIO + 20)
/*测试优先级是否属于实时优先级*/
static inline int rt_prio(int prio)
{
	if (unlikely(prio < MAX_RT_PRIO))
		return 1;
	return 0;
}

/*测试进程是否属于实时进程（其动态优先级是否属于实时优先级）*/
static inline int rt_task(struct task_struct *p)
{
	return rt_prio(p->prio);
}

/*设置进程的会话ID*/
static inline void set_task_session(struct task_struct *tsk, pid_t session)
{
	tsk->signal->__session = session;
}

/*设置进程的进程组ID*/
static inline void set_task_pgrp(struct task_struct *tsk, pid_t pgrp)
{
	tsk->signal->__pgrp = pgrp;
}

/*获取进程的pid*/
static inline struct pid *task_pid(struct task_struct *task)
{
	return task->pids[PIDTYPE_PID].pid;
}

/*获取进程所在线程组的组长pid*/
static inline struct pid *task_tgid(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_PID].pid;
}

/*获取进程所在进程组的组长pid*/
static inline struct pid *task_pgrp(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_PGID].pid;
}

/*获取进程所在会话组组长pid*/
static inline struct pid *task_session(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_SID].pid;
}

struct pid_namespace;

/*
   task_xid_nr():获取全局id。初始化（根）命名空间中可见的id
   task_xid_vnr()    :虚拟id。进程所属命名空间的id，仅在同一命名空间的进程上下文被调用时有意义
   task_xid_nr_ns()    :特定命名空间id
   set_task_vxid()    :给一个进程赋值一个虚拟id
*/

/*获取进程的全局pid编号*/
static inline pid_t task_pid_nr(struct task_struct *tsk)
{
	return tsk->pid;
}

pid_t task_pid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);
/*获取进程所在最底层命名空间中的局部pid编号*/
static inline pid_t task_pid_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_pid(tsk));
}

/*获得线程所在线程组组长的pid*/
static inline pid_t task_tgid_nr(struct task_struct *tsk)
{
	return tsk->tgid;
}

pid_t task_tgid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);

/*获得线程所在线程组组长的最底层命名空间中的局部pid编号*/
static inline pid_t task_tgid_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_tgid(tsk));
}

/*获取进程的所在进程组组长pid*/
static inline pid_t task_pgrp_nr(struct task_struct *tsk)
{
	return tsk->signal->__pgrp;
}

pid_t task_pgrp_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);

/*获得进程所在进程组组长的最底层命名空间中的局部pid编号*/
static inline pid_t task_pgrp_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_pgrp(tsk));
}

/*获取进程所在会话组组长pid*/
static inline pid_t task_session_nr(struct task_struct *tsk)
{
	return tsk->signal->__session;
}

pid_t task_session_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);

/*获得进程所在会话组组长的最底层命名空间中的局部pid编号*/
static inline pid_t task_session_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_session(tsk));
}


/**
 * pid_alive - check that a task structure is not stale
 * @p: Task structure to be checked.
 *
 * Test if a process is not yet dead (at most zombie state)
 * If pid_alive fails, then pointers within the task structure
 * can be stale and must not be dereferenced.
 */
/*检查进程是否还没有完全死亡（至少处于僵尸状态）。如果pid不存在了，进程结构体内
指针一定不能被引用*/
static inline int pid_alive(struct task_struct *p)
{
	return p->pids[PIDTYPE_PID].pid != NULL;
}

/*检查进程是否是初始化进程。是否是内核创建的用户空间中第一个进程*/
static inline int is_global_init(struct task_struct *tsk)
{
	return tsk->pid == 1;
}

extern int is_container_init(struct task_struct *tsk);

extern struct pid *cad_pid;

extern void free_task(struct task_struct *tsk);
/*引用指定进程*/
#define get_task_struct(tsk) do { atomic_inc(&(tsk)->usage); } while(0)

extern void __put_task_struct(struct task_struct *t);

/*取消对指定进程的引用，如果进程没有被引用。则释放该进程资源*/
static inline void put_task_struct(struct task_struct *t)
{
	if (atomic_dec_and_test(&t->usage))
		__put_task_struct(t);
}

/*打印对齐警告，目前仅在486上实现*/
#define PF_ALIGNWARN			0x00000001
/*进程正在被创建*/
#define PF_STARTING				0x00000002
/*进程正在被撤销*/
#define PF_EXITING				0x00000004
/**/
#define PF_EXITPIDONE			0x00000008	/* pi exit done on shut down */
/*虚拟cpu*/
#define PF_VCPU					0x00000010
/*fork后还没有执行exec*/
#define PF_FORKNOEXEC			0x00000040
/*使用超级用户权限*/
#define PF_SUPERPRIV			0x00000100
/*内存转储*/
#define PF_DUMPCORE				0x00000200
/*被信号终止*/
#define PF_SIGNALED				0x00000400
/*正在申请内存*/
#define PF_MEMALLOC				0x00000800
/*正在向磁盘回写数据*/
#define PF_FLUSHER				0x00001000
/*使用前初始化为重置的fpu*/
#define PF_USED_MATH			0x00002000
/*当前进程不应该被冻结*/
#define PF_NOFREEZE				0x00008000
/*进程因为系统挂起冻结*/
#define PF_FROZEN				0x00010000
/*文件系统传输*/
#define PF_FSTRANS				0x00020000
/*进程是内核页面交换进程*/
#define PF_KSWAPD				0x00040000
/*页面换出进程*/
#define PF_SWAPOFF				0x00080000
/**/
#define PF_LESS_THROTTLE		0x00100000	/* Throttle me less: I clean memory */
/*正在使用进程虚拟地址空间的内核线程*/
#define PF_BORROWED_MM			0x00200000
/*随机化虚拟地址空间*/
#define PF_RANDOMIZE			0x00400000
/*允许写交换区*/
#define PF_SWAPWRITE			0x00800000
/**/
#define PF_SPREAD_PAGE			0x01000000	/* Spread page cache over cpuset */
/**/
#define PF_SPREAD_SLAB			0x02000000	/* Spread some slab caches over cpuset */
/*非默认的非一致内存访问策略*/
#define PF_MEMPOLICY			0x10000000
/*测试实时互斥量的线程*/
#define PF_MUTEX_TESTER			0x20000000
/**/
#define PF_FREEZER_SKIP			0x40000000	/* Freezer should not count it as freezeable */

/*
 * Only the _current_ task can read/write to tsk->flags, but other
 * tasks can access tsk->flags in readonly mode for example
 * with tsk_used_math (like during threaded core dumping).
 * There is however an exception to this rule during ptrace
 * or during fork: the ptracer task is allowed to write to the
 * child->flags of its traced child (same goes for fork, the parent
 * can write to the child->flags), because we're guaranteed the
 * child is not running and in turn not changing child->flags
 * at the same time the parent does it.
 */
#define clear_stopped_child_used_math(child) do { (child)->flags &= ~PF_USED_MATH; } while (0)
#define set_stopped_child_used_math(child) do { (child)->flags |= PF_USED_MATH; } while (0)
#define clear_used_math() clear_stopped_child_used_math(current)
#define set_used_math() set_stopped_child_used_math(current)
#define conditional_stopped_child_used_math(condition, child) \
	do { (child)->flags &= ~PF_USED_MATH, (child)->flags |= (condition) ? PF_USED_MATH : 0; } while (0)
#define conditional_used_math(condition) \
	conditional_stopped_child_used_math(condition, current)
#define copy_to_stopped_child_used_math(child) \
	do { (child)->flags &= ~PF_USED_MATH, (child)->flags |= current->flags & PF_USED_MATH; } while (0)
/* NOTE: this will return 0 or PF_USED_MATH, it will never return 1 */
#define tsk_used_math(p) ((p)->flags & PF_USED_MATH)
#define used_math() tsk_used_math(current)

#ifdef CONFIG_SMP
extern int set_cpus_allowed(struct task_struct *p, cpumask_t new_mask);
#else
static inline int set_cpus_allowed(struct task_struct *p, cpumask_t new_mask)
{
	if (!cpu_isset(0, new_mask))
		return -EINVAL;
	return 0;
}
#endif

extern unsigned long long sched_clock(void);

/*sched_clock()创建的高速但轻微不准确的仅供内核内部使用的per_cpu clock*/
extern unsigned long long cpu_clock(int cpu);

extern unsigned long long task_sched_runtime(struct task_struct *task);

/* sched_exec is called by processes performing an exec */
#ifdef CONFIG_SMP
extern void sched_exec(void);
#else
#define sched_exec()   {}
#endif

extern void sched_clock_idle_sleep_event(void);
extern void sched_clock_idle_wakeup_event(u64 delta_ns);

#ifdef CONFIG_HOTPLUG_CPU
extern void idle_task_exit(void);
#else
static inline void idle_task_exit(void) {}
#endif

extern void sched_idle_next(void);

#ifdef CONFIG_SCHED_DEBUG
extern unsigned int sysctl_sched_latency;
extern unsigned int sysctl_sched_min_granularity;
extern unsigned int sysctl_sched_wakeup_granularity;
extern unsigned int sysctl_sched_batch_wakeup_granularity;
extern unsigned int sysctl_sched_child_runs_first;
extern unsigned int sysctl_sched_features;
extern unsigned int sysctl_sched_migration_cost;
extern unsigned int sysctl_sched_nr_migrate;

int sched_nr_latency_handler(struct ctl_table *table, int write,
		struct file *file, void __user *buffer, size_t *length,
		loff_t *ppos);
#endif

extern unsigned int sysctl_sched_compat_yield;

#ifdef CONFIG_RT_MUTEXES
extern int rt_mutex_getprio(struct task_struct *p);
extern void rt_mutex_setprio(struct task_struct *p, int prio);
extern void rt_mutex_adjust_pi(struct task_struct *p);
#else
static inline int rt_mutex_getprio(struct task_struct *p)
{
	return p->normal_prio;
}
#define rt_mutex_adjust_pi(p)		do { } while (0)
#endif

extern void set_user_nice(struct task_struct *p, long nice);
extern int task_prio(const struct task_struct *p);
extern int task_nice(const struct task_struct *p);
extern int can_nice(const struct task_struct *p, const int nice);
extern int task_curr(const struct task_struct *p);
extern int idle_cpu(int cpu);
extern int sched_setscheduler(struct task_struct *, int, struct sched_param *);
extern struct task_struct *idle_task(int cpu);
extern struct task_struct *curr_task(int cpu);
extern void set_curr_task(int cpu, struct task_struct *p);

void yield(void);

extern struct exec_domain	default_exec_domain;

/*进程内核栈和thread_info公用的联合体，thead_info位于低地址，内核栈位于高地址向下增长*/
union thread_union
{
	/*保存了线程所需的所有特定于处理器的信息*/
	struct thread_info thread_info;
	/*内核栈，默认大小8K*/
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

#ifndef __HAVE_ARCH_KSTACK_END
/*内核栈结束地址*/
static inline int kstack_end(void *addr)
{
	/*可靠的内核栈结尾检测，有些高级电源管理bios版本不对齐栈。0x1FFC*/
	return !(((unsigned long)addr+sizeof(void*)-1) & (THREAD_SIZE-sizeof(void*)));
}
#endif

extern union thread_union init_thread_union;
extern struct task_struct init_task;

extern struct   mm_struct init_mm;

extern struct pid_namespace init_pid_ns;

/*
 * find a task by one of its numerical ids
 *
 * find_task_by_pid_type_ns():
 *      it is the most generic call - it finds a task by all id,
 *      type and namespace specified
 * find_task_by_pid_ns():
 *      finds a task by its pid in the specified namespace
 * find_task_by_vpid():
 *      finds a task by its virtual pid
 * find_task_by_pid():
 *      finds a task by its global pid
 *
 * see also find_pid() etc in include/linux/pid.h
 */

extern struct task_struct *find_task_by_pid_type_ns(int type, int pid, struct pid_namespace *ns);
extern struct task_struct *find_task_by_pid(pid_t nr);
extern struct task_struct *find_task_by_vpid(pid_t nr);
extern struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns);

extern void __set_special_pids(pid_t session, pid_t pgrp);

extern struct user_struct * alloc_uid(struct user_namespace *, uid_t);
/*引用指定用户*/
static inline struct user_struct *get_uid(struct user_struct *u)
{
	atomic_inc(&u->__count);
	return u;
}
extern void free_uid(struct user_struct *);
extern void switch_uid(struct user_struct *);
extern void release_uids(struct user_namespace *ns);

#include <asm/current.h>

extern void do_timer(unsigned long ticks);

extern int FASTCALL(wake_up_state(struct task_struct * tsk, unsigned int state));
extern int FASTCALL(wake_up_process(struct task_struct * tsk));
extern void FASTCALL(wake_up_new_task(struct task_struct * tsk,
						unsigned long clone_flags));
#ifdef CONFIG_SMP
 extern void kick_process(struct task_struct *tsk);
#else
 static inline void kick_process(struct task_struct *tsk) { }
#endif
extern void sched_fork(struct task_struct *p, int clone_flags);
extern void sched_dead(struct task_struct *p);

extern int in_group_p(gid_t);
extern int in_egroup_p(gid_t);

extern void proc_caches_init(void);
extern void flush_signals(struct task_struct *);
extern void ignore_signals(struct task_struct *);
extern void flush_signal_handlers(struct task_struct *, int force_default);
extern int dequeue_signal(struct task_struct *tsk, sigset_t *mask, siginfo_t *info);

static inline int dequeue_signal_lock(struct task_struct *tsk, sigset_t *mask, siginfo_t *info)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&tsk->sighand->siglock, flags);
	ret = dequeue_signal(tsk, mask, info);
	spin_unlock_irqrestore(&tsk->sighand->siglock, flags);

	return ret;
}

extern void block_all_signals(int (*notifier)(void *priv), void *priv,
			      sigset_t *mask);
extern void unblock_all_signals(void);
extern void release_task(struct task_struct * p);
extern int send_sig_info(int, struct siginfo *, struct task_struct *);
extern int send_group_sig_info(int, struct siginfo *, struct task_struct *);
extern int force_sigsegv(int, struct task_struct *);
extern int force_sig_info(int, struct siginfo *, struct task_struct *);
extern int __kill_pgrp_info(int sig, struct siginfo *info, struct pid *pgrp);
extern int kill_pgrp_info(int sig, struct siginfo *info, struct pid *pgrp);
extern int kill_pid_info(int sig, struct siginfo *info, struct pid *pid);
extern int kill_pid_info_as_uid(int, struct siginfo *, struct pid *, uid_t, uid_t, u32);
extern int kill_pgrp(struct pid *pid, int sig, int priv);
extern int kill_pid(struct pid *pid, int sig, int priv);
extern int kill_proc_info(int, struct siginfo *, pid_t);
extern void do_notify_parent(struct task_struct *, int);
extern void force_sig(int, struct task_struct *);
extern void force_sig_specific(int, struct task_struct *);
extern int send_sig(int, struct task_struct *, int);
extern void zap_other_threads(struct task_struct *p);
extern int kill_proc(pid_t, int, int);
extern struct sigqueue *sigqueue_alloc(void);
extern void sigqueue_free(struct sigqueue *);
extern int send_sigqueue(int, struct sigqueue *,  struct task_struct *);
extern int send_group_sigqueue(int, struct sigqueue *,  struct task_struct *);
extern int do_sigaction(int, struct k_sigaction *, struct k_sigaction *);
extern int do_sigaltstack(const stack_t __user *, stack_t __user *, unsigned long);

static inline int kill_cad_pid(int sig, int priv)
{
	return kill_pid(cad_pid, sig, priv);
}

/* These can be the second arg to send_sig_info/send_group_sig_info.  */
#define SEND_SIG_NOINFO ((struct siginfo *) 0)
#define SEND_SIG_PRIV	((struct siginfo *) 1)
#define SEND_SIG_FORCED	((struct siginfo *) 2)

static inline int is_si_special(const struct siginfo *info)
{
	return info <= SEND_SIG_FORCED;
}

/* True if we are on the alternate signal stack.  */

static inline int on_sig_stack(unsigned long sp)
{
	return (sp - current->sas_ss_sp < current->sas_ss_size);
}

static inline int sas_ss_flags(unsigned long sp)
{
	return (current->sas_ss_size == 0 ? SS_DISABLE
		: on_sig_stack(sp) ? SS_ONSTACK : 0);
}

/*
 * Routines for handling mm_structs
 */
extern struct mm_struct * mm_alloc(void);

/* mmdrop drops the mm and the page tables */
extern void FASTCALL(__mmdrop(struct mm_struct *));
static inline void mmdrop(struct mm_struct * mm)
{
	if (unlikely(atomic_dec_and_test(&mm->mm_count)))
		__mmdrop(mm);
}

/* mmput gets rid of the mappings and all user-space */
extern void mmput(struct mm_struct *);
/* Grab a reference to a task's mm, if it is not already going away */
extern struct mm_struct *get_task_mm(struct task_struct *task);
/* Remove the current tasks stale references to the old mm_struct */
extern void mm_release(struct task_struct *, struct mm_struct *);

extern int  copy_thread(int, unsigned long, unsigned long, unsigned long, struct task_struct *, struct pt_regs *);
extern void flush_thread(void);
extern void exit_thread(void);

extern void exit_files(struct task_struct *);
extern void __cleanup_signal(struct signal_struct *);
extern void __cleanup_sighand(struct sighand_struct *);
extern void exit_itimers(struct signal_struct *);

extern NORET_TYPE void do_group_exit(int);

extern void daemonize(const char *, ...);
extern int allow_signal(int);
extern int disallow_signal(int);

extern int do_execve(char *, char __user * __user *, char __user * __user *, struct pt_regs *);
extern long do_fork(unsigned long, unsigned long, struct pt_regs *, unsigned long, int __user *, int __user *);
struct task_struct *fork_idle(int);

extern void set_task_comm(struct task_struct *tsk, char *from);
extern void get_task_comm(char *to, struct task_struct *tsk);

#ifdef CONFIG_SMP
extern void wait_task_inactive(struct task_struct * p);
#else
#define wait_task_inactive(p)	do { } while (0)
#endif

/*解除进程的父子关系。从进程的sibling链表中删除该结点并初始化*/
#define remove_parent(p)	list_del_init(&(p)->sibling)
/*新的子进程放在sibling链表的起始位置，这意味着可以重建进程分支的时间顺序*/
#define add_parent(p)		list_add_tail(&(p)->sibling,&(p)->parent->children)
/*rcu保护下从进程的tasks链表中获取下一个进程，tasks双链表连接系统中所有进程*/
#define next_task(p)	list_entry(rcu_dereference((p)->tasks.next), struct task_struct, tasks)
/*rcu保护下从初始化进程开始，遍历系统中所有进程*/
#define for_each_process(p) for (p = &init_task ; (p = next_task(p)) != &init_task ; )

/*注意！do_each_thread和while_each_thread是一个双链表循环，因此。break将不会像预期
那样起作用，这时要用goto*/
/**/
#define do_each_thread(g, t) \
	for (g = t = &init_task ; (g = t = next_task(g)) != &init_task ; ) do

/**/
#define while_each_thread(g, t) while ((t = next_thread(t)) != g)

/*测试线程是否是其所在线程组的组长，不是基于pid==tgid检查*/
#define thread_group_leader(p)	(p == p->group_leader)

/* Do to the insanities of de_thread it is possible for a process
 * to have the pid of the thread group leader without actually being
 * the thread group leader.  For iteration through the pids in proc
 * all we care about is that we have a task with the appropriate
 * pid, we don't actually care if we have the right task.
 */
/*判断进程pid是否与其所在线程的线程组组长pid相同*/
static inline int has_group_leader_pid(struct task_struct *p)
{
	return p->pid == p->tgid;
}

/*判断两个线程是否同属一个线程组，测试线程所在线程组的组长是否相同*/
static inline int same_thread_group(struct task_struct *p1, struct task_struct *p2)
{
	return p1->tgid == p2->tgid;
}

/*获取线程所在线程组中的下一个线程*/
static inline struct task_struct *next_thread(const struct task_struct *p)
{
	return list_entry(rcu_dereference(p->thread_group.next),
			  struct task_struct, thread_group);
}

/*测试进程是否没有线程，判断其线程组是否为空*/
static inline int thread_group_empty(struct task_struct *p)
{
	return list_empty(&p->thread_group);
}

/*判断进程是有已创建的线程。进程的线程组非空且进程是其所在线程组的组长*/
#define delay_group_leader(p) (thread_group_leader(p) && !thread_group_empty(p))

/*
 * Protects ->fs, ->files, ->mm, ->group_info, ->comm, keyring
 * subscriptions and synchronises with wait4().  Also used in procfs.  Also
 * pins the final release of task.io_context.  Also protects ->cpuset and
 * ->cgroup.subsys[].
 *
 * Nests both inside and outside of read_lock(&tasklist_lock).
 * It must not be nested with write_lock_irq(&tasklist_lock),
 * neither inside nor outside.
 */
/*保护进程的文件系统、已打开文件、内存、组信息、进程名、密匙subscriptions和同步wait4。
经常用于procfs虚拟文件系统，也钉住最终的task.io_context释放，也保护进程的cpu集合和
cgroup子系统*/
/*申请task_struct的alloc_lock锁*/
static inline void task_lock(struct task_struct *p)
{
	spin_lock(&p->alloc_lock);
}

/*释放task_struct的alloc_lock锁*/
static inline void task_unlock(struct task_struct *p)
{
	spin_unlock(&p->alloc_lock);
}

extern struct sighand_struct *lock_task_sighand(struct task_struct *tsk, unsigned long *flags);

/*释放进程的信号处理描述符自旋锁，并启用本次cpu中断*/
static inline void unlock_task_sighand(struct task_struct *tsk, unsigned long *flags)
{
	spin_unlock_irqrestore(&tsk->sighand->siglock, *flags);
}

#ifndef __HAVE_THREAD_FUNCTIONS
/*获取进程的低层thread_info信息*/
#define task_thread_info(task)	((struct thread_info *)(task)->stack)
/*获取进程的内核栈*/
#define task_stack_page(task)	((task)->stack)
/*复制进程的内核栈信息*/
static inline void setup_thread_stack(struct task_struct *p, struct task_struct *org)
{
	/*将源进程org的内核栈信息直接通过结构体赋值给目标进程p*/
	*task_thread_info(p) = *task_thread_info(org);
	/*将目标进程p的线程低层task指针指向自身*/
	task_thread_info(p)->task = p;
}

/*设置内核栈结束位置，也即thread_info的上一字节地址*/
static inline unsigned long *end_of_stack(struct task_struct *p)
{
	return (unsigned long *)(task_thread_info(p) + 1);
}

#endif

/* set thread flags in other task's structures
 * - see asm/thread_info.h for TIF_xxxx flags available
 */

/*设置进程低层TIF_标识*/
static inline void set_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	set_ti_thread_flag(task_thread_info(tsk), flag);
}
/*清除进程低层TIF_标识*/
static inline void clear_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	clear_ti_thread_flag(task_thread_info(tsk), flag);
}
/*测试进程低层的TIF_标识是否已设置，并返回之前的状态*/
static inline int test_and_set_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_and_set_ti_thread_flag(task_thread_info(tsk), flag);
}
/*测试并清除进程低层的TIF_标识，并返回之前的状态*/
static inline int test_and_clear_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_and_clear_ti_thread_flag(task_thread_info(tsk), flag);
}
/*测试进程低层TIF_标识是否已设置*/
static inline int test_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_ti_thread_flag(task_thread_info(tsk), flag);
}
/*设置进程TIF_NEED_RESCHED需要重调度标识*/
static inline void set_tsk_need_resched(struct task_struct *tsk)
{
	set_tsk_thread_flag(tsk,TIF_NEED_RESCHED);
}
/*清除进程的TIF_NEED_RESCHED标识*/
static inline void clear_tsk_need_resched(struct task_struct *tsk)
{
	clear_tsk_thread_flag(tsk,TIF_NEED_RESCHED);
}
/*测试进程是否有挂起信号待处理*/
static inline int signal_pending(struct task_struct *p)
{
	return unlikely(test_tsk_thread_flag(p,TIF_SIGPENDING));
}
/*测试进程是否需要重调度*/
static inline int need_resched(void)
{
	return unlikely(test_thread_flag(TIF_NEED_RESCHED));
}

/*
 * cond_resched() and cond_resched_lock(): latency reduction via
 * explicit rescheduling in places that are safe. The return
 * value indicates whether a reschedule was done in fact.
 * cond_resched_lock() will drop the spinlock before scheduling,
 * cond_resched_softirq() will enable bhs before scheduling.
 */
extern int cond_resched(void);
extern int cond_resched_lock(spinlock_t * lock);
extern int cond_resched_softirq(void);

/*
 * Does a critical section need to be broken due to another
 * task waiting?:
 */
#if defined(CONFIG_PREEMPT) && defined(CONFIG_SMP)
#define need_lockbreak(lock) ((lock)->break_lock)
#else
#define need_lockbreak(lock) 0
#endif

/*
 * Does a critical section need to be broken due to another
 * task waiting or preemption being signalled:
 */
static inline int lock_need_resched(spinlock_t *lock)
{
	if (need_lockbreak(lock) || need_resched())
		return 1;
	return 0;
}

/*
 * Reevaluate whether the task has signals pending delivery.
 * Wake the task if so.
 * This is required every time the blocked sigset_t changes.
 * callers must hold sighand->siglock.
 */
extern void recalc_sigpending_and_wake(struct task_struct *t);
extern void recalc_sigpending(void);

extern void signal_wake_up(struct task_struct *t, int resume_stopped);

/*
 * Wrappers for p->thread_info->cpu access. No-op on UP.
 */
#ifdef CONFIG_SMP
/*获取当前进程正在使用的cpu*/
static inline unsigned int task_cpu(const struct task_struct *p)
{
	return task_thread_info(p)->cpu;
}

extern void set_task_cpu(struct task_struct *p, unsigned int cpu);

#else

static inline unsigned int task_cpu(const struct task_struct *p)
{
	return 0;
}

static inline void set_task_cpu(struct task_struct *p, unsigned int cpu)
{
}

#endif /* CONFIG_SMP */

#ifdef HAVE_ARCH_PICK_MMAP_LAYOUT
/*如果体系结构想要在不同的mmap区域布局之间做出选择，提供该函数*/
extern void arch_pick_mmap_layout(struct mm_struct *mm);
#else
/**/
static inline void arch_pick_mmap_layout(struct mm_struct *mm)
{
	mm->mmap_base = TASK_UNMAPPED_BASE;
	/*在mmap区域中为新映射找到适当的位置*/
	mm->get_unmapped_area = arch_get_unmapped_area;
	mm->unmap_area = arch_unmap_area;
}
#endif

extern long sched_setaffinity(pid_t pid, cpumask_t new_mask);
extern long sched_getaffinity(pid_t pid, cpumask_t *mask);

extern int sched_mc_power_savings, sched_smt_power_savings;

extern void normalize_rt_tasks(void);

#ifdef CONFIG_FAIR_GROUP_SCHED

extern struct task_group init_task_group;

extern struct task_group *sched_create_group(void);
extern void sched_destroy_group(struct task_group *tg);
extern void sched_move_task(struct task_struct *tsk);
extern int sched_group_set_shares(struct task_group *tg, unsigned long shares);
extern unsigned long sched_group_shares(struct task_group *tg);

#endif

#ifdef CONFIG_TASK_XACCT
static inline void add_rchar(struct task_struct *tsk, ssize_t amt)
{
	tsk->rchar += amt;
}

static inline void add_wchar(struct task_struct *tsk, ssize_t amt)
{
	tsk->wchar += amt;
}

static inline void inc_syscr(struct task_struct *tsk)
{
	tsk->syscr++;
}

static inline void inc_syscw(struct task_struct *tsk)
{
	tsk->syscw++;
}
#else
static inline void add_rchar(struct task_struct *tsk, ssize_t amt)
{
}

static inline void add_wchar(struct task_struct *tsk, ssize_t amt)
{
}

static inline void inc_syscr(struct task_struct *tsk)
{
}

static inline void inc_syscw(struct task_struct *tsk)
{
}
#endif

#ifdef CONFIG_SMP
void migration_init(void);
#else
static inline void migration_init(void)
{
}
#endif

#endif /* __KERNEL__ */

#endif
