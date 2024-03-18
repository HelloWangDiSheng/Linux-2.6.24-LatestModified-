#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H
#include <asm/types.h>

#ifdef	__KERNEL__
/*nr字长范围内的权重*/
#define BIT(nr)			(1UL << (nr))
/*字长范围内的有效权重*/
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
/*需要的字长数目*/
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
/*不小于nr的字长的最低倍数*/
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_LONG)
/*字节包含的位数目*/
#define BITS_PER_BYTE		8
#endif

/*有些体系结构需要通用的查找第一/最后一个置位的比特位位置*/
#include <asm/bitops.h>

#define for_each_bit(bit, addr, size) \
	for ((bit) = find_first_bit((addr), (size)); \
	     (bit) < (size); \
	     (bit) = find_next_bit((addr), (size), (bit) + 1))


static __inline__ int get_bitmask_order(unsigned int count)
{
	int order;
	
	order = fls(count);
	return order;
}

static __inline__ int get_count_order(unsigned int count)
{
	int order;
	
	order = fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}

/*计算数字的二进制表达式中置位的数目*/
static inline unsigned long hweight_long(unsigned long w)
{
	return sizeof(w) == 4 ? hweight32(w) : hweight64(w);
}

/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u32 rol32(__u32 word, unsigned int shift)
{
	return (word << shift) | (word >> (32 - shift));
}

/**
 * ror32 - rotate a 32-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u32 ror32(__u32 word, unsigned int shift)
{
	return (word >> shift) | (word << (32 - shift));
}

/*查询输入数字的二进制表达式中，最后置位的位置*/
static inline unsigned fls_long(unsigned long l)
{
	if (sizeof(l) == 4)
		return fls(l);
	return fls64(l);
}

#endif
