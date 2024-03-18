#ifndef _ASM_X86_SIGNAL_H
#define _ASM_X86_SIGNAL_H

#ifndef __ASSEMBLY__
#include <linux/types.h>
#include <linux/time.h>
#include <linux/compiler.h>

/*����̫��ͷ�ļ���������*/
struct siginfo;

#ifdef __KERNEL__
#include <linux/linkage.h>

/*Ŀǰ֧�ֵ��ź�����*/
#define _NSIG		64

/*ÿ���ְ����ı���λ��Ŀ*/
#ifdef __i386__
#define _NSIG_BPW	32
#else
#define _NSIG_BPW	64
#endif
/*�ź���Ŀ�����ֳ�����Ŀ*/
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)

/*����32λ*/
typedef unsigned long old_sigset_t;

/*�ź�λͼ��λ���룩��ÿһ������λ��Ӧһ���ź�*/
typedef struct
{
	/*�������ı���λ��Ŀ������������֧�ֵ��ź���Ŀ��ͬ*/
	unsigned long sig[_NSIG_WORDS];
} sigset_t;

#else
/* Here we must cater to libcs that poke about in kernel headers.  */

#define NSIG		32
typedef unsigned long sigset_t;

#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */
/* �Ҷϣ����ź����û����������������������ʱ������ͨ�������ն˿��ƽ��̽���ʱ��֪ͨͬһ�Ự
�ڵĸ�����ҵ����ʱ����������ն˲��ٹ���  ��
	��¼Linuxʱ��ϵͳ��������¼�û�һ���ն�(Session)��������ն����е����г��򣬰���ǰ̨��
����� ��̨�����飬һ�㶼������� Session�����û��˳�Linux��¼ʱ��ǰ̨������ͺ�̨�ж��ն���
���Ľ��̽����յ�SIGHUP�źš�����źŵ�Ĭ�ϲ���Ϊ��ֹ���̣����ǰ̨�� ����ͺ�̨���ն������
���̾ͻ���ֹ���������Բ�������źţ�����wget�ܲ���SIGHUP�źţ��������������������˳���Linux
��¼��wgetҲ �ܼ������ء����⣬�������ն������ϵ���ػ����̣�����ź�����֪ͨ�����¶�ȡ�����ļ���*/
#define SIGHUP		 1/* Hangup   */
/* ������ֹ(interrupt)�ź�, ���û�����INTR�ַ�(ͨ����Ctrl+C)ʱ����������֪ͨǰ̨��������ֹ���̡�*/
#define SIGINT		 2/* Interrupt */
/* ��SIGINT����, ����QUIT�ַ�(ͨ����Ctrl+)������. ���������յ�SIGQUIT�˳�ʱ�����core�ļ�, 
�����������������һ����������źš�  */
#define SIGQUIT		 3
/* illeage���Ƿ��ġ�ִ���˷Ƿ�ָ� ͨ������Ϊ��ִ���ļ�������ִ���, ������ͼִ�����ݶ�. 
��ջ���Ҳ�п��ܲ�������źš�  */
#define SIGILL		 4
/* �ɶϵ�ָ����������壨trap��ָ�����. ��debuggerʹ�á�  */
#define SIGTRAP		 5
/* ����abort�������ɵ��źš�  */
#define SIGABRT		 6
/*   */
#define SIGIOT		 6
/* �Ƿ���ַ, �����ڴ��ַ����(alignment)�����������һ���ĸ��ֳ�������, �����ַ
����4�ı���������SIGSEGV���������ں��������ڶԺϷ��洢��ַ�ķǷ����ʴ�����(����ʲ�
�����Լ��洢�ռ��ֻ���洢�ռ�)��  */
#define SIGBUS		 7
/* FPE��floating-point exception�������쳣��������ĸ�����֡��ڷ��������������������
ʱ����. �������������������, ���������������Ϊ0���������е������Ĵ���SIGFPE�ķ���
������ͷ�ļ�signal.h�ж��塣  */
#define SIGFPE		 8
/* ���������������������. ���źŲ��ܱ�����������ͺ��ԡ�*/
#define SIGKILL		 9
/* �����û�ʹ��  */
#define SIGUSR1		10
/* ��ͼ����δ������Լ����ڴ�, ����ͼ��û��дȨ�޵��ڴ��ַд����.  */
#define SIGSEGV		11
/* �����û�ʹ��  */
#define SIGUSR2		12
/*  �ܵ����ѡ�����ź�ͨ���ڽ��̼�ͨ�Ų������������FIFO(�ܵ�)ͨ�ŵ��������̣����ܵ�û��
����������ֹ�����ܵ�д��д���̻��յ�SIGPIPE�źš�������Socketͨ�ŵ��������̣�д������д
Socket��ʱ�򣬶������Ѿ���ֹ��  */
#define SIGPIPE		13
/* ʱ�Ӷ�ʱ�ź�, �������ʵ�ʵ�ʱ���ʱ��ʱ��. alarm����ʹ�ø��ź�.  */
#define SIGALRM		14
/*  �������(terminate)�ź�, ��SIGKILL��ͬ���Ǹ��źſ��Ա������ʹ���ͨ������Ҫ������Լ�
�����˳���shell����killȱʡ��������źš����������ֹ���ˣ����ǲŻ᳢��SIGKILL��  */
#define SIGTERM		15
/* stack fault��Э������ջ���ϣ�һ��malloc����NULL������errnoΪENOMEM������Щϵͳ���ܻ�ʹ�ø��ź�*/
#define SIGSTKFLT	16
/* �ӽ��̣�child������ʱ, �����̻��յ�����źš����������û�д�������źţ�Ҳû��
�ȴ�(wait)�ӽ��̣��ӽ�����Ȼ��ֹ�����ǻ������ں˽��̱���ռ�б���� ʱ���ӽ��̳�Ϊ
��ʬ���̡������� ������Ӧ�ñ���(�����̻��ߺ���SIGCHILD�źţ����߲�׽��������wait����
�����ӽ��̣����߸���������ֹ����ʱ�ӽ��̵���ֹ�Զ���init���� ���ӹ�) */
#define SIGCHLD		17
/* ��һ��ֹͣ(stopped)�Ľ��̼���ִ��. ���źŲ��ܱ�����. ������һ��handler���ó�����
��stopped״̬��Ϊ����ִ��ʱ����ض��Ĺ���. ����, ������ʾ��ʾ�� */
#define SIGCONT		18
/* ��ͣ(stopped)���̵�ִ��. ע������terminate�Լ�interrupt������:�ý��̻�δ����, ֻ��
��ִͣ��. ���źŲ��ܱ�����, �������� */
#define SIGSTOP		19
/* ֹͣ���̵�����, �����źſ��Ա�����ͺ���. �û�����SUSP�ַ�ʱ(ͨ����Ctrl+Z)��������ź� */
#define SIGTSTP		20
/* ����̨��ҵҪ���û��ն˶�����ʱ, ����ҵ�е����н��̻��յ�SIGTTIN�ź�. ȱʡʱ��Щ���̻�
ִֹͣ�С�       Unix�����£���һ�������Ժ�̨��ʽ������������ȥ��д����̨�ն�ʱ�����ᴥ��SIGTTIN
��������SIGTTOU��д���ź��������ţ����̽�����ͣ��linuxĬ������£���read/write���᷵�ش���
���ʱ��shell���ᷢ��֪ͨ���û��������û��л��˽���Ϊǰ̨���̣��Ա����ִ�С��ɺ�̨�л���
ǰ̨�ķ�ʽ��fg���ǰ̨תΪ��̨��ΪCTRL+Z��ݼ���
    ��ô�������ˣ���β����ڲ��ѽ����л���ǰ̨������£���д���������ᱻ��ͣ���𰸣�ֻҪ����
SIGTTIN��SIGTTOU�ź������ɣ�signal(SIGTTOU, SIG_IGN)��
     stty stop/-stop���������������յ�SIGTTOU�ź������Ƿ�ִ����ͣ����Ϊ��Щϵͳ��Ĭ����Ϊ��һ�£�
����mac��Ĭ�Ϻ��ԣ���linux��Ĭ�����á�stty -a���Բ鿴��ǰtty�����ò��� */
#define SIGTTIN		21
/*  ������SIGTTIN, ����д�ն�(���޸��ն�ģʽ)ʱ�յ������������SIGTTIN  */
#define SIGTTOU		22
/* urgent, �����ġ��С����������ݻ�out-of-band���ݵ���socketʱ���� */
#define SIGURG		23
/* ����CPUʱ����Դ����. ������ƿ�����getrlimit/setrlimit����ȡ/�ı�  */
#define SIGXCPU		24
/* ��������ͼ�����ļ������ڳ����ļ���С��Դ����  */
#define SIGXFSZ		25
/* ����ʱ���ź�. ������SIGALRM, ���Ǽ�����Ǹý���ռ�õ�CPUʱ��  */
#define SIGVTALRM	26
/* ������SIGALRM/SIGVTALRM, �������ý����õ�CPUʱ���Լ�ϵͳ���õ�ʱ�� */
#define SIGPROF		27
/* Windows Change, ���ڴ�С�ı�ʱ���� */
#define SIGWINCH	28
/* �ļ�������׼������, ���Կ�ʼ��������/�������  */
#define SIGIO		29
#define SIGPOLL		SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR		30	/* Power failure */
#define SIGSYS		31	/* �Ƿ���ϵͳ����*/
#define	SIGUNUSED	31

/* These should not be considered constants from userland.  */
#define SIGRTMIN	32		/* ʵʱ�ź�����  ���û��ռ䲻�ÿ���*/
#define SIGRTMAX	_NSIG	/*   ʵʱ�ź�����*/

/* �������г����ź���
	���򲻿ɲ�����������Ե��ź��У�SIGKILL,SIGSTOP
    ���ָܻ���Ĭ�϶������ź��У� SIGILL,SIGTRAP
    Ĭ�ϻᵼ�½����������ź��У�SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGIOT,SIGQUIT,SIGSEGV,SIGTRAP,
SIGXCPU,SIGXFSZ
    Ĭ�ϻᵼ�½����˳����ź��У�SIGALRM,SIGHUP,SIGINT,SIGKILL,SIGPIPE,SIGPOLL,SIGPROF,SIGSYS,
SIGTERM,SIGUSR1,SIGUSR2,SIGVTALRM
    Ĭ�ϻᵼ�½���ֹͣ���ź��У�SIGSTOP,SIGTSTP,SIGTTIN,SIGTTOU
    Ĭ�Ͻ��̺��Ե��ź��У�SIGCHLD,SIGPWR,SIGURG,SIGWINCH
    ���⣬SIGIO��SVR4���˳�����4.3BSD���Ǻ��ԣ�SIGCONT�ڽ��̹���ʱ�Ǽ����������Ǻ��ԣ����ܱ�����
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
	/*����ָ�����ͣ��ں����źŵ���ʱ���õĴ�����*/
	__sighandler_t sa_handler;
	/*�����ʶ������ָ���źŴ���ʱ��һЩԼ��*/
	unsigned long sa_flags;
	/*����һ��λ���룬ÿ������λ��Ӧ��ϵͳ�е�һ���źţ��������ڴ�����������ڼ�����
	�����źţ��ڴ���������ʱ���ں˻�������ֵ���ָ����źŴ���֮ǰ��ԭֵ*/
	__sigrestore_t sa_restorer;
	/* mask last for extensibility */
	sigset_t sa_mask;
};

struct k_sigaction
{
	/*�źŴ�����������Ϣ*/
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
