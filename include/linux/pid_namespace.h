#ifndef _LINUX_PID_NS_H
#define _LINUX_PID_NS_H

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/threads.h>
#include <linux/nsproxy.h>
#include <linux/kref.h>

/*pidλͼ*/
struct pidmap
{
	/*λͼ�п���λ��Ŀ*/
	atomic_t nr_free;
    /*λͼ������ҳ֡*/
	void *page;
};
/*λͼ��������Ŀ*/
#define PIDMAP_ENTRIES         ((PID_MAX_LIMIT + 8*PAGE_SIZE - 1)/PAGE_SIZE/8)
/*pid�����ռ�*/
struct pid_namespace
{
	/*�����pid�����ռ�Ľ��̵���Ŀ*/
	struct kref kref;
	/*pidλͼ*/
	struct pidmap pidmap[PIDMAP_ENTRIES];
	/*�ϴη���ʹ�õ�pid���´η���ʱ�����һ����ʼ����*/
	int last_pid;
	/*ÿ��pid�����ռ䶼����һ�����̣��䷢�ӵ������൱��ȫ�ֵ�init���̡�init���̵�һ��Ŀ��
	�ǶԹ¶����̵���wait4�������ռ�ֲ���init����Ҳ������ɸù�����child_reaper������ָ��
	�ý��̵�task_struct��ָ��*/
	struct task_struct *child_reaper;
	/*��ǰ�����ռ�����slab�����λ��*/
	struct kmem_cache *pid_cachep;
	/*��ǰ�����ռ��������ռ��νṹ�е���ȣ���ʼ�����ռ��levelΪ0���������ռ���ӿռ�Ϊ
	1����һ����ӿռ�levelΪ2�����ε��ơ�level�ļ���Ƚ���Ҫ����Ϊlevel�ϸߵ������ռ��е�
	ID����level�ϵ͵������ռ���˵���ǿɼ��ġ��Ӹ�����level���ã��ں˼����ƶϽ��̻��������
	�ٸ�ID*/
	int level;
	/*�������ռ��ָ��*/
	struct pid_namespace *parent;
#ifdef CONFIG_PROC_FS
	/*proc�ļ�ϵͳ*/
	struct vfsmount *proc_mnt;
#endif
};

/*��ʼ��pid�����ռ�*/
extern struct pid_namespace init_pid_ns;


#ifdef CONFIG_PID_NS
/*����pid�����ռ䣬�Էǳ�ʼ��pid�����ռ䣬�����ü�����Ҫ����1*/
static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	if (ns != &init_pid_ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct pid_namespace *copy_pid_ns(unsigned long flags, struct pid_namespace *ns);
extern void free_pid_ns(struct kref *kref);

/*ȡ����pid�����ռ�����ã����pid�����ռ䲻�ǳ�ʼpid�����ռ䣬�򽫸�pid�����ռ������
�����Լ�1*/
static inline void put_pid_ns(struct pid_namespace *ns)
{
	/*ȡ���ԷǸ�pid�����ռ������*/
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

/*��ȡ���̵�pid�����ռ䣬��ȡ���̵������ռ�ʱҪʹ��rcu����*/
static inline struct pid_namespace *task_active_pid_ns(struct task_struct *tsk)
{
	return tsk->nsproxy->pid_ns;
}

/*��ȡ��ǰ�������������ռ��е����ʼ������*/
static inline struct task_struct *task_child_reaper(struct task_struct *tsk)
{
	BUG_ON(tsk != current);
	return tsk->nsproxy->pid_ns->child_reaper;
}

#endif /* _LINUX_PID_NS_H */
