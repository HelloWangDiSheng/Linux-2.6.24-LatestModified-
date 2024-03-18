#ifndef _LINUX_NSPROXY_H
#define _LINUX_NSPROXY_H

#include <linux/spinlock.h>
#include <linux/sched.h>

struct mnt_namespace;
struct uts_namespace;
struct ipc_namespace;
struct pid_namespace;

/*	 命名空间访问规则：
	（1）仅当前进程可以修改其命名空间指针或其命名空间中的任一指针。
    （2）当访问当前进程的命名空间时应使用RCU机制。
    （3）当访问其它进程的命名空间时，需要在rcu_read_lock()和rcu_read_unlock()保护下进行，
命名空间不存在意味着进程处于僵尸状态，将近死亡。
*/

 /*每个进程都关联到一个选定的命名空间。每个可以感知命名空间的内核子系统都必须提供一个
 数据结构，将所有通过命名空间形式提供的对象集中起来。struct nsproxy用于汇集指向特定于
 子系统的命名空间包装器的指针*/
struct nsproxy
{
	/*命名空间被进程共享的数目*/
	atomic_t count;
	/*uts(Unix Timesharing System)命名空间包含了内核的名称、版本、低层体系结构类型等信息*/
	struct uts_namespace *uts_ns;
	/*保存所有与进程间通信(ipc有关的信息)*/
	struct ipc_namespace *ipc_ns;
	/*保存已装载的文件系统的视图*/
	struct mnt_namespace *mnt_ns;
	/*有关进程ID的信息*/
	struct pid_namespace *pid_ns;
	/*保存用于限制每个用户资源使用的信息*/
	struct user_namespace *user_ns;
	/*网络相关的命名空间参数*/
	struct net *net_ns;
};

/*声明初始化进程的命名空间*/
extern struct nsproxy init_nsproxy;

/*命名空间访问规则：（1）仅当前进程可以修改其命名空间指针或其命名空间中的任一指针。
（2）当访问当前进程的命名空间时应使用RCU机制。
（3）当访问其它进程的命名空间时，需要在rcu_read_lock()和rcu_read_unlock()保护下进行，
命名空间不存在意味着进程处于僵尸状态，将近死亡。
（4）*/

/*通过rcu机制获取进程的命名空间*/
static inline struct nsproxy *task_nsproxy(struct task_struct *tsk)
{
	return rcu_dereference(tsk->nsproxy);
}

int copy_namespaces(unsigned long flags, struct task_struct *tsk);
void exit_task_namespaces(struct task_struct *tsk);
void switch_task_namespaces(struct task_struct *tsk, struct nsproxy *new);
void free_nsproxy(struct nsproxy *ns);
int unshare_nsproxy_namespaces(unsigned long, struct nsproxy **,
	struct fs_struct *);

/*取消对命名空间的引用，如果命名空间没有被引用，则释放该命名空间*/
static inline void put_nsproxy(struct nsproxy *ns)
{
	if (atomic_dec_and_test(&ns->count))
	{
		/*命名空间没有被引用，则释放该命名空间*/
		free_nsproxy(ns);
	}
}

/*引用命名空间*/
static inline void get_nsproxy(struct nsproxy *ns)
{
	atomic_inc(&ns->count);
}

#ifdef CONFIG_CGROUP_NS
int ns_cgroup_clone(struct task_struct *tsk);
#else
static inline int ns_cgroup_clone(struct task_struct *tsk) { return 0; }
#endif

#endif
