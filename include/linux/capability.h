/*
 * This is <linux/capability.h>
 * See here for the libcap library ("POSIX draft" compliance):
 * ftp://linux.kernel.org/pub/linux/libs/security/linux-privs/kernel-2.6/
 */

#ifndef _LINUX_CAPABILITY_H
#define _LINUX_CAPABILITY_H

#include <linux/types.h>
#include <linux/compiler.h>

struct task_struct;

/* User-level do most of the mapping between kernel and user
   capabilities based on the version tag given by the kernel. The
   kernel might be somewhat backwards compatible, but don't bet on
   it. */

/* XXX - Note, cap_t, is defined by POSIX to be an "opaque" pointer to
   a set of three capability sets.  The transposition of 3*the
   following structure to such a composite is better handled in a user
   library since the draft standard requires the use of malloc/free
   etc.. */

#define _LINUX_CAPABILITY_VERSION 0x19980330

/**/
typedef struct __user_cap_header_struct
{
	__u32 version;
	int pid;
} __user *cap_user_header_t;

/*Ȩ�����ݽṹ*/
typedef struct __user_cap_data_struct
{
        __u32 effective;
        __u32 permitted;
        __u32 inheritable;
} __user *cap_user_data_t;

/*������չ����ǰ׺*/
#define XATTR_CAPS_SUFFIX "capability"
/*������չ������"security.capability"*/
#define XATTR_NAME_CAPS XATTR_SECURITY_PREFIX XATTR_CAPS_SUFFIX
/*������չ���Գ���С����12�ֽڣ�����λͼ�ֱ��Ӧ��ͬ��Ȩ��λ*/
#define XATTR_CAPS_SZ (3*sizeof(__le32))
/**/
#define VFS_CAP_REVISION_MASK			0xFF000000
/**/
#define VFS_CAP_REVISION_1				0x01000000
/**/
#define VFS_CAP_REVISION				VFS_CAP_REVISION_1
/**/
#define VFS_CAP_FLAGS_MASK				~VFS_CAP_REVISION_MASK
/**/
#define VFS_CAP_FLAGS_EFFECTIVE			0x000001

struct vfs_cap_data
{
	__u32 magic_etc;  /* Little endian */
	__u32 permitted;    /* Little endian */
	__u32 inheritable;  /* Little endian */
};

#ifdef __KERNEL__

/*U32���ͱ�����ʵ�Ǹ���ӦȨ�޵�λͼ��Ȩ�ް�λ����*/
#ifdef STRICT_CAP_T_TYPECHECKS
/*�ϸ�Ȩ�����ͼ�飬��Ȩ�޷�װΪU32���ͱ���*/
typedef struct kernel_cap_struct
{
	__u32 cap;
} kernel_cap_t;
#else
/*�Ƿ�װ��Ȩ��λͼ*/
typedef __u32 kernel_cap_t;
#endif

#define _USER_CAP_HEADER_SIZE  (2*sizeof(__u32))
#define _KERNEL_CAP_T_SIZE     (sizeof(kernel_cap_t))

#endif

/*��_POSIX_CHOWN_RESTRICTED�Ѷ��������£�ϵͳ����ı��ļ������ߺ���������Ȩ��*/
#define CAP_CHOWN					0

/*��������DACȨ��*/
#define CAP_DAC_OVERRIDE     		1

/*����DAC�����ж��Ͳ����ļ���Ŀ¼����Ȩ�ޣ������Ѷ����_POSIX_ACLACL����Ȩ�ޣ�������
CAP_LINUX_IMMUTABLE���Ƶ�DACȨ��*/
#define CAP_DAC_READ_SEARCH  		2

/* Overrides all restrictions about allowed operations on files, where
   file owner ID must be equal to the user ID, except where CAP_FSETID
   is applicable. It doesn't override MAC and DAC restrictions. */
/**/
#define CAP_FOWNER           		3

/*
(1)���ļ���λS_ISUID��S_ISGIDʱ������euid��fsuid�Ƿ�ƥ��
(2)���ļ���λS_ISGIDʱ������egid(������һ����gid)��fsuid�Ƿ�ƥ��
(3)chown(2)�ɹ�����ʱ���S_ISUID��S_ISGIDλ��û��ʵ�֣�
*/
#define CAP_FSETID           		4

/* Used to decide between falling back on the old suser() or fsuser(). */
/**/
#define CAP_FS_MASK					0x1f

/*���Է����źŽ��̵�uid/euid�Ƿ�������źŽ��̵�uid/euid�Ƿ�ƥ������*/ 
#define CAP_KILL             		5

/*����setgid(2)����������setgroup(2)���������׽��ִ�����֤�����м�ðgids*/
#define CAP_SETGID           		6

/*��������uid(2)����(����fsuid)���������׽��ִ�����֤�����м�ðpids*/
#define CAP_SETUID           		7

/*Linux�ر�Ȩ�ޣ��ɶ��κν�����ӻ�ɾ���κ��ѱ������Ȩ��*/
#define CAP_SETPCAP          		8

/*�����޸��ļ���S_IMMUTABLE��S_AAPEND����*/
#define CAP_LINUX_IMMUTABLE  		9

/*�����С��1024��TCP/UDP�׽���*/
/*�����С��32��ATMVCIs*/
#define CAP_NET_BIND_SERVICE 		10

/*����㲥�������ಥ*/
#define CAP_NET_BROADCAST		    11

/*
(1)�������ýӿ�
(2)��������IP����ǽ��masquerading��αװͳ����Ϣ
(3)�������׽��������õ���ѡ��
(4)�����޸�·�ɱ�
(5)�����׽��ֵĽ���������������Ϊ�������
(6)���������󶨵��κε�ַ
(7)�������÷�������
(8)�������û���ģʽ
(9)�����·�㲥
(10)�������д�豸������Ĵ���
(11)����ATM�����׽��ִ��ڻ״̬
*/
#define CAP_NET_ADMIN				12

/*����ʹ��ԭʼ�׽��֣�����ʹ���׽��ְ�*/
#define CAP_NET_RAW          		13

/*��������һ�ι����ڴ桢�������mlock��mlockall����(��ʹ��ipc�޹�)*/
#define CAP_IPC_LOCK         		14

/*��������IPC����Ȩ���*/
#define CAP_IPC_OWNER        		15

/*�����ɾ���ں�ģ�飬�������޸��ںˡ��޸�Ȩ�ܼ�*/
#define CAP_SYS_MODULE       		16

/*����ioperm/ioplȨ�ޡ�������USB��Ϣ���κ������豸 /proc/bus/usb*/
#define CAP_SYS_RAWIO        		17

/*����ʹ��chroot()*/
#define CAP_SYS_CHROOT       		18

/*��������κν���*/
#define CAP_SYS_PTRACE       		19

/*���������κν���ͳ����*/
#define CAP_SYS_PACCT				20

/*
(1)�������ð�ȫ���ѹؼ���
(2)������������豸
(3)������Ժ�����Ӳ�����
(4)���������ں�ϵͳ��־����ӡ��Ϊ��
(5)������������
(6)��������������
(7)�������bdflash()
(8)������غ�ȡ�����أ�����һ���µ�smb����
(9)����һЩ�Զ��ļ�ϵͳioctrls
(10)����nfsservctl
(11)����VM86_REQUEST_IRQ
(12)������alph�϶�дpci����
(13)������mips��irix_prctl(setstacksize)
(14)������m68k�ϵ���sys_cacheflush������л���
(15)����ɾ���ź���
(16)���̼��ź���/��Ϣ����/�����ڴ�ʹ��chown����CAP_CHOWN
(17)������������������ڴ��
(18)��������رս���
(19)�������׽��ִ�����֤�����м�ðpids
(20)�����ڿ��豸��Ԥ����ˢ��������
(21)��������������������geometry
(22)������xd��չ�����д򿪹ر�DMA
(23)�������md�豸
(24)Allow tuning the ide driver
(25)�������nvram�豸
(26)�������apm_bios/serial/bttv (TV)�豸
(27)Allow manufacturer commands in isdn CAPI support driver
(28)�����ȡpci���ÿռ��еķǱ�׼������
(29)Allow DDI debug ioctl on sbpcd driver
(30)�������ô��ж˿�
(31)������ԭʼqic-117����
(32)����/����SCSI�������ͷ�������SCSI�����ǵĶ���
(33)�����ڻػ��ļ�ϵͳ���ü����ܳ�
(34)�������û��ղ�����
*/
#define CAP_SYS_ADMIN        		21

/*����ʹ��reboot()*/
#define CAP_SYS_BOOT         		22

/*
(1)�����������ȼ�����������ͬ�û������������ȼ�
(2)Allow use of FIFO and round-robin (realtime) scheduling on own processes and setting
the scheduling algorithm used by another process
(3)����������������������cpu���׺���
*/
#define CAP_SYS_NICE      		   23

/*
(1)������Դ���ơ�������Դ����
(2)�����������
(3)����ext2�ļ�ϵͳ��Ԥ���ռ�
(4)��ext3�ļ��ļ�ϵͳ��������־��Դ���޸���־ģʽ����
(5)NOTE: ext2 honors fsuid when checking for resource overrides, so you can override
using fsuid too
(6)����IPC��Ϣ���д�С����
(7)�����ʵʱʱ�ӳ���64HZ�ж�
(8)���Ŀ���̨�����ϵĿ���̨�����Ŀ
(9)�����ܳ�λͼ�������Ŀ
*/

#define CAP_SYS_RESOURCE			24

/*�������ϵͳʱ��;������mips irix_stime;��������ʵʱʱ��*/
#define CAP_SYS_TIME      		   	25

/*���������ն��豸�������ն��豸ִ������Ҷϲ���*/
#define CAP_SYS_TTY_CONFIG			26

/*����moknod()�������Ȩ*/
#define CAP_MKNOD            		27

/*�����޸��ļ�����ʶ*/
#define CAP_LEASE            		28

#define CAP_AUDIT_WRITE      		29

#define CAP_AUDIT_CONTROL    		30

#define CAP_SETFCAP	     			31

#ifdef __KERNEL__

/*�ں��ڲ�����*/
#ifdef STRICT_CAP_T_TYPECHECKS
/*��Ȩ�޶�Ӧ��λͼ��װ*/
#define to_cap_t(x) { x }
/*���Ȩ��λͼ*/
#define cap_t(x) (x).cap
#else
/*���ϸ�Ȩ�����ͼ��û��װ��û�䶯*/
#define to_cap_t(x) (x)
#define cap_t(x) (x)
#endif

/*���Ȩ�޼���*/
#define CAP_EMPTY_SET       to_cap_t(0)
/*����ȫȨ��*/
#define CAP_FULL_SET        to_cap_t(~0)
/*��ЧȨ�޼��ϳ�ʼ��������ɶ��κν�����ӻ�ɾ�����ѱ������Ȩ��*/
#define CAP_INIT_EFF_SET to_cap_t(~0 & (~ CAP_TO_MASK(CAP_SETPCAP)))
/*�̳�Ȩ�޼��ϳ�ʼ��Ϊ��*/
#define CAP_INIT_INH_SET to_cap_t(0)
/*Ȩ�޵�Ȩ��*/
#define CAP_TO_MASK(x) (1 << (x))
/*����(���)ָ��Ȩ��*/
#define cap_raise(c, flag)   (cap_t(c) |=  CAP_TO_MASK(flag))
/*���ָ��Ȩ��*/
#define cap_lower(c, flag)   (cap_t(c) &= (~ CAP_TO_MASK(flag)))
/*Ȩ�޼������Ƿ���ָ��Ȩ��*/
#define cap_raised(c, flag)  (cap_t(c) & CAP_TO_MASK(flag))

/*�ϲ���Ȩ�޼�����λ�������*/
static inline kernel_cap_t cap_combine(kernel_cap_t a, kernel_cap_t b)
{
     kernel_cap_t dest;
     cap_t(dest) = cap_t(a) | cap_t(b);
     return dest;
}

/*������Ȩ�޼����Ƿ�����ͬȨ�ޣ���λ�������*/
static inline kernel_cap_t cap_intersect(kernel_cap_t a, kernel_cap_t b)
{
     kernel_cap_t dest;
     cap_t(dest) = cap_t(a) & cap_t(b);
     return dest;
}

/*���Ȩ�޼����е�ĳЩȨ�ޣ���λ��ǲ�����*/
static inline kernel_cap_t cap_drop(kernel_cap_t a, kernel_cap_t drop)
{
     kernel_cap_t dest;
     cap_t(dest) = cap_t(a) & ~ cap_t(drop);
     return dest;
}

/*��ȡȨ�޼��ϵĲ������ǲ�����*/
static inline kernel_cap_t cap_invert(kernel_cap_t c)
{
     kernel_cap_t dest;
     cap_t(dest) = ~ cap_t(c);
     return dest;
}

/*���Կ�Ȩ�޼�*/
#define cap_isclear(c)       (!cap_t(c))
/*����Ȩ�޼��Ƿ�����һȨ�޼����Ӽ�*/
#define cap_issubset(a,set)  (!(cap_t(a) & ~ cap_t(set)))
/*���Ȩ�޼�*/
#define cap_clear(c)         do { cap_t(c) =  0; } while(0)
/*����ȫ����Ȩ��*/
#define cap_set_full(c)      do { cap_t(c) = ~0; } while(0)
/*��ȡ��Ȩ�޽���*/
#define cap_mask(c,mask)     do { cap_t(c) &= cap_t(mask); } while(0)
/**/
#define cap_is_fs_cap(c)     (CAP_TO_MASK(c) & CAP_FS_MASK)
/*�жϵ�ǰ�����Ƿ����ʹ�ó����û�Ȩ��*/
int capable(int cap);
/*�жϽ����Ƿ����ʹ�ó����û�Ȩ��*/
int __capable(struct task_struct *t, int cap);

#endif /* __KERNEL__ */

#endif /* !_LINUX_CAPABILITY_H */
