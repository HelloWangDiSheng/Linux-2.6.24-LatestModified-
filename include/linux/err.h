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
 /*-MAX_ERRNO��Ӧ�ں˿ռ������ҳ�棬����Ϊ�жϴ���ָ��*/
#define MAX_ERRNO	4095

#ifndef __ASSEMBLY__

/*ָ�������ں˵�ַ�ռ�����ҳ��ʱ����Ϊ��������*/
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

/*��������Ϣת��Ϊͨ��ָ��*/
static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

/*������ָ��ת��Ϊ������*/
static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

/*��������ĳ���ָ���Ƿ������ں������ַ�ռ�����ҳ�棬����˵����������*/
static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

#endif

#endif /* _LINUX_ERR_H */
