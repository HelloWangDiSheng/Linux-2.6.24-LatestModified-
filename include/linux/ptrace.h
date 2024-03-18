#ifndef _LINUX_PTRACE_H
#define _LINUX_PTRACE_H

/*��ʼ�Ե�ǰ���̵ĸ��٣���ǰ���̵ĸ������Զ��е������ߵĽ�ɫ������׼���ô��ӽ��̽�����Ϣ*/
#define PTRACE_TRACEME		   	0
/*�ӽ��̵Ĵ���ζ�ȡ������*/
#define PTRACE_PEEKTEXT		   	1
/*�ӽ��̵����ݶζ�ȡ������*/
#define PTRACE_PEEKDATA		   	2
/*���û����̶�ȡ��ͨ��CPU�Ĵ�����ʹ�õ��κ��������ԼĴ�������Ȼ������ݱ�ʶ��ֻ��ȡһ���Ĵ���
�����ݣ������������Ĵ������ϵ����ݣ�*/
#define PTRACE_PEEKUSR		   	3
/*�򱻼�ؽ��̵Ĵ����д��ֵ*/
#define PTRACE_POKETEXT		   	4
/*�򱻼�ؽ��̵����ݶ�д��ֵ*/
#define PTRACE_POKEDATA		   	5
/*�򱻼�ؽ��̵ļĴ���д��ֵ����ѡ��֧�ֶԸ߼����Լ�����ʹ�ã�����Լ�ش����¼�����һ������
����ʱ�����ض�λ����ͣ�����ִ��*/
#define PTRACE_POKEUSR		   	6
/*�ָ������ٽ��̵�ִ�У������Զ���ͣ�ý��̵ľ��������������ٵĽ��̽��ڽ��ܵ��ź�ʱ��ͣ*/
#define PTRACE_CONT				7
/*����kill�źţ��رձ����ٽ���*/
#define PTRACE_KILL		   		8
/*����������ִ�б����ٽ����ڼ䣬���ڵ���ִ��ģʽ��������ģʽ�£�ִ��ÿ���������ָ��֮��
�������߷���һ��SIGCHLD�ź�Ȼ�����˯�ߣ������߿��Է��ʱ����ٽ��̣�����Ȼ��һ�ַǳ����е�
Ӧ�ó�����Լ������ر�������ͼ���ٱ���������������Ƚ�΢�������ʱ*/
#define PTRACE_SINGLESTEP		9
/*����һ���������ӵ�һ�����̲���ʼ����*/
#define PTRACE_ATTACH		  	16
/*�Ӹý��̶Ͽ�����������*/
#define PTRACE_DETACH		  	17
/*ϵͳ����׷���ǻ��ڸñ�ʶ�ģ�����ø�ѡ���ptrace����ô�ں˽���ʼִ�н��̣�ֱ������һ��
ϵͳ���á��ڱ�׷�ٽ���ֹͣ��wait֪ͨ�����߽��̣������߽���������ʹ������ptraceѡ�����
�������ٽ��̵ĵ�ַ�ռ䣬���ռ��й�ϵͳ���õ���Ϣ�������ϵͳ����֮�󣬱����ٵĽ��̵ڶ�����
ͣ��ʹ�ø����߽��̿��Լ������Ƿ�ɹ�*/
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
/*���˳��ź�����ΪSIGCHLD��fork��clone����֮ǰֹͣ*/
#define PTRACE_EVENT_FORK				1
/*�ڴ�vfork(2)�����CLONE_VFORK��־�Ŀ�¡(2)����֮ǰֹͣ���ڴ�ֹ֮ͣ���������ʱ������
�ڼ���ִ��֮ǰ�ȴ��ӽ����˳�/ִ��(���仰˵��vfork(2)��ͨ����Ϊ)��*/
#define PTRACE_EVENT_VFORK				2
/*��clone(2)����֮ǰֹͣ*/
#define PTRACE_EVENT_CLONE				3
/*�ڴ�execve����֮ǰֹͣ*/
#define PTRACE_EVENT_EXEC				4
/*��ʹ��CLONE_VFORK��־��vfork(2)��clone(2)����֮ǰֹͣ���������Ӽ�ͨ���˳���ִ��ȡ����
���˸���֮�󡣶������������ĸ�ֹͣ����ֹͣ�������ڸ���(��Tracee)�У����������´������߳�
�С� PTRACE_GETEVENTMSG�����ڼ������̵߳�ID��*/
#define PTRACE_EVENT_VFORK_DONE			5
/*���˳�(����exit_group(2)������)�������ź���������execve(2)�ڶ��߳̽����е����˳�֮ǰֹͣ
��PTRACE_GETEVENTMSG�����˳�״̬�����Լ��Ĵ���(��"ʵ��"�˳�����ʱ��ͬ)��ʾ�ٻ����ţ�����
�˳�PTRACE_CONT��PTRACE_DETACH��*/
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
/*�ڴ�vfork�����CLONE_VFORK��־��clone(2)����֮ǰֹͣ���ڴ�ֹ֮ͣ���������ʱ�������ڼ���ִ��
֮ǰ�ȴ��ӽ����˳���ִ��*/
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
