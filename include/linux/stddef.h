#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

#include <linux/compiler.h>

/*取消NULL宏定义*/
#undef NULL
/*NULL在C++中定义为0，在C中定义为((void *) 0)通用类型的空指针*/
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#ifdef __KERNEL__

/*布尔值true定义为无符号整形1，false 0*/
enum
{
	false	= 0,
	true	= 1
};

/*取消offsetof宏定义，获取TYPE结构体成员MEMBER的偏移*/
#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#endif /* __KERNEL__ */

#endif
