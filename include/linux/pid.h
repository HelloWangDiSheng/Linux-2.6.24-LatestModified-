#ifndef _LINUX_PID_H
#define _LINUX_PID_H

#include <linux/rcupdate.h>

/*ÿ�����̳���pid�������ֵ֮�⣬����������id�������м��п��ܵ����ͣ�
��1������ĳ���߳��飨��һ�������У��Ա�ʶCLONE_THREAD������clone�����ĸý��̵Ĳ�ͬ��
ִ�������ģ��е����н��̶���һ��ͳһ���߳���ID��TGID�����������û��ʹ���̣߳���PID��
TGID��ͬ���߳����е������̱���Ϊ�鳤��group_leader����ͨ��clone�����Ķ����̵߳�struct
 task_struct��group_leader��Ա����ָ���鳤��struct task_structʵ����
��2���������̿��Ժϲ�Ϊ�����飨ʹ��setgprpϵͳ���ã����������Ա��struct task_struct��
pgrp����ֵ������ͬ�ģ����������鳤��PID�������������������г�Ա�����źŵĲ�������
���ڸ���ϵͳ�������Ӧ�������õģ�ע�⣡�ùܵ����ӵĽ��̰�����ͬһ����������
��3��������������Ժϲ���һ���Ự���Ự�е����н��̶���ͬ���ĻỰID��������task_struct
��session��Ա�У�SID����ʹ��setsidϵͳ��������*/

enum pid_type
{
	/*����ID*/
	PIDTYPE_PID,
	/*������ID*/
	PIDTYPE_PGID,
	/*�ỰID*/
	PIDTYPE_SID,
	/*ID������Ŀ*/
	PIDTYPE_MAX
};

/*
 * What is struct pid?
 *
 * A struct pid is the kernel's internal notion of a process identifier.
 * It refers to individual tasks, process groups, and sessions.  While
 * there are processes attached to it the struct pid lives in a hash
 * table, so it and then the processes that it refers to can be found
 * quickly from the numeric pid value.  The attached processes may be
 * quickly accessed by following pointers from struct pid.
 *
 * Storing pid_t values in the kernel and refering to them later has a
 * problem.  The process originally with that pid may have exited and the
 * pid allocator wrapped, and another process could have come along
 * and been assigned that pid.
 *
 * Referring to user space processes by holding a reference to struct
 * task_struct has a problem.  When the user space process exits
 * the now useless task_struct is still kept.  A task_struct plus a
 * stack consumes around 10K of low kernel memory.  More precisely
 * this is THREAD_SIZE + sizeof(struct task_struct).  By comparison
 * a struct pid is about 64 bytes.
 *
 * Holding a reference to struct pid solves both of these problems.
 * It is small so holding a reference does not consume a lot of
 * resources, and since a new struct pid is allocated when the numeric pid
 * value is reused (when pids wrap around) we don't mistakenly refer to new
 * processes.
 */


/*
 * struct upid is used to get the id of the struct pid, as it is
 * seen in particular namespace. Later the struct pid is found with
 * find_pid_ns() using the int nr and struct pid_namespace *ns.
 */
 
/*�ض������ռ��пɼ�����Ϣ*/
struct upid
{
	/*Ϊ��find_pid���Ҹ�Ч�������ý��ֲ̾�pid��pid_chain������һ��������*/
	/*���ֲ̾�pid���*/
	int nr;
	/*�ֲ������ռ�*/
	struct pid_namespace *ns;
	/*���е�upidʵ����������һ��ɢ�б���*/
	struct hlist_node pid_chain;
};

/*�ں˶�pid���ڲ���ʾ*/
struct pid
{
	/*���ü�����*/
	atomic_t count;
	/*�������һ��ɢ�б���ͷ����Ӧ��һ��ID���͡����������б�Ҫ�ģ���Ϊһ��ID����
	���ڼ������̣����й���ͬһ����ID��task_structʵ������ͨ����������������*/
	struct hlist_head tasks[PIDTYPE_MAX];
	/*����rcu���Ʋ���*/
	struct rcu_head rcu;
	/*���Կ����ý��̵������ռ����Ŀ���������ý��̵������ռ��������ռ��νṹ�е����*/
	int level;
	/*������ÿһ���Ӧ��һ�������ռ䣬ע�⣡��������ʽ��ֻ��һ����������һ������ֻ
	������ȫ�������ռ��У���ôȷʵ��ˣ����ڸ�����λ�ڽṹ��ĩβ����ˣ�ֻҪ��������ڴ�
	�ռ䣬�������������Ӹ��ӵ���*/
	struct upid numbers[1];
};

/*��ʼ��pid�����ռ�*/
extern struct pid init_struct_pid;

/*�������й���ͬһID��task_structʵ���������̴洢��һ��ɢ�б��й�����ˣ���Ҫ��task_struct
������һ��ɢ��Ԫ�ء�pid_link��������pid��pidɢ�б�����ϵ*/
struct pid_link
{
	/*ɢ��Ԫ��*/
	struct hlist_node node;
	/*pid������pid�ṹʵ��*/
	struct pid *pid;
};

/*����pidʵ��*/
static inline struct pid *get_pid(struct pid *pid)
{
	/*pid�ǿ�ʱ�������ü�������1*/
	if (pid)
		atomic_inc(&pid->count);
	return pid;
}

extern void FASTCALL(put_pid(struct pid *pid));
extern struct task_struct *FASTCALL(pid_task(struct pid *pid, enum pid_type));
extern struct task_struct *FASTCALL(get_pid_task(struct pid *pid,
						enum pid_type));

extern struct pid *get_task_pid(struct task_struct *task, enum pid_type type);

/*
 * attach_pid() and detach_pid() must be called with the tasklist_lock
 * write-held.
 */
extern int FASTCALL(attach_pid(struct task_struct *task,
				enum pid_type type, struct pid *pid));
extern void FASTCALL(detach_pid(struct task_struct *task, enum pid_type));
extern void FASTCALL(transfer_pid(struct task_struct *old,
				  struct task_struct *new, enum pid_type));

struct pid_namespace;
extern struct pid_namespace init_pid_ns;

/*
 * look up a PID in the hash table. Must be called with the tasklist_lock
 * or rcu_read_lock() held.
 *
 * find_pid_ns() finds the pid in the namespace specified
 * find_pid() find the pid by its global id, i.e. in the init namespace
 * find_vpid() finr the pid by its virtual id, i.e. in the current namespace
 *
 * see also find_task_by_pid() set in include/linux/sched.h
 */
extern struct pid *FASTCALL(find_pid_ns(int nr, struct pid_namespace *ns));
extern struct pid *find_vpid(int nr);
extern struct pid *find_pid(int nr);

/*
 * Lookup a PID in the hash table, and return with it's count elevated.
 */
extern struct pid *find_get_pid(int nr);
extern struct pid *find_ge_pid(int nr, struct pid_namespace *);

extern struct pid *alloc_pid(struct pid_namespace *ns);
extern void FASTCALL(free_pid(struct pid *pid));
extern void zap_pid_ns_processes(struct pid_namespace *pid_ns);

/*
 * the helpers to get the pid's id seen from different namespaces
 *
 * pid_nr()    : global id, i.e. the id seen from the init namespace;
 * pid_vnr()   : virtual id, i.e. the id seen from the namespace this pid
 *               belongs to. this only makes sence when called in the
 *               context of the task that belongs to the same namespace;
 * pid_nr_ns() : id seen from the ns specified.
 *
 * see also task_xid_nr() etc in include/linux/sched.h
 */

/*��ø������ռ��е�ȫ��pid*/
static inline pid_t pid_nr(struct pid *pid)
{
	pid_t nr = 0;
	/*pid�ǿ�ʱ��ȡlevel=0��ȫ��pid*/
	if (pid)
		nr = pid->numbers[0].nr;
	return nr;
}

pid_t pid_nr_ns(struct pid *pid, struct pid_namespace *ns);

/*���pidʵ�������ռ��νṹ����ײ������ռ��е�pid*/
static inline pid_t pid_vnr(struct pid *pid)
{
	pid_t nr = 0;
	/*pidʵ���ǿ�ʱ��ȡ��ײ������ռ��е�pid*/
	if (pid)
		nr = pid->numbers[pid->level].nr;
	return nr;
}

/*����pidɢ�б���type���͵����н���*/
#define do_each_pid_task(pid, type, task)				\
	do {												\
		struct hlist_node *pos___;						\
		if (pid != NULL)								\
			/**/										\
 			hlist_for_each_entry_rcu((task), pos___,	&pid->tasks[type], pids[type].node)\
			{
#define while_each_pid_task(pid, type, task)			\
			}											\
	} while (0)

#endif /* _LINUX_PID_H */