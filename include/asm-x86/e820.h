#ifndef __ASM_E820_H
#define __ASM_E820_H

/*our map*/
#define E820MAP				0x2d0
/*number of entries in E820MAP*/
#define E820MAX				128
/* # entries in E820MAP */
#define E820NR				0x1e8

/*ϵͳ�����ڴ�*/
#define E820_RAM			1
/*ϵͳ�����ڴ棬����ROM*/
#define E820_RESERVED		2
/*ACPI Reclaim�ڴ�*/
#define E820_ACPI			3
/*ACPI NVS�ڴ�*/
#define E820_NVS			4

#ifndef __ASSEMBLY__
/*E820ÿ��ʵ��ռ20�ֽڣ�8�ֽڻ���ַ + 8�ֽڳ��� + 4�ֽڱ��ڴ�����*/
struct e820entry
{
	/*8�ֽڻ���ַ*/
	__u64 addr;
	/*�ڴ泤��*/
	__u64 size;
	/*�ڴ�����*/
	__u32 type;
} __attribute__((packed));

/*E820ӳ��ṹ����*/
struct e820map
{
	__u32 nr_map;
	struct e820entry map[E820MAX];
};
#endif /* __ASSEMBLY__ */

#ifdef __KERNEL__
#ifdef CONFIG_X86_32
# include "e820_32.h"
#else
# include "e820_64.h"
#endif
#endif /* __KERNEL__ */

#endif  /* __ASM_E820_H */
