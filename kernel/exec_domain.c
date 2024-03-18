/*
 * Handling of different ABIs (personalities).
 *
 * We group personalities into execution domains which have their
 * own handlers for kernel entry points, signal mapping, etc...
 *
 * 2001-05-06	Complete rewrite,  Christoph Hellwig (hch@infradead.org)
 */
/*处理不同ABI（个性化），我们将具有独自的信号映射等内核入口点执行域中的个性化信息归组*/

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
/*定义保护执行域信息的读写自旋锁*/
static DEFINE_RWLOCK(exec_domains_lock);


static u_long ident_map[32] =
{
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31
};

/*linux默认的执行域*/
struct exec_domain default_exec_domain =
{
	/*执行域名称*/
	.name		= "Linux",		/* name */
	/*lcall7将引起一个段错误*/
	.handler	= default_handler,
	/*personality : PER_LINUX*/
	.pers_low	= 0,
	/*personality : PER_LINUX*/
	.pers_high	= 0,
	/*信号位图*/
	.signal_map	= ident_map,
	/*翻转的信号位图*/
	.signal_invmap	= ident_map,
};

/**/
static void default_handler(int segment, struct pt_regs *regp)
{
	/*改变当前运行进程的个性化信息*/
	set_personality(0);
	/*如果当前线程执行域中系统调用函数不是默认的处理函数,则执行该处理函数*/
	if (current_thread_info()->exec_domain->handler != default_handler)
		current_thread_info()->exec_domain->handler(segment, regp);
	else
		send_sig(SIGSEGV, current, 1);
}

/*查找执行域信息*/
static struct exec_domain *lookup_exec_domain(u_long personality)
{
	struct exec_domain *	ep;
	/*获取当前进程非标识符个性化基本类型,只取低8位*/
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
