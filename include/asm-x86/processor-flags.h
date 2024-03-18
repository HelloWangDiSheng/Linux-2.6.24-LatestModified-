#ifndef __ASM_I386_PROCESSOR_FLAGS_H
#define __ASM_I386_PROCESSOR_FLAGS_H
/* Various flags defined: can be included from assembler. */

/*
 * EFLAGS bits
 */
/*CF(bit 0) [Carry flag] 若算术操作产生的结果在最高有效位(most-significant bit)发
生进位或借位则将其置1，反之清零。这个标志指示无符号整型运算的溢出状态，这个标志同
样在多倍精度运算(multiple-precision arithmetic)中使用*/
#define X86_EFLAGS_CF		0x00000001
/*PF(bit 2) [Parity flag] 如果结果的最低有效字节(least-significant byte)包含偶数
个1位则该位置1，否则清零*/
#define X86_EFLAGS_PF		0x00000004
/*AF(bit 4) [Adjust flag] 如果算术操作在结果的第3位发生进位或借位则将该标志置1，
否则清零。这个标志在BCD(binary-code decimal)算术运算中被使用*/
#define X86_EFLAGS_AF		0x00000010
/*ZF(bit 6) [Zero flag] 若结果为0则将其置1，反之清零*/
#define X86_EFLAGS_ZF		0x00000040
/*SF(bit 7) [Sign flag] 该标志被设置为有符号整型的最高有效位。(0指示结果为正，反
之则为负)*/
#define X86_EFLAGS_SF		0x00000080
/*TF(bit 8) [Trap flag] 将该位设置为1以允许单步调试模式，清零则禁用该模式*/
#define X86_EFLAGS_TF		0x00000100
/*IF(bit 9) [Interrupt enable flag] 该标志用于控制处理器对可屏蔽中断请求(maskable
interrupt requests)的响应。置1以响应可屏蔽中断，反之则禁止可屏蔽中断*/
#define X86_EFLAGS_IF		0x00000200
/*DF(bit 10)[DF flag]	方向标志控制串指令(MOVS, CMPS, SCAS, LODS以及STOS)。设置DF标
志使得串指令自动递减（从高地址向低地址方向处理字符串），清除该标志则使得串指令自动
递增。STD以及CLD指令分别用于设置以及清除DF标志*/
#define X86_EFLAGS_DF		0x00000400
/*OF(bit 11) [Overflow flag] 如果整型结果是较大的正数或较小的负数，并且无法匹配目
的操作数时将该位置1，反之清零。这个标志为带符号整型运算指示溢出状态*/
#define X86_EFLAGS_OF		0x00000800
/*IOPL(bits 12 and 13) [I/O privilege level field] 指示当前运行任务的I/O特权级(I/O
privilege level)，正在运行任务的当前特权级(CPL)必须小于或等于I/O特权级才能允许访问
I/O地址空间。这个域只能在CPL为0时才能通过POPF以及IRET指令修改。*/
#define X86_EFLAGS_IOPL		0x00003000
/*NT(bit 14) [Nested task flag] 这个标志控制中断链和被调用任务。若当前任务与前一个
执行任务相关则置1，反之则清零*/
#define X86_EFLAGS_NT		0x00004000
/*RF(bit 16) [Resume flag] 控制处理器对调试异常的响应*/
#define X86_EFLAGS_RF		0x00010000
/*VM(bit 17) [Virtual-8086 mode flag] 置1以允许虚拟8086模式，清除则返回保护模式*/
#define X86_EFLAGS_VM		0x00020000
/*AC(bit 18) [Alignment check flag] 该标志以及在CR0寄存器中的AM位置1时将允许内存
引用的对齐检查，以上两个标志中至少有一个被清零则禁用对齐检查*/
#define X86_EFLAGS_AC		0x00040000
/*VIF(bit 19) [Virtual interrupt flag] 该标志是IF标志的虚拟镜像(Virtual image)，
与VIP标志结合起来使用。使用这个标志以及VIP标志，并设置CR4控制寄存器中的VME标志就
可以允许虚拟模式扩展(virtual mode extensions)*/
#define X86_EFLAGS_VIF		0x00080000
/*VIP(bit 20) [Virtual interrupt pending flag] 该位置1以指示一个中断正在被挂起，
当没有中断挂起时该位清零。【Software sets and clears this flag; the processor
only reads it.】与VIF标志结合使用*/
#define X86_EFLAGS_VIP		0x00100000
/*ID(bit 21) [Identification flag] 程序能够设置或清除这个标志指示了处理器对CPUID
指令的支持*/
#define X86_EFLAGS_ID		0x00200000

/*
 * Basic CPU control in CR0
 */
#define X86_CR0_PE	0x00000001 /* Protection Enable */
#define X86_CR0_MP	0x00000002 /* Monitor Coprocessor */
#define X86_CR0_EM	0x00000004 /* Emulation */
#define X86_CR0_TS	0x00000008 /* Task Switched */
#define X86_CR0_ET	0x00000010 /* Extension Type */
#define X86_CR0_NE	0x00000020 /* Numeric Error */
#define X86_CR0_WP	0x00010000 /* Write Protect */
#define X86_CR0_AM	0x00040000 /* Alignment Mask */
#define X86_CR0_NW	0x20000000 /* Not Write-through */
#define X86_CR0_CD	0x40000000 /* Cache Disable */
#define X86_CR0_PG	0x80000000 /* Paging */

/*
 * Paging options in CR3
 */
#define X86_CR3_PWT	0x00000008 /* Page Write Through */
#define X86_CR3_PCD	0x00000010 /* Page Cache Disable */

/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME	0x00000001 /* enable vm86 extensions */
#define X86_CR4_PVI	0x00000002 /* virtual interrupts flag enable */
#define X86_CR4_TSD	0x00000004 /* disable time stamp at ipl 3 */
#define X86_CR4_DE	0x00000008 /* enable debugging extensions */
#define X86_CR4_PSE	0x00000010 /* enable page size extensions */
#define X86_CR4_PAE	0x00000020 /* enable physical address extensions */
#define X86_CR4_MCE	0x00000040 /* Machine check enable */
#define X86_CR4_PGE	0x00000080 /* enable global pages */
#define X86_CR4_PCE	0x00000100 /* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR	0x00000200 /* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT 0x00000400 /* enable unmasked SSE exceptions */
#define X86_CR4_VMXE	0x00002000 /* enable VMX virtualization */

/*
 * x86-64 Task Priority Register, CR8
 */
#define X86_CR8_TPR	0x0000000F /* task priority register */

/*
 * AMD and Transmeta use MSRs for configuration; see <asm/msr-index.h>
 */

/*
 *      NSC/Cyrix CPU configuration register indexes
 */
#define CX86_PCR0	0x20
#define CX86_GCR	0xb8
#define CX86_CCR0	0xc0
#define CX86_CCR1	0xc1
#define CX86_CCR2	0xc2
#define CX86_CCR3	0xc3
#define CX86_CCR4	0xe8
#define CX86_CCR5	0xe9
#define CX86_CCR6	0xea
#define CX86_CCR7	0xeb
#define CX86_PCR1	0xf0
#define CX86_DIR0	0xfe
#define CX86_DIR1	0xff
#define CX86_ARR_BASE	0xc4
#define CX86_RCR_BASE	0xdc

#endif	/* __ASM_I386_PROCESSOR_FLAGS_H */
