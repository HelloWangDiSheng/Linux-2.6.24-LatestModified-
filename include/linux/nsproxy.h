#ifndef _LINUX_NSPROXY_H
#define _LINUX_NSPROXY_H

#include <linux/spinlock.h>
#include <linux/sched.h>

struct mnt_namespace;
struct uts_namespace;
struct ipc_namespace;
struct pid_namespace;

/*	 �����ռ���ʹ���
	��1������ǰ���̿����޸��������ռ�ָ����������ռ��е���һָ�롣
    ��2�������ʵ�ǰ���̵������ռ�ʱӦʹ��RCU���ơ�
    ��3���������������̵������ռ�ʱ����Ҫ��rcu_read_lock()��rcu_read_unlock()�����½��У�
�����ռ䲻������ζ�Ž��̴��ڽ�ʬ״̬������������
*/

 /*ÿ�����̶�������һ��ѡ���������ռ䡣ÿ�����Ը�֪�����ռ���ں���ϵͳ�������ṩһ��
 ���ݽṹ��������ͨ�������ռ���ʽ�ṩ�Ķ�����������struct nsproxy���ڻ㼯ָ���ض���
 ��ϵͳ�������ռ��װ����ָ��*/
struct nsproxy
{
	/*�����ռ䱻���̹������Ŀ*/
	atomic_t count;
	/*uts(Unix Timesharing System)�����ռ�������ں˵����ơ��汾���Ͳ���ϵ�ṹ���͵���Ϣ*/
	struct uts_namespace *uts_ns;
	/*������������̼�ͨ��(ipc�йص���Ϣ)*/
	struct ipc_namespace *ipc_ns;
	/*������װ�ص��ļ�ϵͳ����ͼ*/
	struct mnt_namespace *mnt_ns;
	/*�йؽ���ID����Ϣ*/
	struct pid_namespace *pid_ns;
	/*������������ÿ���û���Դʹ�õ���Ϣ*/
	struct user_namespace *user_ns;
	/*������ص������ռ����*/
	struct net *net_ns;
};

/*������ʼ�����̵������ռ�*/
extern struct nsproxy init_nsproxy;

/*�����ռ���ʹ��򣺣�1������ǰ���̿����޸��������ռ�ָ����������ռ��е���һָ�롣
��2�������ʵ�ǰ���̵������ռ�ʱӦʹ��RCU���ơ�
��3���������������̵������ռ�ʱ����Ҫ��rcu_read_lock()��rcu_read_unlock()�����½��У�
�����ռ䲻������ζ�Ž��̴��ڽ�ʬ״̬������������
��4��*/

/*ͨ��rcu���ƻ�ȡ���̵������ռ�*/
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

/*ȡ���������ռ�����ã���������ռ�û�б����ã����ͷŸ������ռ�*/
static inline void put_nsproxy(struct nsproxy *ns)
{
	if (atomic_dec_and_test(&ns->count))
	{
		/*�����ռ�û�б����ã����ͷŸ������ռ�*/
		free_nsproxy(ns);
	}
}

/*���������ռ�*/
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
