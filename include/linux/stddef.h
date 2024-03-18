#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

#include <linux/compiler.h>

/*ȡ��NULL�궨��*/
#undef NULL
/*NULL��C++�ж���Ϊ0����C�ж���Ϊ((void *) 0)ͨ�����͵Ŀ�ָ��*/
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#ifdef __KERNEL__

/*����ֵtrue����Ϊ�޷�������1��false 0*/
enum
{
	false	= 0,
	true	= 1
};

/*ȡ��offsetof�궨�壬��ȡTYPE�ṹ���ԱMEMBER��ƫ��*/
#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#endif /* __KERNEL__ */

#endif
