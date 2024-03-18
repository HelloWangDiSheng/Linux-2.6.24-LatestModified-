#ifndef _ASM_X86_SIGNAL_H
#define _ASM_X86_SIGNAL_H

#ifndef __ASSEMBLY__
#include <linux/types.h>
#include <linux/time.h>
#include <linux/compiler.h>

/*避免太多头文件排序问题*/
struct siginfo;

#ifdef __KERNEL__
#include <linux/linkage.h>

/*目前支持的信号数量*/
#define _NSIG		64

/*每个字包含的比特位数目*/
#ifdef __i386__
#define _NSIG_BPW	32
#else
#define _NSIG_BPW	64
#endif
/*信号数目所需字长的数目*/
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)

/*至少32位*/
typedef unsigned long old_sigset_t;

/*信号位图（位掩码），每一个比特位对应一个信号*/
typedef struct
{
	/*所包含的比特位数目必须至少与所支持的信号数目相同*/
	unsigned long sig[_NSIG_WORDS];
} sigset_t;

#else
/* Here we must cater to libcs that poke about in kernel headers.  */

#define NSIG		32
typedef unsigned long sigset_t;

#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */
/* 挂断，本信号在用户端连接正常或非正常结束时发出，通常是在终端控制进程结束时，通知同一会话
内的各个作业，这时他们与控制终端不再关联  。
	登录Linux时，系统会分配给登录用户一个终端(Session)。在这个终端运行的所有程序，包括前台进
程组和 后台进程组，一般都属于这个 Session。当用户退出Linux登录时，前台进程组和后台有对终端输
出的进程将会收到SIGHUP信号。这个信号的默认操作为终止进程，因此前台进 程组和后台有终端输出的
进程就会中止。不过可以捕获这个信号，比如wget能捕获SIGHUP信号，并忽略它，这样就算退出了Linux
登录，wget也 能继续下载。此外，对于与终端脱离关系的守护进程，这个信号用于通知它重新读取配置文件。*/
#define SIGHUP		 1/* Hangup   */
/* 程序终止(interrupt)信号, 在用户键入INTR字符(通常是Ctrl+C)时发出，用于通知前台进程组终止进程。*/
#define SIGINT		 2/* Interrupt */
/* 和SIGINT类似, 但由QUIT字符(通常是Ctrl+)来控制. 进程在因收到SIGQUIT退出时会产生core文件, 
在这个意义上类似于一个程序错误信号。  */
#define SIGQUIT		 3
/* illeage，非法的。执行了非法指令， 通常是因为可执行文件本身出现错误, 或者试图执行数据段. 
堆栈溢出也有可能产生这个信号。  */
#define SIGILL		 4
/* 由断点指令或其它陷阱（trap）指令产生. 由debugger使用。  */
#define SIGTRAP		 5
/* 调用abort函数生成的信号。  */
#define SIGABRT		 6
/*   */
#define SIGIOT		 6
/* 非法地址, 包括内存地址对齐(alignment)出错。比如访问一个四个字长的整数, 但其地址
不是4的倍数。它与SIGSEGV的区别在于后者是由于对合法存储地址的非法访问触发的(如访问不
属于自己存储空间或只读存储空间)。  */
#define SIGBUS		 7
/* FPE是floating-point exception（浮点异常）的首字母缩略字。在发生致命的算术运算错误
时发出. 不仅包括浮点运算错误, 还包括溢出及除数为0等其它所有的算术的错误。SIGFPE的符号
常量在头文件signal.h中定义。  */
#define SIGFPE		 8
/* 用来立即结束程序的运行. 本信号不能被阻塞、处理和忽略。*/
#define SIGKILL		 9
/* 留给用户使用  */
#define SIGUSR1		10
/* 试图访问未分配给自己的内存, 或试图往没有写权限的内存地址写数据.  */
#define SIGSEGV		11
/* 留给用户使用  */
#define SIGUSR2		12
/*  管道破裂。这个信号通常在进程间通信产生，比如采用FIFO(管道)通信的两个进程，读管道没打开
或者意外终止就往管道写，写进程会收到SIGPIPE信号。此外用Socket通信的两个进程，写进程在写
Socket的时候，读进程已经终止。  */
#define SIGPIPE		13
/* 时钟定时信号, 计算的是实际的时间或时钟时间. alarm函数使用该信号.  */
#define SIGALRM		14
/*  程序结束(terminate)信号, 与SIGKILL不同的是该信号可以被阻塞和处理。通常用来要求程序自己
正常退出，shell命令kill缺省产生这个信号。如果进程终止不了，我们才会尝试SIGKILL。  */
#define SIGTERM		15
/* stack fault，协处理器栈故障，一般malloc返回NULL且设置errno为ENOMEM，但有些系统可能会使用该信号*/
#define SIGSTKFLT	16
/* 子进程（child）结束时, 父进程会收到这个信号。如果父进程没有处理这个信号，也没有
等待(wait)子进程，子进程虽然终止，但是还会在内核进程表中占有表项，这 时的子进程称为
僵尸进程。这种情 况我们应该避免(父进程或者忽略SIGCHILD信号，或者捕捉它，或者wait它派
生的子进程，或者父进程先终止，这时子进程的终止自动由init进程 来接管) */
#define SIGCHLD		17
/* 让一个停止(stopped)的进程继续执行. 本信号不能被阻塞. 可以用一个handler来让程序在
由stopped状态变为继续执行时完成特定的工作. 例如, 重新显示提示符 */
#define SIGCONT		18
/* 暂停(stopped)进程的执行. 注意它和terminate以及interrupt的区别:该进程还未结束, 只是
暂停执行. 本信号不能被阻塞, 处理或忽略 */
#define SIGSTOP		19
/* 停止进程的运行, 但该信号可以被处理和忽略. 用户键入SUSP字符时(通常是Ctrl+Z)发出这个信号 */
#define SIGTSTP		20
/* 当后台作业要从用户终端读数据时, 该作业中的所有进程会收到SIGTTIN信号. 缺省时这些进程会
停止执行。       Unix环境下，当一个进程以后台形式启动，但尝试去读写控制台终端时，将会触发SIGTTIN
（读）和SIGTTOU（写）信号量，接着，进程将会暂停（linux默认情况下），read/write将会返回错误。
这个时候，shell将会发送通知给用户，提醒用户切换此进程为前台进程，以便继续执行。由后台切换至
前台的方式是fg命令，前台转为后台则为CTRL+Z快捷键。
    那么问题来了，如何才能在不把进程切换至前台的情况下，读写控制器不会被暂停？答案：只要忽略
SIGTTIN和SIGTTOU信号量即可：signal(SIGTTOU, SIG_IGN)。
     stty stop/-stop命令是用于设置收到SIGTTOU信号量后是否执行暂停，因为有些系统的默认行为不一致，
比如mac是默认忽略，而linux是默认启用。stty -a可以查看当前tty的配置参数 */
#define SIGTTIN		21
/*  类似于SIGTTIN, 但在写终端(或修改终端模式)时收到。具体见上面SIGTTIN  */
#define SIGTTOU		22
/* urgent, 紧急的。有”紧急”数据或out-of-band数据到达socket时产生 */
#define SIGURG		23
/* 超过CPU时间资源限制. 这个限制可以由getrlimit/setrlimit来读取/改变  */
#define SIGXCPU		24
/* 当进程企图扩大文件以至于超过文件大小资源限制  */
#define SIGXFSZ		25
/* 虚拟时钟信号. 类似于SIGALRM, 但是计算的是该进程占用的CPU时间  */
#define SIGVTALRM	26
/* 类似于SIGALRM/SIGVTALRM, 但包括该进程用的CPU时间以及系统调用的时间 */
#define SIGPROF		27
/* Windows Change, 窗口大小改变时发出 */
#define SIGWINCH	28
/* 文件描述符准备就绪, 可以开始进行输入/输出操作  */
#define SIGIO		29
#define SIGPOLL		SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR		30	/* Power failure */
#define SIGSYS		31	/* 非法的系统调用*/
#define	SIGUNUSED	31

/* These should not be considered constants from userland.  */
#define SIGRTMIN	32		/* 实时信号下限  ，用户空间不用考虑*/
#define SIGRTMAX	_NSIG	/*   实时信号上限*/

/* 在以上列出的信号中
	程序不可捕获、阻塞或忽略的信号有：SIGKILL,SIGSTOP
    不能恢复至默认动作的信号有： SIGILL,SIGTRAP
    默认会导致进程流产的信号有：SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGIOT,SIGQUIT,SIGSEGV,SIGTRAP,
SIGXCPU,SIGXFSZ
    默认会导致进程退出的信号有：SIGALRM,SIGHUP,SIGINT,SIGKILL,SIGPIPE,SIGPOLL,SIGPROF,SIGSYS,
SIGTERM,SIGUSR1,SIGUSR2,SIGVTALRM
    默认会导致进程停止的信号有：SIGSTOP,SIGTSTP,SIGTTIN,SIGTTOU
    默认进程忽略的信号有：SIGCHLD,SIGPWR,SIGURG,SIGWINCH
    此外，SIGIO在SVR4是退出，在4.3BSD中是忽略；SIGCONT在进程挂起时是继续，否则是忽略，不能被阻塞
*/

/*
 * SA_FLAGS values:
 *
 * SA_ONSTACK indicates that a registered stack_t will be used.
 * SA_RESTART flag to get restarting signals (which were the default long ago)
 * SA_NOCLDSTOP flag to turn off SIGCHLD when children stop.
 * SA_RESETHAND clears the handler when the signal is delivered.
 * SA_NOCLDWAIT flag on SIGCHLD to inhibit zombies.
 * SA_NODEFER prevents the current signal from being masked in the handler.
 *
 * SA_ONESHOT and SA_NOMASK are the historical Linux names for the Single
 * Unix names RESETHAND and NODEFER respectively.
 */
#define SA_NOCLDSTOP	0x00000001u
#define SA_NOCLDWAIT	0x00000002u
#define SA_SIGINFO	0x00000004u
#define SA_ONSTACK	0x08000000u
#define SA_RESTART	0x10000000u
#define SA_NODEFER	0x40000000u
#define SA_RESETHAND	0x80000000u

#define SA_NOMASK	SA_NODEFER
#define SA_ONESHOT	SA_RESETHAND

#define SA_RESTORER	0x04000000

/*
 * sigaltstack controls
 */
#define SS_ONSTACK	1
#define SS_DISABLE	2

#define MINSIGSTKSZ	2048
#define SIGSTKSZ	8192

#include <asm-generic/signal.h>

#ifndef __ASSEMBLY__

#ifdef __i386__
#ifdef __KERNEL__
struct old_sigaction {
	__sighandler_t sa_handler;
	old_sigset_t sa_mask;
	unsigned long sa_flags;
	__sigrestore_t sa_restorer;
};

struct sigaction
{
	/*函数指针类型，内核在信号到达时调用的处理函数*/
	__sighandler_t sa_handler;
	/*额外标识，用于指定信号处理当时的一些约束*/
	unsigned long sa_flags;
	/*包含一个位掩码，每个比特位对应于系统中的一个信号，它用于在处理程序运行期间阻塞
	其它信号，在处理程序结束时，内核会重置其值，恢复到信号处理之前的原值*/
	__sigrestore_t sa_restorer;
	/* mask last for extensibility */
	sigset_t sa_mask;
};

struct k_sigaction
{
	/*信号处理程序相关信息*/
	struct sigaction sa;
};

#else /* __KERNEL__ */
/* Here we must cater to libcs that poke about in kernel headers.  */

struct sigaction {
	union {
	  __sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, struct siginfo *, void *);
	} _u;
	sigset_t sa_mask;
	unsigned long sa_flags;
	void (*sa_restorer)(void);
};

#define sa_handler	_u._sa_handler
#define sa_sigaction	_u._sa_sigaction

#endif /* ! __KERNEL__ */
#else /* __i386__ */

struct sigaction {
	__sighandler_t sa_handler;
	unsigned long sa_flags;
	__sigrestore_t sa_restorer;
	sigset_t sa_mask;		/* mask last for extensibility */
};

struct k_sigaction {
	struct sigaction sa;
};

#endif /* !__i386__ */

typedef struct sigaltstack {
	void __user *ss_sp;
	int ss_flags;
	size_t ss_size;
} stack_t;

#ifdef __KERNEL__
#include <asm/sigcontext.h>

#ifdef __386__

#define __HAVE_ARCH_SIG_BITOPS

#define sigaddset(set,sig)		   \
	(__builtin_constantp(sig) ?	   \
	 __const_sigaddset((set),(sig)) :  \
	 __gen_sigaddset((set),(sig)))

static __inline__ void __gen_sigaddset(sigset_t *set, int _sig)
{
	__asm__("btsl %1,%0" : "+m"(*set) : "Ir"(_sig - 1) : "cc");
}

static __inline__ void __const_sigaddset(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	set->sig[sig / _NSIG_BPW] |= 1 << (sig % _NSIG_BPW);
}

#define sigdelset(set,sig)		   \
	(__builtin_constant_p(sig) ?       \
	 __const_sigdelset((set),(sig)) :  \
	 __gen_sigdelset((set),(sig)))


static __inline__ void __gen_sigdelset(sigset_t *set, int _sig)
{
	__asm__("btrl %1,%0" : "+m"(*set) : "Ir"(_sig - 1) : "cc");
}

static __inline__ void __const_sigdelset(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	set->sig[sig / _NSIG_BPW] &= ~(1 << (sig % _NSIG_BPW));
}

static __inline__ int __const_sigismember(sigset_t *set, int _sig)
{
	unsigned long sig = _sig - 1;
	return 1 & (set->sig[sig / _NSIG_BPW] >> (sig % _NSIG_BPW));
}

static __inline__ int __gen_sigismember(sigset_t *set, int _sig)
{
	int ret;
	__asm__("btl %2,%1\n\tsbbl %0,%0"
		: "=r"(ret) : "m"(*set), "Ir"(_sig-1) : "cc");
	return ret;
}

#define sigismember(set,sig)			\
	(__builtin_constant_p(sig) ?		\
	 __const_sigismember((set),(sig)) :	\
	 __gen_sigismember((set),(sig)))

static __inline__ int sigfindinword(unsigned long word)
{
	__asm__("bsfl %1,%0" : "=r"(word) : "rm"(word) : "cc");
	return word;
}

struct pt_regs;

#define ptrace_signal_deliver(regs, cookie)		\
	do {						\
		if (current->ptrace & PT_DTRACE) {	\
			current->ptrace &= ~PT_DTRACE;	\
			(regs)->eflags &= ~TF_MASK;	\
		}					\
	} while (0)

#else /* __i386__ */

#undef __HAVE_ARCH_SIG_BITOPS

#define ptrace_signal_deliver(regs, cookie) do { } while (0)

#endif /* !__i386__ */
#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */

#endif
