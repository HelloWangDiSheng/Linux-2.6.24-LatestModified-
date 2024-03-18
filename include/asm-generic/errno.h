#ifndef _ASM_GENERIC_ERRNO_H
#define _ASM_GENERIC_ERRNO_H

#include <asm-generic/errno-base.h>
/*可能发生了资源死锁*/
#define	EDEADLK					35
/*文件名太长*/
#define	ENAMETOOLONG			36
/*没有记录锁可用*/
#define	ENOLCK					37
/*函数未实现*/
#define	ENOSYS					38
/*目录不为空*/
#define	ENOTEMPTY				39
/*太多符号连接*/
#define	ELOOP					40
/*操作能被被阻塞*/
#define	EWOULDBLOCK				EAGAIN
/*没有期望类型的消息*/
#define	ENOMSG					42
/*标识符被删除*/
#define	EIDRM					43
/*频道编号溢出*/
#define	ECHRNG					44
/*2级不同步*/
#define	EL2NSYNC				45
/*3级中断*/
#define	EL3HLT					46
/*3级复位*/
#define	EL3RST					47
/*链接数溢出*/
#define	ELNRNG					48
/*协议驱动未连接*/
#define	EUNATCH					49
/*没有可用的CSI结构*/
#define	ENOCSI					50
/*2级中断*/
#define	EL2HLT					51
/*无效交换*/
#define	EBADE					52
/*无效的请求描述*/
#define	EBADR					53
/*完全交换*/
#define	EXFULL					54	/* Exchange full */
/**/
#define	ENOANO					55	/* No anode */
/*无效请求码*/
#define	EBADRQC					56
/*无效插槽*/
#define	EBADSLT					57
/*可能有资源死锁*/
#define	EDEADLOCK				EDEADLK
/*错误的字体格式*/
#define	EBFONT					59
/*设备没有数据流*/
#define	ENOSTR					60
/*没有可用数据*/
#define	ENODATA					61	/* No data available */
/*过期*/
#define	ETIME					62	/* Timer expired */
/*流资源不足*/
#define	ENOSR					63	/* Out of streams resources */
/*主机不在网络中*/
#define	ENONET					64	/* Machine is not on the network */
/*未安装包*/
#define	ENOPKG					65	/* Package not installed */
/*对象已被移除*/
#define	EREMOTE					66	/* Object is remote */
/**/
#define	ENOLINK					67	/* Link has been severed */
/**/
#define	EADV					68	/* Advertise error */
/**/
#define	ESRMNT					69	/* Srmount error */
/*发送时通讯错误*/
#define	ECOMM					70	/* Communication error on send */
/*协议错误*/
#define	EPROTO					71	/* Protocol error */
/**/
#define	EMULTIHOP				72	/* Multihop attempted */
/**/
#define	EDOTDOT					73	/* RFS specific error */
/*非数据消息*/
#define	EBADMSG					74	/* Not a data message */
/*已定义数据类型的值上溢*/
#define	EOVERFLOW				75	/* Value too large for defined data type */
/*网络中的名称不唯一*/
#define	ENOTUNIQ				76	/* Name not unique on network */
/*文件描述符状态错误*/
#define	EBADFD					77	/* File descriptor in bad state */
/*移除改变的地址*/
#define	EREMCHG					78	/* Remote address changed */
/*不能访问一个需要的共享库*/
#define	ELIBACC					79	/* Can not access a needed shared library */
/*访问一个损坏的共享库*/
#define	ELIBBAD					80	/* Accessing a corrupted shared library */
/*目标文件中库被破坏*/
#define	ELIBSCN					81	/* .lib section in a.out corrupted */
/*试图连接到太多共享库*/
#define	ELIBMAX					82	/* Attempting to link in too many shared libraries */
/*不能执行一个共享的库*/
#define	ELIBEXEC				83	/* Cannot exec a shared library directly */
/*非法的字节序*/
#define	EILSEQ					84	/* Illegal byte sequence */
/*应该重启中断的系统调用*/
#define	ERESTART				85	/* Interrupted system call should be restarted */
/*流式管道错误*/
#define	ESTRPIPE				86	/* Streams pipe error */
/*太多用户*/
#define	EUSERS					87	/* Too many users */
/*在非套接字上操作套接字*/
#define	ENOTSOCK				88	/* Socket operation on non-socket */
/*需要目的地址*/
#define	EDESTADDRREQ			89	/* Destination address required */
/*消息太长*/
#define	EMSGSIZE				90	/* Message too long */
/*套接字协议类型错误*/
#define	EPROTOTYPE				91	/* Protocol wrong type for socket */
/*协议不可用*/
#define	ENOPROTOOPT				92	/* Protocol not available */
/*不支持的协议*/
#define	EPROTONOSUPPORT			93	/* Protocol not supported */
/*不支持的套接字类型*/
#define	ESOCKTNOSUPPORT			94	/* Socket type not supported */
/*传输结点不支持该操作*/
#define	EOPNOTSUPP				95	/* Operation not supported on transport endpoint */
/*协议族不支持*/
#define	EPFNOSUPPORT			96	/* Protocol family not supported */
/*协议不支持地址族*/
#define	EAFNOSUPPORT			97	/* Address family not supported by protocol */
/*地址已经在使用中*/
#define	EADDRINUSE				98	/* Address already in use */
/**/
#define	EADDRNOTAVAIL			99	/* Cannot assign requested address */
/*网络瘫痪*/
#define	ENETDOWN				100	/* Network is down */
/*网络不可到达*/
#define	ENETUNREACH				101	/* Network is unreachable */
/*重启导致网络时区连接*/
#define	ENETRESET				102	/* Network dropped connection because of reset */
/*软件导致连接终止*/
#define	ECONNABORTED			103	/* Software caused connection abort */
/**/
#define	ECONNRESET				104	/* Connection reset by peer */
/*没有可用的缓冲区*/
#define	ENOBUFS					105	/* No buffer space available */
/*传输结点已经连接*/
#define	EISCONN					106	/* Transport endpoint is already connected */
/*传输结点还没有连接*/
#define	ENOTCONN				107	/* Transport endpoint is not connected */
/**/
#define	ESHUTDOWN				108	/* Cannot send after transport endpoint shutdown */
/**/
#define	ETOOMANYREFS			109	/* Too many references: cannot splice */
/*连接超时*/
#define	ETIMEDOUT				110	/* Connection timed out */
/*连接被拒绝*/
#define	ECONNREFUSED			111	/* Connection refused */
/*主机已关闭*/
#define	EHOSTDOWN				112	/* Host is down */
/*没有到主机的路由*/
#define	EHOSTUNREACH			113	/* No route to host */
/*操作已经运行*/
#define	EALREADY				114	/* Operation already in progress */
/*操作正在运行*/
#define	EINPROGRESS				115	/* Operation now in progress */
/**/
#define	ESTALE					116	/* Stale NFS file handle */
/**/
#define	EUCLEAN					117	/* Structure needs cleaning */
/**/
#define	ENOTNAM					118	/* Not a XENIX named type file */
/**/
#define	ENAVAIL					119	/* No XENIX semaphores available */
/**/
#define	EISNAM					120	/* Is a named type file */
/*远程IO错误*/
#define	EREMOTEIO				121	/* Remote I/O error */
/*超出磁盘配额*/
#define	EDQUOT					122	/* Quota exceeded */
/*没有发现媒体*/
#define	ENOMEDIUM				123
/*媒体类型错误*/
#define	EMEDIUMTYPE				124
/*操作被取消*/
#define	ECANCELED				125
/*所需的密匙不可用*/
#define	ENOKEY					126
/*密匙过期*/
#define	EKEYEXPIRED				127
/*密匙已经撤销*/
#define	EKEYREVOKED				128
/*服务拒绝了密匙*/
#define	EKEYREJECTED			129

/*所有者死亡*/
#define	EOWNERDEAD				130
/*状态不可恢复*/
#define	ENOTRECOVERABLE			131

#endif
