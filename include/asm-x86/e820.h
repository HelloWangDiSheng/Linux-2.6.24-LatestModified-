#ifndef __ASM_E820_H
#define __ASM_E820_H

/*our map*/
#define E820MAP				0x2d0
/*number of entries in E820MAP*/
#define E820MAX				128
/* # entries in E820MAP */
#define E820NR				0x1e8

/*系统可用内存*/
#define E820_RAM			1
/*系统保留内存，例如ROM*/
#define E820_RESERVED		2
/*ACPI Reclaim内存*/
#define E820_ACPI			3
/*ACPI NVS内存*/
#define E820_NVS			4

#ifndef __ASSEMBLY__
/*E820每个实体占20字节，8字节基地址 + 8字节长度 + 4字节被内存类型*/
struct e820entry
{
	/*8字节基地址*/
	__u64 addr;
	/*内存长度*/
	__u64 size;
	/*内存类型*/
	__u32 type;
} __attribute__((packed));

/*E820映射结构定义*/
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
