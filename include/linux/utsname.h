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

/*����ϵͳ�����Ϣ��uts�����ռ�*/
struct new_utsname
{
	/*ϵͳ����*/
	char sysname[65];
	/*�����*/
	char nodename[65];
	/*���к�*/
	char release[65];
	/*�汾��*/
	char version[65];
	/*�Ͳ�ƽ̨������i386��arm��spac��powerpc��*/
	char machine[65];
	/*����*/
	char domainname[65];
};

#ifdef __KERNEL__

#include <linux/sched.h>
#include <linux/kref.h>
#include <linux/nsproxy.h>
#include <asm/atomic.h>

/*uts�����ռ�*/
struct uts_namespace
{
	/*�����������ռ䱻����Ĵ���*/
	struct kref kref;
	/*uts�����Ϣ*/
	struct new_utsname name;
};

/*��ʼ�������ռ�uts*/
extern struct uts_namespace init_uts_ns;

/*����uts�����ռ�*/
static inline void get_uts_ns(struct uts_namespace *ns)
{
	kref_get(&ns->kref);
}

extern struct uts_namespace *copy_utsname(unsigned long flags,
					struct uts_namespace *ns);
extern void free_uts_ns(struct kref *kref);

/*ȡ����uts�����ռ�����ã����uts�����ռ�û�б����ã����ͷ�����ռ�õ��ڴ�ռ�*/
static inline void put_uts_ns(struct uts_namespace *ns)
{
	kref_put(&ns->kref, free_uts_ns);
}

/*��ȡ��ǰ���̵�uts�����ռ�*/
static inline struct new_utsname *utsname(void)
{
	return &current->nsproxy->uts_ns->name;
}

/*��ȡ��uts�����ռ�*/
static inline struct new_utsname *init_utsname(void)
{
	return &init_uts_ns.name;
}��
/*����һ������uts�����ռ�Ķ�д�ź���*/
extern struct rw_semaphore uts_sem;

#endif /* __KERNEL__ */

#endif /* _LINUX_UTSNAME_H */
