#ifndef _ASM_X86_PTRACE_H
#define _ASM_X86_PTRACE_H

#include <linux/compiler.h>	/* For __user */
#include <asm/ptrace-abi.h>

#ifndef __ASSEMBLY__

#ifdef __i386__

/*结构体定义了一个系统调用期间寄存器集合存储在内核栈的组织形式*/
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
/*测试当前是否处于用户态。最低2位是特权级别标识位，0标识内核态，3标识用户态*/
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

/*获取下一条指令地址*/
#define instruction_pointer(regs) ((regs)->eip)
/*栈帧指针*/
#define frame_pointer(regs) ((regs)->ebp)
/*栈指针，默认ebx*/
#define stack_pointer(regs) ((unsigned long)(regs))
/*函数调用返回值*/
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
其中EIP寄存器主要用于存放当前代码段即将被执行的下一条指令的偏移，但其本质上并不能直接
被指令直接访问。 【it is controlled implicitly by control-transfer instructions (such
as JMP, Jcc, CALL, and RET), interrupts, and exceptions.】 很显然，这个寄存器指令由控
制转移指令、中断及异常所控制。 【The only way to read the EIP register is to execute
a CALL instruction and then read the value of the return instruction pointer from the
procedure stack.The EIP register can be loaded indirectly by modifying the value of a
return instruction pointer on the procedure stack and executing a return instruction
(RET or IRET)】 这里也已经说的很清楚了，读操作通过执行call指令并取得栈中所存放的地址来
实现，而写操作则通过修改程序栈中的返回指令指针并执行RET/IRET指令来完成，因此尽管这个寄
存器相当重要，但其实并不是操作系统在实现过程中所需关注的焦点。
相对来讲，EFLAGS寄存器对于操作系统则重要得多。EFLAGS(program status and control)
register主要用于提供程序的状态及进行相应的控制， 【The EFLGAS register report on the
status of the program being executed and allows limited(application-program level)
control of the process.】 在64-bit模式下，EFLGAS寄存器被扩展为64位的RFLGAS寄存器，高32
位被保留，而低32位则与EFLAGS寄存器相同。
32位的EFLAGS寄存器包含一组状态标志、系统标志以及一个控制标志。在x86处理器初始化之后，
EFLAGS寄存器的状态值为0000 0002H。第1、3、5、15以及22到31位均被保留，这个寄存器中的有
些标志通过使用特殊的通用指令可以直接被修改，但并没有指令能够检查或者修改整个寄存器。
通过使用LAHF/SAHF/PUSHF/POPF/POPFD等指令，可以将EFLAGS寄存器的标志位成组移到程序栈或
EAX寄存器，或者从这些设施中将操作后的结果保存到EFLAGS寄存器中。在EFLAGS寄存器的内容被
传送到栈或是EAX寄存器后，可以通过位操作指令(BT, BTS, BTR, BTC)检查或修改这些标志位。
当调用中断或异常处理程序时，处理器将在程序栈上自动保存EFLAGS的状态值。若在中断或异常
处理时发生任务切换，那么EFLAGS寄存器的状态将被保存在TSS中 【the state of the EFLAGS
register is saved in the TSS for the task being suspended.】 ，注意是将要被挂起的本次
任务的状态。
EFLAGS Register

1、状态标志(Status Flags)
EFLAGS寄存器的状态标志(0、2、4、6、7以及11位)指示算术指令（如ADD, SUB, MUL以及DIV指令）
的结果，这些状态标志的作用如下：
CF(bit 0) [Carry flag] 若算术操作产生的结果在最高有效位(most-significant bit)发生进位
或借位则将其置1，反之清零。这个标志指示无符号整型运算的溢出状态，这个标志同样在多倍精
度运算(multiple-precision arithmetic)中使用。
PF(bit 2) [Parity flag] 如果结果的最低有效字节(least-significant byte)包含偶数个1位则
该位置1，否则清零。
AF(bit 4) [Adjust flag] 如果算术操作在结果的第3位发生进位或借位则将该标志置1，否则清零
。这个标志在BCD(binary-code decimal)算术运算中被使用。
ZF(bit 6) [Zero flag] 若结果为0则将其置1，反之清零。
SF(bit 7) [Sign flag] 该标志被设置为有符号整型的最高有效位。(0指示结果为正，反之则为负)
OF(bit 11) [Overflow flag] 如果整型结果是较大的正数或较小的负数，并且无法匹配目的操作
数时将该位置1，反之清零。这个标志为带符号整型运算指示溢出状态。
在这些状态标志中，只有CF标志能够通过使用STC, CLC以及CMC指令被直接修改，或者通过位指令
(BT, BTS, BTR以及BTC)将指定的位拷贝至CF标志中。
这些状态标志允许单个的算术操作产生三种不同数据类型的结果：无符号整型，有符号整型以及
BCD整型。如果把该结果当做无符号整型，那么CF标志指示越界(out-of-range)状态——即进位或借
位，如果被当做有符号整型，则OF标志指示进位或借位，若作为BCD数，那么AF标志指示进位或借
位。SF标志指示有符号整数的符号位，ZF指示结果为零。此外在执行多倍精度算术运算时，CF标志
用来将一次运算过程中带进位的加法(ADC)或带借位的减法(SBB)产生的进位或借位传递到下一次运
算过程中。
2、DF标志(DF flag)
这个方向标志(位于EFLAGS寄存器的第10位)控制串指令(MOVS, CMPS, SCAS, LODS以及STOS)。设置
DF标志使得串指令自动递减（从高地址向低地址方向处理字符串），清除该标志则使得串指令自动
递增。STD以及CLD指令分别用于设置以及清除DF标志。
3、系统标志以及IOPL域(System Flags and IOPL Field)
EFLAGS寄存器中的这部分标志用于控制操作系统或是执行操作，它们不允许被应用程序所修改。这
些标志的作用如下：
TF(bit 8) [Trap flag] 将该位设置为1以允许单步调试模式，清零则禁用该模式。
IF(bit 9) [Interrupt enable flag] 该标志用于控制处理器对可屏蔽中断请求(maskable
interrupt requests)的响应。置1以响应可屏蔽中断，反之则禁止可屏蔽中断。
IOPL(bits 12 and 13) [I/O privilege level field] 指示当前运行任务的I/O特权级(I/O
privilege level)，正在运行任务的当前特权级(CPL)必须小于或等于I/O特权级才能允许访问I/O
地址空间。这个域只能在CPL为0时才能通过POPF以及IRET指令修改。
NT(bit 14) [Nested task flag] 这个标志控制中断链和被调用任务。若当前任务与前一个执行任
务相关则置1，反之则清零。
RF(bit 16) [Resume flag] 控制处理器对调试异常的响应。
VM(bit 17) [Virtual-8086 mode flag] 置1以允许虚拟8086模式，清除则返回保护模式。
AC(bit 18) [Alignment check flag] 该标志以及在CR0寄存器中的AM位置1时将允许内存引用的对齐检查，以上两个标志中至少有一个被清零则禁用对齐检查。
VIF(bit 19) [Virtual interrupt flag] 该标志是IF标志的虚拟镜像(Virtual image)，与VIP标志结合起来使用。使用这个标志以及VIP标志，并设置CR4控制寄存器中的VME标志就可以允许虚拟模式扩展(virtual mode extensions)
VIP(bit 20) [Virtual interrupt pending flag] 该位置1以指示一个中断正在被挂起，当没有中断挂起时该位清零。【Software sets and clears this flag; the processor only reads it.】与VIF标志结合使用。
ID(bit 21) [Identification flag] 程序能够设置或清除这个标志指示了处理器对CPUID指令的支持。

*/
enum
{
	/*CF(bit 0) [Carry flag] 若算术操作产生的结果在最高有效位(most-significant bit)发
	生进位或借位则将其置1，反之清零。这个标志指示无符号整型运算的溢出状态，这个标志同
	样在多倍精度运算(multiple-precision arithmetic)中使用*/
	EF_CF   = 0x00000001,
	/*PF(bit 2) [Parity flag] 如果结果的最低有效字节(least-significant byte)包含偶数
	个1位则该位置1，否则清零*/
	EF_PF   = 0x00000004,
	/*AF(bit 4) [Adjust flag] 如果算术操作在结果的第3位发生进位或借位则将该标志置1，
	否则清零。这个标志在BCD(binary-code decimal)算术运算中被使用*/
	EF_AF   = 0x00000010,
	/*ZF(bit 6) [Zero flag] 若结果为0则将其置1，反之清零*/
	EF_ZF   = 0x00000040,
	/*SF(bit 7) [Sign flag] 该标志被设置为有符号整型的最高有效位。(0指示结果为正，反
	之则为负)*/
	EF_SF   = 0x00000080,
	/*TF(bit 8) [Trap flag] 将该位设置为1以允许单步调试模式，清零则禁用该模式*/
	EF_TF   = 0x00000100,
	/*IF(bit 9) [Interrupt enable flag] 该标志用于控制处理器对可屏蔽中断请求(maskable
	interrupt requests)的响应。置1以响应可屏蔽中断，反之则禁止可屏蔽中断*/
	EF_IE   = 0x00000200,
	/*DF(bit 10)[DF flag]	方向标志控制串指令(MOVS, CMPS, SCAS, LODS以及STOS)。设置DF标
	志使得串指令自动递减（从高地址向低地址方向处理字符串），清除该标志则使得串指令自动
	递增。STD以及CLD指令分别用于设置以及清除DF标志*/
	EF_DF   = 0x00000400,
	/*OF(bit 11) [Overflow flag] 如果整型结果是较大的正数或较小的负数，并且无法匹配目
	的操作数时将该位置1，反之清零。这个标志为带符号整型运算指示溢出状态*/
	EF_OF   = 0x00000800,
	/*IOPL(bits 12 and 13) [I/O privilege level field] 指示当前运行任务的I/O特权级(I/O
	privilege level)，正在运行任务的当前特权级(CPL)必须小于或等于I/O特权级才能允许访问
	I/O地址空间。这个域只能在CPL为0时才能通过POPF以及IRET指令修改。*/
	EF_IOPL = 0x00003000,
	/**/
	EF_IOPL_RING0 = 0x00000000,
	EF_IOPL_RING1 = 0x00001000,
	/**/
	EF_IOPL_RING2 = 0x00002000,
	/*NT(bit 14) [Nested task flag] 这个标志控制中断链和被调用任务。若当前任务与前一个
	执行任务相关则置1，反之则清零*/
	EF_NT   = 0x00004000,
	/*RF(bit 16) [Resume flag] 控制处理器对调试异常的响应*/
	EF_RF   = 0x00010000,
	/*VM(bit 17) [Virtual-8086 mode flag] 置1以允许虚拟8086模式，清除则返回保护模式*/
	EF_VM   = 0x00020000,
	/*AC(bit 18) [Alignment check flag] 该标志以及在CR0寄存器中的AM位置1时将允许内存
	引用的对齐检查，以上两个标志中至少有一个被清零则禁用对齐检查*/
	EF_AC   = 0x00040000,
	/*VIF(bit 19) [Virtual interrupt flag] 该标志是IF标志的虚拟镜像(Virtual image)，
	与VIP标志结合起来使用。使用这个标志以及VIP标志，并设置CR4控制寄存器中的VME标志就
	可以允许虚拟模式扩展(virtual mode extensions)*/
	EF_VIF  = 0x00080000,
	/*VIP(bit 20) [Virtual interrupt pending flag] 该位置1以指示一个中断正在被挂起，
	当没有中断挂起时该位清零。【Software sets and clears this flag; the processor
	only reads it.】与VIF标志结合使用*/
	EF_VIP  = 0x00100000,
	/*ID(bit 21) [Identification flag] 程序能够设置或清除这个标志指示了处理器对CPUID
	指令的支持*/
	EF_ID   = 0x00200000,
};
#endif /* __KERNEL__ */
#endif /* !__i386__ */
#endif /* !__ASSEMBLY__ */

#endif
