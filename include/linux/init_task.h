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

/*��ʼ�����̵��ļ���������*/
#define INIT_FDTABLE 														\
{																			\
	/*Ĭ�ϴ��ļ�����ĿΪ�ֳ��б���λ����Ŀ*/													\
	.max_fds	= NR_OPEN_DEFAULT, 											\
	/*�ļ�ָ�������ָ��*/															\
	.fd		= &init_files.fd_array[0], 										\
	/*ִ��exec��Ҫ�رյ��ļ�λͼ��Ϣ*/													\
	.close_on_exec	= (fd_set *)&init_files.close_on_exec_init, 			\
	/*�Ѵ򿪵��ļ�λͼ��Ϣ*/															\
	.open_fds	= (fd_set *)&init_files.open_fds_init, 						\
	/*��ʼ�������ļ����������rcu*/														\
	.rcu		= RCU_HEAD_INIT, 											\
	/*��һ���ļ���������ָ��Ϊ��*/														\
	.next		= NULL,														\
}

/*��ʼ�����̴��ļ�����Ϣ*/
#define INIT_FILES 															\
{ 																			\
	/*���ļ��ṹ��Ϣ�����ü���*/\
	.count		= ATOMIC_INIT(1), 											\
	/*��ʼ�������ļ���������ָ��ָ�������ļ���������ṹ*/											\
	.fdt		= &init_files.fdtab, 										\
	/*��ʼ�������ļ���������*/\
	.fdtab		= INIT_FDTABLE,												\
	/*��ʼ�������ļ�������Ϊδ����״̬*/													\
	.file_lock	= __SPIN_LOCK_UNLOCKED(init_task.file_lock), 				\
	/*�ļ�������������һ���ļ��������ı��*/													\
	.next_fd	= 0, 														\
	/*ִ��exec����ʱ��Ҫ�رյ��ļ�������λͼ��Ϣ��ʼΪ0*/											\
	.close_on_exec_init = { { 0, } }, 										\
	/*�Ѵ��ļ���Ϣ��ʼΪ0����û�д��ļ�*/													\
	.open_fds_init	= { { 0, } }, 											\
	/*�ļ�������ָ�������ʼ��Ϊ��*/														\
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

/*��ʼ�����̵������ַ�ռ�*/
#define INIT_MM(name) 														\
{																			\
	/*��ʼ�������ڴ�����vm_area_struct�Ķ����������*/										\
	.mm_rb		= RB_ROOT,													\
	/*��ʼ������ȫ��ҳĿ¼��*/															\
	.pgd		= swapper_pg_dir, 											\
	/*��ʼ���������ڴ������ʹ����*/														\
	.mm_users	= ATOMIC_INIT(2), 											\
	/*��ʼ���������ַ�ռ�����ü���*/														\
	.mm_count	= ATOMIC_INIT(1), 											\
	/*��ʼ���������ַ�ռ��е��ڴ�ӳ���д�ź���*/												\
	.mmap_sem	= __RWSEM_INITIALIZER(name.mmap_sem),						\
	/*ҳ�����������*/																\
	.page_table_lock =  __SPIN_LOCK_UNLOCKED(name.page_table_lock),			\
	/*����mm_struct��˫�����ʼ��*/													\
	.mmlist		= LIST_HEAD_INIT(name.mmlist),								\
	/**/\
	.cpu_vm_mask	= CPU_MASK_ALL,											\
}

/*��ʼ�������ź���������ʼ��*/
#define INIT_SIGNALS(sig)
{																				\
	/*�ź������������ü���*/																\
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
	/*��ʼ�����̵���Դ��������*/															\
	.rlim		= INIT_RLIMITS,													\
}

extern struct nsproxy init_nsproxy;
/*�������ռ��ʼ��*/
#define INIT_NSPROXY(nsproxy)
{												\
	/*��pid�����ռ��ʼ��*/								\
	.pid_ns		= &init_pid_ns,					\
	/*�������ռ䱻���ü���*/								\
	.count		= ATOMIC_INIT(1),				\
	/*��uts�����ռ��ʼ��*/								\
	.uts_ns		= &init_uts_ns,					\
	/*��mnt�����ռ��ʼ��*/								\
	.mnt_ns		= NULL,							\
	/*�����������ռ��ʼ��*/								\
	INIT_NET_NS(net_ns)                         \
	/*��ipc�����ռ��ʼ��*/								\
	INIT_IPC_NS(ipc_ns)							\
	/*���û������ռ��ʼ��*/								\
	.user_ns	= &init_user_ns,				\
}

/*��ʼ�����̵��źŴ�����������ʼ��*/
#define INIT_SIGHAND(sighand)
{																					\
	/*�źŴ��������������ü���*/																\
	.count		= ATOMIC_INIT(1), 													\
	/*�źŴ��������*/																	\
	.action		= { { { .sa_handler = NULL, } }, },									\
	/*�źŲ�������������Ϊδ����״̬*/																\
	.siglock	= __SPIN_LOCK_UNLOCKED(sighand.siglock),							\
	/*�ȴ������źŴ���Ľ���*/																	\
	.signalfd_wqh	= __WAIT_QUEUE_HEAD_INITIALIZER(sighand.signalfd_wqh),	\
}

extern struct group_info init_groups;
/*��ʼ�����̵�pid���ں˶�pid���ڲ���ʾ��ʵ����ʼ��*/
#define INIT_STRUCT_PID
{																	\
	/*��pidʵ�������ü�����ʼ��*/												\
	.count 		= ATOMIC_INIT(1),									\
	/*pidɢ�б��ʼ��*/													\
	.tasks		= 													\
	{
		/*��ʼ������pidɢ�б��ʼ��*/											\
		{ .first = &init_task.pids[PIDTYPE_PID].node },				\
		/*��ʼ���������ڽ������鳤ɢ�б��ʼ��*/										\
		{ .first = &init_task.pids[PIDTYPE_PGID].node },	\
		/*��ʼ�����̻Ự���鳤ɢ�б��ʼ��*/										\
		{ .first = &init_task.pids[PIDTYPE_SID].node },				\
	},																\
	/*rcuͷ��ʼ��*/														\
	.rcu		= RCU_HEAD_INIT,									\
	/*�����ռ���Ϊȫ�ֻ�������ռ䡣��������ռ�*/										\
	.level		= 0,												\
	/*pid����ȫ�֣������ռ�Ҳ����numbers[0]��ʼ��*/								\
	.numbers	= 													\
	{ 																\
		{															\
			/*�����ڸ���ȫ�֣������ռ��е�pid���Ϊ0*/								\
			.nr		= 0,											\
			/*����ȫ�֣������ռ�*/											\
			.ns		= &init_pid_ns,									\
			/*��ȫ�������ռ���ɢ�н���ʼ��Ϊ��*/									\
			.pid_chain	= { .next = NULL, .pprev = NULL },			\
		}, 															\
	}																\
}

/*����ʼ������ʵ�������뵽��pidʵ���ж�Ӧ���͵�ɢ�б���*/
#define INIT_PID_LINK(type) 									\
{																\
	/*��ʼ������ɢ�н���ʼ��*/											\
	.node = 													\
	{															\
		/*��ʼ������ɢ�н���ָ��������Ϊ��*/									\
		.next = NULL,											\
		/*��ʼ������ɢ�н��ǰ�����nextָ������Ϊ��Ӧ���͵�ɢ��ͷ*/\
		.pprev = &init_struct_pid.tasks[type].first,			\
	},															\
	/*��ʼ�����̵�pidʵ����ʼ��*/											\
	.pid = &init_struct_pid,									\
}

/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x1fffff (=2MB)
 */
/*��ʼ�������Ǳ������ĵ�һ������*/
 #define INIT_TASK(tsk)											\
{																\
	/*��������Ϊ������״̬*/												\
	.state		= 0,											\
	/*��ʼ���̵߳Ͳ���Ϣ*/												\
	.stack		= &init_thread_info,							\
	/*��ʼ�����̱����ô���*/												\
	.usage		= ATOMIC_INIT(2),								\
	/*��ʼ������PF_*��ʶ*/												\
	.flags		= 0,											\
	/*û��ʹ�ô��ں�����������������״̬*/										\
	.lock_depth	= -1,											\
	/*�������ȼ�����ΪĬ�����ȼ�*/											\
	.prio		= MAX_PRIO-20,									\
	/*���̵ľ�̬���ȼ�����ΪĬ�����ȼ�*/										\
	.static_prio	= MAX_PRIO-20,								\
	/*������ͨ���ȼ�����ΪĬ�����ȼ�*/											\
	.normal_prio	= MAX_PRIO-20,								\
	/*���̵��Ȳ�������ͨ����*/												\
	.policy		= SCHED_NORMAL,									\
	/*���ý��̵�cpu�׺��ԣ����̿�������cpu������*/								\
	.cpus_allowed	= CPU_MASK_ALL,								\
	/*���̵������ַ�ռ�����Ϊ��*/											\
	.mm		= NULL,												\
	/*������ַ�ռ�����Ϊ��ʼ�����������ַ�ռ�*/\
	.active_mm	= &init_mm,										\
	/*��ʼ�����̾�������ͷ���*/											\
	.run_list	= LIST_HEAD_INIT(tsk.run_list),					\
	/**/\
	.ioprio		= 0,											\
	/*��ʼ��SCHED_RR���Ȳ��Ե�ʱ��Ƭ��1/HZ��*/								\
	.time_slice	= HZ,											\
	/*��ʼ��linux��������*/											\
	.tasks		= LIST_HEAD_INIT(tsk.tasks),					\
	/*��ʼ���������ӽ�������*/												\
	.ptrace_children= LIST_HEAD_INIT(tsk.ptrace_children),		\
	/*��ʼ������������*/												\
	.ptrace_list	= LIST_HEAD_INIT(tsk.ptrace_list),			\
	/*��ʼ�����̵�����������Ϊ�����������������*/\
	.real_parent	= &tsk,										\
	/*��ʼ�����̵ĸ��ڵ�Ϊ���������������*/										\
	.parent		= &tsk,											\
	/*��ʼ�����̵��ӽ���*/												\
	.children	= LIST_HEAD_INIT(tsk.children),					\
	/*��ʼ�����̵��ֵܽ���*/												\
	.sibling	= LIST_HEAD_INIT(tsk.sibling),					\
	/*��ʼ�����̵��߳����鳤������*/											\
	.group_leader	= &tsk,										\
	/*��ʼ�����̵�����Ϣ*/\
	.group_info	= &init_groups,									\
	/*��ʼ�����̵���ЧȨ�޼��ϣ���ȫȨ��������ɶ��κν�����ӻ�ɾ���ѱ������Ȩ��*/\
	.cap_effective	= CAP_INIT_EFF_SET,							\
	/*����Ȩ�޳�ʼ��Ϊ��*/\
	.cap_inheritable = CAP_INIT_INH_SET,						\
	/*������Ȩ������Ϊ����Ȩ��*/\
	.cap_permitted	= CAP_FULL_SET,								\
	/**/\
	.keep_capabilities = 0,										\
	/*��ʼ���û���Ϣ*/													\
	.user		= INIT_USER,									\
	/*���ó�ʼ�����̵�����Ϊswapper*/										\
	.comm		= "swapper",									\
	/*��ʼ�����̵ײ�Ĵ����������Ϣ*/											\
	.thread		= INIT_THREAD,									\
	/*��ʼ�����̵��ļ�ϵͳ*/												\
	.fs		= &init_fs,											\
	/*��ʼ���Ѵ��ļ���Ϣ*/												\
	.files		= &init_files,									\
	/*��ʼ�������ź���������Ϣ*/											\
	.signal		= &init_signals,								\
	/*��ʼ�������źŴ�����������Ϣ*/											\
	.sighand	= &init_sighand,								\
	/*��ʼ�����̵������ռ�*/												\
	.nsproxy	= &init_nsproxy,								\
	/*��ʼ�����̴������ź������Ϣ*/											\
	.pending	= 												\
	{															\
		/*��ʼ�����̴������ź�����*/										\
		.list = LIST_HEAD_INIT(tsk.pending.list),				\
		/*��ս��̴������źż���*/											\
		.signal = {{0}}											\
	},															\
	/*��ս��̱��������źż���*/											\
	.blocked	= {{0}},										\
	/*���÷��估�ͷ��ڴ����Դ��������Ϊδ����״̬*/\
	.alloc_lock	= __SPIN_LOCK_UNLOCKED(tsk.alloc_lock),			\
	/*���ý�����־�����ϢΪ��*/											\
	.journal_info	= NULL,										\
	/*�����ʱ��*/\
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
