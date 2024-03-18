#ifndef _ASM_GENERIC_PAGE_H
#define _ASM_GENERIC_PAGE_H

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#include <linux/compiler.h>

/*����size��Ӧ�ķ���ף����붨��Ϊ��������*/
static __inline__ __attribute_const__ int get_order(unsigned long size)
{
	int order;
	/*����ں�ջ����sizeΪ8k��ҳ���СΪ4k��size-1���ǵ�13λȫΪ1������11λ���ǵ�2λ
	ȫΪ1����size=3��ѭ��2�Σ�order����2�α�Ϊ1*/
	size = (size - 1) >> (PAGE_SHIFT - 1);
	/*ҳ����������Ϊ-1*/
	order = -1;
	/*do-whileѭ���м������׵�����������ѭ������ΪԤ�����С��1��ȥ����λ
	��PAGE_SHIFT-1����1��ʣ��1��λ����Ϊѭ����Ŀ�����ٱ�֤�����ѭ������һ��*/
	do
	{
		size >>= 1;
		order++;
	} while (size);

	return order;
}

#endif	/* __ASSEMBLY__ */
#endif	/* __KERNEL__ */

#endif	/* _ASM_GENERIC_PAGE_H */
