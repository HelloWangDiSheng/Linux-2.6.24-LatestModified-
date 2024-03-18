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

/*权限数据结构*/
typedef struct __user_cap_data_struct
{
        __u32 effective;
        __u32 permitted;
        __u32 inheritable;
} __user *cap_user_data_t;

/*能力扩展属性前缀*/
#define XATTR_CAPS_SUFFIX "capability"
/*能力扩展属性名"security.capability"*/
#define XATTR_NAME_CAPS XATTR_SECURITY_PREFIX XATTR_CAPS_SUFFIX
/*能力扩展属性长度小端序12字节，三个位图分别对应不同的权限位*/
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

/*U32类型变量其实是个对应权限的位图，权限按位操作*/
#ifdef STRICT_CAP_T_TYPECHECKS
/*严格权限类型检查，将权限封装为U32类型变量*/
typedef struct kernel_cap_struct
{
	__u32 cap;
} kernel_cap_t;
#else
/*非封装的权限位图*/
typedef __u32 kernel_cap_t;
#endif

#define _USER_CAP_HEADER_SIZE  (2*sizeof(__u32))
#define _KERNEL_CAP_T_SIZE     (sizeof(kernel_cap_t))

#endif

/*在_POSIX_CHOWN_RESTRICTED已定义的情况下，系统允许改变文件所有者和组所有者权限*/
#define CAP_CHOWN					0

/*覆盖所有DAC权限*/
#define CAP_DAC_OVERRIDE     		1

/*更改DAC中所有读和查找文件及目录限制权限，包括已定义的_POSIX_ACLACL限制权限，不含被
CAP_LINUX_IMMUTABLE控制的DAC权限*/
#define CAP_DAC_READ_SEARCH  		2

/* Overrides all restrictions about allowed operations on files, where
   file owner ID must be equal to the user ID, except where CAP_FSETID
   is applicable. It doesn't override MAC and DAC restrictions. */
/**/
#define CAP_FOWNER           		3

/*
(1)当文件置位S_ISUID和S_ISGID时，忽略euid和fsuid是否匹配
(2)当文件置位S_ISGID时，忽略egid(或者任一辅助gid)和fsuid是否匹配
(3)chown(2)成功返回时清除S_ISUID和S_ISGID位（没有实现）
*/
#define CAP_FSETID           		4

/* Used to decide between falling back on the old suser() or fsuser(). */
/**/
#define CAP_FS_MASK					0x1f

/*忽略发送信号进程的uid/euid是否与接受信号进程的uid/euid是否匹配限制*/ 
#define CAP_KILL             		5

/*允许setgid(2)操作；允许setgroup(2)；允许在套接字传递认证过程中假冒gids*/
#define CAP_SETGID           		6

/*允许设置uid(2)操作(包括fsuid)；允许在套接字传递认证过程中假冒pids*/
#define CAP_SETUID           		7

/*Linux特别权限，可对任何进程添加或删除任何已被允许的权限*/
#define CAP_SETPCAP          		8

/*允许修改文件的S_IMMUTABLE和S_AAPEND属性*/
#define CAP_LINUX_IMMUTABLE  		9

/*允许绑定小于1024的TCP/UDP套接字*/
/*允许绑定小于32的ATMVCIs*/
#define CAP_NET_BIND_SERVICE 		10

/*允许广播，监听多播*/
#define CAP_NET_BROADCAST		    11

/*
(1)允许配置接口
(2)允许配置IP防火墙、masquerading（伪装统计信息
(3)允许在套接字上设置调试选项
(4)允许修改路由表
(5)允许将套接字的进程组所有者设置为任意进程
(6)允许传输代理绑定到任何地址
(7)允许设置服务类型
(8)允许设置混杂模式
(9)允许多路广播
(10)允许读或写设备的特殊寄存器
(11)允许ATM控制套接字处于活动状态
*/
#define CAP_NET_ADMIN				12

/*允许使用原始套接字；允许使用套接字包*/
#define CAP_NET_RAW          		13

/*允许锁定一段共享内存、允许调用mlock和mlockall函数(即使和ipc无关)*/
#define CAP_IPC_LOCK         		14

/*允许重载IPC所有权检查*/
#define CAP_IPC_OWNER        		15

/*插入和删除内核模块，无限制修改内核、修改权能集*/
#define CAP_SYS_MODULE       		16

/*允许ioperm/iopl权限、允许发送USB消息到任何虚拟设备 /proc/bus/usb*/
#define CAP_SYS_RAWIO        		17

/*允许使用chroot()*/
#define CAP_SYS_CHROOT       		18

/*允许跟踪任何进程*/
#define CAP_SYS_PTRACE       		19

/*允许配置任何进程统计项*/
#define CAP_SYS_PACCT				20

/*
(1)允许配置安全提醒关键字
(2)允许管理闪存设备
(3)允许测试和配置硬盘配额
(4)允许配置内核系统日志（打印行为）
(5)允许设置域名
(6)允许设置主机名
(7)允许调用bdflash()
(8)允许挂载和取消挂载，建立一个新的smb连接
(9)允许一些自动文件系统ioctrls
(10)允许nfsservctl
(11)允许VM86_REQUEST_IRQ
(12)允许在alph上读写pci配置
(13)允许在mips上irix_prctl(setstacksize)
(14)允许在m68k上调用sys_cacheflush清空所有缓存
(15)允许删除信号量
(16)进程间信号量/消息队列/共享内存使用chown而非CAP_CHOWN
(17)允许锁定或解锁共享内存段
(18)允许开启或关闭交换
(19)允许在套接字传递认证过程中假冒pids
(20)允许在块设备上预读或刷出缓冲区
(21)允许在软盘驱动中设置geometry
(22)允许在xd扩展驱动中打开关闭DMA
(23)允许管理md设备
(24)Allow tuning the ide driver
(25)允许访问nvram设备
(26)允许管理apm_bios/serial/bttv (TV)设备
(27)Allow manufacturer commands in isdn CAPI support driver
(28)允许读取pci配置空间中的非标准化部分
(29)Allow DDI debug ioctl on sbpcd driver
(30)允许设置串行端口
(31)允许发送原始qic-117命令
(32)允许开/关在SCSI控制器和发送任意SCSI命令标记的队列
(33)允许在回环文件系统设置加密密匙
(34)允许设置回收策略域
*/
#define CAP_SYS_ADMIN        		21

/*允许使用reboot()*/
#define CAP_SYS_BOOT         		22

/*
(1)允许提升优先级和在其它不同用户进程设置优先级
(2)Allow use of FIFO and round-robin (realtime) scheduling on own processes and setting
the scheduling algorithm used by another process
(3)允许在其它处理器上设置cpu的亲和性
*/
#define CAP_SYS_NICE      		   23

/*
(1)更改资源限制。设置资源限制
(2)更改配额限制
(3)更改ext2文件系统的预留空间
(4)在ext3文件文件系统（可用日志资源）修改日志模式数据
(5)NOTE: ext2 honors fsuid when checking for resource overrides, so you can override
using fsuid too
(6)更改IPC消息队列大小限制
(7)允许从实时时钟超过64HZ中断
(8)更改控制台分配上的控制台最大数目
(9)更改密匙位图的最大数目
*/

#define CAP_SYS_RESOURCE			24

/*允许操作系统时钟;允许在mips irix_stime;允许设置实时时钟*/
#define CAP_SYS_TIME      		   	25

/*允许配置终端设备；允许终端设备执行虚拟挂断操作*/
#define CAP_SYS_TTY_CONFIG			26

/*允许moknod()方面的特权*/
#define CAP_MKNOD            		27

/*允许修改文件锁标识*/
#define CAP_LEASE            		28

#define CAP_AUDIT_WRITE      		29

#define CAP_AUDIT_CONTROL    		30

#define CAP_SETFCAP	     			31

#ifdef __KERNEL__

/*内核内部函数*/
#ifdef STRICT_CAP_T_TYPECHECKS
/*将权限对应的位图封装*/
#define to_cap_t(x) { x }
/*解封权限位图*/
#define cap_t(x) (x).cap
#else
/*非严格权限类型检查没封装，没变动*/
#define to_cap_t(x) (x)
#define cap_t(x) (x)
#endif

/*清空权限集合*/
#define CAP_EMPTY_SET       to_cap_t(0)
/*设置全权限*/
#define CAP_FULL_SET        to_cap_t(~0)
/*有效权限集合初始化，清除可对任何进程添加或删除的已被允许的权限*/
#define CAP_INIT_EFF_SET to_cap_t(~0 & (~ CAP_TO_MASK(CAP_SETPCAP)))
/*继承权限集合初始化为空*/
#define CAP_INIT_INH_SET to_cap_t(0)
/*权限的权重*/
#define CAP_TO_MASK(x) (1 << (x))
/*提升(添加)指定权限*/
#define cap_raise(c, flag)   (cap_t(c) |=  CAP_TO_MASK(flag))
/*清除指定权限*/
#define cap_lower(c, flag)   (cap_t(c) &= (~ CAP_TO_MASK(flag)))
/*权限集合中是否有指定权限*/
#define cap_raised(c, flag)  (cap_t(c) & CAP_TO_MASK(flag))

/*合并两权限集（按位或操作）*/
static inline kernel_cap_t cap_combine(kernel_cap_t a, kernel_cap_t b)
{
     kernel_cap_t dest;
     cap_t(dest) = cap_t(a) | cap_t(b);
     return dest;
}

/*测试两权限集中是否有相同权限（按位与操作）*/
static inline kernel_cap_t cap_intersect(kernel_cap_t a, kernel_cap_t b)
{
     kernel_cap_t dest;
     cap_t(dest) = cap_t(a) & cap_t(b);
     return dest;
}

/*清除权限集合中的某些权限（按位与非操作）*/
static inline kernel_cap_t cap_drop(kernel_cap_t a, kernel_cap_t drop)
{
     kernel_cap_t dest;
     cap_t(dest) = cap_t(a) & ~ cap_t(drop);
     return dest;
}

/*获取权限集合的补集（非操作）*/
static inline kernel_cap_t cap_invert(kernel_cap_t c)
{
     kernel_cap_t dest;
     cap_t(dest) = ~ cap_t(c);
     return dest;
}

/*测试空权限集*/
#define cap_isclear(c)       (!cap_t(c))
/*测试权限集是否是另一权限集的子集*/
#define cap_issubset(a,set)  (!(cap_t(a) & ~ cap_t(set)))
/*清除权限集*/
#define cap_clear(c)         do { cap_t(c) =  0; } while(0)
/*设置全功能权限*/
#define cap_set_full(c)      do { cap_t(c) = ~0; } while(0)
/*获取两权限交集*/
#define cap_mask(c,mask)     do { cap_t(c) &= cap_t(mask); } while(0)
/**/
#define cap_is_fs_cap(c)     (CAP_TO_MASK(c) & CAP_FS_MASK)
/*判断当前进程是否可以使用超级用户权限*/
int capable(int cap);
/*判断进程是否可以使用超级用户权限*/
int __capable(struct task_struct *t, int cap);

#endif /* __KERNEL__ */

#endif /* !_LINUX_CAPABILITY_H */
