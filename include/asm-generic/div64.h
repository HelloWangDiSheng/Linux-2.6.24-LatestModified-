#ifndef _ASM_GENERIC_DIV64_H
#define _ASM_GENERIC_DIV64_H
/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 * Based on former asm-ppc/div64.h and asm-m68knommu/div64.h
 *
 * The semantics of do_div() are:
 *
 * uint32_t do_div(uint64_t *n, uint32_t base)
 * {
 * 	uint32_t remainder = *n % base;
 * 	*n = *n / base;
 * 	return remainder;
 * }
 *
 * NOTE: macro parameter n is evaluated multiple times,
 *       beware of side effects!
 */

#include <linux/types.h>
#include <linux/compiler.h>

#if BITS_PER_LONG == 64

# define do_div(n,base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
 })

static inline uint64_t div64_64(uint64_t dividend, uint64_t divisor)
{
	return dividend / divisor;
}

#elif BITS_PER_LONG == 32

extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
/*此处非必要的指针比较是为了检查类型的有效性（n必须是64位数值），uint64_t变量n与
uint32_t变量base相除后将商保存在n中*/
#define do_div(n,base)											\
({																\
	uint32_t __base = (base);									\
	uint32_t __rem;												\
	/*检查n类型，确保是uint64_t类型*/										\
	(void)(((typeof((n)) *)0) == ((uint64_t *)0));				\
	/*如果uint64_t类型被除数的最高32位是0，简化求两者相除后的商和余数*/\
	if (likely(((n) >> 32) == 0))								\
	{															\
		/*最高位32位是0，利用强制类型转换，直接截断最高32位，保留有效的低32位，计算并保存两者相除后的余数*/\
		__rem = (uint32_t)(n) % __base;							\
		/*保存商*/													\
		(n) = (uint32_t)(n) / __base;							\
	}															\
	else								 						\
		/*保存高32位非0的uint64_t类型被除数与uint32_t除数相除后的余数*/\
		__rem = __div64_32(&(n), __base);						\
	__rem;														\
 })

extern uint64_t div64_64(uint64_t dividend, uint64_t divisor);

#else /* BITS_PER_LONG == ?? */

# error do_div() does not yet support the C64

#endif /* BITS_PER_LONG */

#endif /* _ASM_GENERIC_DIV64_H */
