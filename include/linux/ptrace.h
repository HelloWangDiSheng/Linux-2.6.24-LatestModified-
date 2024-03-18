#ifndef _LINUX_PTRACE_H
#define _LINUX_PTRACE_H

/*开始对当前进程的跟踪，当前进程的父进程自动承担跟踪者的角色，必须准备好从子进程接收信息*/
#define PTRACE_TRACEME		   	0
/*从进程的代码段读取任意字*/
#define PTRACE_PEEKTEXT		   	1
/*从进程的数据段读取任意字*/
#define PTRACE_PEEKDATA		   	2
/*从用户进程读取普通的CPU寄存器和使用的任何其它调试寄存器（当然，会根据标识符只读取一个寄存器
的内容，而不是整个寄存器集合的内容）*/
#define PTRACE_PEEKUSR		   	3
/*向被监控进程的代码段写入值*/
#define PTRACE_POKETEXT		   	4
/*向被监控进程的数据段写入值*/
#define PTRACE_POKEDATA		   	5
/*向被监控进程的寄存器写入值，该选项支持对高级调试技术的使用，如可以监控此类事件：在一定条件
满足时，在特定位置暂停程序的执行*/
#define PTRACE_POKEUSR		   	6
/*恢复被跟踪进程的执行，但不自动暂停该进程的具体条件，被跟踪的进程将在接受到信号时暂停*/
#define PTRACE_CONT				7
/*发送kill信号，关闭被跟踪进程*/
#define PTRACE_KILL		   		8
/*将处理器在执行被跟踪进程期间，置于单步执行模式，在这种模式下，执行每个汇编语言指令之后，
被跟踪者发送一个SIGCHLD信号然后进入睡眠，跟踪者可以访问被跟踪进程，这仍然是一种非常流行的
应用程序调试技术，特别是在试图跟踪编译器错误或其他比较微秒的问题时*/
#define PTRACE_SINGLESTEP		9
/*发出一个请求，连接到一个进程并开始跟踪*/
#define PTRACE_ATTACH		  	16
/*从该进程断开并结束跟踪*/
#define PTRACE_DETACH		  	17
/*系统调用追踪是基于该标识的，如果用该选项激活ptrace，那么内核将开始执行进程，直至调用一个
系统调用。在被追踪进程停止后，wait通知跟踪者进程，跟踪者接下来可以使用上述ptrace选项，来分
析被跟踪进程的地址空间，以收集有关系统调用的信息，在完成系统调用之后，被跟踪的进程第二次暂
停，使得跟踪者进程可以检查调用是否成功*/
#define PTRACE_SYSCALL		  	24

/* 0x4200-0x4300 are reserved for architecture-independent additions.  */
#define PTRACE_SETOPTIONS		0x4200
#define PTRACE_GETEVENTMSG		0x4201
#define PTRACE_GETSIGINFO		0x4202
#define PTRACE_SETSIGINFO		0x4203

/* options set using PTRACE_SETOPTIONS */
#define PTRACE_O_TRACESYSGOOD			0x00000001
#define PTRACE_O_TRACEFORK				0x00000002
#define PTRACE_O_TRACEVFORK				0x00000004
#define PTRACE_O_TRACECLONE				0x00000008
#define PTRACE_O_TRACEEXEC				0x00000010
#define PTRACE_O_TRACEVFORKDONE			0x00000020
#define PTRACE_O_TRACEEXIT				0x00000040

#define PTRACE_O_MASK					0x0000007f

/* Wait extended result codes for the above trace options.  */
/*在退出信号设置为SIGCHLD的fork或clone返回之前停止*/
#define PTRACE_EVENT_FORK				1
/*在从vfork(2)或带有CLONE_VFORK标志的克隆(2)返回之前停止。在此停止之后继续跟踪时，它将
在继续执行之前等待子进程退出/执行(换句话说，vfork(2)的通常行为)。*/
#define PTRACE_EVENT_VFORK				2
/*在clone(2)返回之前停止*/
#define PTRACE_EVENT_CLONE				3
/*在从execve返回之前停止*/
#define PTRACE_EVENT_EXEC				4
/*在使用CLONE_VFORK标志从vfork(2)或clone(2)返回之前停止，但是在子级通过退出或执行取消阻
塞此跟踪之后。对于上述所有四个停止，该停止都发生在父级(即Tracee)中，而不是在新创建的线程
中。 PTRACE_GETEVENTMSG可用于检索新线程的ID。*/
#define PTRACE_EVENT_VFORK_DONE			5
/*在退出(包括exit_group(2)的死亡)，发出信号死亡或由execve(2)在多线程进程中导致退出之前停止
。PTRACE_GETEVENTMSG返回退出状态。可以检查寄存器(与"实际"退出发生时不同)。示踪还活着；必须
退出PTRACE_CONT或PTRACE_DETACH。*/
#define PTRACE_EVENT_EXIT				6

#include <asm/ptrace.h>

#ifdef __KERNEL__
/*
 * Ptrace flags
 *
 * The owner ship rules for task->ptrace which holds the ptrace
 * flags is simple.  When a task is running it owns it's task->ptrace
 * flags.  When the a task is stopped the ptracer owns task->ptrace.
 */
/**/
#define PT_PTRACED				0x00000001
/**/
#define PT_DTRACE				0x00000002	/* delayed trace (used on m68k, i386) */
/**/
#define PT_TRACESYSGOOD			0x00000004
/**/
#define PT_PTRACE_CAP			0x00000008	/* ptracer can follow suid-exec */
/**/
#define PT_TRACE_FORK			0x00000010
/*在从vfork或带有CLONE_VFORK标志的clone(2)返回之前停止。在此停止之后继续跟踪时，它将在继续执行
之前等待子进程退出或执行*/
#define PT_TRACE_VFOR			0x00000020
/**/
#define PT_TRACE_CLONE			0x00000040
/**/
#define PT_TRACE_EXEC			0x00000080
/**/
#define PT_TRACE_VFORK_DONE	0x00000100
/**/
#define PT_TRACE_EXIT			0x00000200
/**/
#define PT_ATTACHED				0x00000400	/* parent != real_parent */
/**/
#define PT_TRACE_MASK	0x000003f4

/* single stepping state bits (used on ARM and PA-RISC) */
#define PT_SINGLESTEP_BIT	31
#define PT_SINGLESTEP		(1<<PT_SINGLESTEP_BIT)
#define PT_BLOCKSTEP_BIT	30
#define PT_BLOCKSTEP		(1<<PT_BLOCKSTEP_BIT)

#include <linux/compiler.h>		/* For unlikely.  */
#include <linux/sched.h>		/* For struct task_struct.  */


extern long arch_ptrace(struct task_struct *child, long request, long addr, long data);
extern struct task_struct *ptrace_get_task_struct(pid_t pid);
extern int ptrace_traceme(void);
extern int ptrace_readdata(struct task_struct *tsk, unsigned long src, char __user *dst, int len);
extern int ptrace_writedata(struct task_struct *tsk, char __user *src, unsigned long dst, int len);
extern int ptrace_attach(struct task_struct *tsk);
extern int ptrace_detach(struct task_struct *, unsigned int);
extern void ptrace_disable(struct task_struct *);
extern int ptrace_check_attach(struct task_struct *task, int kill);
extern int ptrace_request(struct task_struct *child, long request, long addr, long data);
extern void ptrace_notify(int exit_code);
extern void __ptrace_link(struct task_struct *child,
			  struct task_struct *new_parent);
extern void __ptrace_unlink(struct task_struct *child);
extern void ptrace_untrace(struct task_struct *child);
extern int ptrace_may_attach(struct task_struct *task);
extern int __ptrace_may_attach(struct task_struct *task);

static inline void ptrace_link(struct task_struct *child,
			       struct task_struct *new_parent)
{
	if (unlikely(child->ptrace))
		__ptrace_link(child, new_parent);
}
static inline void ptrace_unlink(struct task_struct *child)
{
	if (unlikely(child->ptrace))
		__ptrace_unlink(child);
}

int generic_ptrace_peekdata(struct task_struct *tsk, long addr, long data);
int generic_ptrace_pokedata(struct task_struct *tsk, long addr, long data);

#ifndef force_successful_syscall_return
/*
 * System call handlers that, upon successful completion, need to return a
 * negative value should call force_successful_syscall_return() right before
 * returning.  On architectures where the syscall convention provides for a
 * separate error flag (e.g., alpha, ia64, ppc{,64}, sparc{,64}, possibly
 * others), this macro can be used to ensure that the error flag will not get
 * set.  On architectures which do not support a separate error flag, the macro
 * is a no-op and the spurious error condition needs to be filtered out by some
 * other means (e.g., in user-level, by passing an extra argument to the
 * syscall handler, or something along those lines).
 */
#define force_successful_syscall_return() do { } while (0)
#endif

#endif

#endif
