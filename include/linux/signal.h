#ifndef _LINUX_SIGNAL_H
#define _LINUX_SIGNAL_H

#include <asm/signal.h>
#include <asm/siginfo.h>

#ifdef __KERNEL__
#include <linux/list.h>

/*实时信号可以排队*/
struct sigqueue
{
	/*各链表项通过list连接起来*/
	struct list_head list;
	/**/
	int flags;
	/*包含待决信号的更多详细信息*/
	siginfo_t info;
	/**/
	struct user_struct *user;
};

/* flags values. */
#define SIGQUEUE_PREALLOC	1
/*包含所有已经引发、仍然有待内核处理的信号*/
struct sigpending
{
	/*通过双链表管理所有待决信号， 链表元素时sigqueue*/
	struct list_head list;
	/*位掩码指定了仍然有待处理的所有信号*/
	sigset_t signal;
};

/*定义一些原始的信号集操作*/
#ifndef __HAVE_ARCH_SIG_BITOPS
#include <linux/bitops.h>

/*此处不使用linux的比特位原子操作*/
/*将信号添加到信号集合，直接根据信号编号确定所在信号位图中的索引，然后计算权重按位或*/
static inline void sigaddset(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	if (_NSIG_WORDS == 1)
		set->sig[0] |= 1UL << sig;
	else
		set->sig[sig / _NSIG_BPW] |= 1UL << (sig % _NSIG_BPW);
}

/*将信号从信号集合中删除*/
static inline void sigdelset(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	if (_NSIG_WORDS == 1)
		set->sig[0] &= ~(1UL << sig);
	else
		set->sig[sig / _NSIG_BPW] &= ~(1UL << (sig % _NSIG_BPW));
}
/*信号是否已经在信号集合中*/
static inline int sigismember(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	if (_NSIG_WORDS == 1)
		return 1 & (set->sig[0] >> sig);
	else
		return 1 & (set->sig[sig / _NSIG_BPW] >> (sig % _NSIG_BPW));
}

/*查询第一个没有被包含的信号*/
static inline int sigfindinword(unsigned long word)
{
	return ffz(~word);
}

#endif /* __HAVE_ARCH_SIG_BITOPS */
/*测试信号集是否为空，按无符号长整形类型变量测试值是否全为0，系统只支持1/2/4数目字长，
其它不支持*/
static inline int sigisemptyset(sigset_t *set)
{
	extern void _NSIG_WORDS_is_unsupported_size(void);
	switch (_NSIG_WORDS)
	{
		case 4:
			return (set->sig[3] | set->sig[2] |
				set->sig[1] | set->sig[0]) == 0;
		case 2:
			return (set->sig[1] | set->sig[0]) == 0;
		case 1:
			return set->sig[0] == 0;
		default:
			_NSIG_WORDS_is_unsupported_size();
			return 0;
	}
}

/*信号权重，也就是信号在信号掩码中的索引位*/
#define sigmask(sig)	(1UL << ((sig) - 1))

#ifndef __HAVE_ARCH_SIG_SETOPS
#include <linux/string.h>

#define _SIG_SET_BINOP(name, op)					\
static inline void name(sigset_t *r, const sigset_t *a, const sigset_t *b) \
{									\
	extern void _NSIG_WORDS_is_unsupported_size(void);		\
	unsigned long a0, a1, a2, a3, b0, b1, b2, b3;			\
									\
	switch (_NSIG_WORDS) {						\
	    case 4:							\
		a3 = a->sig[3]; a2 = a->sig[2];				\
		b3 = b->sig[3]; b2 = b->sig[2];				\
		r->sig[3] = op(a3, b3);					\
		r->sig[2] = op(a2, b2);					\
	    case 2:							\
		a1 = a->sig[1]; b1 = b->sig[1];				\
		r->sig[1] = op(a1, b1);					\
	    case 1:							\
		a0 = a->sig[0]; b0 = b->sig[0];				\
		r->sig[0] = op(a0, b0);					\
		break;							\
	    default:							\
		_NSIG_WORDS_is_unsupported_size();			\
	}								\
}

#define _sig_or(x,y)	((x) | (y))
_SIG_SET_BINOP(sigorsets, _sig_or)

#define _sig_and(x,y)	((x) & (y))
_SIG_SET_BINOP(sigandsets, _sig_and)

#define _sig_nand(x,y)	((x) & ~(y))
_SIG_SET_BINOP(signandsets, _sig_nand)

#undef _SIG_SET_BINOP
#undef _sig_or
#undef _sig_and
#undef _sig_nand

#define _SIG_SET_OP(name, op)						\
static inline void name(sigset_t *set)					\
{									\
	extern void _NSIG_WORDS_is_unsupported_size(void);		\
									\
	switch (_NSIG_WORDS) {						\
	    case 4: set->sig[3] = op(set->sig[3]);			\
		    set->sig[2] = op(set->sig[2]);			\
	    case 2: set->sig[1] = op(set->sig[1]);			\
	    case 1: set->sig[0] = op(set->sig[0]);			\
		    break;						\
	    default:							\
		_NSIG_WORDS_is_unsupported_size();			\
	}								\
}

#define _sig_not(x)	(~(x))
_SIG_SET_OP(signotset, _sig_not)

#undef _SIG_SET_OP
#undef _sig_not

static inline void sigemptyset(sigset_t *set)
{
	switch (_NSIG_WORDS) {
	default:
		memset(set, 0, sizeof(sigset_t));
		break;
	case 2: set->sig[1] = 0;
	case 1:	set->sig[0] = 0;
		break;
	}
}

static inline void sigfillset(sigset_t *set)
{
	switch (_NSIG_WORDS) {
	default:
		memset(set, -1, sizeof(sigset_t));
		break;
	case 2: set->sig[1] = -1;
	case 1:	set->sig[0] = -1;
		break;
	}
}

/*低32位信号的一些扩展操作*/
/*低32位mask信号集添加到set信号集中*/
static inline void sigaddsetmask(sigset_t *set, unsigned long mask)
{
	set->sig[0] |= mask;
}
/*将低32位mask信号集从set信号集中删除*/
static inline void sigdelsetmask(sigset_t *set, unsigned long mask)
{
	set->sig[0] &= ~mask;
}
/*测试低32位mask信号集是否被包含在set信号集中*/
static inline int sigtestsetmask(sigset_t *set, unsigned long mask)
{
	return (set->sig[0] & mask) != 0;
}
/*将信号集set低32位信号集赋值为mask，并清空其它信号*/
static inline void siginitset(sigset_t *set, unsigned long mask)
{
	set->sig[0] = mask;
	switch (_NSIG_WORDS)
	{
		default:
			memset(&set->sig[1], 0, sizeof(long)*(_NSIG_WORDS-1));
			break;
		case 2: set->sig[1] = 0;
		case 1: ;
	}
}

/*信号集set低32位信号设置为mask的补集，其它信号全部添加到给信号集中*/
static inline void siginitsetinv(sigset_t *set, unsigned long mask)
{
	set->sig[0] = ~mask;
	switch (_NSIG_WORDS) {
	default:
		memset(&set->sig[1], -1, sizeof(long)*(_NSIG_WORDS-1));
		break;
	case 2: set->sig[1] = -1;
	case 1: ;
	}
}

#endif /* __HAVE_ARCH_SIG_SETOPS */

/*初始化待决信号队列*/
static inline void init_sigpending(struct sigpending *sig)
{
	sigemptyset(&sig->signal);
	INIT_LIST_HEAD(&sig->list);
}

extern void flush_sigqueue(struct sigpending *queue);

/* Test if 'sig' is valid signal. Use this instead of testing _NSIG directly */
/*测试信号是否是无效信号（大于__NSIG为无效信号）*/
static inline int valid_signal(unsigned long sig)
{
	return sig <= _NSIG ? 1 : 0;
}

extern int next_signal(struct sigpending *pending, sigset_t *mask);
extern int group_send_sig_info(int sig, struct siginfo *info, struct task_struct *p);
extern int __group_send_sig_info(int, struct siginfo *, struct task_struct *);
extern long do_sigpending(void __user *, unsigned long);
extern int sigprocmask(int, sigset_t *, sigset_t *);
extern int show_unhandled_signals;

struct pt_regs;
extern int get_signal_to_deliver(siginfo_t *info, struct k_sigaction *return_ka, struct pt_regs *regs, void *cookie);

extern struct kmem_cache *sighand_cachep;

int unhandled_signal(struct task_struct *tsk, int sig);

/*
 * In POSIX a signal is sent either to a specific thread (Linux task)
 * or to the process as a whole (Linux thread group).  How the signal
 * is sent determines whether it's to one thread or the whole group,
 * which determines which signal mask(s) are involved in blocking it
 * from being delivered until later.  When the signal is delivered,
 * either it's caught or ignored by a user handler or it has a default
 * effect that applies to the whole thread group (POSIX process).
 *
 * The possible effects an unblocked signal set to SIG_DFL can have are:
 *   ignore	- Nothing Happens
 *   terminate	- kill the process, i.e. all threads in the group,
 * 		  similar to exit_group.  The group leader (only) reports
 *		  WIFSIGNALED status to its parent.
 *   coredump	- write a core dump file describing all threads using
 *		  the same mm and then kill all those threads
 *   stop 	- stop all the threads in the group, i.e. TASK_STOPPED state
 *
 * SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.
 * Other signals when not blocked and set to SIG_DFL behaves as follows.
 * The job control signals also have other special effects.
 *
 *	+--------------------+------------------+
 *	|  POSIX signal      |  default action  |
 *	+--------------------+------------------+
 *	|  SIGHUP            |  terminate	|
 *	|  SIGINT            |	terminate	|
 *	|  SIGQUIT           |	coredump 	|
 *	|  SIGILL            |	coredump 	|
 *	|  SIGTRAP           |	coredump 	|
 *	|  SIGABRT/SIGIOT    |	coredump 	|
 *	|  SIGBUS            |	coredump 	|
 *	|  SIGFPE            |	coredump 	|
 *	|  SIGKILL           |	terminate(+)	|
 *	|  SIGUSR1           |	terminate	|
 *	|  SIGSEGV           |	coredump 	|
 *	|  SIGUSR2           |	terminate	|
 *	|  SIGPIPE           |	terminate	|
 *	|  SIGALRM           |	terminate	|
 *	|  SIGTERM           |	terminate	|
 *	|  SIGCHLD           |	ignore   	|
 *	|  SIGCONT           |	ignore(*)	|
 *	|  SIGSTOP           |	stop(*)(+)  	|
 *	|  SIGTSTP           |	stop(*)  	|
 *	|  SIGTTIN           |	stop(*)  	|
 *	|  SIGTTOU           |	stop(*)  	|
 *	|  SIGURG            |	ignore   	|
 *	|  SIGXCPU           |	coredump 	|
 *	|  SIGXFSZ           |	coredump 	|
 *	|  SIGVTALRM         |	terminate	|
 *	|  SIGPROF           |	terminate	|
 *	|  SIGPOLL/SIGIO     |	terminate	|
 *	|  SIGSYS/SIGUNUSED  |	coredump 	|
 *	|  SIGSTKFLT         |	terminate	|
 *	|  SIGWINCH          |	ignore   	|
 *	|  SIGPWR            |	terminate	|
 *	|  SIGRTMIN-SIGRTMAX |	terminate       |
 *	+--------------------+------------------+
 *	|  non-POSIX signal  |  default action  |
 *	+--------------------+------------------+
 *	|  SIGEMT            |  coredump	|
 *	+--------------------+------------------+
 *
 * (+) For SIGKILL and SIGSTOP the action is "always", not just "default".
 * (*) Special job control effects:
 * When SIGCONT is sent, it resumes the process (all threads in the group)
 * from TASK_STOPPED state and also clears any pending/queued stop signals
 * (any of those marked with "stop(*)").  This happens regardless of blocking,
 * catching, or ignoring SIGCONT.  When any stop signal is sent, it clears
 * any pending/queued SIGCONT signals; this happens regardless of blocking,
 * catching, or ignored the stop signal, though (except for SIGSTOP) the
 * default action of stopping the process may happen later or never.
 */

#ifdef SIGEMT
#define SIGEMT_MASK	rt_sigmask(SIGEMT)
#else
#define SIGEMT_MASK	0
#endif
/*信号权重*/
#if SIGRTMIN > BITS_PER_LONG
#define rt_sigmask(sig)	(1ULL << ((sig)-1))
#else
#define rt_sigmask(sig)	sigmask(sig)
#endif
/*测试信号是否在mask信号集合中*/
#define siginmask(sig, mask) (rt_sigmask(sig) & (mask))

/*默认处理操作为结束的标准信号集*/
#define SIG_KERNEL_ONLY_MASK (								\
	rt_sigmask(SIGKILL)   |  rt_sigmask(SIGSTOP))
/*默认处理方式为停止的标准信号集*/
#define SIG_KERNEL_STOP_MASK (								\
	rt_sigmask(SIGSTOP)   |  rt_sigmask(SIGTSTP)   | 		\
	rt_sigmask(SIGTTIN)   |  rt_sigmask(SIGTTOU)   )
/*默认处理方式为内存转储的标准信号集*/
#define SIG_KERNEL_COREDUMP_MASK (							\
        rt_sigmask(SIGQUIT)   |  rt_sigmask(SIGILL)    |	\
	rt_sigmask(SIGTRAP)   |  rt_sigmask(SIGABRT)   |	 	\
        rt_sigmask(SIGFPE)    |  rt_sigmask(SIGSEGV)   |	\
	rt_sigmask(SIGBUS)    |  rt_sigmask(SIGSYS)    | 		\
        rt_sigmask(SIGXCPU)   |  rt_sigmask(SIGXFSZ)   | 	\
	SIGEMT_MASK				       )
/*默认处理方式为忽略的标准信号集*/
#define SIG_KERNEL_IGNORE_MASK (							\
        rt_sigmask(SIGCONT)   |  rt_sigmask(SIGCHLD)   | 	\
	rt_sigmask(SIGWINCH)  |  rt_sigmask(SIGURG)    )

/*信号是否为结束进程的非实时信号*/
#define sig_kernel_only(sig) \
	(((sig) < SIGRTMIN) && siginmask(sig, SIG_KERNEL_ONLY_MASK))
/*信号是否为内存转储的非实时信号*/
#define sig_kernel_coredump(sig) \
	(((sig) < SIGRTMIN) && siginmask(sig, SIG_KERNEL_COREDUMP_MASK))
/*信号是否为终止进程的非实时信号*/
#define sig_kernel_ignore(sig) \
	(((sig) < SIGRTMIN) && siginmask(sig, SIG_KERNEL_IGNORE_MASK))
/*信号是否为停止进程的非实时信号*/
#define sig_kernel_stop(sig) \
	(((sig) < SIGRTMIN) && siginmask(sig, SIG_KERNEL_STOP_MASK))

#define sig_needs_tasklist(sig)	((sig) == SIGCONT)

/*进程收到是否是非忽略和非默认处理的信号*/
#define sig_user_defined(t, signr) \
	(((t)->sighand->action[(signr)-1].sa.sa_handler != SIG_DFL) &&	\
	 ((t)->sighand->action[(signr)-1].sa.sa_handler != SIG_IGN))

/*信号不属于忽略或停止进程信号，并且对该信号提供系统默认处理*/
#define sig_fatal(t, signr) \
	(!siginmask(signr, SIG_KERNEL_IGNORE_MASK|SIG_KERNEL_STOP_MASK) && \
	 (t)->sighand->action[(signr)-1].sa.sa_handler == SIG_DFL)

#endif /* __KERNEL__ */

#endif /* _LINUX_SIGNAL_H */
