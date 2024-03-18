#ifndef _LINUX_PID_H
#define _LINUX_PID_H

#include <linux/rcupdate.h>

/*每个进程除了pid这个特征值之外，还有其它的id，有下列几中可能的类型：
（1）处于某个线程组（在一个进程中，以标识CLONE_THREAD来调用clone建立的该进程的不同的
执行上下文）中的所有进程都有一个统一的线程组ID（TGID），如果进程没有使用线程，则PID与
TGID相同。线程组中的主进程被称为组长（group_leader），通过clone创建的多有线程的struct
 task_struct的group_leader成员，会指向组长的struct task_struct实例。
（2）独立进程可以合并为进程组（使用setgprp系统调用）。进程组成员的struct task_struct的
pgrp属性值都是相同的，即进程组组长的PID，进程组简化了向组的所有成员发送信号的操作。这
对于各种系统程序设计应用是有用的，注意！用管道连接的进程包含在同一个进程组中
（3）几个进程组可以合并成一个会话。会话中的所有进程都有同样的会话ID，保存在task_struct
的session成员中，SID可以使用setsid系统调用设置*/

enum pid_type
{
	/*进程ID*/
	PIDTYPE_PID,
	/*进程组ID*/
	PIDTYPE_PGID,
	/*会话ID*/
	PIDTYPE_SID,
	/*ID类型数目*/
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
 
/*特定命名空间中可见的信息*/
struct upid
{
	/*为了find_pid查找高效，尽量让进程局部pid和pid_chain保持在一个缓存行*/
	/*进程局部pid编号*/
	int nr;
	/*局部命名空间*/
	struct pid_namespace *ns;
	/*所有的upid实例都保存在一个散列表中*/
	struct hlist_node pid_chain;
};

/*内核对pid的内部表示*/
struct pid
{
	/*引用计数器*/
	atomic_t count;
	/*数组项都是一个散列表表头，对应于一个ID类型。这样做是有必要的，因为一个ID可能
	用于几个进程，所有共享同一给定ID的task_struct实例，都通过该链表连接起来*/
	struct hlist_head tasks[PIDTYPE_MAX];
	/*采用rcu机制操作*/
	struct rcu_head rcu;
	/*可以看到该进程的命名空间的数目，即包含该进程的命名空间在命名空间层次结构中的深度*/
	int level;
	/*数组中每一项都对应于一个命名空间，注意！该数组形式上只有一个数组项，如果一个进程只
	包含在全局命名空间中，那么确实如此，由于该数组位于结构的末尾，因此，只要分配更多内存
	空间，即可向数组添加附加的项*/
	struct upid numbers[1];
};

/*初始化pid命名空间*/
extern struct pid init_struct_pid;

/*由于所有共享同一ID的task_struct实例都按进程存储在一个散列表中国，因此，需要在task_struct
中增加一个散列元素。pid_link即建立了pid与pid散列表的联系*/
struct pid_link
{
	/*散列元素*/
	struct hlist_node node;
	/*pid所属的pid结构实例*/
	struct pid *pid;
};

/*引用pid实例*/
static inline struct pid *get_pid(struct pid *pid)
{
	/*pid非空时将其引用计数器加1*/
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

/*获得根命名空间中的全局pid*/
static inline pid_t pid_nr(struct pid *pid)
{
	pid_t nr = 0;
	/*pid非空时获取level=0的全局pid*/
	if (pid)
		nr = pid->numbers[0].nr;
	return nr;
}

pid_t pid_nr_ns(struct pid *pid, struct pid_namespace *ns);

/*获得pid实例命名空间层次结构中最底层命名空间中的pid*/
static inline pid_t pid_vnr(struct pid *pid)
{
	pid_t nr = 0;
	/*pid实例非空时获取最底层命名空间中的pid*/
	if (pid)
		nr = pid->numbers[pid->level].nr;
	return nr;
}

/*遍历pid散列表中type类型的所有进程*/
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
