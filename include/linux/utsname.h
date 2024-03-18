#ifndef _LINUX_UTSNAME_H
#define _LINUX_UTSNAME_H

#define __OLD_UTS_LEN 8

struct oldold_utsname {
	char sysname[9];
	char nodename[9];
	char release[9];
	char version[9];
	char machine[9];
};

#define __NEW_UTS_LEN 64

struct old_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
};

/*包含系统相关信息的uts命名空间*/
struct new_utsname
{
	/*系统类型*/
	char sysname[65];
	/*结点名*/
	char nodename[65];
	/*发行号*/
	char release[65];
	/*版本号*/
	char version[65];
	/*低层平台名，如i386、arm、spac、powerpc等*/
	char machine[65];
	/*域名*/
	char domainname[65];
};

#ifdef __KERNEL__

#include <linux/sched.h>
#include <linux/kref.h>
#include <linux/nsproxy.h>
#include <asm/atomic.h>

/*uts命名空间*/
struct uts_namespace
{
	/*包含该命名空间被共享的次数*/
	struct kref kref;
	/*uts相关信息*/
	struct new_utsname name;
};

/*初始化命名空间uts*/
extern struct uts_namespace init_uts_ns;

/*引用uts命名空间*/
static inline void get_uts_ns(struct uts_namespace *ns)
{
	kref_get(&ns->kref);
}

extern struct uts_namespace *copy_utsname(unsigned long flags,
					struct uts_namespace *ns);
extern void free_uts_ns(struct kref *kref);

/*取消对uts命名空间的引用，如果uts命名空间没有被引用，则释放其所占用的内存空间*/
static inline void put_uts_ns(struct uts_namespace *ns)
{
	kref_put(&ns->kref, free_uts_ns);
}

/*获取当前进程的uts命名空间*/
static inline struct new_utsname *utsname(void)
{
	return &current->nsproxy->uts_ns->name;
}

/*获取根uts命名空间*/
static inline struct new_utsname *init_utsname(void)
{
	return &init_uts_ns.name;
}、
/*声明一个保护uts命名空间的读写信号量*/
extern struct rw_semaphore uts_sem;

#endif /* __KERNEL__ */

#endif /* _LINUX_UTSNAME_H */
