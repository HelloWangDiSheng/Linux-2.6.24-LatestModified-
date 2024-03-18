#ifndef _ASM_GENERIC_PAGE_H
#define _ASM_GENERIC_PAGE_H

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#include <linux/compiler.h>

/*返回size对应的分配阶，必须定义为内联函数*/
static __inline__ __attribute_const__ int get_order(unsigned long size)
{
	int order;
	/*如果内核栈长度size为8k，页面大小为4k，size-1就是低13位全为1，右移11位就是低2位
	全为1，则size=3，循环2次，order自增2次变为1*/
	size = (size - 1) >> (PAGE_SHIFT - 1);
	/*页面分配阶设置为-1*/
	order = -1;
	/*do-while循环中计算分配阶的自增次数，循环次数为预分配大小减1后去掉低位
	（PAGE_SHIFT-1）个1，剩余1的位数即为循环数目，至少保证分配阶循环自增一次*/
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
