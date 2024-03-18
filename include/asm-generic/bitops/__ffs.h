#ifndef _ASM_GENERIC_BITOPS___FFS_H_
#define _ASM_GENERIC_BITOPS___FFS_H_

#include <asm/types.h>

/*查找数字中第一个置位的比特位索引，需要先检查输入参数是否非零，然后先按一半字长所占
的比特位数目作为比较长度，测试低位是否为0，结果为0时，循环右移比较长度的一半，并更新
已比较的索引，直至比较长度为1时结束比较，返回索引。如IA-32 __ffs(23)=__ffs(0x17)=
__ffs(10111)=0,__ffs(24)=__ffs(0x18)=__ffs(11000)=3*/
static inline unsigned long __ffs(unsigned long word)
{
	int num = 0;
	/*需要先检查该值是否非0*/
	if(word == 0)
	{
		return 0;
	}
#if BITS_PER_LONG == 64
	if ((word & 0xffffffff) == 0)
	{
		/*64位中低32位全为0，右移32位，判断高32位*/
		num += 32;
		word >>= 32;
	}
#endif
	if ((word & 0xffff) == 0)
	{
		/*低16位全为0，右移16位，判断高16位*/
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0)
	{
		/*低8位全为0，右移8位，判断高8位*/
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0)
	{
		/*低4位全为0，右移4位，判断高4位*/
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0)
	{
		/*低2位为0，右移2位，判断高2位*/
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

#endif /* _ASM_GENERIC_BITOPS___FFS_H_ */
