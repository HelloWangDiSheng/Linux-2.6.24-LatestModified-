#ifndef _LINUX_PID_NS_H
#define _LINUX_PID_NS_H

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/threads.h>
#include <linux/nsproxy.h>
#include <linux/kref.h>

/*pid位图*/
struct pidmap
{
	/*位图中空闲位数目*/
	atomic_t nr_free;
    /*位图关联的页帧*/
	void *page;
};
/*位图数组项数目*/
#define PIDMAP_ENTRIES         ((PID_MAX_LIMIT + 8*PAGE_SIZE - 1)/PAGE_SIZE/8)
/*pid命名空间*/
struct pid_namespace
{
	/*共享该pid命名空间的进程的数目*/
	struct kref kref;
	/*pid位图*/
	struct pidmap pidmap[PIDMAP_ENTRIES];
	/*上次分配使用的pid，下次分配时会从下一个开始分配*/
	int last_pid;
	/*每个pid命名空间都具有一个进程，其发挥的作用相当于全局的init进程。init进程的一个目的
	是对孤儿进程调用wait4，命名空间局部的init变体也必须完成该工作，child_reaper保存了指向
	该进程的task_struct的指针*/
	struct task_struct *child_reaper;
	/*当前命名空间所在slab缓存的位置*/
	struct kmem_cache *pid_cachep;
	/*当前命名空间在命名空间层次结构中的深度，初始命名空间的level为0，该命名空间的子空间为
	1，下一层的子空间level为2，依次递推。level的计算比较重要，因为level较高的命名空间中的
	ID，对level较低的命名空间来说，是可见的。从给定的level设置，内核即可推断进程会关联到多
	少个ID*/
	int level;
	/*父命名空间的指针*/
	struct pid_namespace *parent;
#ifdef CONFIG_PROC_FS
	/*proc文件系统*/
	struct vfsmount *proc_mnt;
#endif
};

/*初始化pid命名空间*/
extern struct pid_namespace init_pid_ns;


#ifdef CONFIG_PID_NS
/*共享pid命名空间，对非初始化pid命名空间，其引用计数都要自增1*/
static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	if (ns != &init_pid_ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct pid_namespace *copy_pid_ns(unsigned long flags, struct pid_namespace *ns);
extern void free_pid_ns(struct kref *kref);

/*取消对pid命名空间的引用，如果pid命名空间不是初始pid命名空间，则将该pid命名空间的引用
计数自减1*/
static inline void put_pid_ns(struct pid_namespace *ns)
{
	/*取消对非根pid命名空间的引用*/
	if (ns != &init_pid_ns)
		kref_put(&ns->kref, free_pid_ns);
}

#else /* !CONFIG_PID_NS */
#include <linux/err.h>

static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	return ns;
}

static inline struct pid_namespace *
copy_pid_ns(unsigned long flags, struct pid_namespace *ns)
{
	if (flags & CLONE_NEWPID)
		ns = ERR_PTR(-EINVAL);
	return ns;
}

static inline void put_pid_ns(struct pid_namespace *ns)
{
}

#endif /* CONFIG_PID_NS */

/*获取进程的pid命名空间，获取进程的命名空间时要使用rcu机制*/
static inline struct pid_namespace *task_active_pid_ns(struct task_struct *tsk)
{
	return tsk->nsproxy->pid_ns;
}

/*获取当前进程所在命名空间中的类初始化进程*/
static inline struct task_struct *task_child_reaper(struct task_struct *tsk)
{
	BUG_ON(tsk != current);
	return tsk->nsproxy->pid_ns->child_reaper;
}

#endif /* _LINUX_PID_NS_H */
