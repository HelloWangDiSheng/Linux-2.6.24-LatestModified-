#ifndef __ASM_I386_PROCESSOR_FLAGS_H
#define __ASM_I386_PROCESSOR_FLAGS_H
/* Various flags defined: can be included from assembler. */

/*
 * EFLAGS bits
 */
/*CF(bit 0) [Carry flag] ���������������Ľ���������Чλ(most-significant bit)��
����λ���λ������1����֮���㡣�����־ָʾ�޷���������������״̬�������־ͬ
���ڶ౶��������(multiple-precision arithmetic)��ʹ��*/
#define X86_EFLAGS_CF		0x00000001
/*PF(bit 2) [Parity flag] �������������Ч�ֽ�(least-significant byte)����ż��
��1λ���λ��1����������*/
#define X86_EFLAGS_PF		0x00000004
/*AF(bit 4) [Adjust flag] ������������ڽ���ĵ�3λ������λ���λ�򽫸ñ�־��1��
�������㡣�����־��BCD(binary-code decimal)���������б�ʹ��*/
#define X86_EFLAGS_AF		0x00000010
/*ZF(bit 6) [Zero flag] �����Ϊ0������1����֮����*/
#define X86_EFLAGS_ZF		0x00000040
/*SF(bit 7) [Sign flag] �ñ�־������Ϊ�з������͵������Чλ��(0ָʾ���Ϊ������
֮��Ϊ��)*/
#define X86_EFLAGS_SF		0x00000080
/*TF(bit 8) [Trap flag] ����λ����Ϊ1������������ģʽ����������ø�ģʽ*/
#define X86_EFLAGS_TF		0x00000100
/*IF(bit 9) [Interrupt enable flag] �ñ�־���ڿ��ƴ������Կ������ж�����(maskable
interrupt requests)����Ӧ����1����Ӧ�������жϣ���֮���ֹ�������ж�*/
#define X86_EFLAGS_IF		0x00000200
/*DF(bit 10)[DF flag]	�����־���ƴ�ָ��(MOVS, CMPS, SCAS, LODS�Լ�STOS)������DF��
־ʹ�ô�ָ���Զ��ݼ����Ӹߵ�ַ��͵�ַ�������ַ�����������ñ�־��ʹ�ô�ָ���Զ�
������STD�Լ�CLDָ��ֱ����������Լ����DF��־*/
#define X86_EFLAGS_DF		0x00000400
/*OF(bit 11) [Overflow flag] ������ͽ���ǽϴ���������С�ĸ����������޷�ƥ��Ŀ
�Ĳ�����ʱ����λ��1����֮���㡣�����־Ϊ��������������ָʾ���״̬*/
#define X86_EFLAGS_OF		0x00000800
/*IOPL(bits 12 and 13) [I/O privilege level field] ָʾ��ǰ���������I/O��Ȩ��(I/O
privilege level)��������������ĵ�ǰ��Ȩ��(CPL)����С�ڻ����I/O��Ȩ�������������
I/O��ַ�ռ䡣�����ֻ����CPLΪ0ʱ����ͨ��POPF�Լ�IRETָ���޸ġ�*/
#define X86_EFLAGS_IOPL		0x00003000
/*NT(bit 14) [Nested task flag] �����־�����ж����ͱ�������������ǰ������ǰһ��
ִ�������������1����֮������*/
#define X86_EFLAGS_NT		0x00004000
/*RF(bit 16) [Resume flag] ���ƴ������Ե����쳣����Ӧ*/
#define X86_EFLAGS_RF		0x00010000
/*VM(bit 17) [Virtual-8086 mode flag] ��1����������8086ģʽ������򷵻ر���ģʽ*/
#define X86_EFLAGS_VM		0x00020000
/*AC(bit 18) [Alignment check flag] �ñ�־�Լ���CR0�Ĵ����е�AMλ��1ʱ�������ڴ�
���õĶ����飬����������־��������һ������������ö�����*/
#define X86_EFLAGS_AC		0x00040000
/*VIF(bit 19) [Virtual interrupt flag] �ñ�־��IF��־�����⾵��(Virtual image)��
��VIP��־�������ʹ�á�ʹ�������־�Լ�VIP��־��������CR4���ƼĴ����е�VME��־��
������������ģʽ��չ(virtual mode extensions)*/
#define X86_EFLAGS_VIF		0x00080000
/*VIP(bit 20) [Virtual interrupt pending flag] ��λ��1��ָʾһ���ж����ڱ�����
��û���жϹ���ʱ��λ���㡣��Software sets and clears this flag; the processor
only reads it.����VIF��־���ʹ��*/
#define X86_EFLAGS_VIP		0x00100000
/*ID(bit 21) [Identification flag] �����ܹ����û���������־ָʾ�˴�������CPUID
ָ���֧��*/
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
