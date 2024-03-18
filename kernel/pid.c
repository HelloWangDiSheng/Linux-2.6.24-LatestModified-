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

/*将命名空间中pid编号和该命名空间的地址的加值作为计算因子，通过与GOLDEN_RATIO_PRIME进行
一些列计算计算得到散列值，IA32上计算结果是计算因子*GOLDEN_RATIO_PRIME>>22，把因子转化为
表索引*/
#define pid_hashfn(nr, ns)	\
	hash_long((unsigned long)nr + (unsigned long)ns, pidhash_shift)
/*为在给定的命名空间中查找对应于指定pid数值的pid结构实例，使用了散列表*/
static struct hlist_head *pid_hash;
/*存放散列表索引长度偏移，系统初始化期间赋值，IA-32上物理内存大于128M及以上时，该值为10*/
static int pidhash_shift;
/*根pid命名空间初始化*/
struct pid init_struct_pid = INIT_STRUCT_PID;
/*pid命名空间在slab缓存上的地址*/
static struct kmem_cache *pid_ns_cachep;
/*系统允许的pid最大编号*/
int pid_max = PID_MAX_DEFAULT;
/*保留的pid数目*/
#define RESERVED_PIDS		300
/*默认分配的pid下限，从301开始*/
int pid_max_min = RESERVED_PIDS + 1;
/*默认的pid上限*/
int pid_max_max = PID_MAX_LIMIT;
/*每页中的比特位数目：32768*/
#define BITS_PER_PAGE		(PAGE_SIZE*8)
#define BITS_PER_PAGE_MASK	(BITS_PER_PAGE-1)
/*根据pid所在命名空间中位图的偏移位置，计算pid的实际值*/
static inline int mk_pid(struct pid_namespace *pid_ns, struct pidmap *map, int off)
{
	/*计算pid相对于起始位图已经跨越的编号数目*/
	return (map - pid_ns->pidmap)*BITS_PER_PAGE + off;
}
/*查询位图内，从偏移处开始的下一个可用（未置位）位的编号*/
#define find_next_offset(map, off)	find_next_zero_bit((map)->page, BITS_PER_PAGE, off)

/*根命名空间中pid命名空间初始化*/
 struct pid_namespace init_pid_ns =
{
	.kref =
	{
		.refcount       = ATOMIC_INIT(2),
	},
	/*pid位图页刚开始时为NULL，首次使用前分配内存，并且从不释放。较小的pid上限值不会导致
	分配很多位图，除非运行时pid系统规模到达4M*/
	.pidmap =
	{
		[ 0 ... PIDMAP_ENTRIES-1] = { ATOMIC_INIT(BITS_PER_PAGE), NULL }
	},
	/*初始化过程中将第一个比特位保留，第一次申请pid应该从1开始*/
	.last_pid = 0,
	/*根命名空间层次*/
	.level = 0,
	/*命名空间中的初始化进程，类似于全局初始化进程，可接收孤儿进程*/
	.child_reaper = &init_task,
};
EXPORT_SYMBOL_GPL(init_pid_ns);

/*测试进程是否是其命名空间中（功能类似）的初始化进程，如果其最底层命名空间中的局部pid
编号为1，则说明进程是其命名空间的初始化进程，操作期间使用rcu机制保护*/
int is_container_init(struct task_struct *tsk)
{
	int ret = 0;
	struct pid *pid;
	/*rcu机制下获取进程的pid信息*/
	rcu_read_lock();
	/*获取进程的pid信息*/
	pid = task_pid(tsk);
	/*判断进程最底层命名空间的局部pid编号是否是1*/
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
/*定义一个SMP系统上页对齐的保护pid位图操作的自旋锁。持有pidmap_lock时不能被中断，在执行
read_lock(&tasklist_lock)时可能会中断*/
static  __cacheline_aligned_in_smp DEFINE_SPINLOCK(pidmap_lock);

/*释放局部pid所属命名空间中位图中对应的位*/
static fastcall void free_pidmap(struct pid_namespace *pid_ns, int pid)
{
	/*获取pid在其所属命名空中的位图*/
	struct pidmap *map = pid_ns->pidmap + pid / BITS_PER_PAGE;
	/*获得pid在位图中的偏移*/
	int offset = pid & BITS_PER_PAGE_MASK;
	/*清空位图中的pid对应的比特位*/
	clear_bit(offset, map->page);
	/*更新位图中可用的pid数目*/
	atomic_inc(&map->nr_free);
}

/*在指定命名空间中分配pid，成功则返回位图偏移在整个位图数组中的编号，否则，失败返回-1*/
static int alloc_pidmap(struct pid_namespace *pid_ns)
{
	int i, offset, max_scan, pid;
	/*上次成功分配pid的位置*/
	int last = pid_ns->last_pid;
	struct pidmap *map;
	/*查找空闲起始位*/
	pid = last + 1;
	/*如果已经达到该位图的末尾，则重置到保留位，继续查找*/
	if (pid >= pid_max)
		pid = RESERVED_PIDS;
	/*获取pid在此位图中的偏移*/
	offset = pid & BITS_PER_PAGE_MASK;
	/*获取pid在命名空间数组位图中的位图*/
	map = &pid_ns->pidmap[pid/BITS_PER_PAGE];
	/*获取系统中需扫描位图的数目，IA-32上只有一个位图，最多需扫描2次（即第一次扫描位图，没
	有空闲位，然后将起始位定位到系统保留位，再次扫描后，发现还是没有空闲位），则失败退出*/
	max_scan = (pid_max + BITS_PER_PAGE - 1)/BITS_PER_PAGE - !offset;
	for (i = 0; i <= max_scan; ++i)
	{
		/*位图在首次使用时才分配*/
		if (unlikely(!map->page))
		{
			/*申请一个页面保存pid位图*/
			void *page = kzalloc(PAGE_SIZE, GFP_KERNEL);
			/*申请成功后启用位图锁保护，如果有进程也在创建给位图（竟态），则释放该位图页*/
			spin_lock_irq(&pidmap_lock);
			if (map->page)
				kfree(page);
			else
				map->page = page;
			spin_unlock_irq(&pidmap_lock);
			/*位图申请失败，跳出*/
			if (unlikely(!map->page))
				break;
		}
		/*非首次使用位图，获取当前位图中空闲位数目*/
		if (likely(atomic_read(&map->nr_free)))
		{
			do 
			{
				/*测试查找位是否可用*/
				if (!test_and_set_bit(offset, map->page))
				{
					/*查找位可用，则用该位，更新位图中空闲位数目*/
					atomic_dec(&map->nr_free);
					/*更新上次已成功分配位图的索引*/
					pid_ns->last_pid = pid;
					/*位图成功分配，直接返回*/
					return pid;
				}
				/*当前起始位不可用，查找下一个可用的起始位*/
				offset = find_next_offset(map, offset);
				/*计算下一个可用起始位在整个数组位图中对应的编号*/
				pid = mk_pid(pid_ns, map, offset);
			 /*find_next_offset()查找一个比特位，pid属于该区间，如果查找到最后的pid位图，
			都没有找到，就从起始位图继续开始查找，这时pid编号要小于上次成功分配pid的编号，
			也就是回绕查找*/
			/*注意下面while条件：
				offset < BITS_PER_PAGE offset本来就要格式化为页内比特位编号
				pid < pid_max 系统要求
				i != max_scan 扫描未结束
				pid < last 回绕从头开始查找期间
				!(last+1)&BITS_PER_PAGE_MASK 是否已达到该位图的末尾
			*/
			} while (offset < BITS_PER_PAGE && pid < pid_max &&
					(i != max_scan || pid < last ||
					    !((last+1) & BITS_PER_PAGE_MASK)));
		}
		/*位图数组还有未扫描位图，该条件仅在BITS_PER_LONG != 32位的系统中成立，如64位位系统。
		IA-32只有一个位图，该条件不成立*/
		if (map < &pid_ns->pidmap[(pid_max-1)/BITS_PER_PAGE])
		{
			/*从后续位图的0起始位开始继续搜索可用位*/
			++map;
			offset = 0;
		} 
		else
		{
			/*IA-32系统中，只有一个位图*/
			map = &pid_ns->pidmap[0];
			/*将查询位置定位到保留的起始位*/
			offset = RESERVED_PIDS;
			/*如果保留起始位至位图末尾都没有可用位，则失败跳出，if条件说明从上次查询都
			没有空闲位，循环一次还是没有空闲位*/
			if (unlikely(last == offset))
				break;
		}
		/*根据pid位图及在位图中的偏移位，计算pid的数值*/
		pid = mk_pid(pid_ns, map, offset);
	}
	return -1;
}

/*从命名空间所属的位图中偏移位开始，查找整个位图数组（跳过空位图）中下一个已置位的比特位，
查找成功则返回对应的位图偏移在整个有效位图中的编号，否则，返回-1*/
static int next_pidmap(struct pid_namespace *pid_ns, int last)
{
	int offset;
	struct pidmap *map, *end;

	/*计算查找起始位置在位图中的偏移*/
	offset = (last + 1) & BITS_PER_PAGE_MASK;
	/*计算查找开始的位置所属的位图*/
	map = &pid_ns->pidmap[(last + 1)/BITS_PER_PAGE];
	/*查找结束位图*/
	end = &pid_ns->pidmap[PIDMAP_ENTRIES];
	/*从当前位图开始，循环遍历所有位图至结束位图*/
	for (; map < end; map++, offset = 0) 
	{	
		/*位图未分配，跳过*/
		if (unlikely(!map->page))
			continue;
		/*查找位图中从offset起始位开始的下一个置位的位*/
		offset = find_next_bit((map)->page, BITS_PER_PAGE, offset);
		/*找到则计算该位在整个位图数组（除去空位图）中的编号，然后返回*/
		if (offset < BITS_PER_PAGE)
			return mk_pid(pid_ns, map, offset);
	}
	return -1;
}

/*取消对pid的引用*/
fastcall void put_pid(struct pid *pid)
{
	struct pid_namespace *ns;
	/*无效pid直接返回*/
	if (!pid)
		return;
	/*获取pid最底层的命名空间*/
	ns = pid->numbers[pid->level].ns;
	/*pid结构只被引用一次，则释放该pid相关信息*/
	if ((atomic_read(&pid->count) == 1) || atomic_dec_and_test(&pid->count)) 
	{
		/*释放pid对象对应的缓存*/
		kmem_cache_free(ns->pid_cachep, pid);
		/*取消对pid所属命名空间的引用*/
		put_pid_ns(ns);
	}
}
EXPORT_SYMBOL_GPL(put_pid);

/*采用rcu机制释放对pid的引用*/
static void delayed_put_pid(struct rcu_head *rhp)
{
	struct pid *pid = container_of(rhp, struct pid, rcu);
	put_pid(pid);
}

/*使用rcu机制释放pid对应信息。如从散列表中删除pid信息，释放pid在各个命名空间中占用的位，释放
顺序与创建顺序相反*/
fastcall void free_pid(struct pid *pid)
{
	/* We can be called with write_lock_irq(&tasklist_lock) held */
	int i;
	unsigned long flags;
	/*获取位图保护锁，并禁用本地CPU中断*/
	spin_lock_irqsave(&pidmap_lock, flags);
	/*解除pid散列表信息，从最顶层开始将其从pid散列表中删除*/
	for (i = 0; i <= pid->level; i++)
		hlist_del_rcu(&pid->numbers[i].pid_chain);
	/*解除散列表信息后释放位图保护锁，并启用本地cpu中断*/
	spin_unlock_irqrestore(&pidmap_lock, flags);
	/*释放局部pid编号所属的命名空间位图中对应的位，从最顶层命名空间开始*/
	for (i = 0; i <= pid->level; i++)
		free_pidmap(pid->numbers[i].ns, pid->numbers[i].nr);
	/*同步rcu，等待释放占用位完成后在取消对pid的引用*/
	call_rcu(&pid->rcu, delayed_put_pid);
}

/*在命名空间中创建pid信息*/
struct pid *alloc_pid(struct pid_namespace *ns)
{
	struct pid *pid;
	enum pid_type type;
	int i, nr;
	struct pid_namespace *tmp;
	struct upid *upid;
	/*为pid命名空间申请slab缓存*/
	pid = kmem_cache_alloc(ns->pid_cachep, GFP_KERNEL);
	if (!pid)
		goto out;

	/*命名空间申请成功后，从最底层命名空间开始，分别为该pid申请局部pid信息，此时还不能
	设置pid相关的散列表信息，命名空间层次结构完全搭建成功后才操作散列表*/
	tmp = ns;
	for (i = ns->level; i >= 0; i--)
	{
		/*申请局部pid编号*/
		nr = alloc_pidmap(tmp);
		if (nr < 0)
		{
			/*申请失败，跳转释放已申请的资源*/
			goto out_free;
		}
		/*申请成功后赋值局部pid编号*/
		pid->numbers[i].nr = nr;
		/*初始化命名空间*/
		pid->numbers[i].ns = tmp;
		/*设置该命名空间的父命名空间*/
		tmp = tmp->parent;
	}
	/*引用最底层命名空间*/
	get_pid_ns(ns);
	/*设置pid可以看到的命名空间的最大深度*/
	pid->level = ns->level;
	/*初始化pid命名空间的引用*/
	atomic_set(&pid->count, 1);
	/*初始化pid的pid/pgid/sid散列表*/
	for (type = 0; type < PIDTYPE_MAX; ++type)
		INIT_HLIST_HEAD(&pid->tasks[type]);
	/*在pidmap_lock锁的保护下，将各层次命名空间添加到散列表中*/
	spin_lock_irq(&pidmap_lock);
	for (i = ns->level; i >= 0; i--)
	{
		/*获取特定pid命名空间中可见的信息*/
		upid = &pid->numbers[i];
		/*将特定命名空间信息添加到pid散列表中*/
		hlist_add_head_rcu(&upid->pid_chain, &pid_hash[pid_hashfn(upid->nr, upid->ns)]);
	}
	spin_unlock_irq(&pidmap_lock);

out:
	/*成功分配pid，返回*/
	return pid;

out_free:
	/*构建命名空间层次结构失败时，要释放前面已申请的局部pid对应位图中的位*/
	for (i++; i <= ns->level; i++)
		free_pidmap(pid->numbers[i].ns, pid->numbers[i].nr);
	/*释放为pid命名空间已申请的slab缓存*/
	kmem_cache_free(ns->pid_cachep, pid);
	/*创建pid失败，返回NULL*/
	pid = NULL;
	goto out;
}

/*根据命名空间及对应的局部pid编号，利用散列函数查询到对应的散列表，然后遍历该散列表中
所有散列结点，找到对应的散列点，再利用container_of机制获取pid信息*/
struct pid * fastcall find_pid_ns(int nr, struct pid_namespace *ns)
{
	struct hlist_node *elem;
	struct upid *pnr;
	/*根据命名空间及对应的pid编号锁确定的散列函数，定位到对应的散列表头，遍历该散列表查询pid*/
	hlist_for_each_entry_rcu(pnr, elem, &pid_hash[pid_hashfn(nr, ns)], pid_chain)
		if (pnr->nr == nr && pnr->ns == ns)
		{
			/*根据pid特定的命名空间信息，利用container_of机制获取pid信息*/
			return container_of(pnr, struct pid, numbers[ns->level]);
		}
	return NULL;
}
EXPORT_SYMBOL_GPL(find_pid_ns);

/*根据当前进程的pid命名空间和其局部pid编号，查询当前进程的pid信息*/
struct pid *find_vpid(int nr)
{
	return find_pid_ns(nr, current->nsproxy->pid_ns);
}
EXPORT_SYMBOL_GPL(find_vpid);

/*查询当前进程的根命名空间中的pid信息*/
struct pid *find_pid(int nr)
{
	return find_pid_ns(nr, &init_pid_ns);
}
EXPORT_SYMBOL_GPL(find_pid);


/*将task_struct连接到表头在struct pid中的散列表上*/
int fastcall attach_pid(struct task_struct *task, enum pid_type type,
							struct pid *pid)
{
	/*pid_link可以将task_struct连接到表头在struct pid中的散列表上*/
	struct pid_link *link;
	/*初始化进程的pid_link信息*/
	link = &task->pids[type];
	link->pid = pid;
	/*将进程task_struct连接到struct pid对应类型的散列表上*/
	hlist_add_head_rcu(&link->node, &pid->tasks[type]);

	return 0;
}

/*初始化进程的pid_link信息*/
void fastcall detach_pid(struct task_struct *task, enum pid_type type)
{
	struct pid_link *link;
	struct pid *pid;
	int tmp;

	/*获取进程的pid散列链信息*/
	link = &task->pids[type];
	/*获取进程的pid信息*/
	pid = link->pid;
	/*将进程从pid散列表中删除*/
	hlist_del_rcu(&link->node);
	link->pid = NULL;
	/*如果进程的pid散列表为空，则释放该pid*/
	for (tmp = PIDTYPE_MAX; --tmp >= 0; )
		/*pid散列表非空，直接返回*/
		if (!hlist_empty(&pid->tasks[tmp]))
			return;
	/*pid散列表为空，释放pid*/
	free_pid(pid);
}

/* transfer_pid是attach_pid(new)和detach_pid(old)的一种优化处理方法 */
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

/*根据pid及pid类型，获取进程实例*/
struct task_struct * fastcall pid_task(struct pid *pid, enum pid_type type)
{
	struct task_struct *result = NULL;
	if (pid)
	{	
		struct hlist_node *first;
		/*pid非空时，通过rcu机制，获取pid指定类型的散列表第一个结点*/
		first = rcu_dereference(pid->tasks[type].first);
		/*散列表非空时通过container_of机制获取对应的进程实例*/
		if (first)
			result = hlist_entry(first, struct task_struct, pids[(type)].node);
	}
	return result;
}

/*根据进程类型，命名空间及对应的局部pid编号，获取进程实例。必须在rcu_read_lock()或者
tasklist_lock锁持有期间读*/
struct task_struct *find_task_by_pid_type_ns(int type, int nr,
		struct pid_namespace *ns)
{
	return pid_task(find_pid_ns(nr, ns), type);
}
EXPORT_SYMBOL(find_task_by_pid_type_ns);

/*根据全局命名空间pid编号，获取进程实例*/
struct task_struct *find_task_by_pid(pid_t nr)
{
	return find_task_by_pid_type_ns(PIDTYPE_PID, nr, &init_pid_ns);
}
EXPORT_SYMBOL(find_task_by_pid);

/*根据当前进程pid命名空间的局部pid编号，获取当前进程的实例*/
struct task_struct *find_task_by_vpid(pid_t vnr)
{
	return find_task_by_pid_type_ns(PIDTYPE_PID, vnr, current->nsproxy->pid_ns);
}
EXPORT_SYMBOL(find_task_by_vpid);

/*根据特定命名空间的局部pid编号，获取对应的进程实例*/
struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns)
{
	return find_task_by_pid_type_ns(PIDTYPE_PID, nr, ns);
}
EXPORT_SYMBOL(find_task_by_pid_ns);

/*利用rcu机制，引用进程特定类型的pid实例*/
struct pid *get_task_pid(struct task_struct *task, enum pid_type type)
{
	struct pid *pid;
	rcu_read_lock();
	pid = get_pid(task->pids[type].pid);
	rcu_read_unlock();
	return pid;
}

/*引用pid实例及其特定类型的进程实例*/
struct task_struct *fastcall get_pid_task(struct pid *pid, enum pid_type type)
{
	struct task_struct *result;
	rcu_read_lock();
	/*根据pid实例及特定类型，获取进程实例*/
	result = pid_task(pid, type);
	/*进程实例有效则引用进程*/
	if (result)
		get_task_struct(result);
	rcu_read_unlock();
	return result;
}

/*根据当前进程的局部pid编号，获取当前进程的pid实例，然后引用该实例，操作需要获得
rcu_read_lock锁保护*/
struct pid *find_get_pid(pid_t nr)
{
	struct pid *pid;

	rcu_read_lock();
	pid = get_pid(find_vpid(nr));
	rcu_read_unlock();

	return pid;
}

/*获取pid实例的特定命名空间中的局部pid编号*/
pid_t pid_nr_ns(struct pid *pid, struct pid_namespace *ns)
{
	struct upid *upid;
	pid_t nr = 0;

	if (pid && ns->level <= pid->level)
	{
		/*pid实例有效且pid所处的命名空间层次在该局部命名空间层次之上，可以访问该命名空间，
	获取该特定命名空间信息*/
		upid = &pid->numbers[ns->level];
		/*如果该命名空间是该pid的level命名空间，则获取该命名空间对应的局部pid编号*/
		if (upid->ns == ns)
			nr = upid->nr;
	}
	return nr;
}

/*获取进程的特定pid命名空间中的局部pid编号*/
pid_t task_pid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_pid(tsk), ns);
}
EXPORT_SYMBOL(task_pid_nr_ns);

/*获取进程特定pid命名空间中线程组组长的pid*/
pid_t task_tgid_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_tgid(tsk), ns);
}
EXPORT_SYMBOL(task_tgid_nr_ns);

/*获取进程特定pid命名空间中进程组组长的局部pid编号*/
pid_t task_pgrp_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_pgrp(tsk), ns);
}
EXPORT_SYMBOL(task_pgrp_nr_ns);

/*获得进程特定pid命名空间中会话组组长的pid编号*/
pid_t task_session_nr_ns(struct task_struct *tsk, struct pid_namespace *ns)
{
	return pid_nr_ns(task_session(tsk), ns);
}
EXPORT_SYMBOL(task_session_nr_ns);

/*获取pid特定命名空间中不小于局部pid的编号所对应的pid实例，用来查询第一个不小于特定
 pid编号的pid实例，如果刚好有一个局部pid编号对应的pid实例，该函数等同于find_pid*/
struct pid *find_ge_pid(int nr, struct pid_namespace *ns)
{
	struct pid *pid;

	do
	{	/*根据局部命名空间和其局部pid编号，获取对应的pid实例*/
		pid = find_pid_ns(nr, ns);
		/*找到退出*/
		if (pid)
			break;
		/*在该命名空间的位图中，从nr位开始查找下一已置位的比特位索引*/
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

/*为pid实例创建slab缓存*/
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

/*释放命名空间所属位图对应的页面及之前申请的slab缓存*/
static void destroy_pid_namespace(struct pid_namespace *ns)
{
	int i;
	/*释放所有位图页*/
	for (i = 0; i < PIDMAP_ENTRIES; i++)
		kfree(ns->pidmap[i].page);
	kmem_cache_free(pid_ns_cachep, ns);
}

/*复制pid命名空间*/
struct pid_namespace *copy_pid_ns(unsigned long flags, struct pid_namespace *old_ns)
{
	struct pid_namespace *new_ns;
	/*输入的pid命名空间不能为空*/
	BUG_ON(!old_ns);
	/*引用输入的pid命名空间*/
	new_ns = get_pid_ns(old_ns);
	/*非复制的共享pid命名空间*/
	if (!(flags & CLONE_NEWPID))
		goto out;
	/*为啥先赋值无效参数而非内存不足错误码，并且不是紧邻create_pid_namespace*/
	new_ns = ERR_PTR(-EINVAL);
	/*同一线程组页共享pid命名空间*/
	if (flags & CLONE_THREAD)
		goto out_put;
	/*创建子命名空间*/
	new_ns = create_pid_namespace(old_ns->level + 1);
	if (!IS_ERR(new_ns))
		new_ns->parent = get_pid_ns(old_ns);

out_put:
	put_pid_ns(old_ns);
out:
	return new_ns;
}

/*释放kref对应的pid命名空间*/
void free_pid_ns(struct kref *kref)
{
	struct pid_namespace *ns, *parent;
	/*通过container_of机制获取kref对应的pid命名空间*/
	ns = container_of(kref, struct pid_namespace, kref);
	/*获取其父命名空间*/
	parent = ns->parent;
	destroy_pid_namespace(ns);
	/*如果父命名空间存在，则取消对父命名空间的引用*/
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

/*pid散列表的区域根据机器物理内存的数量来计算，每GB从最小16到4096（或者更大，但代码中
最大就是4096）个索引*/
void __init pidhash_init(void)
{
	int i, pidhash_size;
	/*以MB为单位计算物理内存的容量*/
	unsigned long megabytes = nr_kernel_pages >> (20 - PAGE_SHIFT);
	/*以IA-32系统128M内存为例，0x80，对应计算后的结果时10*/
	pidhash_shift = max(4, fls(megabytes * 4));
	pidhash_shift = min(12, pidhash_shift);
	/*计算的结果在4和12之间，依上例结果为10，对应的散列表索引数目就是1024*/
	pidhash_size = 1 << pidhash_shift;
	/*显示散列表索引数目，分配阶和应该为该散列表头分配的空间*/
	printk("PID hash table entries: %d (order: %d, %Zd bytes)\n", pidhash_size, pidhash_shift,
				pidhash_size * sizeof(struct hlist_head));
	/*为散列表分配空间*/
	pid_hash = alloc_bootmem(pidhash_size *	sizeof(*(pid_hash)));
	/*内存不足挂机*/
	if (!pid_hash)
		panic("Could not alloc pidhash!\n");
	/*初始化已分配的散列表头*/
	for (i = 0; i < pidhash_size; i++)
		INIT_HLIST_HEAD(&pid_hash[i]);
}

/*根命名空间的pid命名空间中pid位图初始化*/
void __init pidmap_init(void)
{
	/*申请一个页帧用于保存pid位图*/
	init_pid_ns.pidmap[0].page = kzalloc(PAGE_SIZE, GFP_KERNEL);
	/* Reserve PID 0. We never call free_pidmap(0) */
	/*设置位图中第一个位，即保留pid 0，该位从来不释放*/
	set_bit(0, init_pid_ns.pidmap[0].page);
	/*位图中可用位减1*/
	atomic_dec(&init_pid_ns.pidmap[0].nr_free);
	
	init_pid_ns.pid_cachep = create_pid_cachep(1);
	if (init_pid_ns.pid_cachep == NULL)
		panic("Can't create pid_1 cachep\n");
	/*创建pid命名空间slab池*/
	pid_ns_cachep = KMEM_CACHE(pid_namespace, SLAB_PANIC);
}
