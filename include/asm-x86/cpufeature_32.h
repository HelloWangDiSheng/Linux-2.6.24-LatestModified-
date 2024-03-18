/*定义x86处理器特性字节*/

#ifndef __ASM_I386_CPUFEATURE_H
#define __ASM_I386_CPUFEATURE_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif
#include <asm/required-features.h>

/*cpu特性信息需要N个32位字表示*/
#define NCAPINTS						8

/* Intel-defined CPU features, CPUID level 0x00000001 (edx), word 0 */
/*Intel公司定义的cpu特性，CPUID层级0x00000001(edx)，第一个字长位置*/
/* Onboard FPU */
#define X86_FEATURE_FPU					(0*32+ 0)
/* Virtual Mode Extensions */
#define X86_FEATURE_VME					(0*32+ 1)
/* Debugging Extensions */
#define X86_FEATURE_DE					(0*32+ 2)
/*对超大内存页（Page Size Extensions）的支持。这些特别标记的页，其长度为4MB，而不是
普通的4KB。该选项用于不会换出的内核页。增加页的大小，意味着需要的页表项变少，这对地
址转换后备缓冲器（TLB）的影响时正面的，可以减少其中来自内核的缓存页*/
#define X86_FEATURE_PSE 				(0*32+ 3)
/* Time Stamp Counter */
#define X86_FEATURE_TSC					(0*32+ 4)
/* Model-Specific Registers, RDMSR, WRMSR */
#define X86_FEATURE_MSR					(0*32+ 5)
/*PAE（Physical Address Extensions）模式。IA-32在启用PAE模式的情况下可以管理多达64G
内存，该特性为内存指针提供了额外的比特位。但并非所有64G内存都可以同时寻址，而是每次
只能寻址一个4G的内存段*/
#define X86_FEATURE_PAE					(0*32+ 6)
/* Machine Check Architecture */
#define X86_FEATURE_MCE					(0*32+ 7)
/* CMPXCHG8 instruction */
#define X86_FEATURE_CX8					(0*32+ 8)
/* Onboard APIC */
#define X86_FEATURE_APIC				(0*32+ 9)
/* SYSENTER/SYSEXIT */
#define X86_FEATURE_SEP					(0*32+11)
/* Memory Type Range Registers */
#define X86_FEATURE_MTRR				(0*32+12)
/*Page Global Enable。在上下文切换期间，设置了	__PAGE_GLOBAL为的页，对应的TLB缓存项
不从TLB中刷出*/
#define X86_FEATURE_PGE					(0*32+13)
/* Machine Check Architecture */
#define X86_FEATURE_MCA					(0*32+14)
/* CMOV instruction (FCMOVCC and FCOMI too if FPU present) */
#define X86_FEATURE_CMOV				(0*32+15)
/* Page Attribute Table */
#define X86_FEATURE_PAT					(0*32+16)
/* 36-bit PSEs */
#define X86_FEATURE_PSE36				(0*32+17)
/* Processor serial number */
#define X86_FEATURE_PN					(0*32+18)
/* Supports the CLFLUSH instruction */
#define X86_FEATURE_CLFLSH				(0*32+19)
/* Debug Store */
#define X86_FEATURE_DS					(0*32+21)
/* ACPI via MSR */
#define X86_FEATURE_ACPI				(0*32+22)
/* Multimedia Extensions */
#define X86_FEATURE_MMX					(0*32+23)
/* FXSAVE and FXRSTOR instructions (fast save and restore of FPU context)
, and CR4.OSFXSR available */
#define X86_FEATURE_FXSR				(0*32+24)
/* Streaming SIMD Extensions */
#define X86_FEATURE_XMM					(0*32+25)
/* Streaming SIMD Extensions-2 */
#define X86_FEATURE_XMM2				(0*32+26)
/* CPU self snoop */
#define X86_FEATURE_SELFSNOOP			(0*32+27)
/* Hyper-Threading */
#define X86_FEATURE_HT					(0*32+28)
/* Automatic clock control */
#define X86_FEATURE_ACC					(0*32+29)
/* IA-64 processor */
#define X86_FEATURE_IA64				(0*32+30)

/* AMD-defined CPU features, CPUID level 0x80000001, word 1 */
/* Don't duplicate feature flags which are redundant with Intel! */
/* SYSCALL/SYSRET */
#define X86_FEATURE_SYSCALL				(1*32+11)
/* MP Capable. */
#define X86_FEATURE_MP					(1*32+19)
/*用于将页标记为不可执行（在IA-32系统上，只有启用了可寻址64GB内存的页面地址扩展（PAE）
功能时，才能使用该保护位。例如，它可以防止执行栈页上的代码。否则，恶意代码可能通过缓
冲区溢出手段在栈上执行代码，导致程序的安全漏洞。NX位无法放置缓冲区溢出，但可以抑止其
效果*/
/* Execute Disable */
#define X86_FEATURE_NX					(1*32+20)
/* AMD MMX extensions */
#define X86_FEATURE_MMXEXT				(1*32+22)
/* RDTSCP */
#define X86_FEATURE_RDTSCP				(1*32+27)
/* Long Mode (x86-64) */
#define X86_FEATURE_LM					(1*32+29)
/* AMD 3DNow! extensions */
#define X86_FEATURE_3DNOWEXT			(1*32+30)
/* 3DNow! */
#define X86_FEATURE_3DNOW				(1*32+31)

/* Transmeta-defined CPU features, CPUID level 0x80860001, word 2 */
/* CPU in recovery mode */
#define X86_FEATURE_RECOVERY			(2*32+ 0)
/* Longrun power control */
#define X86_FEATURE_LONGRUN				(2*32+ 1)
/* LongRun table interface */
#define X86_FEATURE_LRTI				(2*32+ 3)

/* Other features, Linux-defined mapping, word 3 */
/* This range is used for feature bits which conflict or are synthesized */
/* Cyrix MMX extensions */
#define X86_FEATURE_CXMMX				(3*32+ 0)
/* AMD K6 nonstandard MTRRs */
#define X86_FEATURE_K6_MTRR				(3*32+ 1)
/* Cyrix ARRs (= MTRRs) */
#define X86_FEATURE_CYRIX_ARR			(3*32+ 2)
/* Centaur MCRs (= MTRRs) */
#define X86_FEATURE_CENTAUR_MCR			(3*32+ 3)
/* cpu types for specific tunings: */
/* Opteron, Athlon64 */
#define X86_FEATURE_K8					(3*32+ 4)
/* Athlon */
#define X86_FEATURE_K7					(3*32+ 5)
/* P3 */
#define X86_FEATURE_P3					(3*32+ 6)
/* P4 */
#define X86_FEATURE_P4					(3*32+ 7)
/* TSC ticks at a constant rate */
#define X86_FEATURE_CONSTANT_TSC 	(3*32+ 8)
/* smp kernel running on up */
#define X86_FEATURE_UP					(3*32+ 9)
/* FXSAVE leaks FOP/FIP/FOP */
#define X86_FEATURE_FXSAVE_LEAK 		(3*32+10)
/* Intel Architectural PerfMon */
#define X86_FEATURE_ARCH_PERFMON 	(3*32+11)
/* Precise-Event Based Sampling */
#define X86_FEATURE_PEBS				(3*32+12) 
/* Branch Trace Store */
#define X86_FEATURE_BTS					(3*32+13) 
/* 14 free */
/* RDTSC synchronizes the CPU */
#define X86_FEATURE_SYNC_RDTSC			(3*32+15) 
/* rep microcode works well on this CPU */
#define X86_FEATURE_REP_GOOD   			(3*32+16)

/* Intel-defined CPU features, CPUID level 0x00000001 (ecx), word 4 */
/* Streaming SIMD Extensions-3 */
#define X86_FEATURE_XMM3				(4*32+ 0)
/* Monitor/Mwait support */
#define X86_FEATURE_MWAIT				(4*32+ 3)
/* CPL Qualified Debug Store */
#define X86_FEATURE_DSCPL				(4*32+ 4)
/* Enhanced SpeedStep */
#define X86_FEATURE_EST					(4*32+ 7)
/* Thermal Monitor 2 */
#define X86_FEATURE_TM2					(4*32+ 8)
/* Context ID */
#define X86_FEATURE_CID					(4*32+10)
/* CMPXCHG16B */
#define X86_FEATURE_CX16   				(4*32+13)
/* Send Task Priority Messages */
#define X86_FEATURE_XTPR				(4*32+14)
/* Direct Cache Access */
#define X86_FEATURE_DCA					(4*32+18)

/* VIA/Cyrix/Centaur-defined CPU features, CPUID level 0xC0000001, word 5 */
/* on-CPU RNG present (xstore insn) */
#define X86_FEATURE_XSTORE				(5*32+ 2)
/* on-CPU RNG enabled */
#define X86_FEATURE_XSTORE_EN			(5*32+ 3)
/* on-CPU crypto (xcrypt insn) */
#define X86_FEATURE_XCRYPT				(5*32+ 6)
/* on-CPU crypto enabled */
#define X86_FEATURE_XCRYPT_EN			(5*32+ 7)
/* Advanced Cryptography Engine v2 */
#define X86_FEATURE_ACE2				(5*32+ 8)
/* ACE v2 enabled */
#define X86_FEATURE_ACE2_EN				(5*32+ 9)
/* PadLock Hash Engine */
#define X86_FEATURE_PHE					(5*32+ 10)
/* PHE enabled */
#define X86_FEATURE_PHE_EN				(5*32+ 11)
/* PadLock Montgomery Multiplier */
#define X86_FEATURE_PMM					(5*32+ 12)
/* PMM enabled */
#define X86_FEATURE_PMM_EN				(5*32+ 13)

/* More extended AMD flags: CPUID level 0x80000001, ecx, word 6 */
/* LAHF/SAHF in long mode */
#define X86_FEATURE_LAHF_LM				(6*32+ 0)
/* If yes HyperThreading not valid */
#define X86_FEATURE_CMP_LEGACY			(6*32+ 1)

/*
 * Auxiliary flags: Linux defined - For features scattered in various
 * CPUID levels like 0x6, 0xA etc
 */
/* Intel Dynamic Acceleration */
#define X86_FEATURE_IDA					(7*32+ 0)

#define cpu_has(c, bit)																	\
	(__builtin_constant_p(bit) &&														\
		(																				\
			(((bit)>>5)==0 && (1UL<<((bit)&31) & REQUIRED_MASK0)) ||					\
			(((bit)>>5)==1 && (1UL<<((bit)&31) & REQUIRED_MASK1)) ||					\
			(((bit)>>5)==2 && (1UL<<((bit)&31) & REQUIRED_MASK2)) ||					\
			(((bit)>>5)==3 && (1UL<<((bit)&31) & REQUIRED_MASK3)) ||					\
			(((bit)>>5)==4 && (1UL<<((bit)&31) & REQUIRED_MASK4)) ||					\
			(((bit)>>5)==5 && (1UL<<((bit)&31) & REQUIRED_MASK5)) ||					\
			(((bit)>>5)==6 && (1UL<<((bit)&31) & REQUIRED_MASK6)) ||					\
			(((bit)>>5)==7 && (1UL<<((bit)&31) & REQUIRED_MASK7)) 						\
		) ? 1 : test_bit(bit, (c)->x86_capability)										\
	)
#define boot_cpu_has(bit)	cpu_has(&boot_cpu_data, bit)

#define cpu_has_fpu					boot_cpu_has(X86_FEATURE_FPU)
#define cpu_has_vme					boot_cpu_has(X86_FEATURE_VME)
#define cpu_has_de					boot_cpu_has(X86_FEATURE_DE)
#define cpu_has_pse					boot_cpu_has(X86_FEATURE_PSE)
#define cpu_has_tsc					boot_cpu_has(X86_FEATURE_TSC)
#define cpu_has_pae					boot_cpu_has(X86_FEATURE_PAE)
#define cpu_has_pge					boot_cpu_has(X86_FEATURE_PGE)
#define cpu_has_apic				boot_cpu_has(X86_FEATURE_APIC)
#define cpu_has_sep					boot_cpu_has(X86_FEATURE_SEP)
#define cpu_has_mtrr				boot_cpu_has(X86_FEATURE_MTRR)
#define cpu_has_mmx					boot_cpu_has(X86_FEATURE_MMX)
#define cpu_has_fxsr				boot_cpu_has(X86_FEATURE_FXSR)
#define cpu_has_xmm					boot_cpu_has(X86_FEATURE_XMM)
#define cpu_has_xmm2				boot_cpu_has(X86_FEATURE_XMM2)
#define cpu_has_xmm3				boot_cpu_has(X86_FEATURE_XMM3)
#define cpu_has_ht					boot_cpu_has(X86_FEATURE_HT)
#define cpu_has_mp					boot_cpu_has(X86_FEATURE_MP)
#define cpu_has_nx					boot_cpu_has(X86_FEATURE_NX)
#define cpu_has_k6_mtrr				boot_cpu_has(X86_FEATURE_K6_MTRR)
#define cpu_has_cyrix_arr			boot_cpu_has(X86_FEATURE_CYRIX_ARR)
#define cpu_has_centaur_mcr			boot_cpu_has(X86_FEATURE_CENTAUR_MCR)
#define cpu_has_xstore				boot_cpu_has(X86_FEATURE_XSTORE)
#define cpu_has_xstore_enabled		boot_cpu_has(X86_FEATURE_XSTORE_EN)
#define cpu_has_xcrypt				boot_cpu_has(X86_FEATURE_XCRYPT)
#define cpu_has_xcrypt_enabled		boot_cpu_has(X86_FEATURE_XCRYPT_EN)
#define cpu_has_ace2				boot_cpu_has(X86_FEATURE_ACE2)
#define cpu_has_ace2_enabled		boot_cpu_has(X86_FEATURE_ACE2_EN)
#define cpu_has_phe					boot_cpu_has(X86_FEATURE_PHE)
#define cpu_has_phe_enabled			boot_cpu_has(X86_FEATURE_PHE_EN)
#define cpu_has_pmm					boot_cpu_has(X86_FEATURE_PMM)
#define cpu_has_pmm_enabled			boot_cpu_has(X86_FEATURE_PMM_EN)
#define cpu_has_ds					boot_cpu_has(X86_FEATURE_DS)
#define cpu_has_pebs 				boot_cpu_has(X86_FEATURE_PEBS)
#define cpu_has_clflush				boot_cpu_has(X86_FEATURE_CLFLSH)
#define cpu_has_bts 				boot_cpu_has(X86_FEATURE_BTS)

#endif /* __ASM_I386_CPUFEATURE_H */
