#ifndef _LINUX_HASH_H
#define _LINUX_HASH_H
/*一个长整形的快速散列函数*/

/*简单一点的hash算法就是直接取模，高级一点的hash算法会乘以一个大素数来将源数据的分布
 变得不那么显著，也就是说，不管输入的是一串分布很紧凑的数字序列还是很稀疏的数字序列，
 生成的结果都是很分散的数字序列，linux内核里面的这个hash要更高级一点，包含两个特征，
 最接近32位或者64位上限黄金分割点的素数，只要最小的移位和求和就能生成，前者确保即便是
 1、2这样的小数也会被乘溢出，被轻易打散，后者确保任何一个整数乘这个大素数的时候都可以
 通过最少的移位和求和就可以得到乘积，不用真的相乘*/
/*设置黄金分割比素数*/
#if BITS_PER_LONG == 32
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME 0x9e370001UL
#elif BITS_PER_LONG == 64
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME 0x9e37fffffffc0001UL
#else
#error Define GOLDEN_RATIO_PRIME for your wordsize.
#endif

/*获取一个数字在指定散列偏移内的索引*/
static inline unsigned long hash_long(unsigned long val, unsigned int bits)
{
	unsigned long hash = val;

#if BITS_PER_LONG == 64
	/*  Sigh, gcc can't optimise this alone like it does for 32 bits. */
	unsigned long n = hash;
	n <<= 18;
	hash -= n;
	n <<= 33;
	hash -= n;
	n <<= 3;
	hash += n;
	n <<= 3;
	hash -= n;
	n <<= 4;
	hash += n;
	n <<= 2;
	hash += n;
#else
	/* On some cpus multiply is faster, on others gcc will do shifts */
	hash *= GOLDEN_RATIO_PRIME;
#endif

	/* High bits are more random, so use them. */
	return hash >> (BITS_PER_LONG - bits);
}

/*获取一个地址在指定散列偏移[0, (1<<bits)-1]内的索引*/
static inline unsigned long hash_ptr(void *ptr, unsigned int bits)
{
	return hash_long((unsigned long)ptr, bits);
}
#endif /* _LINUX_HASH_H */
