/*i386低层线程信息*/

#ifndef _ASM_THREAD_INFO_H
#define _ASM_THREAD_INFO_H

#ifdef __KERNEL__

#include <linux/compiler.h>
#include <asm/page.h>

#ifndef __ASSEMBLY__
#include <asm/processor.h>
#endif


 /*entry.s可立即访问的进程低层数据，该结构大小完全可以被一个缓存行大小容纳，与内核栈共享页面
 ，内容改变时汇编常量一定同步修改*/
#ifndef __ASSEMBLY__

struct thread_info
{
	/*指向当前进程的指针*/
	struct task_struct	*task;
	/*执行域用于实现执行区间（execution domain），后者用于在一类计算机上实现多种ABI
	（Applicioin Binary Interface，应用程序二进制接口），例如在AMD64系统的64位模式下
	运行32位应用程序*/
	struct exec_domain	*exec_domain;
	/*低层标识，TIF_开头*/
	unsigned long	flags;
	/*线程同步标识，TSS_USEDFPU、TSS_PILLING*/
	unsigned long	status;
	/*当前进程在哪个cpu上运行*/
	__u32	cpu;
	/*抢占计数，小于0表示漏洞，0表示可抢占，大于0表示不可抢占*/
	int	preempt_count;
	/*地址空间上限，[0, 0xBFFFFFFF]为用户线程空间，[0, 0xFFFFFFFF]为内核线程空间*/
	mm_segment_t		addr_limit;
	/*系统调用返回值*/
	void	*sysenter_return;
	struct restart_block    restart_block;
	/*前一个活动记录的栈帧地址，栈回溯时有用*/
	unsigned long	previous_esp;
	__u8	supervisor_stack[0];
};

#else /* !__ASSEMBLY__ */

#include <asm/asm-offsets.h>

#endif
/*内核抢占。说明调度是由抢占机制发起的*/
#define PREEMPT_ACTIVE		0x10000000
/*内核栈长度，默认8k，可配置为4k*/
#ifdef CONFIG_4KSTACKS
#define THREAD_SIZE			(4096)
#else
#define THREAD_SIZE			(8192)
#endif
/*栈空间不足警告值*/
#define STACK_WARN          (THREAD_SIZE/8)

#ifndef __ASSEMBLY__
/*获取线程低层信息结构体，抢占计数需要初始化为1，直到调度器函数可用*/
#define INIT_THREAD_INFO(tsk)						\
{													\
	/*当前进程实例指针*/									\
	.task		= &tsk,								\
	/*当前进程的执行域信息*/									\
	.exec_domain	= &default_exec_domain,			\
	/*TIF_*线程信息标识*/									\
	.flags		= 0,								\
	/*所在运行的cpu*/									\
	.cpu		= 0,								\
	/*刚开始禁用抢占*/										\
	.preempt_count	= 1,							\
	/*地址上限时内核地址空间的结束地址*/							\
	.addr_limit	= KERNEL_DS,						\
	/*实现信号机制，重启系统调用*/								\
	.restart_block = 								\
	{												\
		/**/\
		.fn = do_no_restart_syscall,				\
	},												\
}

/*获取初始化进程的thread_info信息*/
#define init_thread_info		(init_thread_union.thread_info)
/*获取初始化进程的内核栈信息*/
#define init_stack		(init_thread_union.stack)


/*获取当前进程的内核栈指针*/
register unsigned long current_stack_pointer asm("esp") __attribute_used__;


/*获取当前进程的thread_info信息，栈顶指针与栈长度对齐即可*/
static inline struct thread_info *current_thread_info(void)
{
	return (struct thread_info *)(current_stack_pointer & ~(THREAD_SIZE - 1));
}

/*使用_get_free_pages为内核栈分配页面*/
#ifdef CONFIG_DEBUG_STACK_USAGE
#define alloc_thread_info(tsk) ((struct thread_info *) \
	__get_free_pages(GFP_KERNEL| __GFP_ZERO, get_order(THREAD_SIZE)))
#else
#define alloc_thread_info(tsk) ((struct thread_info *) \
	__get_free_pages(GFP_KERNEL, get_order(THREAD_SIZE)))
#endif
/*释放内核栈空间*/
#define free_thread_info(info)	free_pages((unsigned long)(info), get_order(THREAD_SIZE))

#else /* !__ASSEMBLY__ */

/* how to get the thread information struct from ASM */
#define GET_THREAD_INFO(reg) \
	movl $-THREAD_SIZE, reg; \
	andl %esp, reg

/* use this one if reg already contains %esp */
#define GET_THREAD_INFO_WITH_ESP(reg) \
	andl $-THREAD_SIZE, reg

#endif

 /*线程信息标识，这些线程标识因为不同汇编文件可能需要访问挂起需要完成的标识为小端序
 ，其他的为大端序*/
 /*激活系统调试*/
#define TIF_SYSCALL_TRACE			0
/*有需要处理的待决信号*/
#define TIF_SIGPENDING				1
/*需要重新调度*/
#define TIF_NEED_RESCHED			2
/*用户模式下保存单步调试返回值*/
#define TIF_SINGLESTEP				3
/*中断返回*/
#define TIF_IRET					4
/*激活系统模拟*/
#define TIF_SYSCALL_EMU				5
/*激活审计*/
#define TIF_SYSCALL_AUDIT			6
/*安全计算相关*/
#define TIF_SECCOMP					7
/*保存do_signal执行中信号掩码*/
#define TIF_RESTORE_SIGMASK			8
#define TIF_MEMDIE					16
/*使用调试寄存器*/
#define TIF_DEBUG					17
/*使用I/O位图*/
#define TIF_IO_BITMAP				18
/*暂停冻结*/
#define TIF_FREEZE					19
/*用户空间不可访问TSC(时间戳计数器)*/
#define TIF_NOTSC					20

#define _TIF_SYSCALL_TRACE			(1<<TIF_SYSCALL_TRACE)
#define _TIF_SIGPENDING				(1<<TIF_SIGPENDING)
#define _TIF_NEED_RESCHED			(1<<TIF_NEED_RESCHED)
#define _TIF_SINGLESTEP				(1<<TIF_SINGLESTEP)
#define _TIF_IRET					(1<<TIF_IRET)
#define _TIF_SYSCALL_EMU			(1<<TIF_SYSCALL_EMU)
#define _TIF_SYSCALL_AUDIT			(1<<TIF_SYSCALL_AUDIT)
#define _TIF_SECCOMP				(1<<TIF_SECCOMP)
#define _TIF_RESTORE_SIGMASK		(1<<TIF_RESTORE_SIGMASK)
#define _TIF_DEBUG					(1<<TIF_DEBUG)
#define _TIF_IO_BITMAP				(1<<TIF_IO_BITMAP)
#define _TIF_FREEZE					(1<<TIF_FREEZE)
#define _TIF_NOTSC					(1<<TIF_NOTSC)

/* work to do on interrupt/exception return */
#define _TIF_WORK_MASK				(0x0000FFFF &\
		~(_TIF_SYSCALL_TRACE | _TIF_SYSCALL_AUDIT | _TIF_SECCOMP | _TIF_SYSCALL_EMU))
/* work to do on any return to u-space */
#define _TIF_ALLWORK_MASK			(0x0000FFFF & ~_TIF_SECCOMP)

/* flags to check in __switch_to() */
#define _TIF_WORK_CTXSW_NEXT 		(_TIF_IO_BITMAP | _TIF_NOTSC | _TIF_DEBUG)
#define _TIF_WORK_CTXSW_PREV 		(_TIF_IO_BITMAP | _TIF_NOTSC)


 /*线程同步标识，非原子操作*/
/*使用FPU*/
#define TS_USEDFPU		0x0001
/*处于非睡眠空闲循环*/
#define TS_POLLING		0x0002
/*线程是否处于非睡眠空循环状态*/
#define tsk_is_polling(t) (task_thread_info(t)->status & TS_POLLING)

#endif /* __KERNEL__ */

#endif /* _ASM_THREAD_INFO_H */
