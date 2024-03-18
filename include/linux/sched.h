#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

/*�����˳�ʱ���͵��ź�*/
#define CSIGNAL						0x000000ff
/*���̼乲�������ڴ��ַ�ռ�*/
#define CLONE_VM					0x00000100
/*���̼乲���ļ�ϵͳ*/
#define CLONE_FS					0x00000200
/*���̼乲���Ѵ��ļ�*/
#define CLONE_FILES					0x00000400
/*���̼乲���źŴ������ͱ��������ź�*/
#define CLONE_SIGHAND				0x00000800
/*�����̱�����ʱ�ӽ���Ҳ�ᱻ����*/
#define CLONE_PTRACE				0x00002000
/*��������ӽ��������У������̱�������ֱ������mm_release���Ѹ�����*/
#define CLONE_VFORK					0x00004000
/*�������̺ͱ��������̹���һ�������̣����ֵܽ���*/
#define CLONE_PARENT				0x00008000
/*�������̺ͱ���������ͬ��һ���߳���*/
#define CLONE_THREAD				0x00010000	/* Same thread group? */
/*���̼䲻���������ռ䣬��ҪΪ�������Ľ��̴����µ������ռ�*/
#define CLONE_NEWNS					0x00020000	/* New namespace group? */
/*������̼�ͨ�Ż���*/
#define CLONE_SYSVSEM				0x00040000
/*Ϊ�½��̴����̱߳��ش洢*/
#define CLONE_SETTLS				0x00080000
/*�������̵߳�pid���Ƶ�clone����ָ�����û�Ӧ�ÿռ��е�ĳ����ַ*/
#define CLONE_PARENT_SETTID			0x00100000
/**/
#define CLONE_CHILD_CLEARTID		0x00200000
/*û��ʹ�ã�����*/
#define CLONE_DETACHED				0x00400000
/*�´����Ľ��̲��ܱ�����*/
#define CLONE_UNTRACED				0x00800000
/*����һ�����ݵ�clone���û��ռ�ָ��(child_tidptr)�������½��̵�task_struct��*/
#define CLONE_CHILD_SETTID			0x01000000
/*���̱���������ֹͣ״̬*/
#define CLONE_STOPPED				0x02000000
/*�����µ�UTS*/
#define CLONE_NEWUTS				0x04000000
/*�����µ�IPC*/
#define CLONE_NEWIPC				0x08000000
/*�����µ�user�����ռ�*/
#define CLONE_NEWUSER				0x10000000
/*�����µ�pid�����ռ�*/
#define CLONE_NEWPID				0x20000000
/*�����µ����������ռ�*/
#define CLONE_NEWNET				0x40000000


/*��ȫ��ƽ���ȵ�������ͨ����*/
#define SCHED_NORMAL		0
/*ʵʱ���Ȳ����е������ȷ�����ȣ��ý���һֱ����������  */
#define SCHED_FIFO			1
/*ʵʱ���̵ķǾ���ʱ��Ƭ��ת����  */
#define SCHED_RR			2
/*���ڷǽ�����CPUʹ���ܼ�����������̣�ͨ����ȫ��ƽ���������������Ⱦ��߶Դ�����̸���"�䴦��"��
���Ǿ�������ռCFS�������������һ�����̣���˲�����Ž���ʽ���̣������������nice���ͽ��̵ľ�̬��
�ȼ���ͬʱ�ֲ�ϣ���ý���Ӱ��ϵͳ�Ľ����ԣ����ʺ��øõ��Ȳ���*/

#define SCHED_BATCH			3
/*�����ڴ�Ҫ�Ľ��̣������Ȩ��������С�ģ�Ҳͨ����ȫ��ƽ������������Ҫע����ǣ�SCHED_IDLE������
���ȿ��н��̣����н������ں��ṩ�����Ļ���������.ֻ��root�û���ͨ��sched_setscheduler()ϵͳ��������
����Ȳ���*/
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

/*�����ں��̵߳ĸ��Ʊ�ʶ*/
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
/*������Ҫ�ı���λ��Ŀ*/
#define FSHIFT		11
#define FIXED_1		(1<<FSHIFT)	/* 1.0 as fixed-point */
#define LOAD_FREQ	(5*HZ+1)	/* 5 sec intervals */
#define EXP_1		1884		/* 1/exp(5sec/1min) as fixed-point */
#define EXP_5		2014		/* 1/exp(5sec/5min) */
#define EXP_15		2037		/* 1/exp(5sec/15min) */
/*����ϵͳƽ������*/
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

/*���̴��ڿ�����״̬���Ⲣ����ζ���Ѿ�ʵ�ʷ�����cpu�����̿��ܻ�һֱ�ȵ�������ѡ������
��״̬ȷ�����̿����������У�������ȴ��ⲿ�¼�*/
#define TASK_RUNNING				0
/*��Եȴ�ĳ�¼���������Դ��˯�߽������õġ����ں˷����źŸ��ý��̣������¼��Ѿ�����ʱ��
����״̬��ΪTASK_RUNNING����ֻҪ������ѡ�иý��̼��ɻָ�ִ��*/
#define TASK_INTERRUPTIBLE			1
/*�������ں�ָʾ��ֹͣ��˯�߽��̣����ǲ������ⲿ�źŻ��ѣ�ֻ�����ں����Ի���*/
#define TASK_UNINTERRUPTIBLE		2
/*��������ֹͣ���С����ɵ�������ͣ*/
#define TASK_STOPPED				4
/*�������ǽ���״̬�����ڴ�ֹͣ�Ľ����У�����ǰ�����Ե���Щ��ʹ��ptrace���ƣ��볣���
�������ֿ���*/
#define TASK_TRACED					8
/* in tsk->exit_state */
/*��ʬ״̬*/
#define EXIT_ZOMBIE					16
/*waitϵͳ�����Ѿ���������������ȫ��ϵͳ�Ƴ�֮ǰ��״̬��ֻ�ж���̶߳�ͬһ���̷���
wait����ʱ����״̬��������*/
#define EXIT_DEAD					32
/* in tsk->state again */
/**/
#define TASK_DEAD					64

/*���ý��̵�״̬*/
#define __set_task_state(tsk, state_value)	do { (tsk)->state = (state_value); } while (0)

/*�����ڴ����ϣ����۸ý����Ƿ���˯��״̬���ܹ���ȷ�޸�*/
#define set_task_state(tsk, state_value)		set_mb((tsk)->state, (state_value))

/*���ڴ�����ʽ�����õ�ǰ���̵�״̬*/
#define __set_current_state(state_value) do { current->state = (state_value); } while (0)
/*�����ڴ����ϣ����۵�ǰ�����Ƿ���˯��״̬���ܹ���ȷ�޸�*/
#define set_current_state(state_value)	set_mb(current->state, (state_value))

/*�������Ƴ���*/
#define TASK_COMM_LEN 16

#include <linux/spinlock.h>

/*���л�schedule()�;������е�ɾ�����޸Ĳ����ı�����������������Ӳ�����Ҫ��һ����������*/
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

/*���̵��ڴ�ת�����������0��ת�����н���*/
extern void show_state_filter(unsigned long state_filter);
static inline void show_state(void)
{
	show_state_filter(0);
}
extern void show_regs(struct pt_regs *);

/*ջ������Ϣ��taskָ��ָ�������ݵĽ��̣�Ϊ��ʱ��ʾ��ǰ������Ϣ��sp�ǵ�һ��Ӧ����ʾ
ջ���ݵ�ջָ֡�룬��һ�����¼��ջָ֡��*/
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

/*���ӵ��κ���wchan�����Ӧ�ñ����Ժ���*/
/*���ñ�ʶ��ǵĺ����洢�ڶ������ļ��е�".sched.text"�������*/
#define __sched		__attribute__((__section__(".sched.text")))

/*������ʼ����ֹ��__sched������ַ*/
extern char __sched_text_start[], __sched_text_end[];

/*���Ե�ַ�Ƿ���__sched ������Χ��*/
extern int in_sched_functions(unsigned long addr);
/*���������ȳ�ʱʱ��*/
#define	MAX_SCHEDULE_TIMEOUT	LONG_MAX
extern signed long FASTCALL(schedule_timeout(signed long timeout));
extern signed long schedule_timeout_interruptible(signed long timeout);
extern signed long schedule_timeout_uninterruptible(signed long timeout);
asmlinkage void schedule(void);

struct nsproxy;
struct user_namespace;

/*�λͼ����������Ŀ����һ����������*/
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
/*�ڴ������û���ܵ�page_table_lock��������˱����Զ�������*/
/*����/��ȡ/����/����/�Լ�mm��������ֵ*/
#define set_mm_counter(mm, member, value) atomic_long_set(&(mm)->_##member, value)
#define get_mm_counter(mm, member) ((unsigned long)atomic_long_read(&(mm)->_##member))
#define add_mm_counter(mm, member, value) atomic_long_add(value, &(mm)->_##member)
#define inc_mm_counter(mm, member) atomic_long_inc(&(mm)->_##member)
#define dec_mm_counter(mm, member) atomic_long_dec(&(mm)->_##member)

#else  /* NR_CPUS < CONFIG_SPLIT_PTLOCK_CPUS */
/*�ڴ�������ܵ�page_table_lock�ı���������ֱ������*/
#define set_mm_counter(mm, member, value) (mm)->_##member = (value)
#define get_mm_counter(mm, member) ((mm)->_##member)
#define add_mm_counter(mm, member, value) (mm)->_##member += (value)
#define inc_mm_counter(mm, member) (mm)->_##member++
#define dec_mm_counter(mm, member) (mm)->_##member--

#endif /* NR_CPUS < CONFIG_SPLIT_PTLOCK_CPUS */
/*��ȡ�ļ�ӳ�������ӳ�䳣פ�ڴ漯��С֮��*/
#define get_mm_rss(mm)					\
	(get_mm_counter(mm, file_rss) + get_mm_counter(mm, anon_rss))
/*��ȡ�ļ�������ӳ�䳣פ�ڴ漯֮�ͣ������ֵ���ڸ�ˮӡʱ�ĳ�פ�ڴ漯��С����ֵ����פ�ڴ漯*/
#define update_hiwater_rss(mm)	do {			\
	unsigned long _rss = get_mm_rss(mm);		\
	if ((mm)->hiwater_rss < _rss)				\
		(mm)->hiwater_rss = _rss;				\
} while (0)
/*���¸�ˮӡ�����ڴ�ҳ��Ŀ��*/
#define update_hiwater_vm(mm)	do {			\
	if ((mm)->hiwater_vm < (mm)->total_vm)		\
		(mm)->hiwater_vm = (mm)->total_vm;	\
} while (0)

extern void set_dumpable(struct mm_struct *mm, int value);
extern int get_dumpable(struct mm_struct *mm);

/* mm flags */
/*�����ں�ת��*/
#define MMF_DUMPABLE      				0
/*ֻ��root�ɶ��ں�ת���ļ�*/
#define MMF_DUMP_SECURELY 				1
#define MMF_DUMPABLE_BITS 				2

/*�ں�ת������λ*/
#define MMF_DUMP_ANON_PRIVATE			2
#define MMF_DUMP_ANON_SHARED			3
#define MMF_DUMP_MAPPED_PRIVATE			4
#define MMF_DUMP_MAPPED_SHARED			5
#define MMF_DUMP_ELF_HEADERS			6
#define MMF_DUMP_FILTER_SHIFT			MMF_DUMPABLE_BITS
#define MMF_DUMP_FILTER_BITS			5
#define MMF_DUMP_FILTER_MASK (((1 << MMF_DUMP_FILTER_BITS) - 1) << MMF_DUMP_FILTER_SHIFT)
#define MMF_DUMP_FILTER_DEFAULT	((1 << MMF_DUMP_ANON_PRIVATE) |	(1 << MMF_DUMP_ANON_SHARED))
/*�������õ��źŴ���������Ϣ*/
struct sighand_struct
{
	/*����ýṹ�Ľ�����Ŀ*/
	atomic_t		count;
	/*�źŴ������*/
	struct k_sigaction	action[_NSIG];
	/*������*/
	spinlock_t		siglock;
	/*�ȴ�����*/
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
/*�ź��������Լ�û��������Ϊһ��������ź�����������impliesһ��������źŴ�����������
��������źŴ�������������Ϊ�������ź��������ĳ���*/
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
	/*���̵�ITIMER_REAL��ʱ��*/
	struct hrtimer real_timer;
	/**/
	struct task_struct *tsk;
	/**/
	ktime_t it_real_incr;

	/* ITIMER_PROF and ITIMER_VIRTUAL timers for the process */
	/*���̵�ITIMER_PROF��ITIMER_VIRTUAL��ʱ������һ�ζ�ʱ�����ڵ�ʱ��*/
	cputime_t it_prof_expires, it_virt_expires;
	/*��ʱ���ڶ೤ʱ��֮�����*/
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
		/*������ID*/
		pid_t __pgrp;
	};
	/**/
	struct pid *tty_old_pgrp;

	union
	{
		/**/
		pid_t session __deprecated;
		/*�ỰID*/
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
	/*�������������̺߳͸����ڴ������������Ĺ¶��߳��ۼ���Դ������*/
	/*�������û�̬���е�ʱ��*/
	cputime_t utime;
	/*�������ں�̬���е�ʱ��*/
	cputime_t stime;
	/*�ܹ����û�̬���е�ʱ��*/
	cputime_t cutime;
	/*�ܹ����ں�̬���е�ʱ��*/
	cputime_t cstime;
	/**/
	cputime_t gtime;
	/**/
	cputime_t cgtime;
	/*��Ϊ�ȴ���Դ��ԭ�����cpu�Ĵ���*/
	unsigned long nvcsw;
	/*��Ϊʱ��Ƭ�þ������Ȼ���ռ���л��Ĵ���*/
	unsigned long nivcsw;
	/*��Ը�������л�����*/
	unsigned long cnvcsw;
	/*����Ը�������л�����*/
	unsigned long cnivcsw;
	/**/
	unsigned long min_flt;
	/*ȱҳ�жϵĴ���*/
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
	/*������Դ����*/
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
 /*�ýṹ������һЩͳ�����ݣ�ÿ���û������ռ�����û���Դʹ�õ�ͳ�ƣ������������ռ�
��ȫ�޹أ���root�û���ͳ��Ҳ����ˣ�������Ϊ���ڿ�¡һ���û������ռ�ʱ��Ϊ��ǰ�û���
root�û����������µ�user_structʵ��*/
struct user_struct
{
	/*�ýṹ�����õĴ���*/
	atomic_t __count;
	/*�û�ӵ�еĽ�����Ŀ*/
	atomic_t processes;
	/*���û��Ѵ��ļ�����Ŀ*/
	atomic_t files;
	/*���û����д������źŵ���Ŀ*/
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
	/*���û���ӵ��û������ռ���ɢ�б�*/
	struct hlist_node uidhash_node;
	/*�û�id*/
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
/*�ڵ�ǰ�������ؾ���*/
#define SD_LOAD_BALANCE		1	/* Do load balancing on this domain. */
/*����ʱ���ؾ���*/
#define SD_BALANCE_NEWIDLE	2	/* Balance when about to become idle */
/*execʱ���ؾ���*/
#define SD_BALANCE_EXEC		4	/* Balance on exec */
/*fork��cloneʱ���ؾ���*/
#define SD_BALANCE_FORK		8	/* Balance on fork, clone */
/*���̻���ʱ���ѿ���cpu*/
#define SD_WAKE_IDLE		16	/* Wake to idle CPU on task wakeup */
/*���Ѻ�������cpu*/
#define SD_WAKE_AFFINE		32	/* Wake task to waking CPU */
/*���̻���ʱִ�и��ؾ���*/
#define SD_WAKE_BALANCE		64	/* Perform balancing at task wakeup */
/**/
#define SD_SHARE_CPUPOWER	128	/* Domain members share cpu power */
/*Ϊ��Լ��Դ������*/
#define SD_POWERSAVINGS_BALANCE	256	/* Balance for power savings */
/**/
#define SD_SHARE_PKG_RESOURCES	512	/* Domain members share cpu pkg resources */
/*����һ�����ؾ���ʵ��*/
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
	/*�����򸸽ڵ㣬��ߵ����������NULL�ս�*/
	struct sched_domain *parent;
	/*�������ӽڵ㣬��͵����������NULL�ս�*/
	struct sched_domain *child;
	/*������������*/
	struct sched_group *groups;
	/**/
	cpumask_t span;			/* span of all CPUs in this domain */
	/*��С�������ʱ�䣬ms��λ*/
	unsigned long min_interval;	/* Minimum balance interval ms */
	/*��������ʱ�䣬ms��λ*/
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
 /*��������Ϣ*/
 #define get_group_info(group_info) do { \
	atomic_inc(&(group_info)->usage); 			\
} while (0)
/*���������Ϣ�����ã�����Ϣû�б�����ʱ���ͷŸ�����Ϣ*/
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

/*������*/
struct sched_class
{
	/*��һ��������*/
	const struct sched_class *next;
	/*�����̲����������*/
	void (*enqueue_task) (struct rq *rq, struct task_struct *p, int wakeup);
	/*�����̴Ӿ���������ɾ��*/
	void (*dequeue_task) (struct rq *rq, struct task_struct *p, int sleep);
	/*�������е�ǰ���н��̷���cpu�������ھ����������Ŷ�*/
	void (*yield_task) (struct rq *rq);
	/*�鿴�����Ƿ������ռ�������е�ǰ�������еĽ���*/
	void (*check_preempt_curr) (struct rq *rq, struct task_struct *p);
	/*������һ����Ҫ���еĽ���*/
	struct task_struct * (*pick_next_task) (struct rq *rq);
	/**/
	void (*put_prev_task) (struct rq *rq, struct task_struct *p);

#ifdef CONFIG_SMP
	/*�ദ����ϵͳ�ϣ��ں˱��뿼�Ǽ�����������⣬��ȷ�����õĵ��ȣ�1��CPU���ɱ��뾡
	���ܹ�ƽ�������еĴ������Ϲ������һ������������3��������Ӧ�ó��򣬶���һ��ֻ��
	������н��̣�����û������ģ�2��������ϵͳ��ĳЩ���������׺��ԣ�affinity��������
	�����õġ�������4��CPUϵͳ�У����Խ������ܼ���Ӧ�ó���󶨵�ǰ3��CPU����ʣ��ģ���
	��ʽ���������ڵ�4��CPU�����У�3���ں˱����ܹ������̴�һ��CPUǨ�Ƶ���һ��������ѡ��
	�������ʹ�ã���Ϊ��������Σ�����ܡ���С��SMPϵͳ��CPU���ٻ������������⡣������
	������ϵͳ��CPU��Ǩ�ƽ��̴�ǰʹ�õ������ڴ��������������ף���˶Ըý����ڴ�ķ���
	���۸߰������̶��ض�CPU���׺��ԣ�������task_struct��cpus_allowed��Ա�С�Linux�ṩ��
	sched_setaffinityϵͳ���ã����޸Ľ�����CPU�����з����ϵ����SMPϵͳ�ϣ�ÿ����������
	�ĵ��ȷ�������������������ĺ�����load_balance��move_one_task����Щ��������ֱ�Ӹ���
	�����ؾ��⡣ÿ���ں���Ϊ�б�Ҫ���¾���ʱ�����ĵ��������붼�������Щ�������ض���
	��������ĺ�������������һ����������ʹ�ú��ĵ������ܹ��������п���Ǩ�Ƶ���һ������
	�ı�ѡ���̣�����������������ڲ��ṹ������Ϊ����������¶�����ĵ�������load_balance
	����ָ�������һ���Եĺ���load_balance����move_one_task��ʹ����iter_move_one_task��
	��Щ�������ڲ�ͬ��Ŀ�ġ�iter_move_one_task����æµ�ľ��������Ƴ�һ�����̣�Ǩ�Ƶ���
	ǰCPU�ľ������С�load_balance���������æ�ľ������з��������̵���ǰCPU�����ƶ���
	���ɲ��ܱ�max_load_move���ࡣ
	���ؾ��⴦���������η���ģ���SMPϵͳ�ϣ������Ե���������scheduler_tick��������
	���������ϵͳ����Ҫ������֮�󣬻����trigger_load_balance�������������SCHEDULE_
	SOFTIRQ���ж�softIRQ��Ӳ���жϵ����ģ�⣩�����ж�ȷ�������ʵ���ʱ��ִ��
	run_rebalance_domains���ú������նԵ�ǰCPU����rebalance_domains��ʵ�ָ��ؾ��⡣
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
/*���̵���Ҫ�Բ��������ȼ�ָ�������һ���Ҫ���Ǳ�����task_struct->se.load�ĸ���Ȩ��
��set_load_weight������ݽ������ͼ��侲̬���ȼ����㸺��Ȩ��*/
struct load_weight
{
	/*����Ȩ��*/
	unsigned long weight;
	/*���渺��Ȩ�صĵ���������ʹ������ͨ��long���ͣ��ں��޷�ֱ�Ӵ洢1/weight����������ڳ˷�����λ��ִ��
	����*/
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
 /*���������Բ����Ƚ��̸�һ���ʵ�壬�����Ҫһ���ʵ������ݽṹ����������ʵ�壬��������*/
struct sched_entity
{
	/*����Ȩ�أ����ڸ��ؾ��⡣Ȩ�ؾ����˸���ʵ��ռ�����ܸ��ɵı��������㸺��Ȩ���ǵ�����
	��һ�����Σ���ΪCFS���������ʱ�ӵ��ٶ����������븺��*/
	struct load_weight	load;
	/*��׼����㣬ʹ��ʵ������ھ��������Ͻ��ܵ���*/
	struct rb_node		run_node;
	/*��ʵ�嵱ǰ�Ƿ����ھ��������Ͻ��ܵ���*/
	unsigned int		on_rq;
	/*ʵ�忪ʼִ�е�ʱ��*/
	u64			exec_start;
	/*��¼ʹ�õ�CPUʱ�䣬��������ȫ��ƽ������*/
	u64			sum_exec_runtime;
	/*����ִ���ڼ�����ʱ������ʧ��ʱ������*/
	u64			vruntime;
	/*�ڽ��̱�����cpuʱ���䵱ǰ��sum_exec_runtimeֵ�ۼƺͣ���ֵ��������������Ҳ���ǽ���
	֮ǰ�ܼ����е�ʱ��*/
	u64			prev_sum_exec_runtime;
/*�����ں�ʱ�����˵�����ͳ��ʱ������ͳ�Ƴ�Ա*/
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
/*���������ʱ�����ĵ�ͳ�Ƴ�Ա*/
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
	/*���̵�ǰ��״̬��-1�������� 0������ >0ֹͣ*/
	volatile long state;
	/*���̵��ں�ջ��ͨ��ϵͳ���ý����ں˿ռ�ʱ�����������Ϣ����Ҫ�ǼĴ�����Ϣ���˳��ں�̬ʱ
	�����ں�ջ��ȡ���ݣ�����ִ�С�ͨ��alloc_thread_info����2��Ĭ�ϣ�ҳ��ͨ��free_thread_info�ͷ�*/
	void *stack;
	/*����������ʹ�ü�����2��ʾ�������ڱ�ʹ��*/
	atomic_t usage;
	/*PF_��ͷ�Ľ��̱�ʶ*/
	unsigned int flags;
	/*�Ƿ����õ��Թ��ܣ�0��ʾ������*/
	unsigned int ptrace;
	/*��ȡ���ں����Ĵ�����-1��ʾ����δ�����*/
	int lock_depth;

#ifdef CONFIG_SMP
#ifdef __ARCH_WANT_UNLOCKED_CTXSW
	/*��ϵ�ṹ���������л�ʱȷ�������������ڵ�cpu*/
	int oncpu;
#endif
#endif
	/*������ʹ�õĽ��̵Ķ�̬���ȼ�����Щ�������Ҫ��ʱ��߽��̵����ȼ���������ȼ���ת�����
	����Ϊ���ǳ־õģ���̬����ͨ���ȼ�����*/
	int prio;
	/*��̬���ȼ�������ʱ��������ȼ�������nice��sched_setscheduler�޸ģ�����ִ���ڼ䲻��*/
	int static_prio;
	/*���ݽ��̵ľ�̬���ȼ��͵��Ȳ���ȷ������ʹ��ͨ���̺�ʵʱ���̾�����ͬ�ľ�̬���ȼ�����
	��ͨ���ȼ�Ҳ�ǲ�ͬ�ģ����̱�������fork��ʱ���̳д������̵���ͨ���ȼ�*/
	int normal_prio;
	/**/
	struct list_head run_list;
	/*���������ĵ�����{stop/rt/fair/idle}_sched_class*/
	const struct sched_class *sched_class;
	/* ������ͨ���̵ĵ���ʵ�塣�����������ڵ��Ƚ��̣������Դ�������ʵ�塣��ʵ�������
	������cpuʱ��������һ��Ľ�����֮����䣬��õ���cpuʱ�����������ٴη��䣩������һ��
	Ҫ���������ֱ�Ӳ������̣����Ǵ���ɵ���ʵ�壬һ��ʵ����sched_entityʵ�����������ɾ�
	�˲�������task_struct*/
	struct sched_entity se;

#ifdef CONFIG_PREEMPT_NOTIFIERS
	/*��ǰ���̱���ռ���ص���ʱ���������Ϣ��ɢ�б�ͷ*/
	struct hlist_head preempt_notifiers;
#endif
	/*����������������ȼ�*/
	unsigned short ioprio;
	/*
	 * fpu_counter contains the number of consecutive context switches
	 * that the FPU is used. If this is over a threshold, the lazy fpu
	 * saving becomes unlazy to save the trap. This is an unsigned char
	 * so that after 256 times the counter wraps and the behavior turns
	 * lazy again; this to deal with bursty apps that only use FPU for
	 * a short time
	 */
	 /*fpuʹ��ʱ�������л�����*/
	unsigned char fpu_counter;
	/**/
	s8 oomkilladj; /* OOM kill score adjustment (bit shift). */
#ifdef CONFIG_BLK_DEV_IO_TRACE
	/*���豸io����ټ���*/
	unsigned int btrace_seq;
#endif
	/*���Ȳ���*/
	unsigned int policy;
	/*cpuλͼ���ڶദ������ʹ�ã����ڿ��ƽ��̿������ĸ�CPU������*/
	cpumask_t cpus_allowed;
	/*SCHED_RR����ʱ��ʱ��Ƭ*/
	unsigned int time_slice;

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	/*������ͳ�ƽ�������ʱ�����Ϣ*/
	struct sched_info sched_info;
#endif
	/*����ǰ���̵�task_struct�������ں˽����б��У�������������*/
	struct list_head tasks;
	/*
	 * ptrace_list/ptrace_children forms the list of my children
	 * that were stolen by a ptracer.
	 */
	 /*���б����Ե��ӽ�������*/
	struct list_head ptrace_children;
	/*���Խ�������*/
	struct list_head ptrace_list;
	/*mmָ�������ӵ�е��ڴ���������������ͨ���̶��ԣ�������ָ�������ֵ��ͬ���ں��̲߳�ӵ
	���κ��ڴ������������ԣ�����mm��Ա���ǿգ����ں��߳�����ʱ,����active_mm��Ա����ʼ��Ϊ
	ǰһ�����н��̵�active_mmֵ*/
	struct mm_struct *mm;
	/*active_mmָ���������ʱ��ʹ�õ��ڴ�������*/
	struct mm_struct *active_mm;

	/*Linux֧�ֵĿ�ִ�ж������ļ��ļ�*/
	struct linux_binfmt *binfmt;
	/*���̵��˳�״̬��*/
	int exit_state;
	/*�������ý��̵���ֹ���룬���ֵҪô��_exit()��exit_groupϵͳ���ã�������ֹ����Ҫô��
	���ں��ṩ��һ��������ţ��쳣��ֹ��*/
	int exit_code;
	/*-1��ʾ��ĳ���߳����е�һԱ��ֻ�е��߳�������һ����Ա��ֹʱ���Ż����һ���źţ���
	֪ͨ�߳����鳤�ĸ�����*/
	int exit_signal;
	/*��������ֹʱ���͵��ź�*/
	int pdeath_signal;
	/*���ڴ���ͬABI*/
	unsigned int personality;
	/*���̴������Ƿ���ִ��exec����*/
	unsigned did_exec:1;
	/*���̵�ȫ��PID*/
	pid_t pid;
	/*���̵�ȫ���߳����鳤PID����Linuxϵͳ�У�һ���߳����е������߳�ʹ�ú͸��߳�����鳤
	(�����еĵ�һ������������)��ͬ��PID�����������tgid��Ա�С�ֻ���߳����鳤��pid��Ա�Ż�
	������Ϊ��tgid��ͬ��ֵ��ע�⣬getpid()ϵͳ���÷��ص��ǵ�ǰ���̵�tgidֵ������pidֵ*/
	pid_t tgid;

#ifdef CONFIG_CC_STACKPROTECTOR
	/* Canary value for the -fstack-protector gcc feature */
	/*��ֹ�����ں�ջ�����0day��������������ʱ����ÿ���������ĺ�����ø�ֵ���������Ϊ�ڱ���
	�����������ʱ���϶����ƻ���ֵ���ں˼�⵽�ô���ʱ�͵������ں˱���ʱ	-fstack-protector
	����ѡ�����ӣ���ʹgcc���ջ���*/
	unsigned long stack_canary;
#endif
	/*ָ�򴴽����ĸ����̣�����������ĸ����̲�������ָ���ʼ������*/
	struct task_struct *real_parent; /* real parent process (when being debugged) */
	/*ָ�򸸽��̣�������ֹʱ�����������ĸ����̷����ź�*/
	struct task_struct *parent;	/* parent process */
	/*�ӽ��������ͷ���������������ӽ���*/
	struct list_head children;
	/*���ӵ������̵��ӽ��������µ��ӽ�������sibling�������ʼλ�ã�����ζ�ſ����ؽ�����
	��֧��ʱ��˳��*/
	struct list_head sibling;
	/*�߳����鳤*/
	struct task_struct *group_leader;
	/*pid_link�ɽ�task_struct���ӵ���ͷ��struct pid�е�ɢ�б���*/
	struct pid_link pids[PIDTYPE_MAX];
	/*�߳��鱣��ý�����ӵ�е��߳�*/
	struct list_head thread_group;
	/*��ִ��do_forkʱ����������ر��ʶ����vfork_done��ָ��һ�������ַ*/
	struct completion *vfork_done;		/*for vfork()*/
	/*���copy_process������clone_flags������ֵ����ΪCLONE_CHILD_SETTID��CLONE_CHILD_CLEARTID��
	����child_tidptr������ֵ�ֱ��Ƶ�set_child_tid��clear_child_tid��Ա����Щ��־˵������ı�
	�ӽ����û�̬��ַ�ռ��child_tidptr��ָ��ı�����ֵ*/
	int __user *set_child_tid;
	/*�ӽ����˳�ʱ����õ�ַ*/
	int __user *clear_child_tid;
	/*ʵʱ���̵����ȼ���ֵԽ�����ȼ�Խ��*/
	unsigned int rt_priority;
	/*���̱��α��������û�̬�����еĽ���������������Ƶ�ʣ�*/
	cputime_t utime;
	/*���̱��α��������ں�̬�����еĽ�����*/
	cputime_t stime;
	/*�������û�̬�����е�ʱ��*/
	cputime_t utimescaled;
	/*�������ں�̬�����е�ʱ��*/
	cputime_t stimescaled;
	/*�Խ��ļ��������������ʱ��*/
	cputime_t gtime;
	/*���ε���֮ǰ�������Ѿ����û�̬�����е�ʱ���ܼ�*/
	cputime_t prev_utime;
	/*���ε���֮ǰ�������Ѿ����û�̬�����е�ʱ���ܼ�*/
	cputime_t prev_stime;
	/*����ӦΪ�ȴ���Դ��ԭ�����cpu�Ĵ���*/
	unsigned long nvcsw;
	/*��������ʱ����ʱ��Ƭ����򱻸������ȼ���ռ��ԭ�������cpu�Ĵ���*/
	unsigned long nivcsw;
	/*���̴���ʱ����ʱ��ʱ��*/
	struct timespec start_time;
	/*����������ʼִ�е�ʱ��*/
	struct timespec real_start_time;
	/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
	/*�����Ѿ��ڽ�������*/
	unsigned long min_flt;
	/*ȱҳ�жϴ�����������Ҫ��󱸴洢���ж�ȡ*/
	unsigned long maj_flt;
	/**/
  	cputime_t it_prof_expires;
	/**/
	cputime_t it_virt_expires;
	/**/
	unsigned long long it_sched_expires;
	/**/
	struct list_head cpu_timers[3];

	/* ����ƾ֤ */
	/*��ʵ�û�ID����ʵ��ID����Щƾ֤������̻������ʵ���û�������ݣ����Ƕ����˽��̵Ŀ͹�
	�����ģ���ͨ���̳��������ý��̵��û�*/
	uid_t uid;
	/*��Чuid��ȷ��������ִ��ʱ���е�Ȩ�޺���Ȩ����ĳЩ����£���Ч�û�ID����Ч��ID������
	���ǵĲ�ͬ�����磬������ִ��һ������������setuid��setgidȨ�޵ĳ���ʱ������ЧID���ܱ��
	�˸ó����������ID���Ӷ�ʹ�����ܹ���һ��ʱ������ʱ���и������ߵ�Ȩ��*/
	uid_t euid;
	/*������û�ID��saved user ID���ͱ������ID��saved group ID������Щƾ֤���ڱ���������ʱ
	������ЧIDʱ��ԭʼ�û�ID����ID��ͨ������Ȩ����������ʹ�ã����������ִ����Ȩ������ָ�
	����ԭʼ���*/
	uid_t suid;
	/*�ļ�ϵͳ�û�ID���ļ�ϵͳ��ID����Щƾ֤�ڽ��̷����ļ�ϵͳ�ϵ��ļ�ʱʹ�ã�����Ϊ�ļ���
	�ز����ṩ����ʱ��ݸ��ģ�����ЧID������������������ļ������о��в�ͬ��Ȩ�ޣ�ͬʱ����
	����ЧID��������Ŀ��*/
	uid_t fsuid;
	/*������ʵ��gid*/
	gid_t gid;
	/*��Ч��id*/
	gid_t egid;
	/*��ʱ�������ʵgid*/
	gid_t sgid;
	/*�����ļ�ʱ��gid*/
	gid_t fsgid;
	/*�����������������Ϣ*/
	struct group_info *group_info;
	/*���̵���ЧȨ��*/
	kernel_cap_t cap_effective;
	/*���̼̳е�Ȩ��*/
	kernel_cap_t cap_inheritable;
	/*���̱������Ȩ��*/
	kernel_cap_t cap_permitted;
	/*�Ƿ�������̱����ض�Ȩ��*/
	unsigned keep_capabilities:1;
	/*�����������û�*/
	struct user_struct *user;
#ifdef CONFIG_KEYS
	/**/
	struct key *request_key_auth;	/* assumed request_key authority */
	/**/
	struct key *thread_keyring;	/* keyring private to this thread */
	/**/
	unsigned char jit_keyring;	/* default keyring to attach requested keys to */
#endif
	/*�����������ͨ��get/set_task_comm()������ȡ�Ĳ���·���Ŀ�ִ���ļ���*/
	char comm[TASK_COMM_LEN];
	/*�����ڲ��һ�������ʱ��ֹ����ѭ����Ĭ������£��ں�����MAX_NESTED_LINKS��ͨ��Ϊ
	8�������ݹ�*/
	int link_count;
	/*����·���������ӵ������Ŀ��Ĭ��40�����������ӣ�Ӳ���룬����ͨ��Ԥ���������Ŷ���*/
	int total_link_count;
#ifdef CONFIG_SYSVIPC
	/*ipc���*/
	struct sysv_sem sysvsem;
#endif
/* CPU-specific state of this task */
	struct thread_struct thread;
	/*�����ļ�ϵͳ�����Ϣ*/
	struct fs_struct *fs;
	/*�Ѵ��ļ������Ϣ*/
	struct files_struct *files;
	/*�����ռ���Ϣ*/
	struct nsproxy *nsproxy;
	/*�ź�������*/
	struct signal_struct *signal;
	/*�������õ��źŴ���������Ϣ*/
	struct sighand_struct *sighand;
	/*���б��������ź�*/
	sigset_t blocked;
	/**/
	sigset_t real_blocked;
	/*��TIF_RESTORE_SIGMASK�ָ�*/
	sigset_t saved_sigmask;
	/*pengding����һ���������������Ѿ���������Ȼ�д��ں˴�����ź�*/
	struct sigpending pending;
	/*�źŴ��������ջ�ĵ�ַ*/
	unsigned long sas_ss_sp;
	/*�źŴ���ջ�Ĵ�С*/
	size_t sas_ss_size;
	/*�豸����������notifierָ��ĺ������������̵�ĳЩ�ź�*/
	int (*notifier)(void *priv);
	/*notifierָ��ĺ�������ʹ�õ�����*/
	void *notifier_data;
	/*��ʶ�������źŵ�λ����*/
	sigset_t *notifier_mask;
#ifdef CONFIG_SECURITY
	/*LSM��Linux Security Module����һ�����������������ж�����ƵĻ��ƣ�����linux��֧��
	���LSMѡ�LSMͨ��Ϊϵͳ�ж�����б�ǣ���Ӧ��һ����������ƾ���һ����ǵ�����Ծ���
	��һ����ǵĶ���ɽ��еĲ�����LSM�Ļ���ʵ����ͨ�����ں�������һ���ɲ�εİ�ȫģ��ӿڣ�
	����ͬ�İ�ȫģ��ʵ���ض��İ�ȫ���ԡ�ÿ����ȫģ�鶼���Զ�ϵͳ�еĶ������ļ������̡�
	�������ӵȣ����б�ǣ���������һ����������ƶ���֮��ķ��ʺͲ���*/
	void *security;
#endif
	/*����������*/
	struct audit_context *audit_context;
	/*���̰�ȫ�������*/
	seccomp_t seccomp;

/* Thread group tracking */
   	/**/
	u32 parent_exec_id;
   	/**/
	u32 self_exec_id;
	/*������ͷ��ڴ��ļ�/�ļ�ϵͳ/�ն��豸/���ƹؼ��ֵı�����*/
	spinlock_t alloc_lock;
	/*���ȼ��̳�PI��priority inheritance�����ݱ�������task_rq_lock����ʹ�õ�������*/
	spinlock_t pi_lock;

#ifdef CONFIG_RT_MUTEXES
	/* PI waiters blocked on a rt_mutex held by this task */
	/**/
	struct plist_head pi_waiters;
	/*�����������ȼ��̳д���*/
	struct rt_mutex_waiter *pi_blocked_on;
#endif

#ifdef CONFIG_DEBUG_MUTEXES
	/*������⻥����*/
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

	/*��־�ļ�ϵͳ��Ϣ*/
	void *journal_info;

	/*���豸ջ��Ϣ�����ܵݹ�������û��ռ�û�����⣬���ں���ջ�ռ�ǳ����ޣ���˿��ܻ�����
	���⣬�����Ҫȷ��һ�������ֵ�������Ƶݹ�������ȣ���������struct bioָ���Ա���ڽ�
	�ݹ������������Ϊ1�������Ͳ��ᶪʧ�κ��ύ��bio*/
	struct bio *bio_list;
	struct bio **bio_tail;

	/*�����ڴ�״̬��Ϣ*/
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
	/*���ж�ռ�ļ�ϵͳ��Դ*/
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
/*�û����̵�������ȼ�*/
#define MAX_USER_RT_PRIO		100
#define MAX_RT_PRIO				MAX_USER_RT_PRIO
/*�û����̵���С���ȼ�*/
#define MAX_PRIO				(MAX_RT_PRIO + 40)
/*�û�����Ĭ�����ȼ�*/
#define DEFAULT_PRIO			(MAX_RT_PRIO + 20)
/*�������ȼ��Ƿ�����ʵʱ���ȼ�*/
static inline int rt_prio(int prio)
{
	if (unlikely(prio < MAX_RT_PRIO))
		return 1;
	return 0;
}

/*���Խ����Ƿ�����ʵʱ���̣��䶯̬���ȼ��Ƿ�����ʵʱ���ȼ���*/
static inline int rt_task(struct task_struct *p)
{
	return rt_prio(p->prio);
}

/*���ý��̵ĻỰID*/
static inline void set_task_session(struct task_struct *tsk, pid_t session)
{
	tsk->signal->__session = session;
}

/*���ý��̵Ľ�����ID*/
static inline void set_task_pgrp(struct task_struct *tsk, pid_t pgrp)
{
	tsk->signal->__pgrp = pgrp;
}

/*��ȡ���̵�pid*/
static inline struct pid *task_pid(struct task_struct *task)
{
	return task->pids[PIDTYPE_PID].pid;
}

/*��ȡ���������߳�����鳤pid*/
static inline struct pid *task_tgid(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_PID].pid;
}

/*��ȡ�������ڽ�������鳤pid*/
static inline struct pid *task_pgrp(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_PGID].pid;
}

/*��ȡ�������ڻỰ���鳤pid*/
static inline struct pid *task_session(struct task_struct *task)
{
	return task->group_leader->pids[PIDTYPE_SID].pid;
}

struct pid_namespace;

/*
   task_xid_nr():��ȡȫ��id����ʼ�������������ռ��пɼ���id
   task_xid_vnr()    :����id���������������ռ��id������ͬһ�����ռ�Ľ��������ı�����ʱ������
   task_xid_nr_ns()    :�ض������ռ�id
   set_task_vxid()    :��һ�����̸�ֵһ������id
*/

/*��ȡ���̵�ȫ��pid���*/
static inline pid_t task_pid_nr(struct task_struct *tsk)
{
	return tsk->pid;
}

pid_t task_pid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);
/*��ȡ����������ײ������ռ��еľֲ�pid���*/
static inline pid_t task_pid_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_pid(tsk));
}

/*����߳������߳����鳤��pid*/
static inline pid_t task_tgid_nr(struct task_struct *tsk)
{
	return tsk->tgid;
}

pid_t task_tgid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);

/*����߳������߳����鳤����ײ������ռ��еľֲ�pid���*/
static inline pid_t task_tgid_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_tgid(tsk));
}

/*��ȡ���̵����ڽ������鳤pid*/
static inline pid_t task_pgrp_nr(struct task_struct *tsk)
{
	return tsk->signal->__pgrp;
}

pid_t task_pgrp_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);

/*��ý������ڽ������鳤����ײ������ռ��еľֲ�pid���*/
static inline pid_t task_pgrp_vnr(struct task_struct *tsk)
{
	return pid_vnr(task_pgrp(tsk));
}

/*��ȡ�������ڻỰ���鳤pid*/
static inline pid_t task_session_nr(struct task_struct *tsk)
{
	return tsk->signal->__session;
}

pid_t task_session_nr_ns(struct task_struct *tsk, struct pid_namespace *ns);

/*��ý������ڻỰ���鳤����ײ������ռ��еľֲ�pid���*/
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
/*�������Ƿ�û����ȫ���������ٴ��ڽ�ʬ״̬�������pid�������ˣ����̽ṹ����
ָ��һ�����ܱ�����*/
static inline int pid_alive(struct task_struct *p)
{
	return p->pids[PIDTYPE_PID].pid != NULL;
}

/*�������Ƿ��ǳ�ʼ�����̡��Ƿ����ں˴������û��ռ��е�һ������*/
static inline int is_global_init(struct task_struct *tsk)
{
	return tsk->pid == 1;
}

extern int is_container_init(struct task_struct *tsk);

extern struct pid *cad_pid;

extern void free_task(struct task_struct *tsk);
/*����ָ������*/
#define get_task_struct(tsk) do { atomic_inc(&(tsk)->usage); } while(0)

extern void __put_task_struct(struct task_struct *t);

/*ȡ����ָ�����̵����ã��������û�б����á����ͷŸý�����Դ*/
static inline void put_task_struct(struct task_struct *t)
{
	if (atomic_dec_and_test(&t->usage))
		__put_task_struct(t);
}

/*��ӡ���뾯�棬Ŀǰ����486��ʵ��*/
#define PF_ALIGNWARN			0x00000001
/*�������ڱ�����*/
#define PF_STARTING				0x00000002
/*�������ڱ�����*/
#define PF_EXITING				0x00000004
/**/
#define PF_EXITPIDONE			0x00000008	/* pi exit done on shut down */
/*����cpu*/
#define PF_VCPU					0x00000010
/*fork��û��ִ��exec*/
#define PF_FORKNOEXEC			0x00000040
/*ʹ�ó����û�Ȩ��*/
#define PF_SUPERPRIV			0x00000100
/*�ڴ�ת��*/
#define PF_DUMPCORE				0x00000200
/*���ź���ֹ*/
#define PF_SIGNALED				0x00000400
/*���������ڴ�*/
#define PF_MEMALLOC				0x00000800
/*��������̻�д����*/
#define PF_FLUSHER				0x00001000
/*ʹ��ǰ��ʼ��Ϊ���õ�fpu*/
#define PF_USED_MATH			0x00002000
/*��ǰ���̲�Ӧ�ñ�����*/
#define PF_NOFREEZE				0x00008000
/*������Ϊϵͳ���𶳽�*/
#define PF_FROZEN				0x00010000
/*�ļ�ϵͳ����*/
#define PF_FSTRANS				0x00020000
/*�������ں�ҳ�潻������*/
#define PF_KSWAPD				0x00040000
/*ҳ�滻������*/
#define PF_SWAPOFF				0x00080000
/**/
#define PF_LESS_THROTTLE		0x00100000	/* Throttle me less: I clean memory */
/*����ʹ�ý��������ַ�ռ���ں��߳�*/
#define PF_BORROWED_MM			0x00200000
/*����������ַ�ռ�*/
#define PF_RANDOMIZE			0x00400000
/*����д������*/
#define PF_SWAPWRITE			0x00800000
/**/
#define PF_SPREAD_PAGE			0x01000000	/* Spread page cache over cpuset */
/**/
#define PF_SPREAD_SLAB			0x02000000	/* Spread some slab caches over cpuset */
/*��Ĭ�ϵķ�һ���ڴ���ʲ���*/
#define PF_MEMPOLICY			0x10000000
/*����ʵʱ���������߳�*/
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

/*sched_clock()�����ĸ��ٵ���΢��׼ȷ�Ľ����ں��ڲ�ʹ�õ�per_cpu clock*/
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

/*�����ں�ջ��thread_info���õ������壬thead_infoλ�ڵ͵�ַ���ں�ջλ�ڸߵ�ַ��������*/
union thread_union
{
	/*�������߳�����������ض��ڴ���������Ϣ*/
	struct thread_info thread_info;
	/*�ں�ջ��Ĭ�ϴ�С8K*/
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

#ifndef __HAVE_ARCH_KSTACK_END
/*�ں�ջ������ַ*/
static inline int kstack_end(void *addr)
{
	/*�ɿ����ں�ջ��β��⣬��Щ�߼���Դ����bios�汾������ջ��0x1FFC*/
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
/*����ָ���û�*/
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

/*������̵ĸ��ӹ�ϵ���ӽ��̵�sibling������ɾ���ý�㲢��ʼ��*/
#define remove_parent(p)	list_del_init(&(p)->sibling)
/*�µ��ӽ��̷���sibling�������ʼλ�ã�����ζ�ſ����ؽ����̷�֧��ʱ��˳��*/
#define add_parent(p)		list_add_tail(&(p)->sibling,&(p)->parent->children)
/*rcu�����´ӽ��̵�tasks�����л�ȡ��һ�����̣�tasks˫��������ϵͳ�����н���*/
#define next_task(p)	list_entry(rcu_dereference((p)->tasks.next), struct task_struct, tasks)
/*rcu�����´ӳ�ʼ�����̿�ʼ������ϵͳ�����н���*/
#define for_each_process(p) for (p = &init_task ; (p = next_task(p)) != &init_task ; )

/*ע�⣡do_each_thread��while_each_thread��һ��˫����ѭ������ˡ�break��������Ԥ��
���������ã���ʱҪ��goto*/
/**/
#define do_each_thread(g, t) \
	for (g = t = &init_task ; (g = t = next_task(g)) != &init_task ; ) do

/**/
#define while_each_thread(g, t) while ((t = next_thread(t)) != g)

/*�����߳��Ƿ����������߳�����鳤�����ǻ���pid==tgid���*/
#define thread_group_leader(p)	(p == p->group_leader)

/* Do to the insanities of de_thread it is possible for a process
 * to have the pid of the thread group leader without actually being
 * the thread group leader.  For iteration through the pids in proc
 * all we care about is that we have a task with the appropriate
 * pid, we don't actually care if we have the right task.
 */
/*�жϽ���pid�Ƿ����������̵߳��߳����鳤pid��ͬ*/
static inline int has_group_leader_pid(struct task_struct *p)
{
	return p->pid == p->tgid;
}

/*�ж������߳��Ƿ�ͬ��һ���߳��飬�����߳������߳�����鳤�Ƿ���ͬ*/
static inline int same_thread_group(struct task_struct *p1, struct task_struct *p2)
{
	return p1->tgid == p2->tgid;
}

/*��ȡ�߳������߳����е���һ���߳�*/
static inline struct task_struct *next_thread(const struct task_struct *p)
{
	return list_entry(rcu_dereference(p->thread_group.next),
			  struct task_struct, thread_group);
}

/*���Խ����Ƿ�û���̣߳��ж����߳����Ƿ�Ϊ��*/
static inline int thread_group_empty(struct task_struct *p)
{
	return list_empty(&p->thread_group);
}

/*�жϽ��������Ѵ������̡߳����̵��߳���ǿ��ҽ������������߳�����鳤*/
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
/*�������̵��ļ�ϵͳ���Ѵ��ļ����ڴ桢����Ϣ�����������ܳ�subscriptions��ͬ��wait4��
��������procfs�����ļ�ϵͳ��Ҳ��ס���յ�task.io_context�ͷţ�Ҳ�������̵�cpu���Ϻ�
cgroup��ϵͳ*/
/*����task_struct��alloc_lock��*/
static inline void task_lock(struct task_struct *p)
{
	spin_lock(&p->alloc_lock);
}

/*�ͷ�task_struct��alloc_lock��*/
static inline void task_unlock(struct task_struct *p)
{
	spin_unlock(&p->alloc_lock);
}

extern struct sighand_struct *lock_task_sighand(struct task_struct *tsk, unsigned long *flags);

/*�ͷŽ��̵��źŴ����������������������ñ���cpu�ж�*/
static inline void unlock_task_sighand(struct task_struct *tsk, unsigned long *flags)
{
	spin_unlock_irqrestore(&tsk->sighand->siglock, *flags);
}

#ifndef __HAVE_THREAD_FUNCTIONS
/*��ȡ���̵ĵͲ�thread_info��Ϣ*/
#define task_thread_info(task)	((struct thread_info *)(task)->stack)
/*��ȡ���̵��ں�ջ*/
#define task_stack_page(task)	((task)->stack)
/*���ƽ��̵��ں�ջ��Ϣ*/
static inline void setup_thread_stack(struct task_struct *p, struct task_struct *org)
{
	/*��Դ����org���ں�ջ��Ϣֱ��ͨ���ṹ�帳ֵ��Ŀ�����p*/
	*task_thread_info(p) = *task_thread_info(org);
	/*��Ŀ�����p���̵߳Ͳ�taskָ��ָ������*/
	task_thread_info(p)->task = p;
}

/*�����ں�ջ����λ�ã�Ҳ��thread_info����һ�ֽڵ�ַ*/
static inline unsigned long *end_of_stack(struct task_struct *p)
{
	return (unsigned long *)(task_thread_info(p) + 1);
}

#endif

/* set thread flags in other task's structures
 * - see asm/thread_info.h for TIF_xxxx flags available
 */

/*���ý��̵Ͳ�TIF_��ʶ*/
static inline void set_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	set_ti_thread_flag(task_thread_info(tsk), flag);
}
/*������̵Ͳ�TIF_��ʶ*/
static inline void clear_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	clear_ti_thread_flag(task_thread_info(tsk), flag);
}
/*���Խ��̵Ͳ��TIF_��ʶ�Ƿ������ã�������֮ǰ��״̬*/
static inline int test_and_set_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_and_set_ti_thread_flag(task_thread_info(tsk), flag);
}
/*���Բ�������̵Ͳ��TIF_��ʶ��������֮ǰ��״̬*/
static inline int test_and_clear_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_and_clear_ti_thread_flag(task_thread_info(tsk), flag);
}
/*���Խ��̵Ͳ�TIF_��ʶ�Ƿ�������*/
static inline int test_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_ti_thread_flag(task_thread_info(tsk), flag);
}
/*���ý���TIF_NEED_RESCHED��Ҫ�ص��ȱ�ʶ*/
static inline void set_tsk_need_resched(struct task_struct *tsk)
{
	set_tsk_thread_flag(tsk,TIF_NEED_RESCHED);
}
/*������̵�TIF_NEED_RESCHED��ʶ*/
static inline void clear_tsk_need_resched(struct task_struct *tsk)
{
	clear_tsk_thread_flag(tsk,TIF_NEED_RESCHED);
}
/*���Խ����Ƿ��й����źŴ�����*/
static inline int signal_pending(struct task_struct *p)
{
	return unlikely(test_tsk_thread_flag(p,TIF_SIGPENDING));
}
/*���Խ����Ƿ���Ҫ�ص���*/
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
/*��ȡ��ǰ��������ʹ�õ�cpu*/
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
/*�����ϵ�ṹ��Ҫ�ڲ�ͬ��mmap���򲼾�֮������ѡ���ṩ�ú���*/
extern void arch_pick_mmap_layout(struct mm_struct *mm);
#else
/**/
static inline void arch_pick_mmap_layout(struct mm_struct *mm)
{
	mm->mmap_base = TASK_UNMAPPED_BASE;
	/*��mmap������Ϊ��ӳ���ҵ��ʵ���λ��*/
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
