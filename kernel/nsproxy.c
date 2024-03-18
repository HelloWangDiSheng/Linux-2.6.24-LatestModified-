#include <linux/module.h>
#include <linux/version.h>
#include <linux/nsproxy.h>
#include <linux/init_task.h>
#include <linux/mnt_namespace.h>
#include <linux/utsname.h>
#include <linux/pid_namespace.h>
#include <net/net_namespace.h>

/*命名空间对应的slab缓存*/
static struct kmem_cache *nsproxy_cachep;
/*根命名空间初始化*/
struct nsproxy init_nsproxy = INIT_NSPROXY(init_nsproxy);

/*创建一个新的命名空间（用输入命名空间赋值），并将其引用计数设置为1*/
static inline struct nsproxy *clone_nsproxy(struct nsproxy *orig)
{
	struct nsproxy *ns;
	/*为命名空间申请slab缓存*/
	ns = kmem_cache_alloc(nsproxy_cachep, GFP_KERNEL);
	if (ns)
	{
		/*申请成功后新创建的命名空间与输入命名空间中的子命名空间指向同一个地址*/
		memcpy(ns, orig, sizeof(struct nsproxy));
		/*将引用计数设置为1*/
		atomic_set(&ns->count, 1);
	}
	return ns;
}

/*
 * Create new nsproxy and all of its the associated namespaces.
 * Return the newly created nsproxy.  Do not attach this to the task,
 * leave it to the caller to do proper locking and attach it to task.
 */
 /*复制进程命名空间*/
static struct nsproxy *create_new_namespaces(unsigned long flags,
			struct task_struct *tsk, struct fs_struct *new_fs)
{
	struct nsproxy *new_nsp;
	int err;
	/*用输入的进程命名空间子命名空间指针赋值新创建命名空间*/
	new_nsp = clone_nsproxy(tsk->nsproxy);
	if (!new_nsp)
	{
		/*内存不足，失败返回*/
		return ERR_PTR(-ENOMEM);
	}
	/*复制mnt命名空间*/
	new_nsp->mnt_ns = copy_mnt_ns(flags, tsk->nsproxy->mnt_ns, new_fs);
	if (IS_ERR(new_nsp->mnt_ns))
	{
		/*失败时获取错误码，然后跳转退出*/
		err = PTR_ERR(new_nsp->mnt_ns);
		goto out_ns;
	}
	/*复制进程uts命名空间*/
	new_nsp->uts_ns = copy_utsname(flags, tsk->nsproxy->uts_ns);
	if (IS_ERR(new_nsp->uts_ns))
	{
		/*失败时获取错误码，然后跳转退出*/
		err = PTR_ERR(new_nsp->uts_ns);
		goto out_uts;
	}
	/*复制进程的ipc命名空间*/
	new_nsp->ipc_ns = copy_ipcs(flags, tsk->nsproxy->ipc_ns);
	if (IS_ERR(new_nsp->ipc_ns))
	{
		/*失败时获取错误码，然后跳转退出*/
		err = PTR_ERR(new_nsp->ipc_ns);
		goto out_ipc;
	}
	/*赋值进程的pid命名空间*/
	new_nsp->pid_ns = copy_pid_ns(flags, task_active_pid_ns(tsk));
	if (IS_ERR(new_nsp->pid_ns))
	{
		/*失败时获取错误码，然后跳转退出*/
		err = PTR_ERR(new_nsp->pid_ns);
		goto out_pid;
	}
	/*复制用户命名空间*/
	new_nsp->user_ns = copy_user_ns(flags, tsk->nsproxy->user_ns);
	if (IS_ERR(new_nsp->user_ns))
	{
		/*失败时获取错误码，然后跳转退出*/
		err = PTR_ERR(new_nsp->user_ns);
		goto out_user;
	}
	/*复制net命名空间*/	
	new_nsp->net_ns = copy_net_ns(flags, tsk->nsproxy->net_ns);
	if (IS_ERR(new_nsp->net_ns))
	{
		/*失败时获取错误码，然后跳转退出*/
		err = PTR_ERR(new_nsp->net_ns);
		goto out_net;
	}
	/*创建新命名空间成功后返回*/
	return new_nsp;

out_net:
	/*创建net命名空间失败，解除对用户命名空间的引用*/
	if (new_nsp->user_ns)
		put_user_ns(new_nsp->user_ns);
out_user:
	/*创建用户命名空间失败，解除对pid命名空间的引用*/
	if (new_nsp->pid_ns)
		put_pid_ns(new_nsp->pid_ns);
out_pid:
	/*创建pid命名空间失败，解除对ipc命名空间的引用*/
	if (new_nsp->ipc_ns)
		put_ipc_ns(new_nsp->ipc_ns);
out_ipc:
	/*创建ipc命名空间失败，解除对uts命名空间的引用*/
	if (new_nsp->uts_ns)
		put_uts_ns(new_nsp->uts_ns);
out_uts:
	/*创建uts命名空间失败，解除对mnt命名空间的引用*/
	if (new_nsp->mnt_ns)
		put_mnt_ns(new_nsp->mnt_ns);
out_ns:
	/*释放为命名空间已申请的slab缓存*/
	kmem_cache_free(nsproxy_cachep, new_nsp);
	/*返回错误信息*/
	return ERR_PTR(err);
}

/*
 * called from clone.  This now handles copy for nsproxy and all
 * namespaces therein.
 */
/*复制命名空间*/
int copy_namespaces(unsigned long flags, struct task_struct *tsk)
{
	/*获得进程的命名空间*/
	struct nsproxy *old_ns = tsk->nsproxy;
	struct nsproxy *new_ns;
	int err = 0;
	/*进程命名空间不存在，结束复制*/
	if (!old_ns)
		return 0;
	/*引用进程的命名空间*/
	get_nsproxy(old_ns);
	/*检查复制标识的有效性*/
	if (!(flags & (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC |
				CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWNET)))
		return 0;
	/*判断当前进程是否有管理员权限，没有则权限不足失败*/
	if (!capable(CAP_SYS_ADMIN))
	{
		/*权限不足，失败跳转*/
		err = -EPERM;
		goto out;
	}
	/*创建新的命名空间*/
	new_ns = create_new_namespaces(flags, tsk, tsk->fs);
	if (IS_ERR(new_ns))
	{
		/*创建失败，获取错误码，跳转返回*/
		err = PTR_ERR(new_ns);
		goto out;
	}
	/**/
	err = ns_cgroup_clone(tsk);
	if (err) {
		put_nsproxy(new_ns);
		goto out;
	}
	/*重置进程的命名空间*/
	tsk->nsproxy = new_ns;

out:
	/*取消对旧命名空间的引用*/
	put_nsproxy(old_ns);
	return err;
}

/*释放命名空间，解除对其中存在的子命名空间的引用*/
void free_nsproxy(struct nsproxy *ns)
{
	/*解除引用mnt命名空间*/	
	if (ns->mnt_ns)
		put_mnt_ns(ns->mnt_ns);
	/*解除引用uts命名空间*/	
	if (ns->uts_ns)
		put_uts_ns(ns->uts_ns);
	/*解除引用ipc命名空间*/	
	if (ns->ipc_ns)
		put_ipc_ns(ns->ipc_ns);
	/*解除引用pid命名空间*/	
	if (ns->pid_ns)
		put_pid_ns(ns->pid_ns);
	/*解除引用user命名空间*/	
	if (ns->user_ns)
		put_user_ns(ns->user_ns);
	/*解除引用net命名空间*/	
	put_net(ns->net_ns);
	/*释放命名空间对应的slab缓存*/	
	kmem_cache_free(nsproxy_cachep, ns);
}

/*
 * Called from unshare. Unshare all the namespaces part of nsproxy.
 * On success, returns the new nsproxy.
 */
 /*unshare函数调用，取消命名空间中多有子命名空间中的共享*/
int unshare_nsproxy_namespaces(unsigned long unshare_flags,
		struct nsproxy **new_nsp, struct fs_struct *new_fs)
{
	int err = 0;
	/*检验取消共享标识的有效性，该标识不能包含CLONE_NEWPID*/
	if (!(unshare_flags & (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC |
			       CLONE_NEWUSER | CLONE_NEWNET)))
		return 0;
	/*无系统管理员权限时失败*/
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	/*创建新的命名空间，如果文件系统不存在，则用当前进程的文件系统赋值*/
	*new_nsp = create_new_namespaces(unshare_flags, current,
				new_fs ? new_fs : current->fs);
	if (IS_ERR(*new_nsp)) 
	{
		/*创建失败，获取错误码，跳转退出*/
		err = PTR_ERR(*new_nsp);
		goto out;
	}
	/**/
	err = ns_cgroup_clone(current);
	if (err)
		put_nsproxy(*new_nsp);

out:
	return err;
}

/*重置进程的命名空间，并利用rcu机制，等所有对该进程原命名空间的读访问结束后，释放该命名空间*/
void switch_task_namespaces(struct task_struct *p, struct nsproxy *new)
{
	struct nsproxy *ns;

	might_sleep();
	/*获取进程的命名空间，前述不是说要利用rcu机制反引用其指针吗？*/
	ns = p->nsproxy;
	/*重置当前进程的命名空间*/
	rcu_assign_pointer(p->nsproxy, new);

	if (ns && atomic_dec_and_test(&ns->count))
	{
		/*如果进程原来的命名空间存在且没有被引用，则利用rcu同步机制，等待所有读访问都结束后
		 释放该命名空间。不能调用call_rcu()函数来释放该命名空间的虚拟地址，因为put_mnt_ns()
		 可能进入睡眠*/
		synchronize_rcu();
		free_nsproxy(ns);
	}
}

/*将进程的命名空间设置为空*/
void exit_task_namespaces(struct task_struct *p)
{
	switch_task_namespaces(p, NULL);
}

/*初始化期间，创建命名空间对应的slab缓存*/
static int __init nsproxy_cache_init(void)
{
	nsproxy_cachep = KMEM_CACHE(nsproxy, SLAB_PANIC);
	return 0;
}

module_init(nsproxy_cache_init);
