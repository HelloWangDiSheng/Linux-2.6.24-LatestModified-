#ifndef __LINUX_BITMAP_H
#define __LINUX_BITMAP_H

#ifndef __ASSEMBLY__

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/string.h>
#include <linux/kernel.h>

/*   比特位图可以认为是一个或多个无符号长整形组成的比特数组，位图接口和可用的操作都列示在该
文件中。对所有体系结构的通用函数实现在lib/bitmap.c文件中，特定体系结构的函数实现在独自的体系
结构文件中，查看lib/bitmap.c获取更多信息*/
/*可用的比特位操作大体上都先将比特位图当做一个或多个无符号整形来处理，然后再将余下的通过位处理
，需要注意的是有效比特位的长度在编译时等价与常数，否则许多内联函数将产生可怕的代码*/

/*
 * bitmap_zero(dst, nbits)			*dst = 0UL
 * bitmap_fill(dst, nbits)			*dst = ~0UL
 * bitmap_copy(dst, src, nbits)			*dst = *src
 * bitmap_and(dst, src1, src2, nbits)		*dst = *src1 & *src2
 * bitmap_or(dst, src1, src2, nbits)		*dst = *src1 | *src2
 * bitmap_xor(dst, src1, src2, nbits)		*dst = *src1 ^ *src2
 * bitmap_andnot(dst, src1, src2, nbits)	*dst = *src1 & ~(*src2)
 * bitmap_complement(dst, src, nbits)		*dst = ~(*src)
 * bitmap_equal(src1, src2, nbits)		Are *src1 and *src2 equal?
 * bitmap_intersects(src1, src2, nbits) 	Do *src1 and *src2 overlap?
 * bitmap_subset(src1, src2, nbits)		Is *src1 a subset of *src2?
 * bitmap_empty(src, nbits)			Are all bits zero in *src?
 * bitmap_full(src, nbits)			Are all bits set in *src?
 * bitmap_weight(src, nbits)			Hamming Weight: number set bits
 * bitmap_shift_right(dst, src, n, nbits)	*dst = *src >> n
 * bitmap_shift_left(dst, src, n, nbits)	*dst = *src << n
 * bitmap_remap(dst, src, old, new, nbits)	*dst = map(old, new)(src)
 * bitmap_bitremap(oldbit, old, new, nbits)	newbit = map(old, new)(oldbit)
 * bitmap_scnprintf(buf, len, src, nbits)	Print bitmap src to buf
 * bitmap_parse(buf, buflen, dst, nbits)	Parse bitmap dst from kernel buf
 * bitmap_parse_user(ubuf, ulen, dst, nbits)	Parse bitmap dst from user buf
 * bitmap_scnlistprintf(buf, len, src, nbits)	Print bitmap src as list to buf
 * bitmap_parselist(buf, dst, nbits)		Parse bitmap dst from list
 * bitmap_find_free_region(bitmap, bits, order)	Find and allocate bit region
 * bitmap_release_region(bitmap, pos, order)	Free specified bit region
 * bitmap_allocate_region(bitmap, pos, order)	Allocate specified bit region
 */

/*
 * Also the following operations in asm/bitops.h apply to bitmaps.
 *
 * set_bit(bit, addr)			*addr |= bit
 * clear_bit(bit, addr)			*addr &= ~bit
 * change_bit(bit, addr)		*addr ^= bit
 * test_bit(bit, addr)			Is bit set in *addr?
 * test_and_set_bit(bit, addr)		Set bit and return old value
 * test_and_clear_bit(bit, addr)	Clear bit and return old value
 * test_and_change_bit(bit, addr)	Change bit and return old value
 * find_first_zero_bit(addr, nbits)	Position first zero bit in *addr
 * find_first_bit(addr, nbits)		Position first set bit in *addr
 * find_next_zero_bit(addr, nbits, bit)	Position next zero bit in *addr >= bit
 * find_next_bit(addr, nbits, bit)	Position next set bit in *addr >= bit
 */

/*lib/bitmap.c文件中提供下列这些函数的实现*/
extern int __bitmap_empty(const unsigned long *bitmap, int bits);
extern int __bitmap_full(const unsigned long *bitmap, int bits);
extern int __bitmap_equal(const unsigned long *bitmap1,
                	const unsigned long *bitmap2, int bits);
extern void __bitmap_complement(unsigned long *dst, const unsigned long *src,
			int bits);
extern void __bitmap_shift_right(unsigned long *dst,
                        const unsigned long *src, int shift, int bits);
extern void __bitmap_shift_left(unsigned long *dst,
                        const unsigned long *src, int shift, int bits);
extern void __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern void __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_intersects(const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_subset(const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_weight(const unsigned long *bitmap, int bits);

extern int bitmap_scnprintf(char *buf, unsigned int len,
			const unsigned long *src, int nbits);
extern int __bitmap_parse(const char *buf, unsigned int buflen, int is_user,
			unsigned long *dst, int nbits);
extern int bitmap_parse_user(const char __user *ubuf, unsigned int ulen,
			unsigned long *dst, int nbits);
extern int bitmap_scnlistprintf(char *buf, unsigned int len,
			const unsigned long *src, int nbits);
extern int bitmap_parselist(const char *buf, unsigned long *maskp,
			int nmaskbits);
extern void bitmap_remap(unsigned long *dst, const unsigned long *src,
		const unsigned long *old, const unsigned long *new, int bits);
extern int bitmap_bitremap(int oldbit,
		const unsigned long *old, const unsigned long *new, int bits);
extern int bitmap_find_free_region(unsigned long *bitmap, int bits, int order);
extern void bitmap_release_region(unsigned long *bitmap, int pos, int order);
extern int bitmap_allocate_region(unsigned long *bitmap, int pos, int order);

/*将位图中最后字长中的有效位全部置位*/
#define BITMAP_LAST_WORD_MASK(nbits)		\
			(((nbits) % BITS_PER_LONG) ? (1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL)
/*将位图中所有位清零，位图长度小于字长时，按无符号长整形变量处理，否则，将位图前部分当做无符号
长整形数组项处理，后部分位处理*/
static inline void bitmap_zero(unsigned long *dst, int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*位图有效长度小于一个字长时，按无符号长整形变量处理*/
		*dst = 0UL;
	}
	else 
	{
		int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memset(dst, 0, len);
	}
}
/*将位图中所有位全部置位，前部分按字节处理，后部分按位处理*/
static inline void bitmap_fill(unsigned long *dst, int nbits)
{
	/*计算位图所需的unsigned long类型变量的数目*/
	size_t nlongs = BITS_TO_LONGS(nbits);
	if (nlongs > 1) 
	{
		/*将前部分按字节全部置位*/
		int len = (nlongs - 1) * sizeof(unsigned long);
		memset(dst, 0xff,  len);
	}
	/*后部分有效位按位操作全部置位*/
	dst[nlongs - 1] = BITMAP_LAST_WORD_MASK(nbits);
}
/*复制指定数目位的位图。将dst赋值为src中前nbits位值*/
static inline void bitmap_copy(unsigned long *dst, const unsigned long *src,
			int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*按无符号长整形变量处理*/
		*dst = *src;	
	}
	else 
	{
		/*计算所需字长数目后复制内存*/
		int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memcpy(dst, src, len);
	}
}
/*计算两个位图的交集*/
static inline void bitmap_and(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*位图有效长度在字长范围内直接按无符号长整形变量按位与*/
		*dst = *src1 & *src2;	
	}
	else/*计算所需字长数目，然后按无符号长整形数组处理，计算每对数据按位与*/
		__bitmap_and(dst, src1, src2, nbits);
}
/*计算两位图的并集*/
static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*位图长度在一个字长范围内的直接无符号长整形变量按位或*/
		*dst = *src1 | *src2;		
	}
	else/**/
		__bitmap_or(dst, src1, src2, nbits);
}
			
/*计算两个位图的异或，先计算需要处理位数目所需的字长数目，在按无符号长整形变量按位异或*/
static inline void bitmap_xor(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		*dst = *src1 ^ *src2;
	else
		__bitmap_xor(dst, src1, src2, nbits);
}
			
/*计算两个位图的与非，先计算需要处理位数目所需的字长数目，在按无符号长整形变量按位与非*/
static inline void bitmap_andnot(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		*dst = *src1 & ~(*src2);
	else
		__bitmap_andnot(dst, src1, src2, nbits);
}
/*计算一个位图相对于另一个位图的补集（对有效位按位取反）*/
static inline void bitmap_complement(unsigned long *dst, const unsigned long *src,
			int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*对有效位按位取反求补集*/
		*dst = ~(*src) & BITMAP_LAST_WORD_MASK(nbits);
	}
	else
		__bitmap_complement(dst, src, nbits);
}
			
/*判断指定长度的两位图是否相同，先对有效位按位异或后在取反*/
static inline int bitmap_equal(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*指定长度小于字长，按无符号长整形变量按位异或（同0异1），然后对结果的有效位取反*/
		return ! ((*src1 ^ *src2) & BITMAP_LAST_WORD_MASK(nbits));
	}
	else
		return __bitmap_equal(src1, src2, nbits);
}

/*测试两位图（从起始位开始）的指定长度中是否有交集，即有效位按位与后的结果是否为零*/
static inline int bitmap_intersects(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		return ((*src1 & *src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	else
		return __bitmap_intersects(src1, src2, nbits);
}
			
/*判断位图是否是另一个位图的子集，将位图与另一位图的补集的按位与，有效位全为0则返回true，否则false*/
static inline int bitmap_subset(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= BITS_PER_LONG)
	{
		/*计算src2的补集与src1按位与，然后判断有效位是否全为0，是则返回true，否则返回false*/
		return ! ((*src1 & ~(*src2)) & BITMAP_LAST_WORD_MASK(nbits));
	}
	else
		return __bitmap_subset(src1, src2, nbits);
}
			
/*判断位图中指定数目位是否全为0*/
static inline int bitmap_empty(const unsigned long *src, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		return ! (*src & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_empty(src, nbits);
}

/*判断位图中指定数目的位是否全为1*/
static inline int bitmap_full(const unsigned long *src, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		return ! (~(*src) & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_full(src, nbits);
}

/*计算位图中指定数目的位中，置位的数目*/
static inline int bitmap_weight(const unsigned long *src, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		return hweight_long(*src & BITMAP_LAST_WORD_MASK(nbits));
	return __bitmap_weight(src, nbits);
}

/**/
static inline void bitmap_shift_right(unsigned long *dst,
			const unsigned long *src, int n, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		*dst = *src >> n;
	else
		__bitmap_shift_right(dst, src, n, nbits);
}

/**/
static inline void bitmap_shift_left(unsigned long *dst,
			const unsigned long *src, int n, int nbits)
{
	if (nbits <= BITS_PER_LONG)
		*dst = (*src << n) & BITMAP_LAST_WORD_MASK(nbits);
	else
		__bitmap_shift_left(dst, src, n, nbits);
}
			
/**/
static inline int bitmap_parse(const char *buf, unsigned int buflen,
			unsigned long *maskp, int nmaskbits)
{
	return __bitmap_parse(buf, buflen, 0, maskp, nmaskbits);
}

#endif /* __ASSEMBLY__ */

#endif /* __LINUX_BITMAP_H */
