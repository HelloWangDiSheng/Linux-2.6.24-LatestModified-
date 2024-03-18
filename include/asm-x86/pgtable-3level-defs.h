#ifndef _I386_PGTABLE_3LEVEL_DEFS_H
#define _I386_PGTABLE_3LEVEL_DEFS_H

#ifdef CONFIG_PARAVIRT
#define SHARED_KERNEL_PMD	(pv_info.shared_kernel_pmd)
#else
#define SHARED_KERNEL_PMD	1
#endif

/*3级页结构中PGD（2）+ PMD（9）+ PTE（9）+OFFSET（12）*/

/*PGDIR_SHIFT决定顶层可以映射的页表项*/
#define PGDIR_SHIFT	30
#define PTRS_PER_PGD	4

/*PMD_SHIFT决定可以映射的中间层页表的区域长度*/
#define PMD_SHIFT	21
#define PTRS_PER_PMD	512

/*页目录层数目*/
#define PTRS_PER_PTE	512

#endif /* _I386_PGTABLE_3LEVEL_DEFS_H */
