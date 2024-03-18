/*
 * Generic pidhash and scalable, time-bounded PID allocator
 *
 * (C) 2002-2003 William Irwin, IBM
 * (C) 2004 William Irwin, Oracle
 * (C) 2002-2004 Ingo Molnar, Red Hat
 *
 * pid-structures are backing objects for tasks sharing a given ID to chain
 * against. There is very little to them aside from hashing them and
 * parking tasks using given ID's on a list.
 *
 * The hash is always changed with the tasklist_lock write-acquired,
 * and the hash is only accessed with the tasklist_lock at least
 * read-acquired, so there's no additional SMP locking needed here.
 *
 * We have a list of bitmap pages, which bitmaps represent the PID space.
 * Allocating and freeing PIDs is completely lockless. The worst-case
 * allocation scenario when all but one out of 1 million PIDs possible are
 * allocated already: the scanning of 32 list entries and at most PAGE_SIZE
 * bytes. The typical fastpath is a single successful setbit. Freeing is O(1).
 *
 * Pid namespaces:
 *    (C) 2007 Pavel Emelyanov <xemul@openvz.org>, OpenVZ, SWsoft Inc.
 *    (C) 2007 Sukadev Bhattiprolu <sukadev@us.ibm.com>, IBM
 *     Many thanks to Oleg Nesterov for comments and help
 *
 */

#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/hash.h>
#include <linux/pid_namespace.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>

/*�������ռ���pid��ź͸������ռ�ĵ�ַ�ļ�ֵ��Ϊ�������ӣ�ͨ����GOLDEN_RATIO_PRIME����
һЩ�м������õ�ɢ��ֵ��IA32�ϼ������Ǽ�������*GOLDEN_RATIO_PRIME>>22��������ת��Ϊ
������*/
#define pid_hashfn(nr, ns)	\
	hash_long((unsigned long)nr + (unsigned long)ns, pidhash_shift)
/*Ϊ�ڸ����������ռ��в��Ҷ�Ӧ��ָ��pid��ֵ��pid�ṹʵ����ʹ����ɢ�б�*/
static struct hlist_head *pid_hash;
/*���ɢ�б���������ƫ�ƣ�ϵͳ��ʼ���ڼ丳ֵ��IA-32�������ڴ����128M������ʱ����ֵΪ10*/
static int pidhash_shift;
/*��pid�����ռ��ʼ��*/
struct pid init_struct_pid = INIT_STRUCT_PID;
/*pid�����ռ���slab�����ϵĵ�ַ*/
static struct kmem_cache *pid_ns_cachep;
/*ϵͳ�����pid�����*/
int pid_max = PID_MAX_DEFAULT;
/*������pid��Ŀ*/
#define RESERVED_PIDS		300
/*Ĭ�Ϸ����pid���ޣ���301��ʼ*/
int pid_max_min = RESERVED_PIDS + 1;
/*Ĭ�ϵ�pid����*/
int pid_max_max = PID_MAX_LIMIT;
/*ÿҳ�еı���λ��Ŀ��32768*/
#define BITS_PER_PAGE		(PAGE_SIZE*8)
#define BITS_PER_PAGE_MASK	(BITS_PER_PAGE-1)
/*����pid���������ռ���λͼ��ƫ��λ�ã�����pid��ʵ��ֵ*/
static inline int mk_pid(struct pid_namespace *pid_ns, struct pidmap *map, int off)
{
	/*����pid�������ʼλͼ�Ѿ���Խ�ı����Ŀ*/
	return (map - pid_ns->pidmap)*BITS_PER_PAGE + off;
}
/*��ѯλͼ�ڣ���ƫ�ƴ���ʼ����һ�����ã�δ��λ��λ�ı��*/
#define find_next_offset(map, off)	find_next_zero_bit((map)->page, BITS_PER_PAGE, off)

/*�������ռ���pid�����ռ��ʼ��*/
 struct pid_namespace init_pid_ns =
{
	.kref =
	{
		.refcount       = ATOMIC_INIT(2),
	},
	/*pidλͼҳ�տ�ʼʱΪNULL���״�ʹ��ǰ�����ڴ棬���ҴӲ��ͷš���С��pid����ֵ���ᵼ��
	����ܶ�λͼ����������ʱpidϵͳ��ģ����4M*/
	.pidmap =
	{
		[ 0 ... PIDMAP_ENTRIES-1] = { ATOMIC_INIT(BITS_PER_PAGE), NULL }
	},
	/*��ʼ�������н���һ������λ��������һ������pidӦ�ô�1��ʼ*/
	.last_pid = 0,
	/*�������ռ���*/
	.level = 0,
	/*�����ռ��еĳ�ʼ�����̣�������ȫ�ֳ�ʼ�����̣��ɽ��չ¶�����*/
	.child_reaper = &init_task,
};
EXPORT_SYMBOL_GPL(init_pid_ns);

/*���Խ����Ƿ����������ռ��У��������ƣ��ĳ�ʼ�����̣��������ײ������ռ��еľֲ�pid
���Ϊ1����˵���������������ռ�ĳ�ʼ�����̣������ڼ�ʹ��rcu���Ʊ���*/
int is_container_init(struct task_struct *tsk)
{
	int ret = 0;
	struct pid *pid;
	/*rcu�����»�ȡ���̵�pid��Ϣ*/
	rcu_read_lock();
	/*��ȡ���̵�pid��Ϣ*/
	pid = task_pid(tsk);
	/*�жϽ�����ײ������ռ�ľֲ�pid����Ƿ���1*/
	if (pid != NULL && pid->numbers[pid->level].nr == 1)
		ret = 1;
	rcu_read_unlock();

	return ret;
}
EXPORT_SYMBOL(is_container_init);

/*
 * Note: disable interrupts while the pidmap_lock is held as an
 * interrupt might come in and do read_lock(&tasklist_lock).
 *
 * If we don't disable interrupts there is a nasty deadlock between
 * detach_pid()->free_pid() and another cpu that does
 * spin_lock(&pidmap_lock) followed by an interrupt routine that does
 * read_lock(&tasklist_lock);
 *
 * After we clean up the tasklist_lock and know there are no
 * irq handlers that take it we can leave the interrupts enabled.
 * For now it is easier to be safe than to prove it can't happen.
 */
/*����һ��SMPϵͳ��ҳ����ı���pidλͼ������������������pidmap_lockʱ���ܱ��жϣ���ִ��
read_lock(&tasklist_lock)ʱ���ܻ��ж�*/
static  __cacheline_aligned_in_smp DEFINE_SPINLOCK(pidmap_lock);

/*�ͷžֲ�pid���������ռ���λͼ�ж�Ӧ��λ*/
static fastcall void free_pidmap(struct pid_namespace *pid_ns, int pid)
{
	/*��ȡpid���������������е�λͼ*/
	struct pidmap *map = pid_ns->pidmap + pid / BITS_PER_PAGE;
	/*���pid��λͼ�е�ƫ��*/
	int offset = pid & BITS_PER_PAGE_MASK;
	/*���λͼ�е�pid��Ӧ�ı���λ*/
	clear_bit(offset, map->page);
	/*����λͼ�п��õ�pid��Ŀ*/
	atomic_inc(&map->nr_free);
}

/*��ָ�������ռ��з���pid���ɹ��򷵻�λͼƫ��������λͼ�����еı�ţ�����ʧ�ܷ���-1*/
static int alloc_pidmap(struct pid_namespace *pid_ns)
{
	int i, offset, max_scan, pid;
	/*�ϴγɹ�����pid��λ��*/
	int last = pid_ns->last_pid;
	struct pidmap *map;
	/*���ҿ�����ʼλ*/
	pid = last + 1;
	/*����Ѿ��ﵽ��λͼ��ĩβ�������õ�����λ����������*/
	if (pid >= pid_max)
		pid = RESERVED_PIDS;
	/*��ȡpid�ڴ�λͼ�е�ƫ��*/
	offset = pid & BITS_PER_PAGE_MASK;
	/*��ȡpid�������ռ�����λͼ�е�λͼ*/
	map = &pid_ns->pidmap[pid/BITS_PER_PAGE];
	/*��ȡϵͳ����ɨ��λͼ����Ŀ��IA-32��ֻ��һ��λͼ�������ɨ��2�Σ�����һ��ɨ��λͼ��û
	�п���λ��Ȼ����ʼλ��λ��ϵͳ����λ���ٴ�ɨ��󣬷��ֻ���û�п���λ������ʧ���˳�*/
	max_scan = (pid_max + BITS_PER_PAGE - 1)/BITS_PER_PAGE - !offset;
	for (i = 0; i <= max_scan; ++i)
	{
		/*λͼ���״�ʹ��ʱ�ŷ���*/
		if (unlikely(!map->page))
		{
			/*����һ��ҳ�汣��pidλͼ*/
			void *page = kzalloc(PAGE_SIZE, GFP_KERNEL);
			/*����ɹ�������λͼ������������н���Ҳ�ڴ�����λͼ����̬�������ͷŸ�λͼҳ*/
			spin_lock_irq(&pidmap_lock);
			if (map->page)
				kfree(page);
			else
				map->page = page;
			spin_unlock_irq(&pidmap_lock);
			/*λͼ����ʧ�ܣ�����*/
			if (unlikely(!map->page))
				break;
		}
		/*���״�ʹ��λͼ����ȡ��ǰλͼ�п���λ��Ŀ*/
		if (likely(atomic_read(&map->nr_free)))
		{
			do 
			{
				/*���Բ���λ�Ƿ����*/
				if (!test_and_set_bit(offset, map->page))
				{
					/*����λ���ã����ø�λ������λͼ�п���λ��Ŀ*/
					atomic_dec(&map->nr_free);
					/*�����ϴ��ѳɹ�����λͼ������*/
					pid_ns->last_pid = pid;
					/*λͼ�ɹ����䣬ֱ�ӷ���*/
					return pid;
				}
				/*��ǰ��ʼλ�����ã�������һ�����õ���ʼλ*/
				offset = find_next_offset(map, offset);
				/*������һ��������ʼλ����������λͼ�ж�Ӧ�ı��*/
				pid = mk_pid(pid_ns, map, offset);
			 /*find_next_offset()����һ������λ��pid���ڸ����䣬������ҵ�����pidλͼ��
			��û���ҵ����ʹ���ʼλͼ������ʼ���ң���ʱpid���ҪС���ϴγɹ�����pid�ı�ţ�
			Ҳ���ǻ��Ʋ���*/
			/*ע������while������
				offset < BITS_PER_PAGE offset������Ҫ��ʽ��Ϊҳ�ڱ���λ���
				pid < pid_max ϵͳҪ��
				i != max_scan ɨ��δ����
				pid < last ���ƴ�ͷ��ʼ�����ڼ�
				!(last+1)&BITS_PER_PAGE_MASK �Ƿ��Ѵﵽ��λͼ��ĩβ
			*/
			} while (offset < BITS_PER_PAGE && pid < pid_max &&
					(i != max_scan || pid < last ||
					    !((last+1) & BITS_PER_PAGE_MASK)));
		}
		/*λͼ���黹��δɨ��λͼ������������BITS_PER_LONG != 32λ��ϵͳ�г�������64λλϵͳ��
		IA-32ֻ��һ��λͼ��������������*/
		if (map < &pid_ns->pidmap[(pid_max-1)/BITS_PER_PAGE])
		{
			/*�Ӻ���λͼ��0��ʼλ��ʼ������������λ*/
			++map;
			offset = 0;
		} 
		else
		{
			/*IA-32ϵͳ�У�ֻ��һ��λͼ*/
			map = &pid_ns->pidmap[0];
			/*����ѯλ�ö�λ����������ʼλ*/
			offset = RESERVED_PIDS;
			/*���������ʼλ��λͼĩβ��û�п���λ����ʧ��������if����˵�����ϴβ�ѯ��
			û�п���λ��ѭ��һ�λ���û�п���λ*/
			if (unlikely(last == offset))
				break;
		}
		/*����pidλͼ����λͼ�е�ƫ��λ������pid����ֵ*/
		pid = mk_pid(pid_ns, map, offset);
	}
	return -1;
}

/*�������ռ�������λͼ��ƫ��λ��ʼ����������λͼ���飨������λͼ������һ������λ�ı���λ��
���ҳɹ��򷵻ض�Ӧ��λͼƫ����������Чλͼ�еı�ţ����򣬷���-1*/
static int next_pidmap(struct pid_namespace *pid_ns, int last)
{
	int offset;
	struct pidmap *map, *end;

	/*���������ʼλ����λͼ�е�ƫ��*/
	offset = (last + 1) & BITS_PER_PAGE_MASK;
	/*������ҿ�ʼ��λ��������λͼ*/
	map = &pid_ns->pidmap[(last + 1)/BITS_PER_PAGE];
	/*���ҽ���λͼ*/
	end = &pid_ns->pidmap[PIDMAP_ENTRIES];
	/*�ӵ�ǰλͼ��ʼ��ѭ����������λͼ������λͼ*/
	for (; map < end; map++, offset = 0) 
	{	
		/*λͼδ���䣬����*/
		if (unlikely(!map->page))
			continue;
		/*����λͼ�д�offset��ʼλ��ʼ����һ����λ��λ*/
		offset = find_next_bit((map)->page, BITS_PER_PAGE, offset);
		/*�ҵ�������λ������λͼ���飨��ȥ��λͼ���еı�ţ�Ȼ�󷵻�*/
		if (offset < BITS_PER_PAGE)
			return mk_pid(pid_ns, map, offset);
	}
	return -1;
}

/*ȡ����pid������*/
fastcall void put_pid(struct pid *pid)
{
	struct pid_namespace *ns;
	/*��Чpidֱ�ӷ���*/
	if (!pid)
		return;
	/*��ȡpid��ײ�������ռ�*/
	ns = pid->numbers[pid->level].ns;
	/*pid�ṹֻ������һ�Σ����ͷŸ�pid�����Ϣ*/
	if ((atomic_read(&pid->count) == 1) || atomic_dec_and_test(&pid->count)) 
	{
		/*�ͷ�pid�����Ӧ�Ļ���*/
		kmem_cache_free(ns->pid_cachep, pid);
		/*ȡ����pid���������ռ������*/
		put_pid_ns(ns);
	}
}
EXPORT_SYMBOL_GPL(put_pid);

/*����rcu�����ͷŶ�pid������*/
static void delayed_put_pid(struct rcu_head *rhp)
{
	struct pid *pid = container_of(rhp, struct pid, rcu);
	put_pid(pid);
}

/*ʹ��rcu�����ͷ�pid��Ӧ��Ϣ�����ɢ�б���ɾ��pid��Ϣ���ͷ�pid�ڸ��������ռ���ռ�õ�λ���ͷ�
˳���봴��˳���෴*/
fastcall void free_pid(struct pid *pid)
{
	/* We can be called with write_lock_irq(&tasklist_lock) held */
	int i;
	unsigned long flags;
	/*��ȡλͼ�������������ñ���CPU�ж�*/
	spin_lock_irqsave(&pidmap_lock, flags);
	/*���pidɢ�б���Ϣ������㿪ʼ�����pidɢ�б���ɾ��*/
	for (i = 0; i <= pid->level; i++)
		hlist_del_rcu(&pid->numbers[i].pid_chain);
	/*���ɢ�б���Ϣ���ͷ�λͼ�������������ñ���cpu�ж�*/
	spin_unlock_irqrestore(&pidmap_lock, flags);
	/*�ͷžֲ�pid��������������ռ�λͼ�ж�Ӧ��λ������������ռ俪ʼ*/
	for (i = 0; i <= pid->level; i++)
		free_pidmap(pid->numbers[i].ns, pid->numbers[i].nr);
	/*ͬ��rcu���ȴ��ͷ�ռ��λ��ɺ���ȡ����pid������*/
	call_rcu(&pid->rcu, delayed_put_pid);
}

/*�������ռ��д���pid��Ϣ*/
struct pid *alloc_pid(struct pid_namespace *ns)
{
	struct pid *pid;
	enum pid_type type;
	int i, nr;
	struct pid_namespace *tmp;
	struct upid *upid;
	/*Ϊpid�����ռ�����slab����*/
	pid = kmem_cache_alloc(ns->pid_cachep, GFP_KERNEL);
	if (!pid)
		goto out;

	/*�����ռ�����ɹ��󣬴���ײ������ռ俪ʼ���ֱ�Ϊ��pid����ֲ�pid��Ϣ����ʱ������
	����pid��ص�ɢ�б���Ϣ�������ռ��νṹ��ȫ��ɹ���Ų���ɢ�б�*/
	tmp = ns;
	for (i = ns->level; i >= 0; i--)
	{
		/*����ֲ�pid���*/
		nr = alloc_pidmap(tmp);
		if (nr < 0)
		{
			/*����ʧ�ܣ���ת�ͷ����������Դ*/
			goto out_free;
		}
		/*����ɹ���ֵ�ֲ�pid���*/
		pid->numbers[i].nr = nr;
		/*��ʼ�������ռ�*/
		pid->numbers[i].ns = tmp;
		/*���ø������ռ�ĸ������ռ�*/
		tmp = tmp->parent;
	}
	/*������ײ������ռ�*/
	get_pid_ns(ns);
	/*����pid���Կ����������ռ��������*/
	pid->level = ns->level;
	/*��ʼ��pid�����ռ������*/
	atomic_set(&pid->count, 1);
	/*��ʼ��pid��pid/pgid/sidɢ�б�*/
	for (type = 0; type < PIDTYPE_MAX; ++type)
		INIT_HLIST_HEAD(&pid->tasks[type]);
	/*��pidmap_lock���ı����£�������������ռ���ӵ�ɢ�б���*/
	spin_lock_irq(&pidmap_lock);
	for (i = ns->level; i >= 0; i--)
	{
		/*��ȡ�ض�pid�����ռ��пɼ�����Ϣ*/
		upid = &pid->numbers[i];
		/*���ض������ռ���Ϣ��ӵ�pidɢ�б���*/
		hlist_add_head_rcu(&upid->pid_chain, &pid_hash[pid_hashfn(upid->nr, upid->ns)]);
	}
	spin_unlock_irq(&pidmap_lock);

out:
	/*�ɹ�����pid������*/
	return pid;

out_free:
	/*���������ռ��νṹʧ��ʱ��Ҫ�ͷ�ǰ��������ľֲ�pid��Ӧλͼ�е�λ*/
	for (i++; i <= ns->level; i++)
		free_pidmap(pid->numbers[i].ns, pid->numbers[i].nr);
	/*�ͷ�Ϊpid�����ռ��������slab����*/
	kmem_cache_free(ns->pid_cachep, pid);
	/*����pidʧ�ܣ�����NULL*/
	pid = NULL;
	goto out;
}

/*���������ռ估��Ӧ�ľֲ�pid��ţ�����ɢ�к�����ѯ����Ӧ��ɢ�б�Ȼ�������ɢ�б���
����ɢ�н�㣬�ҵ���Ӧ��ɢ�е㣬������container_of���ƻ�ȡpid��Ϣ*/
struct pid * fastcall find_pid_ns(int nr, struct pid_namespace *ns)
{
	struct hlist_node *elem;
	struct upid *pnr;
	/*���������ռ估��Ӧ��pid�����ȷ����ɢ�к�������λ����Ӧ��ɢ�б�ͷ��������ɢ�б��ѯpid*/
	hlist_for_each_entry_rcu(pnr, elem, &pid_hash[pid_hashfn(nr, ns)], pid_chain)
		if (pnr->nr == nr && pnr->ns == ns)
		{
			/*����pid�ض��������ռ���Ϣ������container_of���ƻ�ȡpid��Ϣ*/
			return container_of(pnr, struct pid, numbers[ns->level]);
		}
	return NULL;
}
EXPORT_SYMBOL_GPL(find_pid_ns);

/*���ݵ�ǰ���̵�pid�����ռ����ֲ�pid��ţ���ѯ��ǰ���̵�pid��Ϣ*/
struct pid *find_vpid(int nr)
{
	return find_pid_ns(nr, current->nsproxy->pid_ns);
}
EXPORT_SYMBOL_GPL(find_vpid);

/*��ѯ��ǰ���̵ĸ������ռ��е�pid��Ϣ*/
struct pid *find_pid(int nr)
{
	return find_pid_ns(nr, &init_pid_ns);
}
EXPORT_SYMBOL_GPL(find_pid);


/*��task_struct���ӵ���ͷ��struct pid�е�ɢ�б���*/
int fastcall attach_pid(struct task_struct *task, enum pid_type type,
							struct pid *pid)
{
	/*pid_link���Խ�task_struct���ӵ���ͷ��struct pid�е�ɢ�б���*/
	struct pid_link *link;
	/*��ʼ�����̵�pid_link��Ϣ*/
	link = &task->pids[type];
	link->pid = pid;
	/*������task_struct���ӵ�struct pid��Ӧ���͵�ɢ�б���*/
	hlist_add_head_rcu(&link->node, &pid->tasks[type]);

	return 0;
}

/*��ʼ�����̵�pid_link��Ϣ*/
void fastcall detach_pid(struct task_struct *task, enum pid_type type)
{
	struct pid_link *link;
	struct pid *pid;
	int tmp;

	/*��ȡ���̵�pidɢ������Ϣ*/
	link = &task->pids[type];
	/*��ȡ���̵�pid��Ϣ*/
	pid = link->pid;
	/*�����̴�pidɢ�б���ɾ��*/
	hlist_del_rcu(&link->node);
	link->pid = NULL;
	/*������̵�pidɢ�б�Ϊ�գ����ͷŸ�pid*/
	for (tmp = PIDTYPE_MAX; --tmp >= 0; )
		/*pidɢ�б�ǿգ�ֱ�ӷ���*/
		if (!hlist_empty(&pid->tasks[tmp]))
			return;
	/*pidɢ�б�Ϊ�գ��ͷ�pid*/
	free_pid(pid);
}

/* transfer_pid��attach_pid(new)��detach_pid(old)��һ���Ż������� */
void fastcall transfer_pid(struct task_struct *old, struct task_struct *new,
			   enum pid_type type)
{
	/**/
	new->pids[type].pid = old->pids[type].pid;
	/**/
	hlist_replace_rcu(&old->pids[type].node, &new->pids[type].node);
	/**/
	old->pids[type].pid = NULL;
}

/*����pid��pid���ͣ���ȡ����ʵ��*/
struct task_struct * fastcall pid_task(struct pid *pid, enum pid_type type)
{
	struct task_struct *result = NULL;
	if (pid)
	{	
		struct hlist_node *first;
		/*pid�ǿ�ʱ��ͨ��rcu���ƣ���ȡpidָ�����͵�ɢ�б��һ�����*/
		first = rcu_dereference(pid->tasks[type].first);
		/*ɢ�б�ǿ�ʱͨ��container_of���ƻ�ȡ��Ӧ�Ľ���ʵ��*/
		if (first)
			result = hlist_entry(first, struct task_struct, pids[(type)].node);
	}
	return result;
}

/*���ݽ������ͣ������ռ估��Ӧ�ľֲ�pid��ţ���ȡ����ʵ����������rcu_read_lock()����
tasklist_lock�������ڼ��*/
struct task_struct *find_task_by_pid_type_ns(int type, int nr,
		struct pid_namespace *ns)
{
	return pid_task(find_pid_ns(nr, ns), type);
}
EXPORT_SYMBOL(find_task_by_pid_type_ns);

/*����ȫ�������ռ�pid��ţ���ȡ����ʵ��*/
struct task_struct *find_task_by_pid(pid_t nr)
{
	return find_task_by_pid_type_ns(PIDTYPE_PID, nr, &init_pid_ns);
}
EXPORT_SYMBOL(find_task_by_pid);

/*���ݵ�ǰ����pid�����ռ�ľֲ�pid��ţ���ȡ��ǰ���̵�ʵ��*/
struct task_struct *find_task_by_vpid(pid_t vnr)
{
	return find_task_by_pid_type_ns(PIDTYPE_PID, vnr, current->nsproxy->pid_ns);
}
EXPORT_SYMBOL(find_task_by_vpid);

/*�����ض������ռ�ľֲ�pid��ţ���ȡ��Ӧ�Ľ���ʵ��*/
struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns)
{
	return find_task_by_pid_type_ns(PIDTYPE_PID, nr, ns);
}
EXPORT_SYMBOL(find_task_by_pid_ns);

/*����rcu���ƣ����ý����ض����͵�pidʵ��*/
struct pid *get_task_pid(struct task_struct *task, enum pid_type type)
{
	struct pid *pid;
	rcu_read_lock();
	pid = get_pid(task->pids[type].pid);
	rcu_read_unlock();
	return pid;
}

/*����pidʵ�������ض����͵Ľ���ʵ��*/
struct task_struct *fastcall get_pid_task(struct pid *pid, enum pid_type type)
{
	struct task_struct *result;
	rcu_read_lock();
	/*����pidʵ�����ض����ͣ���ȡ����ʵ��*/
	result = pid_task(pid, type);
	/*����ʵ����Ч�����ý���*/
	if (result)
		get_task_struct(result);
	rcu_read_unlock();
	return result;
}

/*���ݵ�ǰ���̵ľֲ�pid��ţ���ȡ��ǰ���̵�pidʵ����Ȼ�����ø�ʵ����������Ҫ���
rcu_read_lock������*/
struct pid *find_get_pid(pid_t nr)
{
	struct pid *pid;

	rcu_read_lock();
	pid = get_pid(find_vpid(nr));
	rcu_read_unlock();

	return pid;
}

/*��ȡpidʵ�����ض������ռ��еľֲ�pid���*/
pid_t pid_nr_ns(struct pid *pid, struct pid_namespace *ns)
{
	struct upid *upid;
	pid_t nr = 0;

	if (pid && ns->level <= pid->level)
	{
		/*pidʵ����Ч��pid�����������ռ����ڸþֲ������ռ���֮�ϣ����Է��ʸ������ռ䣬
	��ȡ���ض������ռ���Ϣ*/
		upid = &pid->numbers[ns->level];
		/*����������ռ��Ǹ�pid��level�����ռ䣬���ȡ�������ռ��Ӧ�ľֲ�pid���*/
		if (upid->ns == ns)
			nr = upid->nr;
	}
	return nr;
}

/*��ȡ���̵��ض�pid�����ռ��еľֲ�pid���*/
pid_t task_pid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_pid(tsk), ns);
}
EXPORT_SYMBOL(task_pid_nr_ns);

/*��ȡ�����ض�pid�����ռ����߳����鳤��pid*/
pid_t task_tgid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_tgid(tsk), ns);
}
EXPORT_SYMBOL(task_tgid_nr_ns);

/*��ȡ�����ض�pid�����ռ��н������鳤�ľֲ�pid���*/
pid_t task_pgrp_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_pgrp(tsk), ns);
}
EXPORT_SYMBOL(task_pgrp_nr_ns);

/*��ý����ض�pid�����ռ��лỰ���鳤��pid���*/
pid_t task_session_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_session(tsk), ns);
}
EXPORT_SYMBOL(task_session_nr_ns);

/*��ȡpid�ض������ռ��в�С�ھֲ�pid�ı������Ӧ��pidʵ����������ѯ��һ����С���ض�
 pid��ŵ�pidʵ��������պ���һ���ֲ�pid��Ŷ�Ӧ��pidʵ�����ú�����ͬ��find_pid*/
struct pid *find_ge_pid(int nr, struct pid_namespace *ns)
{
	struct pid *pid;

	do
	{	/*���ݾֲ������ռ����ֲ�pid��ţ���ȡ��Ӧ��pidʵ��*/
		pid = find_pid_ns(nr, ns);
		/*�ҵ��˳�*/
		if (pid)
			break;
		/*�ڸ������ռ��λͼ�У���nrλ��ʼ������һ����λ�ı���λ����*/
		nr = next_pidmap(ns, nr);
	} while (nr > 0);

	return pid;
}
EXPORT_SYMBOL_GPL(find_get_pid);

/**/

struct pid_cache
{
	/**/
	int nr_ids;
	/**/
	char name[16];
	/**/
	struct kmem_cache *cachep;
	/**/
	struct list_head list;
};

/**/
static LIST_HEAD(pid_caches_lh);
/**/
static DEFINE_MUTEX(pid_caches_mutex);

/*
 * creates the kmem cache to allocate pids from.
 * @nr_ids: the number of numerical ids this pid will have to carry
 */

/*Ϊpidʵ������slab����*/
static struct kmem_cache *create_pid_cachep(int nr_ids)
{
	struct pid_cache *pcache;
	struct kmem_cache *cachep;

	mutex_lock(&pid_caches_mutex);
	list_for_each_entry (pcache, &pid_caches_lh, list)
		if (pcache->nr_ids == nr_ids)
			goto out;

	pcache = kmalloc(sizeof(struct pid_cache), GFP_KERNEL);
	if (pcache == NULL)
		goto err_alloc;

	snprintf(pcache->name, sizeof(pcache->name), "pid_%d", nr_ids);
	cachep = kmem_cache_create(pcache->name,
			sizeof(struct pid) + (nr_ids - 1) * sizeof(struct upid),
			0, SLAB_HWCACHE_ALIGN, NULL);
	if (cachep == NULL)
		goto err_cachep;

	pcache->nr_ids = nr_ids;
	pcache->cachep = cachep;
	list_add(&pcache->list, &pid_caches_lh);
out:
	mutex_unlock(&pid_caches_mutex);
	return pcache->cachep;

err_cachep:
	kfree(pcache);
err_alloc:
	mutex_unlock(&pid_caches_mutex);
	return NULL;
}

#ifdef CONFIG_PID_NS
/**/
static struct pid_namespace *create_pid_namespace(int level)
{
	struct pid_namespace *ns;
	int i;

	ns = kmem_cache_alloc(pid_ns_cachep, GFP_KERNEL);
	if (ns == NULL)
		goto out;

	ns->pidmap[0].page = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!ns->pidmap[0].page)
		goto out_free;

	ns->pid_cachep = create_pid_cachep(level + 1);
	if (ns->pid_cachep == NULL)
		goto out_free_map;

	kref_init(&ns->kref);
	ns->last_pid = 0;
	ns->child_reaper = NULL;
	ns->level = level;

	set_bit(0, ns->pidmap[0].page);
	atomic_set(&ns->pidmap[0].nr_free, BITS_PER_PAGE - 1);

	for (i = 1; i < PIDMAP_ENTRIES; i++) {
		ns->pidmap[i].page = 0;
		atomic_set(&ns->pidmap[i].nr_free, BITS_PER_PAGE);
	}

	return ns;

out_free_map:
	kfree(ns->pidmap[0].page);
out_free:
	kmem_cache_free(pid_ns_cachep, ns);
out:
	return ERR_PTR(-ENOMEM);
}

/*�ͷ������ռ�����λͼ��Ӧ��ҳ�漰֮ǰ�����slab����*/
static void destroy_pid_namespace(struct pid_namespace *ns)
{
	int i;
	/*�ͷ�����λͼҳ*/
	for (i = 0; i < PIDMAP_ENTRIES; i++)
		kfree(ns->pidmap[i].page);
	kmem_cache_free(pid_ns_cachep, ns);
}

/*����pid�����ռ�*/
struct pid_namespace *copy_pid_ns(unsigned long flags, struct pid_namespace *old_ns)
{
	struct pid_namespace *new_ns;
	/*�����pid�����ռ䲻��Ϊ��*/
	BUG_ON(!old_ns);
	/*���������pid�����ռ�*/
	new_ns = get_pid_ns(old_ns);
	/*�Ǹ��ƵĹ���pid�����ռ�*/
	if (!(flags & CLONE_NEWPID))
		goto out;
	/*Ϊɶ�ȸ�ֵ��Ч���������ڴ治������룬���Ҳ��ǽ���create_pid_namespace*/
	new_ns = ERR_PTR(-EINVAL);
	/*ͬһ�߳���ҳ����pid�����ռ�*/
	if (flags & CLONE_THREAD)
		goto out_put;
	/*�����������ռ�*/
	new_ns = create_pid_namespace(old_ns->level + 1);
	if (!IS_ERR(new_ns))
		new_ns->parent = get_pid_ns(old_ns);

out_put:
	put_pid_ns(old_ns);
out:
	return new_ns;
}

/*�ͷ�kref��Ӧ��pid�����ռ�*/
void free_pid_ns(struct kref *kref)
{
	struct pid_namespace *ns, *parent;
	/*ͨ��container_of���ƻ�ȡkref��Ӧ��pid�����ռ�*/
	ns = container_of(kref, struct pid_namespace, kref);
	/*��ȡ�丸�����ռ�*/
	parent = ns->parent;
	destroy_pid_namespace(ns);
	/*����������ռ���ڣ���ȡ���Ը������ռ������*/
	if (parent != NULL)
		put_pid_ns(parent);
}
#endif /* CONFIG_PID_NS */

void zap_pid_ns_processes(struct pid_namespace *pid_ns)
{
	int nr;
	int rc;

	/*
	 * The last thread in the cgroup-init thread group is terminating.
	 * Find remaining pid_ts in the namespace, signal and wait for them
	 * to exit.
	 *
	 * Note:  This signals each threads in the namespace - even those that
	 * 	  belong to the same thread group, To avoid this, we would have
	 * 	  to walk the entire tasklist looking a processes in this
	 * 	  namespace, but that could be unnecessarily expensive if the
	 * 	  pid namespace has just a few processes. Or we need to
	 * 	  maintain a tasklist for each pid namespace.
	 *
	 */
	read_lock(&tasklist_lock);
	nr = next_pidmap(pid_ns, 1);
	while (nr > 0) {
		kill_proc_info(SIGKILL, SEND_SIG_PRIV, nr);
		nr = next_pidmap(pid_ns, nr);
	}
	read_unlock(&tasklist_lock);

	do {
		clear_thread_flag(TIF_SIGPENDING);
		rc = sys_wait4(-1, NULL, __WALL, NULL);
	} while (rc != -ECHILD);


	/* Child reaper for the pid namespace is going away */
	pid_ns->child_reaper = NULL;
	return;
}

/*pidɢ�б��������ݻ��������ڴ�����������㣬ÿGB����С16��4096�����߸��󣬵�������
������4096��������*/
void __init pidhash_init(void)
{
	int i, pidhash_size;
	/*��MBΪ��λ���������ڴ������*/
	unsigned long megabytes = nr_kernel_pages >> (20 - PAGE_SHIFT);
	/*��IA-32ϵͳ128M�ڴ�Ϊ����0x80����Ӧ�����Ľ��ʱ10*/
	pidhash_shift = max(4, fls(megabytes * 4));
	pidhash_shift = min(12, pidhash_shift);
	/*����Ľ����4��12֮�䣬���������Ϊ10����Ӧ��ɢ�б�������Ŀ����1024*/
	pidhash_size = 1 << pidhash_shift;
	/*��ʾɢ�б�������Ŀ������׺�Ӧ��Ϊ��ɢ�б�ͷ����Ŀռ�*/
	printk("PID hash table entries: %d (order: %d, %Zd bytes)\n", pidhash_size, pidhash_shift,
				pidhash_size * sizeof(struct hlist_head));
	/*Ϊɢ�б����ռ�*/
	pid_hash = alloc_bootmem(pidhash_size *	sizeof(*(pid_hash)));
	/*�ڴ治��һ�*/
	if (!pid_hash)
		panic("Could not alloc pidhash!\n");
	/*��ʼ���ѷ����ɢ�б�ͷ*/
	for (i = 0; i < pidhash_size; i++)
		INIT_HLIST_HEAD(&pid_hash[i]);
}

/*�������ռ��pid�����ռ���pidλͼ��ʼ��*/
void __init pidmap_init(void)
{
	/*����һ��ҳ֡���ڱ���pidλͼ*/
	init_pid_ns.pidmap[0].page = kzalloc(PAGE_SIZE, GFP_KERNEL);
	/* Reserve PID 0. We never call free_pidmap(0) */
	/*����λͼ�е�һ��λ��������pid 0����λ�������ͷ�*/
	set_bit(0, init_pid_ns.pidmap[0].page);
	/*λͼ�п���λ��1*/
	atomic_dec(&init_pid_ns.pidmap[0].nr_free);
	
	init_pid_ns.pid_cachep = create_pid_cachep(1);
	if (init_pid_ns.pid_cachep == NULL)
		panic("Can't create pid_1 cachep\n");
	/*����pid�����ռ�slab��*/
	pid_ns_cachep = KMEM_CACHE(pid_namespace, SLAB_PANIC);
}
