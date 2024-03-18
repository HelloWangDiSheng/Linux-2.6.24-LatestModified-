#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#ifdef	__KERNEL__

/*该宏定义一个名称为name，长度为bits的位图，位图用刚好不小于其所占最小无符号长整形
个数的数组表示，位图中的有效位置从0到bits-1*/
#define DECLARE_BITMAP(name,bits)		unsigned long name[BITS_TO_LONGS(bits)]

#endif

#include <linux/posix_types.h>
#include <asm/types.h>

#ifndef __KERNEL_STRICT_NAMES
/*内核内部设备号*/
typedef __u32							__kernel_dev_t;
typedef __kernel_dev_t					dev_t;
/*文件描述符表中已打开文件位图*/
typedef __kernel_fd_set					fd_set;
/*inode编号类型*/
typedef __kernel_ino_t					ino_t;
/*文件类型及访问权限类型*/
typedef __kernel_mode_t					mode_t;
/*文件硬链接数目类型*/
typedef __kernel_nlink_t				nlink_t;
/*偏移*/
typedef __kernel_off_t					off_t;
/*pid类型*/
typedef __kernel_pid_t					pid_t;
/*地址偏移*/
typedef __kernel_daddr_t				daddr_t;
/*IPC key*/
typedef __kernel_key_t					key_t;
typedef __kernel_mqd_t					mqd_t;
/**/
typedef __kernel_suseconds_t			suseconds_t;
/*定时器*/
typedef __kernel_timer_t				timer_t;
/*时钟*/
typedef __kernel_clockid_t				clockid_t;


#ifdef __KERNEL__
/*布尔类型*/
typedef _Bool			bool;
/*uid类型*/
typedef __kernel_uid32_t	uid_t;
/*gid类型*/
typedef __kernel_gid32_t	gid_t;
typedef __kernel_uid16_t        uid16_t;
typedef __kernel_gid16_t        gid16_t;

typedef unsigned long		uintptr_t;

#ifdef CONFIG_UID16
/* This is defined by include/asm-{arch}/posix_types.h */
typedef __kernel_old_uid_t	old_uid_t;
typedef __kernel_old_gid_t	old_gid_t;
#endif /* CONFIG_UID16 */

/* libc5 includes this file to define uid_t, thus uid_t can never change
 * when it is included by non-kernel code
 */
#else
typedef __kernel_uid_t			uid_t;
typedef __kernel_gid_t			gid_t;
#endif /* __KERNEL__ */

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef __kernel_loff_t			loff_t;
#endif

/*历史原因保留的独立的条件编译类型定义*/
#ifndef _SIZE_T
#define _SIZE_T
typedef __kernel_size_t				size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef __kernel_ssize_t			ssize_t;
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef __kernel_ptrdiff_t			ptrdiff_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef __kernel_time_t				time_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef __kernel_clock_t			clock_t;
#endif

#ifndef _CADDR_T
#define _CADDR_T
typedef __kernel_caddr_t			caddr_t;
#endif

/* bsd */
typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;

/* sysv */
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef		__u8		u_int8_t;
typedef		__s8		int8_t;
typedef		__u16		u_int16_t;
typedef		__s16		int16_t;
typedef		__u32		u_int32_t;
typedef		__s32		int32_t;

#endif /* !(__BIT_TYPES_DEFINED__) */

typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef		__u64		uint64_t;
typedef		__u64		u_int64_t;
typedef		__s64		int64_t;
#endif

/*8字节对齐的64位数据类型*/
#define aligned_u64 unsigned long long __attribute__((aligned(8)))
#define aligned_be64 __be64 __attribute__((aligned(8)))
#define aligned_le64 __le64 __attribute__((aligned(8)))

 /*磁盘或分区上的扇区号，linux经常独立于实际的设备块大小默认扇区长512字节*/
#ifdef CONFIG_LBD
typedef u64 sector_t;
#else
typedef unsigned long sector_t;
#endif


 /*inode块数目类型*/
#ifdef CONFIG_LSF
typedef u64 blkcnt_t;
#else
typedef unsigned long blkcnt_t;
#endif

 /*页缓存中的索引，可引用asm/types.h文件进行覆盖*/
#ifndef pgoff_t
#define pgoff_t unsigned long
#endif

#endif /* __KERNEL_STRICT_NAMES */

/*以下是linux特有的与任何应用程序或库都不冲突的类型定义*/
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
/*大小端序定义big/little_endian*/
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif
typedef __u16 __bitwise __sum16;
typedef __u32 __bitwise __wsum;

#ifdef __KERNEL__
/*获得空闲页类型*/
typedef unsigned __bitwise__ gfp_t;

/*资源长度，字节长度*/
#ifdef CONFIG_RESOURCES_64BIT
typedef u64 resource_size_t;
#else
typedef u32 resource_size_t;
#endif

#endif	/* __KERNEL__ */

struct ustat
{
	__kernel_daddr_t	f_tfree;
	__kernel_ino_t		f_tinode;
	char			f_fname[6];
	char			f_fpack[6];
};

#endif /* _LINUX_TYPES_H */
