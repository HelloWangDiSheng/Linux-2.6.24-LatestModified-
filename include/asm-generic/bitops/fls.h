#ifndef _ASM_GENERIC_BITOPS_FLS_H_
#define _ASM_GENERIC_BITOPS_FLS_H_

/*查询大端序表示法（高低地高：高位低地址，低位高地址，如0x12345678的存储地址开始存放
0x12345678）的数字二进制表达式中最后置位的位置。fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32*/
static inline int fls(int x)
{
	/**/
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) 
	{
		/*高16位全为0，左移16位，比较低16位*/
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u))
	{
		/*高8位全为0，左移8位*/
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u))
	{
		/*高4位全为0*/
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u))
	{
		/*高2位全为0，左移2位*/
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u))
	{
		/*最高位为0，左移1位*/
		x <<= 1;
		r -= 1;
	}
	return r;
}

#endif /* _ASM_GENERIC_BITOPS_FLS_H_ */
