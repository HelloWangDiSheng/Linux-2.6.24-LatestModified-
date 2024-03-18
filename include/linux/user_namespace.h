#ifndef _LINUX_USER_NAMESPACE_H
#define _LINUX_USER_NAMESPACE_H

#include <linux/kref.h>
#include <linux/nsproxy.h>
#include <linux/sched.h>
#include <linux/err.h>

/*用户散列表索引偏移位*/
#define UIDHASH_BITS	(CONFIG_BASE_SMALL ? 3 : 8)
/*用户散列表索引数目*/
#define UIDHASH_SZ	(1 << UIDHASH_BITS)

/*保存用于限制每个用户资源使用的信息*/
struct user_namespace
{
	/*计数器用于跟踪多少个地方使用了该用户命名空间实例*/
	struct kref		kref;
	/*用户命名空间散列表头，查询各个用户实例*/
	struct hlist_head	uidhash_table[UIDHASH_SZ];
	/*对命名空间中的每个用户，都有一个user_struct实例负责记录其资源的消耗，各个实例可
	通过散列表uidhash_table访问*/
	struct user_struct	*root_user;
};

/*根命名空间中用户命名空间*/
extern struct user_namespace init_user_ns;

#ifdef CONFIG_USER_NS
/*引用用户命名空间*/
static inline struct user_namespace *get_user_ns(struct user_namespace *ns)
{
	/*用户命名空间非空时，将其引用计数器加1*/
	if (ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct user_namespace *copy_user_ns(int flags, struct user_namespace *old_ns);
extern void free_user_ns(struct kref *kref);

/*取消对用户命名空间的引用，将该用户命名空间引用计数器减1，如果该计数器为0，则释放
该用户命名空间*/
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
