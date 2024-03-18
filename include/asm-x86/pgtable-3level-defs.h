#ifndef _I386_PGTABLE_3LEVEL_DEFS_H
#define _I386_PGTABLE_3LEVEL_DEFS_H

#ifdef CONFIG_PARAVIRT
#define SHARED_KERNEL_PMD	(pv_info.shared_kernel_pmd)
#else
#define SHARED_KERNEL_PMD	1
#endif

/*3��ҳ�ṹ��PGD��2��+ PMD��9��+ PTE��9��+OFFSET��12��*/

/*PGDIR_SHIFT�����������ӳ���ҳ����*/
#define PGDIR_SHIFT	30
#define PTRS_PER_PGD	4

/*PMD_SHIFT��������ӳ����м��ҳ������򳤶�*/
#define PMD_SHIFT	21
#define PTRS_PER_PMD	512

/*ҳĿ¼����Ŀ*/
#define PTRS_PER_PTE	512

#endif /* _I386_PGTABLE_3LEVEL_DEFS_H */
