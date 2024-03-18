#ifndef _LINUX_ERR_H
#define _LINUX_ERR_H

#include <linux/compiler.h>

#include <asm/errno.h>

/*
 * Kernel pointers have redundant information, so we can use a
 * scheme where we can return either an error code or a dentry
 * pointer with the same return value.
 *
 * This should be a per-architecture thing, to allow different
 * error and pointer decisions.
 */
 /*-MAX_ERRNO对应内核空间中最顶层页面，可作为判断错误指针*/
#define MAX_ERRNO	4095

#ifndef __ASSEMBLY__

/*指针落入内核地址空间最顶层的页面时，认为发生错误*/
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

/*将错误信息转换为通用指针*/
static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

/*将常量指针转换为错误码*/
static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

/*测试输入的常量指针是否落在内核虚拟地址空间的最顶层页面，是则说明发生错误*/
static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

#endif

#endif /* _LINUX_ERR_H */
