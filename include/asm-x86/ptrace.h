#ifndef _ASM_X86_PTRACE_H
#define _ASM_X86_PTRACE_H

#include <linux/compiler.h>	/* For __user */
#include <asm/ptrace-abi.h>

#ifndef __ASSEMBLY__

#ifdef __i386__

/*�ṹ�嶨����һ��ϵͳ�����ڼ�Ĵ������ϴ洢���ں�ջ����֯��ʽ*/
struct pt_regs
{
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	int  xfs;
	/* int  xgs; */
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
};

#ifdef __KERNEL__

#include <asm/vm86.h>
#include <asm/segment.h>

struct task_struct;
extern void send_sigtrap(struct task_struct *tsk, struct pt_regs *regs, int error_code);

/*
 * user_mode_vm(regs) determines whether a register set came from user mode.
 * This is true if V8086 mode was enabled OR if the register set was from
 * protected mode with RPL-3 CS value.  This tricky test checks that with
 * one comparison.  Many places in the kernel can bypass this full check
 * if they have already ruled out V8086 mode, so user_mode(regs) can be used.
 */
/*���Ե�ǰ�Ƿ����û�̬�����2λ����Ȩ�����ʶλ��0��ʶ�ں�̬��3��ʶ�û�̬*/
static inline int user_mode(struct pt_regs *regs)
{
	return (regs->xcs & SEGMENT_RPL_MASK) == USER_RPL;
}
/**/
static inline int user_mode_vm(struct pt_regs *regs)
{
	return ((regs->xcs & SEGMENT_RPL_MASK) | (regs->eflags & VM_MASK)) >= USER_RPL;
}
/**/
static inline int v8086_mode(struct pt_regs *regs)
{
	return (regs->eflags & VM_MASK);
}

/*��ȡ��һ��ָ���ַ*/
#define instruction_pointer(regs) ((regs)->eip)
/*ջָ֡��*/
#define frame_pointer(regs) ((regs)->ebp)
/*ջָ�룬Ĭ��ebx*/
#define stack_pointer(regs) ((unsigned long)(regs))
/*�������÷���ֵ*/
#define regs_return_value(regs) ((regs)->eax)

extern unsigned long profile_pc(struct pt_regs *regs);
#endif /* __KERNEL__ */

#else /* __i386__ */

struct pt_regs
{
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long rbp;
	unsigned long rbx;
/* arguments: non interrupts/non tracing syscalls only save upto here*/
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long rax;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsi;
	unsigned long rdi;
	unsigned long orig_rax;
/* end of arguments */
/* cpu exception frame or undefined */
	unsigned long rip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long rsp;
	unsigned long ss;
/* top of stack page */
};

#ifdef __KERNEL__

#define user_mode(regs) (!!((regs)->cs & 3))
#define user_mode_vm(regs) user_mode(regs)
#define instruction_pointer(regs) ((regs)->rip)
#define frame_pointer(regs) ((regs)->rbp)
#define stack_pointer(regs) ((regs)->rsp)
#define regs_return_value(regs) ((regs)->rax)

extern unsigned long profile_pc(struct pt_regs *regs);
void signal_fault(struct pt_regs *regs, void __user *frame, char *where);

struct task_struct;

extern unsigned long
convert_rip_to_linear(struct task_struct *child, struct pt_regs *regs);

/*
����EIP�Ĵ�����Ҫ���ڴ�ŵ�ǰ����μ�����ִ�е���һ��ָ���ƫ�ƣ����䱾���ϲ�����ֱ��
��ָ��ֱ�ӷ��ʡ� ��it is controlled implicitly by control-transfer instructions (such
as JMP, Jcc, CALL, and RET), interrupts, and exceptions.�� ����Ȼ������Ĵ���ָ���ɿ�
��ת��ָ��жϼ��쳣�����ơ� ��The only way to read the EIP register is to execute
a CALL instruction and then read the value of the return instruction pointer from the
procedure stack.The EIP register can be loaded indirectly by modifying the value of a
return instruction pointer on the procedure stack and executing a return instruction
(RET or IRET)�� ����Ҳ�Ѿ�˵�ĺ�����ˣ�������ͨ��ִ��callָ�ȡ��ջ������ŵĵ�ַ��
ʵ�֣���д������ͨ���޸ĳ���ջ�еķ���ָ��ָ�벢ִ��RET/IRETָ������ɣ���˾��������
�����൱��Ҫ������ʵ�����ǲ���ϵͳ��ʵ�ֹ����������ע�Ľ��㡣
���������EFLAGS�Ĵ������ڲ���ϵͳ����Ҫ�öࡣEFLAGS(program status and control)
register��Ҫ�����ṩ�����״̬��������Ӧ�Ŀ��ƣ� ��The EFLGAS register report on the
status of the program being executed and allows limited(application-program level)
control of the process.�� ��64-bitģʽ�£�EFLGAS�Ĵ�������չΪ64λ��RFLGAS�Ĵ�������32
λ������������32λ����EFLAGS�Ĵ�����ͬ��
32λ��EFLAGS�Ĵ�������һ��״̬��־��ϵͳ��־�Լ�һ�����Ʊ�־����x86��������ʼ��֮��
EFLAGS�Ĵ�����״ֵ̬Ϊ0000 0002H����1��3��5��15�Լ�22��31λ��������������Ĵ����е���
Щ��־ͨ��ʹ�������ͨ��ָ�����ֱ�ӱ��޸ģ�����û��ָ���ܹ��������޸������Ĵ�����
ͨ��ʹ��LAHF/SAHF/PUSHF/POPF/POPFD��ָ����Խ�EFLAGS�Ĵ����ı�־λ�����Ƶ�����ջ��
EAX�Ĵ��������ߴ���Щ��ʩ�н�������Ľ�����浽EFLAGS�Ĵ����С���EFLAGS�Ĵ��������ݱ�
���͵�ջ����EAX�Ĵ����󣬿���ͨ��λ����ָ��(BT, BTS, BTR, BTC)�����޸���Щ��־λ��
�������жϻ��쳣�������ʱ�����������ڳ���ջ���Զ�����EFLAGS��״ֵ̬�������жϻ��쳣
����ʱ���������л�����ôEFLAGS�Ĵ�����״̬����������TSS�� ��the state of the EFLAGS
register is saved in the TSS for the task being suspended.�� ��ע���ǽ�Ҫ������ı���
�����״̬��
EFLAGS Register

1��״̬��־(Status Flags)
EFLAGS�Ĵ�����״̬��־(0��2��4��6��7�Լ�11λ)ָʾ����ָ���ADD, SUB, MUL�Լ�DIVָ�
�Ľ������Щ״̬��־���������£�
CF(bit 0) [Carry flag] ���������������Ľ���������Чλ(most-significant bit)������λ
���λ������1����֮���㡣�����־ָʾ�޷���������������״̬�������־ͬ���ڶ౶��
������(multiple-precision arithmetic)��ʹ�á�
PF(bit 2) [Parity flag] �������������Ч�ֽ�(least-significant byte)����ż����1λ��
��λ��1���������㡣
AF(bit 4) [Adjust flag] ������������ڽ���ĵ�3λ������λ���λ�򽫸ñ�־��1����������
�������־��BCD(binary-code decimal)���������б�ʹ�á�
ZF(bit 6) [Zero flag] �����Ϊ0������1����֮���㡣
SF(bit 7) [Sign flag] �ñ�־������Ϊ�з������͵������Чλ��(0ָʾ���Ϊ������֮��Ϊ��)
OF(bit 11) [Overflow flag] ������ͽ���ǽϴ���������С�ĸ����������޷�ƥ��Ŀ�Ĳ���
��ʱ����λ��1����֮���㡣�����־Ϊ��������������ָʾ���״̬��
����Щ״̬��־�У�ֻ��CF��־�ܹ�ͨ��ʹ��STC, CLC�Լ�CMCָ�ֱ���޸ģ�����ͨ��λָ��
(BT, BTS, BTR�Լ�BTC)��ָ����λ������CF��־�С�
��Щ״̬��־�����������������������ֲ�ͬ�������͵Ľ�����޷������ͣ��з��������Լ�
BCD���͡�����Ѹý�������޷������ͣ���ôCF��־ָʾԽ��(out-of-range)״̬��������λ���
λ������������з������ͣ���OF��־ָʾ��λ���λ������ΪBCD������ôAF��־ָʾ��λ���
λ��SF��־ָʾ�з��������ķ���λ��ZFָʾ���Ϊ�㡣������ִ�ж౶������������ʱ��CF��־
������һ����������д���λ�ļӷ�(ADC)�����λ�ļ���(SBB)�����Ľ�λ���λ���ݵ���һ����
������С�
2��DF��־(DF flag)
��������־(λ��EFLAGS�Ĵ����ĵ�10λ)���ƴ�ָ��(MOVS, CMPS, SCAS, LODS�Լ�STOS)������
DF��־ʹ�ô�ָ���Զ��ݼ����Ӹߵ�ַ��͵�ַ�������ַ�����������ñ�־��ʹ�ô�ָ���Զ�
������STD�Լ�CLDָ��ֱ����������Լ����DF��־��
3��ϵͳ��־�Լ�IOPL��(System Flags and IOPL Field)
EFLAGS�Ĵ����е��ⲿ�ֱ�־���ڿ��Ʋ���ϵͳ����ִ�в��������ǲ�����Ӧ�ó������޸ġ���
Щ��־���������£�
TF(bit 8) [Trap flag] ����λ����Ϊ1������������ģʽ����������ø�ģʽ��
IF(bit 9) [Interrupt enable flag] �ñ�־���ڿ��ƴ������Կ������ж�����(maskable
interrupt requests)����Ӧ����1����Ӧ�������жϣ���֮���ֹ�������жϡ�
IOPL(bits 12 and 13) [I/O privilege level field] ָʾ��ǰ���������I/O��Ȩ��(I/O
privilege level)��������������ĵ�ǰ��Ȩ��(CPL)����С�ڻ����I/O��Ȩ�������������I/O
��ַ�ռ䡣�����ֻ����CPLΪ0ʱ����ͨ��POPF�Լ�IRETָ���޸ġ�
NT(bit 14) [Nested task flag] �����־�����ж����ͱ�������������ǰ������ǰһ��ִ����
���������1����֮�����㡣
RF(bit 16) [Resume flag] ���ƴ������Ե����쳣����Ӧ��
VM(bit 17) [Virtual-8086 mode flag] ��1����������8086ģʽ������򷵻ر���ģʽ��
AC(bit 18) [Alignment check flag] �ñ�־�Լ���CR0�Ĵ����е�AMλ��1ʱ�������ڴ����õĶ����飬����������־��������һ������������ö����顣
VIF(bit 19) [Virtual interrupt flag] �ñ�־��IF��־�����⾵��(Virtual image)����VIP��־�������ʹ�á�ʹ�������־�Լ�VIP��־��������CR4���ƼĴ����е�VME��־�Ϳ�����������ģʽ��չ(virtual mode extensions)
VIP(bit 20) [Virtual interrupt pending flag] ��λ��1��ָʾһ���ж����ڱ����𣬵�û���жϹ���ʱ��λ���㡣��Software sets and clears this flag; the processor only reads it.����VIF��־���ʹ�á�
ID(bit 21) [Identification flag] �����ܹ����û���������־ָʾ�˴�������CPUIDָ���֧�֡�

*/
enum
{
	/*CF(bit 0) [Carry flag] ���������������Ľ���������Чλ(most-significant bit)��
	����λ���λ������1����֮���㡣�����־ָʾ�޷���������������״̬�������־ͬ
	���ڶ౶��������(multiple-precision arithmetic)��ʹ��*/
	EF_CF   = 0x00000001,
	/*PF(bit 2) [Parity flag] �������������Ч�ֽ�(least-significant byte)����ż��
	��1λ���λ��1����������*/
	EF_PF   = 0x00000004,
	/*AF(bit 4) [Adjust flag] ������������ڽ���ĵ�3λ������λ���λ�򽫸ñ�־��1��
	�������㡣�����־��BCD(binary-code decimal)���������б�ʹ��*/
	EF_AF   = 0x00000010,
	/*ZF(bit 6) [Zero flag] �����Ϊ0������1����֮����*/
	EF_ZF   = 0x00000040,
	/*SF(bit 7) [Sign flag] �ñ�־������Ϊ�з������͵������Чλ��(0ָʾ���Ϊ������
	֮��Ϊ��)*/
	EF_SF   = 0x00000080,
	/*TF(bit 8) [Trap flag] ����λ����Ϊ1������������ģʽ����������ø�ģʽ*/
	EF_TF   = 0x00000100,
	/*IF(bit 9) [Interrupt enable flag] �ñ�־���ڿ��ƴ������Կ������ж�����(maskable
	interrupt requests)����Ӧ����1����Ӧ�������жϣ���֮���ֹ�������ж�*/
	EF_IE   = 0x00000200,
	/*DF(bit 10)[DF flag]	�����־���ƴ�ָ��(MOVS, CMPS, SCAS, LODS�Լ�STOS)������DF��
	־ʹ�ô�ָ���Զ��ݼ����Ӹߵ�ַ��͵�ַ�������ַ�����������ñ�־��ʹ�ô�ָ���Զ�
	������STD�Լ�CLDָ��ֱ����������Լ����DF��־*/
	EF_DF   = 0x00000400,
	/*OF(bit 11) [Overflow flag] ������ͽ���ǽϴ���������С�ĸ����������޷�ƥ��Ŀ
	�Ĳ�����ʱ����λ��1����֮���㡣�����־Ϊ��������������ָʾ���״̬*/
	EF_OF   = 0x00000800,
	/*IOPL(bits 12 and 13) [I/O privilege level field] ָʾ��ǰ���������I/O��Ȩ��(I/O
	privilege level)��������������ĵ�ǰ��Ȩ��(CPL)����С�ڻ����I/O��Ȩ�������������
	I/O��ַ�ռ䡣�����ֻ����CPLΪ0ʱ����ͨ��POPF�Լ�IRETָ���޸ġ�*/
	EF_IOPL = 0x00003000,
	/**/
	EF_IOPL_RING0 = 0x00000000,
	EF_IOPL_RING1 = 0x00001000,
	/**/
	EF_IOPL_RING2 = 0x00002000,
	/*NT(bit 14) [Nested task flag] �����־�����ж����ͱ�������������ǰ������ǰһ��
	ִ�������������1����֮������*/
	EF_NT   = 0x00004000,
	/*RF(bit 16) [Resume flag] ���ƴ������Ե����쳣����Ӧ*/
	EF_RF   = 0x00010000,
	/*VM(bit 17) [Virtual-8086 mode flag] ��1����������8086ģʽ������򷵻ر���ģʽ*/
	EF_VM   = 0x00020000,
	/*AC(bit 18) [Alignment check flag] �ñ�־�Լ���CR0�Ĵ����е�AMλ��1ʱ�������ڴ�
	���õĶ����飬����������־��������һ������������ö�����*/
	EF_AC   = 0x00040000,
	/*VIF(bit 19) [Virtual interrupt flag] �ñ�־��IF��־�����⾵��(Virtual image)��
	��VIP��־�������ʹ�á�ʹ�������־�Լ�VIP��־��������CR4���ƼĴ����е�VME��־��
	������������ģʽ��չ(virtual mode extensions)*/
	EF_VIF  = 0x00080000,
	/*VIP(bit 20) [Virtual interrupt pending flag] ��λ��1��ָʾһ���ж����ڱ�����
	��û���жϹ���ʱ��λ���㡣��Software sets and clears this flag; the processor
	only reads it.����VIF��־���ʹ��*/
	EF_VIP  = 0x00100000,
	/*ID(bit 21) [Identification flag] �����ܹ����û���������־ָʾ�˴�������CPUID
	ָ���֧��*/
	EF_ID   = 0x00200000,
};
#endif /* __KERNEL__ */
#endif /* !__i386__ */
#endif /* !__ASSEMBLY__ */

#endif
