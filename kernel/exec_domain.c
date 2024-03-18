/*
 * Handling of different ABIs (personalities).
 *
 * We group personalities into execution domains which have their
 * own handlers for kernel entry points, signal mapping, etc...
 *
 * 2001-05-06	Complete rewrite,  Christoph Hellwig (hch@infradead.org)
 */
/*����ͬABI�����Ի��������ǽ����ж��Ե��ź�ӳ����ں���ڵ�ִ�����еĸ��Ի���Ϣ����*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/personality.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/sysctl.h>
#include <linux/types.h>


static void default_handler(int, struct pt_regs *);

/**/
static struct exec_domain *exec_domains = &default_exec_domain;
/*���屣��ִ������Ϣ�Ķ�д������*/
static DEFINE_RWLOCK(exec_domains_lock);


static u_long ident_map[32] =
{
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31
};

/*linuxĬ�ϵ�ִ����*/
struct exec_domain default_exec_domain =
{
	/*ִ��������*/
	.name		= "Linux",		/* name */
	/*lcall7������һ���δ���*/
	.handler	= default_handler,
	/*personality : PER_LINUX*/
	.pers_low	= 0,
	/*personality : PER_LINUX*/
	.pers_high	= 0,
	/*�ź�λͼ*/
	.signal_map	= ident_map,
	/*��ת���ź�λͼ*/
	.signal_invmap	= ident_map,
};

/**/
static void default_handler(int segment, struct pt_regs *regp)
{
	/*�ı䵱ǰ���н��̵ĸ��Ի���Ϣ*/
	set_personality(0);
	/*�����ǰ�߳�ִ������ϵͳ���ú�������Ĭ�ϵĴ�����,��ִ�иô�����*/
	if (current_thread_info()->exec_domain->handler != default_handler)
		current_thread_info()->exec_domain->handler(segment, regp);
	else
		send_sig(SIGSEGV, current, 1);
}

/*����ִ������Ϣ*/
static struct exec_domain *lookup_exec_domain(u_long personality)
{
	struct exec_domain *	ep;
	/*��ȡ��ǰ���̷Ǳ�ʶ�����Ի���������,ֻȡ��8λ*/
	u_long pers = personality(personality);
	/**/
	read_lock(&exec_domains_lock);
	for (ep = exec_domains; ep; ep = ep->next)
	{
		if (pers >= ep->pers_low && pers <= ep->pers_high)
			if (try_module_get(ep->module))
				goto out;
	}

#ifdef CONFIG_KMOD
	read_unlock(&exec_domains_lock);
	request_module("personality-%ld", pers);
	read_lock(&exec_domains_lock);

	for (ep = exec_domains; ep; ep = ep->next)
	{
		if (pers >= ep->pers_low && pers <= ep->pers_high)
			if (try_module_get(ep->module))
				goto out;
	}
#endif

	ep = &default_exec_domain;
out:
	read_unlock(&exec_domains_lock);
	return (ep);
}

int register_exec_domain(struct exec_domain *ep)
{
	struct exec_domain	*tmp;
	int			err = -EBUSY;

	if (ep == NULL)
		return -EINVAL;

	if (ep->next != NULL)
		return -EBUSY;

	write_lock(&exec_domains_lock);
	for (tmp = exec_domains; tmp; tmp = tmp->next) {
		if (tmp == ep)
			goto out;
	}

	ep->next = exec_domains;
	exec_domains = ep;
	err = 0;

out:
	write_unlock(&exec_domains_lock);
	return (err);
}

int
unregister_exec_domain(struct exec_domain *ep)
{
	struct exec_domain	**epp;

	epp = &exec_domains;
	write_lock(&exec_domains_lock);
	for (epp = &exec_domains; *epp; epp = &(*epp)->next) {
		if (ep == *epp)
			goto unregister;
	}
	write_unlock(&exec_domains_lock);
	return -EINVAL;

unregister:
	*epp = ep->next;
	ep->next = NULL;
	write_unlock(&exec_domains_lock);
	return 0;
}

int
__set_personality(u_long personality)
{
	struct exec_domain	*ep, *oep;

	ep = lookup_exec_domain(personality);
	if (ep == current_thread_info()->exec_domain) {
		current->personality = personality;
		module_put(ep->module);
		return 0;
	}

	if (atomic_read(&current->fs->count) != 1) {
		struct fs_struct *fsp, *ofsp;

		fsp = copy_fs_struct(current->fs);
		if (fsp == NULL) {
			module_put(ep->module);
			return -ENOMEM;
		}

		task_lock(current);
		ofsp = current->fs;
		current->fs = fsp;
		task_unlock(current);

		put_fs_struct(ofsp);
	}

	/*
	 * At that point we are guaranteed to be the sole owner of
	 * current->fs.
	 */

	current->personality = personality;
	oep = current_thread_info()->exec_domain;
	current_thread_info()->exec_domain = ep;
	set_fs_altroot();

	module_put(oep->module);
	return 0;
}

int
get_exec_domain_list(char *page)
{
	struct exec_domain	*ep;
	int			len = 0;

	read_lock(&exec_domains_lock);
	for (ep = exec_domains; ep && len < PAGE_SIZE - 80; ep = ep->next)
		len += sprintf(page + len, "%d-%d\t%-16s\t[%s]\n",
			       ep->pers_low, ep->pers_high, ep->name,
			       module_name(ep->module));
	read_unlock(&exec_domains_lock);
	return (len);
}

asmlinkage long sys_personality(u_long personality)
{
	u_long old = current->personality;

	if (personality != 0xffffffff) {
		set_personality(personality);
		if (current->personality != personality)
			return -EINVAL;
	}

	return (long)old;
}


EXPORT_SYMBOL(register_exec_domain);
EXPORT_SYMBOL(unregister_exec_domain);
EXPORT_SYMBOL(__set_personality);
