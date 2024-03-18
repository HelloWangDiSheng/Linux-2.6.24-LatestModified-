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

/*���û������ռ��ʼ��*/
struct user_namespace init_user_ns = 
{
	.kref = 
	{
		.refcount	= ATOMIC_INIT(2),
	},
	/*��ʼ��Ϊ���û�*/
	.root_user = &root_user,
};

EXPORT_SYMBOL_GPL(init_user_ns);

#ifdef CONFIG_USER_NS

/*
 * Clone a new ns copying an original user ns, setting refcount to 1
 * @old_ns: namespace to clone
 * Return NULL on error (failure to kmalloc), new ns otherwise
 */
/*ÿ���û������ռ�����û���Դʹ�õ�ͳ�ƣ������������ռ���ȫ�޹أ���root�û���ͳ��Ҳ��
��ˣ������Ϊ��cloneһ���û������ռ�ʱ��Ϊ��ǰ�û���root���������µ�user_structʵ��*/
/*����һ���µ��û������ռ䣬���丳ֵΪԴ�û������ռ���Ϣ����ʼ�������ü���*/
static struct user_namespace *clone_user_ns(struct user_namespace *old_ns)
{
	struct user_namespace *ns;
	struct user_struct *new_user;
	int n;
	/*Ϊ�û������ռ������ڴ�*/
	ns = kmalloc(sizeof(struct user_namespace), GFP_KERNEL);
	/*�ڴ治�㣬ʧ�ܷ���*/
	if (!ns)
		return ERR_PTR(-ENOMEM);
	/*��ʼ�����û������ռ����ü���*/
	kref_init(&ns->kref);
	/*��ʼ���û������ռ���ɢ�б�ͷ*/
	for (n = 0; n < UIDHASH_SZ; ++n)
		INIT_HLIST_HEAD(ns->uidhash_table + n);

	/* Insert new root user.  */
	/*�������û������ռ�ĸ��û�*/
	ns->root_user = alloc_uid(ns, 0);
	if (!ns->root_user)
	{
		/*���û�����ʧ�ܣ��ͷ���������û������ռ䣬�����ڴ治�������Ϣ*/
		kfree(ns);
		return ERR_PTR(-ENOMEM);
	}

	/* Reset current->user with a new one */
	/*Ϊ��ǰ�����������û����´����������ռ��д�����һ���û���Ϣ*/
	new_user = alloc_uid(ns, current->uid);
	if (!new_user)
	{
		/*����ʧ�����ͷ�ǰ����Ϊ�������ռ���û����������ڴ棬�����ڴ治�������Ϣ*/
		free_uid(ns->root_user);
		kfree(ns);
		return ERR_PTR(-ENOMEM);
	}
	/*ȷ�������ڿ�ʼ�����µ�user_structʵ��������Դͳ�ƣ�ʵ���Ͼ��ǽ�task_struct��user��Ա
	ָ���µ�user_structʵ��*/
	switch_uid(new_user);
	return ns;
}

/*�����û������ռ�*/
struct user_namespace * copy_user_ns(int flags, struct user_namespace *old_ns)
{
	struct user_namespace *new_ns;
	/*Ԥ���Ƶ��û������ռ䲻��Ϊ��*/
	BUG_ON(!old_ns);
	/*����Ԥ���Ƶ��û������ռ�*/
	get_user_ns(old_ns);
	/*����ǹ����û������ռ䣬ֱ�ӷ���*/
	if (!(flags & CLONE_NEWUSER))
		return old_ns;
	/*�����û������ռ�*/
	new_ns = clone_user_ns(old_ns);
	/*������Ѹ��������ռ������*/
	put_user_ns(old_ns);
	return new_ns;
}

/*�ͷ��û������ռ�*/
void free_user_ns(struct kref *kref)
{
	struct user_namespace *ns;
	/*����container_of���ƻ�ȡ��Ӧ���û������ռ�*/
	ns = container_of(kref, struct user_namespace, kref);
	/**/
	release_uids(ns);
	kfree(ns);
}

#endif /* CONFIG_USER_NS */
