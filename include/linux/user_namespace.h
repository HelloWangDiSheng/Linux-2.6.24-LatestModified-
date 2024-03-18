#ifndef _LINUX_USER_NAMESPACE_H
#define _LINUX_USER_NAMESPACE_H

#include <linux/kref.h>
#include <linux/nsproxy.h>
#include <linux/sched.h>
#include <linux/err.h>

/*�û�ɢ�б�����ƫ��λ*/
#define UIDHASH_BITS	(CONFIG_BASE_SMALL ? 3 : 8)
/*�û�ɢ�б�������Ŀ*/
#define UIDHASH_SZ	(1 << UIDHASH_BITS)

/*������������ÿ���û���Դʹ�õ���Ϣ*/
struct user_namespace
{
	/*���������ڸ��ٶ��ٸ��ط�ʹ���˸��û������ռ�ʵ��*/
	struct kref		kref;
	/*�û������ռ�ɢ�б�ͷ����ѯ�����û�ʵ��*/
	struct hlist_head	uidhash_table[UIDHASH_SZ];
	/*�������ռ��е�ÿ���û�������һ��user_structʵ�������¼����Դ�����ģ�����ʵ����
	ͨ��ɢ�б�uidhash_table����*/
	struct user_struct	*root_user;
};

/*�������ռ����û������ռ�*/
extern struct user_namespace init_user_ns;

#ifdef CONFIG_USER_NS
/*�����û������ռ�*/
static inline struct user_namespace *get_user_ns(struct user_namespace *ns)
{
	/*�û������ռ�ǿ�ʱ���������ü�������1*/
	if (ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct user_namespace *copy_user_ns(int flags, struct user_namespace *old_ns);
extern void free_user_ns(struct kref *kref);

/*ȡ�����û������ռ�����ã������û������ռ����ü�������1������ü�����Ϊ0�����ͷ�
���û������ռ�*/
static inline void put_user_ns(struct user_namespace *ns)
{
	if (ns)
		kref_put(&ns->kref, free_user_ns);
}

#else

static inline struct user_namespace *get_user_ns(struct user_namespace *ns)
{
	return &init_user_ns;
}

static inline struct user_namespace *copy_user_ns(int flags,
						  struct user_namespace *old_ns)
{
	if (flags & CLONE_NEWUSER)
		return ERR_PTR(-EINVAL);

	return old_ns;
}

static inline void put_user_ns(struct user_namespace *ns)
{
}

#endif

#endif /* _LINUX_USER_H */
