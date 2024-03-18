#ifndef _ASM_GENERIC_BITOPS_FLS64_H_
#define _ASM_GENERIC_BITOPS_FLS64_H_

#include <asm/types.h>
/*查找字长为64位数的二进制表达式中，查询最后置位的位置，分解为2个32位数处理*/
static inline int fls64(__u64 x)
{
	__u32 h = x >> 32;
	if (h)
		return fls(h) + 32;
	return fls(x);
}

#endif /* _ASM_GENERIC_BITOPS_FLS64_H_ */
