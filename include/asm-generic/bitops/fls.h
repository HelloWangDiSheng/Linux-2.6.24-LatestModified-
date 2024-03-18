#ifndef _ASM_GENERIC_BITOPS_FLS_H_
#define _ASM_GENERIC_BITOPS_FLS_H_

/*��ѯ������ʾ�����ߵ͵ظߣ���λ�͵�ַ����λ�ߵ�ַ����0x12345678�Ĵ洢��ַ��ʼ���
0x12345678�������ֶ����Ʊ��ʽ�������λ��λ�á�fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32*/
static inline int fls(int x)
{
	/**/
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) 
	{
		/*��16λȫΪ0������16λ���Ƚϵ�16λ*/
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u))
	{
		/*��8λȫΪ0������8λ*/
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u))
	{
		/*��4λȫΪ0*/
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u))
	{
		/*��2λȫΪ0������2λ*/
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u))
	{
		/*���λΪ0������1λ*/
		x <<= 1;
		r -= 1;
	}
	return r;
}

#endif /* _ASM_GENERIC_BITOPS_FLS_H_ */
