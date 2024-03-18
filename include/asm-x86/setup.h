#ifndef _ASM_X86_SETUP_H
#define _ASM_X86_SETUP_H

/*�����г�������ַ���Ŀ����Ϊ2KB*/
#define COMMAND_LINE_SIZE 2048

#ifdef __KERNEL__

#ifdef __i386__

#include <linux/pfn.h>
/*
 * Reserved space for vmalloc and iomap - defined in asm/page.h
 */
/*Ϊvmalloc��iomapԤ���Ŀռ�*/
#define MAXMEM_PFN	PFN_DOWN(MAXMEM)
/*û������PAE����ʱ�ں��ܹ�������ҳ��ĿΪ1M��Ҳ���Ƕ�Ӧ4G�ڴ档4GB��32λϵͳ�Ͽ���
Ѱַ������ڴ棨1<<32 = 4GB�������ʹ��һ�㼼�ɣ����ڵ�IA-32ʵ�֣�Pentium PRO�����
�汾��������PAE��Page Address Extension��ģʽ�¿��Թ�����64G�ڴ档������Ϊ�ڴ�ָ��
�ṩ�˶���ı���λ������������64G������ͬʱѰַ������ÿ��ֻ��Ѱַһ��4G���ڴ��*/
#define MAX_NONPAE_PFN	(1 << 20)

#endif /* __i386__ */

/*����������ռ�ռ䳤�ȡ�sizeof(struct boot_params)*/
#define PARAM_SIZE 			4096

#define OLD_CL_MAGIC		0xA33F
#define OLD_CL_ADDRESS		0x020	/* Relative to real mode data */
#define NEW_CL_POINTER		0x228	/* Relative to real mode data */

#ifndef __ASSEMBLY__
#include <asm/bootparam.h>

#ifndef _SETUP


/*�����ڼ���setup��������ʹ��*/
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
