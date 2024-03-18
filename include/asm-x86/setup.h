#ifndef _ASM_X86_SETUP_H
#define _ASM_X86_SETUP_H

/*命令行长度最大字符数目限制为2KB*/
#define COMMAND_LINE_SIZE 2048

#ifdef __KERNEL__

#ifdef __i386__

#include <linux/pfn.h>
/*
 * Reserved space for vmalloc and iomap - defined in asm/page.h
 */
/*为vmalloc和iomap预留的空间*/
#define MAXMEM_PFN	PFN_DOWN(MAXMEM)
/*没有启用PAE功能时内核能管理最大的页数目为1M，也就是对应4G内存。4GB是32位系统上可以
寻址的最大内存（1<<32 = 4GB）。如果使用一点技巧，现在的IA-32实现（Pentium PRO或更高
版本）在启用PAE（Page Address Extension）模式下可以管理多达64G内存。该特性为内存指针
提供了额外的比特位。但并非所有64G都可以同时寻址，而是每次只能寻址一个4G的内存段*/
#define MAX_NONPAE_PFN	(1 << 20)

#endif /* __i386__ */

/*启动参数所占空间长度。sizeof(struct boot_params)*/
#define PARAM_SIZE 			4096

#define OLD_CL_MAGIC		0xA33F
#define OLD_CL_ADDRESS		0x020	/* Relative to real mode data */
#define NEW_CL_POINTER		0x228	/* Relative to real mode data */

#ifndef __ASSEMBLY__
#include <asm/bootparam.h>

#ifndef _SETUP


/*启动期间由setup函数创建使用*/
extern struct boot_params boot_params;

#ifdef __i386__
/*
 * Do NOT EVER look at the BIOS memory size location.
 * It does not work on many machines.
 */
#define LOWMEMSIZE()	(0x9f000)

struct e820entry;

char * __init machine_specific_memory_setup(void);
char *memory_setup(void);

int __init copy_e820_map(struct e820entry * biosmap, int nr_map);
int __init sanitize_e820_map(struct e820entry * biosmap, char * pnr_map);
void __init add_memory_region(unsigned long long start, unsigned long long size, int type);

extern unsigned long init_pg_tables_end;

#ifndef CONFIG_PARAVIRT
#define paravirt_post_allocator_init()	do {} while (0)
#endif

#endif /* __i386__ */
#endif /* _SETUP */
#endif /* __ASSEMBLY__ */
#endif  /*  __KERNEL__  */

#endif /* _ASM_X86_SETUP_H */
