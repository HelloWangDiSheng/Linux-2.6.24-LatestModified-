#ifndef _I386_PGTABLE_2LEVEL_DEFS_H
#define _I386_PGTABLE_2LEVEL_DEFS_H

#define SHARED_KERNEL_PMD	0

/*��ͳi386����ҳ�ṹPGD��10��+ PTE��10��+ OFFSET(12)*/
#define PGDIR_SHIFT	22
#define PTRS_PER_PGD	1024

/*i386����ҳ�ṹ��û��PMD����Ŀ¼*/
#define PTRS_PER_PTE	1024

#endif /* _I386_PGTABLE_2LEVEL_DEFS_H */
