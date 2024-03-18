#include <linux/module.h>
#include <linux/version.h>
#include <linux/nsproxy.h>
#include <linux/init_task.h>
#include <linux/mnt_namespace.h>
#include <linux/utsname.h>
#include <linux/pid_namespace.h>
#include <net/net_namespace.h>

/*�����ռ��Ӧ��slab����*/
static struct kmem_cache *nsproxy_cachep;
/*�������ռ��ʼ��*/
struct nsproxy init_nsproxy = INIT_NSPROXY(init_nsproxy);

/*����һ���µ������ռ䣨�����������ռ丳ֵ�������������ü�������Ϊ1*/
static inline struct nsproxy *clone_nsproxy(struct nsproxy *orig)
{
	struct nsproxy *ns;
	/*Ϊ�����ռ�����slab����*/
	ns = kmem_cache_alloc(nsproxy_cachep, GFP_KERNEL);
	if (ns)
	{
		/*����ɹ����´����������ռ������������ռ��е��������ռ�ָ��ͬһ����ַ*/
		memcpy(ns, orig, sizeof(struct nsproxy));
		/*�����ü�������Ϊ1*/
		atomic_set(&ns->count, 1);
	}
	return ns;
}

/*
 * Create new nsproxy and all of its the associated namespaces.
 * Return the newly created nsproxy.  Do not attach this to the task,
 * leave it to the caller to do proper locking and attach it to task.
 */
 /*���ƽ��������ռ�*/
static struct nsproxy *create_new_namespaces(unsigned long flags,
			struct task_struct *tsk, struct fs_struct *new_fs)
{
	struct nsproxy *new_nsp;
	int err;
	/*������Ľ��������ռ��������ռ�ָ�븳ֵ�´��������ռ�*/
	new_nsp = clone_nsproxy(tsk->nsproxy);
	if (!new_nsp)
	{
		/*�ڴ治�㣬ʧ�ܷ���*/
		return ERR_PTR(-ENOMEM);
	}
	/*����mnt�����ռ�*/
	new_nsp->mnt_ns = copy_mnt_ns(flags, tsk->nsproxy->mnt_ns, new_fs);
	if (IS_ERR(new_nsp->mnt_ns))
	{
		/*ʧ��ʱ��ȡ�����룬Ȼ����ת�˳�*/
		err = PTR_ERR(new_nsp->mnt_ns);
		goto out_ns;
	}
	/*���ƽ���uts�����ռ�*/
	new_nsp->uts_ns = copy_utsname(flags, tsk->nsproxy->uts_ns);
	if (IS_ERR(new_nsp->uts_ns))
	{
		/*ʧ��ʱ��ȡ�����룬Ȼ����ת�˳�*/
		err = PTR_ERR(new_nsp->uts_ns);
		goto out_uts;
	}
	/*���ƽ��̵�ipc�����ռ�*/
	new_nsp->ipc_ns = copy_ipcs(flags, tsk->nsproxy->ipc_ns);
	if (IS_ERR(new_nsp->ipc_ns))
	{
		/*ʧ��ʱ��ȡ�����룬Ȼ����ת�˳�*/
		err = PTR_ERR(new_nsp->ipc_ns);
		goto out_ipc;
	}
	/*��ֵ���̵�pid�����ռ�*/
	new_nsp->pid_ns = copy_pid_ns(flags, task_active_pid_ns(tsk));
	if (IS_ERR(new_nsp->pid_ns))
	{
		/*ʧ��ʱ��ȡ�����룬Ȼ����ת�˳�*/
		err = PTR_ERR(new_nsp->pid_ns);
		goto out_pid;
	}
	/*�����û������ռ�*/
	new_nsp->user_ns = copy_user_ns(flags, tsk->nsproxy->user_ns);
	if (IS_ERR(new_nsp->user_ns))
	{
		/*ʧ��ʱ��ȡ�����룬Ȼ����ת�˳�*/
		err = PTR_ERR(new_nsp->user_ns);
		goto out_user;
	}
	/*����net�����ռ�*/	
	new_nsp->net_ns = copy_net_ns(flags, tsk->nsproxy->net_ns);
	if (IS_ERR(new_nsp->net_ns))
	{
		/*ʧ��ʱ��ȡ�����룬Ȼ����ת�˳�*/
		err = PTR_ERR(new_nsp->net_ns);
		goto out_net;
	}
	/*�����������ռ�ɹ��󷵻�*/
	return new_nsp;

out_net:
	/*����net�����ռ�ʧ�ܣ�������û������ռ������*/
	if (new_nsp->user_ns)
		put_user_ns(new_nsp->user_ns);
out_user:
	/*�����û������ռ�ʧ�ܣ������pid�����ռ������*/
	if (new_nsp->pid_ns)
		put_pid_ns(new_nsp->pid_ns);
out_pid:
	/*����pid�����ռ�ʧ�ܣ������ipc�����ռ������*/
	if (new_nsp->ipc_ns)
		put_ipc_ns(new_nsp->ipc_ns);
out_ipc:
	/*����ipc�����ռ�ʧ�ܣ������uts�����ռ������*/
	if (new_nsp->uts_ns)
		put_uts_ns(new_nsp->uts_ns);
out_uts:
	/*����uts�����ռ�ʧ�ܣ������mnt�����ռ������*/
	if (new_nsp->mnt_ns)
		put_mnt_ns(new_nsp->mnt_ns);
out_ns:
	/*�ͷ�Ϊ�����ռ��������slab����*/
	kmem_cache_free(nsproxy_cachep, new_nsp);
	/*���ش�����Ϣ*/
	return ERR_PTR(err);
}

/*
 * called from clone.  This now handles copy for nsproxy and all
 * namespaces therein.
 */
/*���������ռ�*/
int copy_namespaces(unsigned long flags, struct task_struct *tsk)
{
	/*��ý��̵������ռ�*/
	struct nsproxy *old_ns = tsk->nsproxy;
	struct nsproxy *new_ns;
	int err = 0;
	/*���������ռ䲻���ڣ���������*/
	if (!old_ns)
		return 0;
	/*���ý��̵������ռ�*/
	get_nsproxy(old_ns);
	/*��鸴�Ʊ�ʶ����Ч��*/
	if (!(flags & (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC |
				CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWNET)))
		return 0;
	/*�жϵ�ǰ�����Ƿ��й���ԱȨ�ޣ�û����Ȩ�޲���ʧ��*/
	if (!capable(CAP_SYS_ADMIN))
	{
		/*Ȩ�޲��㣬ʧ����ת*/
		err = -EPERM;
		goto out;
	}
	/*�����µ������ռ�*/
	new_ns = create_new_namespaces(flags, tsk, tsk->fs);
	if (IS_ERR(new_ns))
	{
		/*����ʧ�ܣ���ȡ�����룬��ת����*/
		err = PTR_ERR(new_ns);
		goto out;
	}
	/**/
	err = ns_cgroup_clone(tsk);
	if (err) {
		put_nsproxy(new_ns);
		goto out;
	}
	/*���ý��̵������ռ�*/
	tsk->nsproxy = new_ns;

out:
	/*ȡ���Ծ������ռ������*/
	put_nsproxy(old_ns);
	return err;
}

/*�ͷ������ռ䣬��������д��ڵ��������ռ������*/
void free_nsproxy(struct nsproxy *ns)
{
	/*�������mnt�����ռ�*/	
	if (ns->mnt_ns)
		put_mnt_ns(ns->mnt_ns);
	/*�������uts�����ռ�*/	
	if (ns->uts_ns)
		put_uts_ns(ns->uts_ns);
	/*�������ipc�����ռ�*/	
	if (ns->ipc_ns)
		put_ipc_ns(ns->ipc_ns);
	/*�������pid�����ռ�*/	
	if (ns->pid_ns)
		put_pid_ns(ns->pid_ns);
	/*�������user�����ռ�*/	
	if (ns->user_ns)
		put_user_ns(ns->user_ns);
	/*�������net�����ռ�*/	
	put_net(ns->net_ns);
	/*�ͷ������ռ��Ӧ��slab����*/	
	kmem_cache_free(nsproxy_cachep, ns);
}

/*
 * Called from unshare. Unshare all the namespaces part of nsproxy.
 * On success, returns the new nsproxy.
 */
 /*unshare�������ã�ȡ�������ռ��ж����������ռ��еĹ���*/
int unshare_nsproxy_namespaces(unsigned long unshare_flags,
		struct nsproxy **new_nsp, struct fs_struct *new_fs)
{
	int err = 0;
	/*����ȡ�������ʶ����Ч�ԣ��ñ�ʶ���ܰ���CLONE_NEWPID*/
	if (!(unshare_flags & (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC |
			       CLONE_NEWUSER | CLONE_NEWNET)))
		return 0;
	/*��ϵͳ����ԱȨ��ʱʧ��*/
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	/*�����µ������ռ䣬����ļ�ϵͳ�����ڣ����õ�ǰ���̵��ļ�ϵͳ��ֵ*/
	*new_nsp = create_new_namespaces(unshare_flags, current,
				new_fs ? new_fs : current->fs);
	if (IS_ERR(*new_nsp)) 
	{
		/*����ʧ�ܣ���ȡ�����룬��ת�˳�*/
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

/*���ý��̵������ռ䣬������rcu���ƣ������жԸý���ԭ�����ռ�Ķ����ʽ������ͷŸ������ռ�*/
void switch_task_namespaces(struct task_struct *p, struct nsproxy *new)
{
	struct nsproxy *ns;

	might_sleep();
	/*��ȡ���̵������ռ䣬ǰ������˵Ҫ����rcu���Ʒ�������ָ����*/
	ns = p->nsproxy;
	/*���õ�ǰ���̵������ռ�*/
	rcu_assign_pointer(p->nsproxy, new);

	if (ns && atomic_dec_and_test(&ns->count))
	{
		/*�������ԭ���������ռ������û�б����ã�������rcuͬ�����ƣ��ȴ����ж����ʶ�������
		 �ͷŸ������ռ䡣���ܵ���call_rcu()�������ͷŸ������ռ�������ַ����Ϊput_mnt_ns()
		 ���ܽ���˯��*/
		synchronize_rcu();
		free_nsproxy(ns);
	}
}

/*�����̵������ռ�����Ϊ��*/
void exit_task_namespaces(struct task_struct *p)
{
	switch_task_namespaces(p, NULL);
}

/*��ʼ���ڼ䣬���������ռ��Ӧ��slab����*/
static int __init nsproxy_cache_init(void)
{
	nsproxy_cachep = KMEM_CACHE(nsproxy, SLAB_PANIC);
	return 0;
}

module_init(nsproxy_cache_init);
