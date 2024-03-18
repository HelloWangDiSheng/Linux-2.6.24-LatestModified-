#ifndef _ASM_GENERIC_BITOPS_SCHED_H_
#define _ASM_GENERIC_BITOPS_SCHED_H_

#include <linux/compiler.h>	/* unlikely() */
#include <asm/types.h>

/*每种体系结构都必须定义该函数，它是查询100位位图的最快方法，将位图当做无符号长整形
数组处理，只处理32位或64位字长类型，其它类型未定义*/
static inline int sched_find_first_bit(const unsigned long *b)
{
#if BITS_PER_LONG == 64
	if (b[0])
		return __ffs(b[0]);
	return __ffs(b[1]) + 64;
#elif BITS_PER_LONG == 32
	if (b[0])
		return __ffs(b[0]);
	if (b[1])
		return __ffs(b[1]) + 32;
	if (b[2])
		return __ffs(b[2]) + 64;
	return __ffs(b[3]) + 96;
#else
#error BITS_PER_LONG not defined
#endif
}

#endif /* _ASM_GENERIC_BITOPS_SCHED_H_ */
