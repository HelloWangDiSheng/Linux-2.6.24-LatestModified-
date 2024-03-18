/*i386�Ͳ��߳���Ϣ*/

#ifndef _ASM_THREAD_INFO_H
#define _ASM_THREAD_INFO_H

#ifdef __KERNEL__

#include <linux/compiler.h>
#include <asm/page.h>

#ifndef __ASSEMBLY__
#include <asm/processor.h>
#endif


 /*entry.s���������ʵĽ��̵Ͳ����ݣ��ýṹ��С��ȫ���Ա�һ�������д�С���ɣ����ں�ջ����ҳ��
 �����ݸı�ʱ��ೣ��һ��ͬ���޸�*/
#ifndef __ASSEMBLY__

struct thread_info
{
	/*ָ��ǰ���̵�ָ��*/
	struct task_struct	*task;
	/*ִ��������ʵ��ִ�����䣨execution domain��������������һ��������ʵ�ֶ���ABI
	��Applicioin Binary Interface��Ӧ�ó�������ƽӿڣ���������AMD64ϵͳ��64λģʽ��
	����32λӦ�ó���*/
	struct exec_domain	*exec_domain;
	/*�Ͳ��ʶ��TIF_��ͷ*/
	unsigned long	flags;
	/*�߳�ͬ����ʶ��TSS_USEDFPU��TSS_PILLING*/
	unsigned long	status;
	/*��ǰ�������ĸ�cpu������*/
	__u32	cpu;
	/*��ռ������С��0��ʾ©����0��ʾ����ռ������0��ʾ������ռ*/
	int	preempt_count;
	/*��ַ�ռ����ޣ�[0, 0xBFFFFFFF]Ϊ�û��߳̿ռ䣬[0, 0xFFFFFFFF]Ϊ�ں��߳̿ռ�*/
	mm_segment_t		addr_limit;
	/*ϵͳ���÷���ֵ*/
	void	*sysenter_return;
	struct restart_block    restart_block;
	/*ǰһ�����¼��ջ֡��ַ��ջ����ʱ����*/
	unsigned long	previous_esp;
	__u8	supervisor_stack[0];
};

#else /* !__ASSEMBLY__ */

#include <asm/asm-offsets.h>

#endif
/*�ں���ռ��˵������������ռ���Ʒ����*/
#define PREEMPT_ACTIVE		0x10000000
/*�ں�ջ���ȣ�Ĭ��8k��������Ϊ4k*/
#ifdef CONFIG_4KSTACKS
#define THREAD_SIZE			(4096)
#else
#define THREAD_SIZE			(8192)
#endif
/*ջ�ռ䲻�㾯��ֵ*/
#define STACK_WARN          (THREAD_SIZE/8)

#ifndef __ASSEMBLY__
/*��ȡ�̵߳Ͳ���Ϣ�ṹ�壬��ռ������Ҫ��ʼ��Ϊ1��ֱ����������������*/
#define INIT_THREAD_INFO(tsk)						\
{													\
	/*��ǰ����ʵ��ָ��*/									\
	.task		= &tsk,								\
	/*��ǰ���̵�ִ������Ϣ*/									\
	.exec_domain	= &default_exec_domain,			\
	/*TIF_*�߳���Ϣ��ʶ*/									\
	.flags		= 0,								\
	/*�������е�cpu*/									\
	.cpu		= 0,								\
	/*�տ�ʼ������ռ*/										\
	.preempt_count	= 1,							\
	/*��ַ����ʱ�ں˵�ַ�ռ�Ľ�����ַ*/							\
	.addr_limit	= KERNEL_DS,						\
	/*ʵ���źŻ��ƣ�����ϵͳ����*/								\
	.restart_block = 								\
	{												\
		/**/\
		.fn = do_no_restart_syscall,				\
	},												\
}

/*��ȡ��ʼ�����̵�thread_info��Ϣ*/
#define init_thread_info		(init_thread_union.thread_info)
/*��ȡ��ʼ�����̵��ں�ջ��Ϣ*/
#define init_stack		(init_thread_union.stack)


/*��ȡ��ǰ���̵��ں�ջָ��*/
register unsigned long current_stack_pointer asm("esp") __attribute_used__;


/*��ȡ��ǰ���̵�thread_info��Ϣ��ջ��ָ����ջ���ȶ��뼴��*/
static inline struct thread_info *current_thread_info(void)
{
	return (struct thread_info *)(current_stack_pointer & ~(THREAD_SIZE - 1));
}

/*ʹ��_get_free_pagesΪ�ں�ջ����ҳ��*/
#ifdef CONFIG_DEBUG_STACK_USAGE
#define alloc_thread_info(tsk) ((struct thread_info *) \
	__get_free_pages(GFP_KERNEL| __GFP_ZERO, get_order(THREAD_SIZE)))
#else
#define alloc_thread_info(tsk) ((struct thread_info *) \
	__get_free_pages(GFP_KERNEL, get_order(THREAD_SIZE)))
#endif
/*�ͷ��ں�ջ�ռ�*/
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

 /*�߳���Ϣ��ʶ����Щ�̱߳�ʶ��Ϊ��ͬ����ļ�������Ҫ���ʹ�����Ҫ��ɵı�ʶΪС����
 ��������Ϊ�����*/
 /*����ϵͳ����*/
#define TIF_SYSCALL_TRACE			0
/*����Ҫ����Ĵ����ź�*/
#define TIF_SIGPENDING				1
/*��Ҫ���µ���*/
#define TIF_NEED_RESCHED			2
/*�û�ģʽ�±��浥�����Է���ֵ*/
#define TIF_SINGLESTEP				3
/*�жϷ���*/
#define TIF_IRET					4
/*����ϵͳģ��*/
#define TIF_SYSCALL_EMU				5
/*�������*/
#define TIF_SYSCALL_AUDIT			6
/*��ȫ�������*/
#define TIF_SECCOMP					7
/*����do_signalִ�����ź�����*/
#define TIF_RESTORE_SIGMASK			8
#define TIF_MEMDIE					16
/*ʹ�õ��ԼĴ���*/
#define TIF_DEBUG					17
/*ʹ��I/Oλͼ*/
#define TIF_IO_BITMAP				18
/*��ͣ����*/
#define TIF_FREEZE					19
/*�û��ռ䲻�ɷ���TSC(ʱ���������)*/
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


 /*�߳�ͬ����ʶ����ԭ�Ӳ���*/
/*ʹ��FPU*/
#define TS_USEDFPU		0x0001
/*���ڷ�˯�߿���ѭ��*/
#define TS_POLLING		0x0002
/*�߳��Ƿ��ڷ�˯�߿�ѭ��״̬*/
#define tsk_is_polling(t) (task_thread_info(t)->status & TS_POLLING)

#endif /* __KERNEL__ */

#endif /* _ASM_THREAD_INFO_H */
