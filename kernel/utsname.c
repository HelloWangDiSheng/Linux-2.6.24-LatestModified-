#include <linux/module.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <linux/version.h>
#include <linux/err.h>

/*����һ���µ�uts�����ռ䲢��ֵΪ�����uts�����ռ�*/
static struct uts_namespace *clone_uts_ns(struct uts_namespace *old_ns)
{
	struct uts_namespace *ns;
	/*����uts�����ռ���ص�slab����*/
	ns = kmalloc(sizeof(struct uts_namespace), GFP_KERNEL);
	/*�ڴ治�㣬ʧ�ܷ���*/	
	if (!ns)
		return ERR_PTR(-ENOMEM);
	/*����uts�����ռ�Ķ��ź���*/
	down_read(&uts_sem);
	/*��uts�����ռ丳ֵΪ����������ռ�*/
	memcpy(&ns->name, &old_ns->name, sizeof(ns->name));
	/*�ͷ�uts�����ռ���ź���*/
	up_read(&uts_sem);
	/*��ʼ���µ�uts�����ռ����ü���*/
	kref_init(&ns->kref);
	return ns;
}

/*����uts�����ռ�*/
struct uts_namespace *copy_utsname(unsigned long flags, struct uts_namespace *old_ns)
{
	struct uts_namespace *new_ns;
	/*����uts�����ռ䲻��Ϊ��*/
	BUG_ON(!old_ns);
	/*��������uts�����ռ�*/
	get_uts_ns(old_ns);
	/*���������ռ�*/
	if (!(flags & CLONE_NEWUTS))
		return old_ns;
	/*������uts�����ռ丳ֵ���´�����uts�����ռ�*/
	new_ns = clone_uts_ns(old_ns);
	/*ȡ��������uts�����ռ������*/
	put_uts_ns(old_ns);
	return new_ns;
}

/*�ͷ�uts�����ռ䣨����container_of���ƣ�*/
void free_uts_ns(struct kref *kref)
{
	struct uts_namespace *ns;
	/*����container_of���ƻ�ȡ��Ӧ��uts�����ռ�*/
	ns = container_of(kref, struct uts_namespace, kref);
	/*�ͷ�utsռ�õ��ڴ�ռ�*/
	kfree(ns);
}
