/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, version 2 of the
 *  License.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/nsproxy.h>
#include <linux/user_namespace.h>

/*根用户命名空间初始化*/
struct user_namespace init_user_ns = 
{
	.kref = 
	{
		.refcount	= ATOMIC_INIT(2),
	},
	/*初始化为根用户*/
	.root_user = &root_user,
};

EXPORT_SYMBOL_GPL(init_user_ns);

#ifdef CONFIG_USER_NS

/*
 * Clone a new ns copying an original user ns, setting refcount to 1
 * @old_ns: namespace to clone
 * Return NULL on error (failure to kmalloc), new ns otherwise
 */
/*每个用户命名空间对其用户资源使用的统计，与其它命名空间完全无关，对root用户的统计也是
如此，这个因为在clone一个用户命名空间时，为当前用户和root都创建了新的user_struct实例*/
/*申请一个新的用户命名空间，将其赋值为源用户命名空间信息，初始化其引用计数*/
static struct user_namespace *clone_user_ns(struct user_namespace *old_ns)
{
	struct user_namespace *ns;
	struct user_struct *new_user;
	int n;
	/*为用户命名空间申请内存*/
	ns = kmalloc(sizeof(struct user_namespace), GFP_KERNEL);
	/*内存不足，失败返回*/
	if (!ns)
		return ERR_PTR(-ENOMEM);
	/*初始化新用户命名空间引用计数*/
	kref_init(&ns->kref);
	/*初始化用户命名空间中散列表头*/
	for (n = 0; n < UIDHASH_SZ; ++n)
		INIT_HLIST_HEAD(ns->uidhash_table + n);

	/* Insert new root user.  */
	/*创建该用户命名空间的根用户*/
	ns->root_user = alloc_uid(ns, 0);
	if (!ns->root_user)
	{
		/*根用户创建失败，释放已申请的用户命名空间，返回内存不足错误信息*/
		kfree(ns);
		return ERR_PTR(-ENOMEM);
	}

	/* Reset current->user with a new one */
	/*为当前进程所属的用户在新创建的命名空间中串创建一个用户信息*/
	new_user = alloc_uid(ns, current->uid);
	if (!new_user)
	{
		/*创建失败则释放前述已为该命名空间根用户申请的相关内存，返回内存不足错误信息*/
		free_uid(ns->root_user);
		kfree(ns);
		return ERR_PTR(-ENOMEM);
	}
	/*确保从现在开始，将新的user_struct实例用于资源统计，实质上就是将task_struct的user成员
	指向新的user_struct实例*/
	switch_uid(new_user);
	return ns;
}

/*复制用户命名空间*/
struct user_namespace * copy_user_ns(int flags, struct user_namespace *old_ns)
{
	struct user_namespace *new_ns;
	/*预复制的用户命名空间不能为空*/
	BUG_ON(!old_ns);
	/*引用预复制的用户命名空间*/
	get_user_ns(old_ns);
	/*如果是共享用户命名空间，直接返回*/
	if (!(flags & CLONE_NEWUSER))
		return old_ns;
	/*复制用户命名空间*/
	new_ns = clone_user_ns(old_ns);
	/*解除对已复制命名空间的引用*/
	put_user_ns(old_ns);
	return new_ns;
}

/*释放用户命名空间*/
void free_user_ns(struct kref *kref)
{
	struct user_namespace *ns;
	/*利用container_of机制获取对应的用户命名空间*/
	ns = container_of(kref, struct user_namespace, kref);
	/**/
	release_uids(ns);
	kfree(ns);
}

#endif /* CONFIG_USER_NS */
