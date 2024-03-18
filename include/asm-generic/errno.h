#ifndef _ASM_GENERIC_ERRNO_H
#define _ASM_GENERIC_ERRNO_H

#include <asm-generic/errno-base.h>
/*���ܷ�������Դ����*/
#define	EDEADLK					35
/*�ļ���̫��*/
#define	ENAMETOOLONG			36
/*û�м�¼������*/
#define	ENOLCK					37
/*����δʵ��*/
#define	ENOSYS					38
/*Ŀ¼��Ϊ��*/
#define	ENOTEMPTY				39
/*̫���������*/
#define	ELOOP					40
/*�����ܱ�������*/
#define	EWOULDBLOCK				EAGAIN
/*û���������͵���Ϣ*/
#define	ENOMSG					42
/*��ʶ����ɾ��*/
#define	EIDRM					43
/*Ƶ��������*/
#define	ECHRNG					44
/*2����ͬ��*/
#define	EL2NSYNC				45
/*3���ж�*/
#define	EL3HLT					46
/*3����λ*/
#define	EL3RST					47
/*���������*/
#define	ELNRNG					48
/*Э������δ����*/
#define	EUNATCH					49
/*û�п��õ�CSI�ṹ*/
#define	ENOCSI					50
/*2���ж�*/
#define	EL2HLT					51
/*��Ч����*/
#define	EBADE					52
/*��Ч����������*/
#define	EBADR					53
/*��ȫ����*/
#define	EXFULL					54	/* Exchange full */
/**/
#define	ENOANO					55	/* No anode */
/*��Ч������*/
#define	EBADRQC					56
/*��Ч���*/
#define	EBADSLT					57
/*��������Դ����*/
#define	EDEADLOCK				EDEADLK
/*����������ʽ*/
#define	EBFONT					59
/*�豸û��������*/
#define	ENOSTR					60
/*û�п�������*/
#define	ENODATA					61	/* No data available */
/*����*/
#define	ETIME					62	/* Timer expired */
/*����Դ����*/
#define	ENOSR					63	/* Out of streams resources */
/*��������������*/
#define	ENONET					64	/* Machine is not on the network */
/*δ��װ��*/
#define	ENOPKG					65	/* Package not installed */
/*�����ѱ��Ƴ�*/
#define	EREMOTE					66	/* Object is remote */
/**/
#define	ENOLINK					67	/* Link has been severed */
/**/
#define	EADV					68	/* Advertise error */
/**/
#define	ESRMNT					69	/* Srmount error */
/*����ʱͨѶ����*/
#define	ECOMM					70	/* Communication error on send */
/*Э�����*/
#define	EPROTO					71	/* Protocol error */
/**/
#define	EMULTIHOP				72	/* Multihop attempted */
/**/
#define	EDOTDOT					73	/* RFS specific error */
/*��������Ϣ*/
#define	EBADMSG					74	/* Not a data message */
/*�Ѷ����������͵�ֵ����*/
#define	EOVERFLOW				75	/* Value too large for defined data type */
/*�����е����Ʋ�Ψһ*/
#define	ENOTUNIQ				76	/* Name not unique on network */
/*�ļ�������״̬����*/
#define	EBADFD					77	/* File descriptor in bad state */
/*�Ƴ��ı�ĵ�ַ*/
#define	EREMCHG					78	/* Remote address changed */
/*���ܷ���һ����Ҫ�Ĺ����*/
#define	ELIBACC					79	/* Can not access a needed shared library */
/*����һ���𻵵Ĺ����*/
#define	ELIBBAD					80	/* Accessing a corrupted shared library */
/*Ŀ���ļ��пⱻ�ƻ�*/
#define	ELIBSCN					81	/* .lib section in a.out corrupted */
/*��ͼ���ӵ�̫�๲���*/
#define	ELIBMAX					82	/* Attempting to link in too many shared libraries */
/*����ִ��һ������Ŀ�*/
#define	ELIBEXEC				83	/* Cannot exec a shared library directly */
/*�Ƿ����ֽ���*/
#define	EILSEQ					84	/* Illegal byte sequence */
/*Ӧ�������жϵ�ϵͳ����*/
#define	ERESTART				85	/* Interrupted system call should be restarted */
/*��ʽ�ܵ�����*/
#define	ESTRPIPE				86	/* Streams pipe error */
/*̫���û�*/
#define	EUSERS					87	/* Too many users */
/*�ڷ��׽����ϲ����׽���*/
#define	ENOTSOCK				88	/* Socket operation on non-socket */
/*��ҪĿ�ĵ�ַ*/
#define	EDESTADDRREQ			89	/* Destination address required */
/*��Ϣ̫��*/
#define	EMSGSIZE				90	/* Message too long */
/*�׽���Э�����ʹ���*/
#define	EPROTOTYPE				91	/* Protocol wrong type for socket */
/*Э�鲻����*/
#define	ENOPROTOOPT				92	/* Protocol not available */
/*��֧�ֵ�Э��*/
#define	EPROTONOSUPPORT			93	/* Protocol not supported */
/*��֧�ֵ��׽�������*/
#define	ESOCKTNOSUPPORT			94	/* Socket type not supported */
/*�����㲻֧�ָò���*/
#define	EOPNOTSUPP				95	/* Operation not supported on transport endpoint */
/*Э���岻֧��*/
#define	EPFNOSUPPORT			96	/* Protocol family not supported */
/*Э�鲻֧�ֵ�ַ��*/
#define	EAFNOSUPPORT			97	/* Address family not supported by protocol */
/*��ַ�Ѿ���ʹ����*/
#define	EADDRINUSE				98	/* Address already in use */
/**/
#define	EADDRNOTAVAIL			99	/* Cannot assign requested address */
/*����̱��*/
#define	ENETDOWN				100	/* Network is down */
/*���粻�ɵ���*/
#define	ENETUNREACH				101	/* Network is unreachable */
/*������������ʱ������*/
#define	ENETRESET				102	/* Network dropped connection because of reset */
/*�������������ֹ*/
#define	ECONNABORTED			103	/* Software caused connection abort */
/**/
#define	ECONNRESET				104	/* Connection reset by peer */
/*û�п��õĻ�����*/
#define	ENOBUFS					105	/* No buffer space available */
/*�������Ѿ�����*/
#define	EISCONN					106	/* Transport endpoint is already connected */
/*�����㻹û������*/
#define	ENOTCONN				107	/* Transport endpoint is not connected */
/**/
#define	ESHUTDOWN				108	/* Cannot send after transport endpoint shutdown */
/**/
#define	ETOOMANYREFS			109	/* Too many references: cannot splice */
/*���ӳ�ʱ*/
#define	ETIMEDOUT				110	/* Connection timed out */
/*���ӱ��ܾ�*/
#define	ECONNREFUSED			111	/* Connection refused */
/*�����ѹر�*/
#define	EHOSTDOWN				112	/* Host is down */
/*û�е�������·��*/
#define	EHOSTUNREACH			113	/* No route to host */
/*�����Ѿ�����*/
#define	EALREADY				114	/* Operation already in progress */
/*������������*/
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
/*Զ��IO����*/
#define	EREMOTEIO				121	/* Remote I/O error */
/*�����������*/
#define	EDQUOT					122	/* Quota exceeded */
/*û�з���ý��*/
#define	ENOMEDIUM				123
/*ý�����ʹ���*/
#define	EMEDIUMTYPE				124
/*������ȡ��*/
#define	ECANCELED				125
/*������ܳײ�����*/
#define	ENOKEY					126
/*�ܳ׹���*/
#define	EKEYEXPIRED				127
/*�ܳ��Ѿ�����*/
#define	EKEYREVOKED				128
/*����ܾ����ܳ�*/
#define	EKEYREJECTED			129

/*����������*/
#define	EOWNERDEAD				130
/*״̬���ɻָ�*/
#define	ENOTRECOVERABLE			131

#endif
