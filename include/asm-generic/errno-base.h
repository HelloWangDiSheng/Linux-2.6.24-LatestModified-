#ifndef _ASM_GENERIC_ERRNO_BASE_H
#define _ASM_GENERIC_ERRNO_BASE_H
/*非法操作*/
#define	EPERM		 1
/*没有该文件或目录*/
#define	ENOENT		 2
/*没有该进程*/
#define	ESRCH		 3
/*系统调用中断*/
#define	EINTR		 4
/*输入输出错误*/
#define	EIO			 5
/*没有该设备或地址*/
#define	ENXIO		 6
/*参数表太长*/
#define	E2BIG		 7
/*可执行文件格式错误*/
#define	ENOEXEC		 8
/*文件编号错误*/
#define	EBADF		 9
/*没有该子进程*/
#define	ECHILD		10
/*重试一下*/
#define	EAGAIN		11
/*内存不足*/
#define	ENOMEM		12
/*拒绝访问*/
#define	EACCES		13
/*无效地址*/
#define	EFAULT		14
/*需要块设备*/
#define	ENOTBLK		15
/*设备或资源被占用*/
#define	EBUSY		16
/*文件已存在*/
#define	EEXIST		17
/* Cross-device link */
#define	EXDEV		18
/*没有该设备*/
#define	ENODEV		19
/*非目录*/
#define	ENOTDIR		20
/*目录*/
#define	EISDIR		21
/*无效参数*/
#define	EINVAL		22
/*文件描述符表溢出*/
#define	ENFILE		23
/*打开太多文件*/
#define	EMFILE		24
/*非终端设备*/
#define	ENOTTY		25
/*txt文件被占用*/
#define	ETXTBSY		26
/*文件太大*/
#define	EFBIG		27
/*设备上没有剩余空间*/
#define	ENOSPC		28
/*非法搜索*/
#define	ESPIPE		29
/*只读文件系统*/
#define	EROFS		30
/*太多链接*/
#define	EMLINK		31
/*管道破损*/
#define	EPIPE		32
/*数学参数超出函数定义域*/
#define	EDOM		33
/*数字运算结果无法表示*/
#define	ERANGE		34

#endif
