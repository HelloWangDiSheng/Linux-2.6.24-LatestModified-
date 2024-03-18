#ifndef _ASM_GENERIC_BITOPS___FFS_H_
#define _ASM_GENERIC_BITOPS___FFS_H_

#include <asm/types.h>

/*���������е�һ����λ�ı���λ��������Ҫ�ȼ����������Ƿ���㣬Ȼ���Ȱ�һ���ֳ���ռ
�ı���λ��Ŀ��Ϊ�Ƚϳ��ȣ����Ե�λ�Ƿ�Ϊ0�����Ϊ0ʱ��ѭ�����ƱȽϳ��ȵ�һ�룬������
�ѱȽϵ�������ֱ���Ƚϳ���Ϊ1ʱ�����Ƚϣ�������������IA-32 __ffs(23)=__ffs(0x17)=
__ffs(10111)=0,__ffs(24)=__ffs(0x18)=__ffs(11000)=3*/
static inline unsigned long __ffs(unsigned long word)
{
	int num = 0;
	/*��Ҫ�ȼ���ֵ�Ƿ��0*/
	if(word == 0)
	{
		return 0;
	}
#if BITS_PER_LONG == 64
	if ((word & 0xffffffff) == 0)
	{
		/*64λ�е�32λȫΪ0������32λ���жϸ�32λ*/
		num += 32;
		word >>= 32;
	}
#endif
	if ((word & 0xffff) == 0)
	{
		/*��16λȫΪ0������16λ���жϸ�16λ*/
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0)
	{
		/*��8λȫΪ0������8λ���жϸ�8λ*/
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0)
	{
		/*��4λȫΪ0������4λ���жϸ�4λ*/
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0)
	{
		/*��2λΪ0������2λ���жϸ�2λ*/
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

#endif /* _ASM_GENERIC_BITOPS___FFS_H_ */
