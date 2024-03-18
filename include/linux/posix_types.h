#ifndef _LINUX_POSIX_TYPES_H
#define _LINUX_POSIX_TYPES_H

#include <linux/stddef.h>

/*
 * This allows for 1024 file descriptors: if NR_OPEN is ever grown
 * beyond that you'll have to change this too. But 1024 fd's seem to be
 * enough even for such "real" unices like OSF/1, so hopefully this is
 * one limit that doesn't have to be changed [again].
 *
 * Note that POSIX wants the FD_CLEAR(fd,fdsetp) defines to be in
 * <sys/time.h> (and thus <linux/time.h>) - but this is a more logical
 * place for them. Solved by having dummy defines in <sys/time.h>.
 */

/*
 * Those macros may have been defined in <gnu/types.h>. But we always
 * use the ones here. 
 */
 /*重新定义进程默认打开文件的数目*/
#undef __NFDBITS
#define __NFDBITS	(8 * sizeof(unsigned long))

/*进程可以打开文件数目的上限*/
#undef __FD_SETSIZE
#define __FD_SETSIZE	1024

/*表示已打开文件上限所需要使用的unsigned long类型变量的个数*/
#undef __FDSET_LONGS
#define __FDSET_LONGS	(__FD_SETSIZE/__NFDBITS)
/**/
#undef __FDELT
#define	__FDELT(d)	((d) / __NFDBITS)

#undef __FDMASK
#define	__FDMASK(d)	(1UL << ((d) % __NFDBITS))

/*内核内部用来表示可打开文件的上限的位图*/
typedef struct {
	unsigned long fds_bits [__FDSET_LONGS];
} __kernel_fd_set;

/*定义信号处理函数指针*/
typedef void (*__kernel_sighandler_t)(int);

/*SYSV IPC关键字类型*/
typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

#include <asm/posix_types.h>

#endif /* _LINUX_POSIX_TYPES_H */
