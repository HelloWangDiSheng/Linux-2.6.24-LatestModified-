#ifndef _LINUX__INIT_TASK_H
#define _LINUX__INIT_TASK_H

#include <linux/file.h>
#include <linux/rcupdate.h>
#include <linux/irqflags.h>
#include <linux/utsname.h>
#include <linux/lockdep.h>
#include <linux/ipc.h>
#include <linux/pid_namespace.h>
#include <linux/user_namespace.h>
#include <net/net_namespace.h>

/*初始化进程的文件描述符表*/
#define INIT_FDTABLE 														\
{																			\
	/*默认打开文件的数目为字长中比特位的数目*/													\
	.max_fds	= NR_OPEN_DEFAULT, 											\
	/*文件指针数组的指针*/															\
	.fd		= &init_files.fd_array[0], 										\
	/*执行exec需要关闭的文件位图信息*/													\
	.close_on_exec	= (fd_set *)&init_files.close_on_exec_init, 			\
	/*已打开的文件位图信息*/															\
	.open_fds	= (fd_set *)&init_files.open_fds_init, 						\
	/*初始化保护文件描述符表的rcu*/														\
	.rcu		= RCU_HEAD_INIT, 											\
	/*下一个文件描述符表指针为空*/														\
	.next		= NULL,														\
}

/*初始化进程打开文件的信息*/
#define INIT_FILES 															\
{ 																			\
	/*该文件结构信息的引用计数*/\
	.count		= ATOMIC_INIT(1), 											\
	/*初始化进程文件描述符表指针指向自身文件描述符表结构*/											\
	.fdt		= &init_files.fdtab, 										\
	/*初始化进程文件描述符表*/\
	.fdtab		= INIT_FDTABLE,												\
	/*初始化进程文件锁设置为未锁定状态*/													\
	.file_lock	= __SPIN_LOCK_UNLOCKED(init_task.file_lock), 				\
	/*文件描述符表中下一个文件描述符的编号*/													\
	.next_fd	= 0, 														\
	/*执行exec程序时需要关闭的文件描述符位图信息初始为0*/											\
	.close_on_exec_init = { { 0, } }, 										\
	/*已打开文件信息初始为0，即没有打开文件*/													\
	.open_fds_init	= { { 0, } }, 											\
	/*文件描述符指针数组初始化为空*/														\
	.fd_array	= { NULL, } 												\
}

#define INIT_KIOCTX(name, which_mm) \
{							\
	.users		= ATOMIC_INIT(1),		\
	.dead		= 0,				\
	.mm		= &which_mm,			\
	.user_id	= 0,				\
	.next		= NULL,				\
	.wait		= __WAIT_QUEUE_HEAD_INITIALIZER(name.wait), \
	.ctx_lock	= __SPIN_LOCK_UNLOCKED(name.ctx_lock), \
	.reqs_active	= 0U,				\
	.max_reqs	= ~0U,				\
}

/*初始化进程的虚拟地址空间*/
#define INIT_MM(name) 														\
{																			\
	/*初始化管理内存区域vm_area_struct的二叉树根结点*/										\
	.mm_rb		= RB_ROOT,													\
	/*初始化进程全局页目录项*/															\
	.pgd		= swapper_pg_dir, 											\
	/*初始化该虚拟内存区域的使用者*/														\
	.mm_users	= ATOMIC_INIT(2), 											\
	/*初始化该虚拟地址空间的引用计数*/														\
	.mm_count	= ATOMIC_INIT(1), 											\
	/*初始化该虚拟地址空间中的内存映射读写信号量*/												\
	.mmap_sem	= __RWSEM_INITIALIZER(name.mmap_sem),						\
	/*页表操作保护锁*/																\
	.page_table_lock =  __SPIN_LOCK_UNLOCKED(name.page_table_lock),			\
	/*连接mm_struct的双链表初始化*/													\
	.mmlist		= LIST_HEAD_INIT(name.mmlist),								\
	/**/\
	.cpu_vm_mask	= CPU_MASK_ALL,											\
}

/*初始化进程信号描述符初始化*/
#define INIT_SIGNALS(sig)
{																				\
	/*信号描述符被引用计数*/																\
	.count		= ATOMIC_INIT(1), 												\
	/**/\
	.wait_chldexit	= __WAIT_QUEUE_HEAD_INITIALIZER(sig.wait_chldexit),\
	/**/\
	.shared_pending	=															\
	{ 																			\
		.list = LIST_HEAD_INIT(sig.shared_pending.list),						\
		.signal =  {{0}}														\
	},																			\
	/**/\
	.posix_timers	 = LIST_HEAD_INIT(sig.posix_timers),						\
	/**/\
	.cpu_timers	= INIT_CPU_TIMERS(sig.cpu_timers),								\
	/*初始化进程的资源限制详情*/															\
	.rlim		= INIT_RLIMITS,													\
}

extern struct nsproxy init_nsproxy;
/*根命名空间初始化*/
#define INIT_NSPROXY(nsproxy)
{												\
	/*根pid命名空间初始化*/								\
	.pid_ns		= &init_pid_ns,					\
	/*根命名空间被引用计数*/								\
	.count		= ATOMIC_INIT(1),				\
	/*根uts命名空间初始化*/								\
	.uts_ns		= &init_uts_ns,					\
	/*根mnt命名空间初始化*/								\
	.mnt_ns		= NULL,							\
	/*根网络命名空间初始化*/								\
	INIT_NET_NS(net_ns)                         \
	/*根ipc命名空间初始化*/								\
	INIT_IPC_NS(ipc_ns)							\
	/*根用户命名空间初始化*/								\
	.user_ns	= &init_user_ns,				\
}

/*初始化进程的信号处理描述符初始化*/
#define INIT_SIGHAND(sighand)
{																					\
	/*信号处理描述符被引用计数*/																\
	.count		= ATOMIC_INIT(1), 													\
	/*信号处理函数相关*/																	\
	.action		= { { { .sa_handler = NULL, } }, },									\
	/*信号操作保护锁设置为未锁定状态*/																\
	.siglock	= __SPIN_LOCK_UNLOCKED(sighand.siglock),							\
	/*等待队列信号处理的进程*/																	\
	.signalfd_wqh	= __WAIT_QUEUE_HEAD_INITIALIZER(sighand.signalfd_wqh),	\
}

extern struct group_info init_groups;
/*初始化进程的pid（内核对pid的内部表示）实例初始化*/
#define INIT_STRUCT_PID
{																	\
	/*该pid实例被引用计数初始化*/												\
	.count 		= ATOMIC_INIT(1),									\
	/*pid散列表初始化*/													\
	.tasks		= 													\
	{
		/*初始化进程pid散列表初始化*/											\
		{ .first = &init_task.pids[PIDTYPE_PID].node },				\
		/*初始化进程所在进程组组长散列表初始化*/										\
		{ .first = &init_task.pids[PIDTYPE_PGID].node },	\
		/*初始化进程会话组组长散列表初始化*/										\
		{ .first = &init_task.pids[PIDTYPE_SID].node },				\
	},																\
	/*rcu头初始化*/														\
	.rcu		= RCU_HEAD_INIT,									\
	/*命名空间层次为全局或根命名空间。最顶层命名空间*/										\
	.level		= 0,												\
	/*pid根（全局）命名空间也就是numbers[0]初始化*/								\
	.numbers	= 													\
	{ 																\
		{															\
			/*进程在根（全局）命名空间中的pid编号为0*/								\
			.nr		= 0,											\
			/*根（全局）命名空间*/											\
			.ns		= &init_pid_ns,									\
			/*将全局命名空间中散列结点初始化为空*/									\
			.pid_chain	= { .next = NULL, .pprev = NULL },			\
		}, 															\
	}																\
}

/*将初始化进程实例，插入到其pid实例中对应类型的散列表中*/
#define INIT_PID_LINK(type) 									\
{																\
	/*初始化进程散列结点初始化*/											\
	.node = 													\
	{															\
		/*初始化进程散列结点后指针域设置为空*/									\
		.next = NULL,											\
		/*初始化进程散列结点前向结点的next指针设置为对应类型的散列头*/\
		.pprev = &init_struct_pid.tasks[type].first,			\
	},															\
	/*初始化进程的pid实例初始化*/											\
	.pid = &init_struct_pid,									\
}

/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x1fffff (=2MB)
 */
/*初始化进程是被创建的第一个进程*/
 #define INIT_TASK(tsk)											\
{																\
	/*进程设置为可运行状态*/												\
	.state		= 0,											\
	/*初始化线程低层信息*/												\
	.stack		= &init_thread_info,							\
	/*初始化进程被引用次数*/												\
	.usage		= ATOMIC_INIT(2),								\
	/*初始化进程PF_*标识*/												\
	.flags		= 0,											\
	/*没有使用大内核锁，锁处于无锁定状态*/										\
	.lock_depth	= -1,											\
	/*进程优先级设置为默认优先级*/											\
	.prio		= MAX_PRIO-20,									\
	/*进程的静态优先级设置为默认优先级*/										\
	.static_prio	= MAX_PRIO-20,								\
	/*进程普通优先级设置为默认优先级*/											\
	.normal_prio	= MAX_PRIO-20,								\
	/*进程调度策略是普通调度*/												\
	.policy		= SCHED_NORMAL,									\
	/*设置进程的cpu亲和性，进程可在所有cpu上运行*/								\
	.cpus_allowed	= CPU_MASK_ALL,								\
	/*进程的虚拟地址空间设置为空*/											\
	.mm		= NULL,												\
	/*活动虚拟地址空间设置为初始化进程虚拟地址空间*/\
	.active_mm	= &init_mm,										\
	/*初始化进程就绪队列头结点*/											\
	.run_list	= LIST_HEAD_INIT(tsk.run_list),					\
	/**/\
	.ioprio		= 0,											\
	/*初始化SCHED_RR调度策略的时间片是1/HZ秒*/								\
	.time_slice	= HZ,											\
	/*初始化linux进程链表*/											\
	.tasks		= LIST_HEAD_INIT(tsk.tasks),					\
	/*初始化被调试子进程链表*/												\
	.ptrace_children= LIST_HEAD_INIT(tsk.ptrace_children),		\
	/*初始化被调试链表*/												\
	.ptrace_list	= LIST_HEAD_INIT(tsk.ptrace_list),			\
	/*初始化进程的真正父进程为自身，即进程树根结点*/\
	.real_parent	= &tsk,										\
	/*初始化进程的父节点为自身，进程树根结点*/										\
	.parent		= &tsk,											\
	/*初始化进程的子进程*/												\
	.children	= LIST_HEAD_INIT(tsk.children),					\
	/*初始化进程的兄弟进程*/												\
	.sibling	= LIST_HEAD_INIT(tsk.sibling),					\
	/*初始化进程的线程组组长是自身*/											\
	.group_leader	= &tsk,										\
	/*初始化进程的组信息*/\
	.group_info	= &init_groups,									\
	/*初始化进程的有效权限集合，在全权限中清除可对任何进程添加或删除已被允许的权限*/\
	.cap_effective	= CAP_INIT_EFF_SET,							\
	/*进程权限初始化为空*/\
	.cap_inheritable = CAP_INIT_INH_SET,						\
	/*被允许权限设置为所有权限*/\
	.cap_permitted	= CAP_FULL_SET,								\
	/**/\
	.keep_capabilities = 0,										\
	/*初始化用户信息*/													\
	.user		= INIT_USER,									\
	/*设置初始化进程的名字为swapper*/										\
	.comm		= "swapper",									\
	/*初始化进程底层寄存器的相关信息*/											\
	.thread		= INIT_THREAD,									\
	/*初始化进程的文件系统*/												\
	.fs		= &init_fs,											\
	/*初始化已打开文件信息*/												\
	.files		= &init_files,									\
	/*初始化进程信号描述符信息*/											\
	.signal		= &init_signals,								\
	/*初始化进程信号处理描述符信息*/											\
	.sighand	= &init_sighand,								\
	/*初始化进程的命名空间*/												\
	.nsproxy	= &init_nsproxy,								\
	/*初始化进程待处理信号相关信息*/											\
	.pending	= 												\
	{															\
		/*初始化进程待处理信号链表*/										\
		.list = LIST_HEAD_INIT(tsk.pending.list),				\
		/*清空进程待处理信号集合*/											\
		.signal = {{0}}											\
	},															\
	/*清空进程被阻塞的信号集合*/											\
	.blocked	= {{0}},										\
	/*设置分配及释放内存等资源的自旋锁为未锁定状态*/\
	.alloc_lock	= __SPIN_LOCK_UNLOCKED(tsk.alloc_lock),			\
	/*设置进程日志相关信息为空*/											\
	.journal_info	= NULL,										\
	/*间隔定时器*/\
	.cpu_timers	= INIT_CPU_TIMERS(tsk.cpu_timers),				\
	/**/\
	.fs_excl	= ATOMIC_INIT(0),								\
	/**/\
	.pi_lock	= __SPIN_LOCK_UNLOCKED(tsk.pi_lock),			\
	/**/\
	.pids = 													\
	{															\
		/**/\
		[PIDTYPE_PID]  = INIT_PID_LINK(PIDTYPE_PID),			\
		/**/\
		[PIDTYPE_PGID] = INIT_PID_LINK(PIDTYPE_PGID),			\
		/**/\
		[PIDTYPE_SID]  = INIT_PID_LINK(PIDTYPE_SID),			\
	},															\
	/**/\
	.dirties = INIT_PROP_LOCAL_SINGLE(dirties),					\
	/**/\
	INIT_TRACE_IRQFLAGS											\
	/**/\
	INIT_LOCKDEP												\
}


#define INIT_CPU_TIMERS(cpu_timers)								\
{																\
	/**/\
	LIST_HEAD_INIT(cpu_timers[0]),								\
	/**/\
	LIST_HEAD_INIT(cpu_timers[1]),								\
	/**/\
	LIST_HEAD_INIT(cpu_timers[2]),					\
}


#endif
