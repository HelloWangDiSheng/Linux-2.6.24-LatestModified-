/*映射SCHED_FIFO和SCHED_RR调度策略的实时调度类*/

/*更新指定就绪队列中当前（必须为实时）进程的运行时间统计信息*/
static void update_curr_rt(struct rq *rq)
{
	/*获取就绪队列上正在运行的进程*/
	struct task_struct *curr = rq->curr;
	u64 delta_exec;
	/*如果当前进程不是实时调度策略的进程，则直接退出*/
	if (!task_has_rt_policy(curr))
		return;
	/*获取当前进程本次截至目前已经运行的时间*/
	delta_exec = rq->clock - curr->se.exec_start;
	/*时间回绕？*/
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;
	/*设置进程运行时的最长运行时间*/
	schedstat_set(curr->se.exec_max, max(curr->se.exec_max, delta_exec));
	/*更新进程已累计经运行的时间*/
	curr->se.sum_exec_runtime += delta_exec;
	/*重新设置当前进程的开始运行时间*/
	curr->se.exec_start = rq->clock;
	/**/
	cpuacct_charge(curr, delta_exec);
}

/*将指定实时进程添加到对应优先级的就绪队列末尾，并将实时优先级进程位图中对应位置位*/
static void enqueue_task_rt(struct rq *rq, struct task_struct *p, int wakeup)
{
	/*获取就绪队列中实时调度类优先级数据结构*/
	struct rt_prio_array *array = &rq->rt.active;
	/*将指定实时进程添加到对应优先级队列的末尾*/
	list_add_tail(&p->run_list, array->queue + p->prio);
	/*将对应优先级位图中的位置位*/
	__set_bit(p->prio, array->bitmap);
}

/*将指定的实时进程用实时优先级队列中删除*/
static void dequeue_task_rt(struct rq *rq, struct task_struct *p, int sleep)
{
	/*获取就绪队列中实时调度类优先级队列*/
	struct rt_prio_array *array = &rq->rt.active;
	/*更新当前就绪队列中正在运行的实时进程的运行时间统计信息*/
	update_curr_rt(rq);
	/*将进程从就绪队列中删除*/
	list_del(&p->run_list);
	/*如果被删除进程的实时优先级对应的就绪队列中没有就绪进程，则清除其对应的实时调度
	类优先级位图中的位*/
	if (list_empty(array->queue + p->prio))
		__clear_bit(p->prio, array->bitmap);
}

/*将指定实时进程添加到其优先级对应的优先级队列末尾*/
static void requeue_task_rt(struct rq *rq, struct task_struct *p)
{
	/*获取实时调度类的优先级队列相关信息*/
	struct rt_prio_array *array = &rq->rt.active;
	/*将指定进程添加到对应实时调度类的就绪队列的末尾*/
	list_move_tail(&p->run_list, array->queue + p->prio);
}

/*当前运行进程放弃cpu。重新放到就绪队列的末尾*/
static void yield_task_rt(struct rq *rq)
{
	requeue_task_rt(rq, rq->curr);
}

/*如果新唤醒进程的优先级比当前正在运行的进程优先级高，则抢占当前进程运行*/
static void check_preempt_curr_rt(struct rq *rq, struct task_struct *p)
{
	/*如果指定进程的优先级大于当前正在运行的进程，抢占当前正在运行的进程，并设置当前
	进程重调度标识*/
	if (p->prio < rq->curr->prio)
		resched_task(rq->curr);
}

/*从实时调度类就绪队列中获取已存在的优先级最高的进程*/
static struct task_struct *pick_next_task_rt(struct rq *rq)
{
	/*获取实时调度类就绪队列相关信息*/
	struct rt_prio_array *array = &rq->rt.active;
	struct task_struct *next;
	struct list_head *queue;
	int idx;

	/*从低位开始查找实时调度类位图中第一个置位的比特位，也就是查找实时调度队列中
	优先级最高的第一个进程*/
	idx = sched_find_first_bit(array->bitmap);
	/*如果实时调度队列为空，也就是没有实时进程，则直接返回NULL*/
	if (idx >= MAX_RT_PRIO)
		return NULL;

	/*按优先级由大到小获取实时就绪队列中存在等待进程的就绪队列*/
	queue = array->queue + idx;
	/*获取已存在的最高优先级的进程*/
	next = list_entry(queue->next, struct task_struct, run_list);
	/*设置已获取等待运行的进程的开始运行时间*/
	next->se.exec_start = rq->clock;

	return next;
}

/*更新当前运行的实时进程的统计时间信息，并重置指定的进程的开始运行时间为零*/
static void put_prev_task_rt(struct rq *rq, struct task_struct *p)
{
	/*更新当前实时调度类的就绪队列中正在运行进程的运行时间统计信息*/
	update_curr_rt(rq);
	/*重置指定进程的开始运行时间为零*/
	p->se.exec_start = 0;
}

#ifdef CONFIG_SMP
/*
 * Load-balancing iterator. Note: while the runqueue stays locked
 * during the whole iteration, the current task might be
 * dequeued so the iterator has to be dequeue-safe. Here we
 * achieve that by always pre-iterating before returning
 * the current task:
 */
/**/
static struct task_struct *load_balance_start_rt(void *arg)
{
	/*获取当前就绪队列*/
	struct rq *rq = arg;
	/*获取实时调度类优先级队列相关信息*/
	struct rt_prio_array *array = &rq->rt.active;
	struct list_head *head, *curr;
	struct task_struct *p;
	int idx;

	/*根据优先级由大到小，获取实时调度就绪进程的实时优先级*/
	idx = sched_find_first_bit(array->bitmap);
	/*如果实时就绪队列中没有就绪进程，则直接退出*/
	if (idx >= MAX_RT_PRIO)
		return NULL;

	/*获取已存在的最高优先级的实时就绪队列*/
	head = array->queue + idx;
	/*获取该就绪队列中队尾进程*/
	curr = head->prev;
	p = list_entry(curr, struct task_struct, run_list);
	/*获取就绪队列中原来的倒数第二个进程对应的结点（如果存在）*/
	curr = curr->prev;
	/*设置负载均衡信息中的当前优先级*/
	rq->rt.rt_load_balance_idx = idx;
	/*设置负载均衡信息中的当前优先级对应的队头结点*/
	rq->rt.rt_load_balance_head = head;
	/*设置当前负载均衡信息中当前优先级队尾结点*/
	rq->rt.rt_load_balance_curr = curr;

	return p;
}

/**/
static struct task_struct *load_balance_next_rt(void *arg)
{
	/*获取当前就绪队列*/
	struct rq *rq = arg;
	/*获取实时调度类优先级队列相关信息*/
	struct rt_prio_array *array = &rq->rt.active;
	struct list_head *head, *curr;
	struct task_struct *p;
	int idx;

	/*获取负载均衡优先级*/
	idx = rq->rt.rt_load_balance_idx;
	/*获取负载均衡优先级对应的队列第一个结点*/
	head = rq->rt.rt_load_balance_head;
	/*获取负载均衡优先级对应队列逆序第一个结点*/
	curr = rq->rt.rt_load_balance_curr;

	/*
	 * If we arrived back to the head again then
	 * iterate to the next queue (if any):
	 */
	/*负载均衡优先级队列中只有一个就绪进程*/
	if (unlikely(head == curr))
	{
		/*查找下一个包含就绪进程的优先级队列对应的优先级*/
		int next_idx = find_next_bit(array->bitmap, MAX_RT_PRIO, idx+1);
		/*当前优先级以下优先级对一个的就绪队列中没有就绪进程，返回NULL*/
		if (next_idx >= MAX_RT_PRIO)
			return NULL;
		/*找到下一个包含就绪进程的优先级队列对应的优先级*/
		idx = next_idx;
		/*获取该实时就绪队列的第一个结点*/
		head = array->queue + idx;
		/*获取该实时就绪队列的最后一个结点*/
		curr = head->prev;
		/*重置负载均衡中优先级信息*/
		rq->rt.rt_load_balance_idx = idx;
		/*重置负载均衡优先级队列第一个结点信息*/
		rq->rt.rt_load_balance_head = head;
	}
	/*获取实时就绪队列中最后一个进程*/
	p = list_entry(curr, struct task_struct, run_list);
	/*获取实时就绪队列中倒数第二个进程队列的结点*/
	curr = curr->prev;
	/*重置负载均衡队列中当前结点*/
	rq->rt.rt_load_balance_curr = curr;

	return p;
}

/**/
static unsigned long load_balance_rt(struct rq *this_rq, int this_cpu, struct rq *busiest,
		unsigned long max_load_move,	struct sched_domain *sd, enum cpu_idle_type idle,
		int *all_pinned, int *this_best_prio)
{
	struct rq_iterator rt_rq_iterator;

	rt_rq_iterator.start = load_balance_start_rt;
	rt_rq_iterator.next = load_balance_next_rt;
	/* pass 'busiest' rq argument into
	 * load_balance_[start|next]_rt iterators
	 */
	rt_rq_iterator.arg = busiest;

	return balance_tasks(this_rq, this_cpu, busiest, max_load_move, sd,	idle,
			all_pinned, this_best_prio, &rt_rq_iterator);
}

/**/
static int move_one_task_rt(struct rq *this_rq, int this_cpu, struct rq *busiest,
			struct sched_domain *sd, enum cpu_idle_type idle)
{
	struct rq_iterator rt_rq_iterator;

	rt_rq_iterator.start = load_balance_start_rt;
	rt_rq_iterator.next = load_balance_next_rt;
	rt_rq_iterator.arg = busiest;

	return iter_move_one_task(this_rq, this_cpu, busiest, sd, idle,	&rt_rq_iterator);
}
#endif

/*更新（自减1）当前SCHED_RR调度策略的进程时间片信息，如果进程时间片没有用完，则继
续运行，如果本轮时间片用完则需要重置该进程的时间片为默认时间片，如果该进程的优先级
对应的就绪队列中还有其它进程，则需要将该进程放置队尾重新排队，并且设置该进程的重调
度标识*/
static void task_tick_rt(struct rq *rq, struct task_struct *p)
{
	/*更新指定就绪队列中正在运行的实时进程运行时间统计信息*/
	update_curr_rt(rq);

	/*实时轮转调度进程需时间片管理，实时先来线服务进程没有时间片*/
	if (p->policy != SCHED_RR)
		return;

	/*时间片自减1，如果非零说明本轮调度还没有结束，继续运行*/
	if (--p->time_slice)
		return;

	/*时间片用完，本轮调度结束后，重置时间片为默认时间片100ms*/
	p->time_slice = DEF_TIMESLICE;

	/*如果指定进程不是当前实时优先级队列中唯一的就绪进程，则重新将该进程放置到队尾，
	并且设置该进程的重调度标识*/
	if (p->run_list.prev != p->run_list.next)
	{
		requeue_task_rt(rq, p);
		set_tsk_need_resched(p);
	}
}

/*获取指定就绪队列中正在运行的进程，并设置进程的开始运行时间为就绪队列的时钟时间*/
static void set_curr_task_rt(struct rq *rq)
{
	/*获取指定就绪队列中当前正在运行的进程*/
	struct task_struct *p = rq->curr;
	/*设置当前进程的开始运行时间为就绪队列的时钟时间*/
	p->se.exec_start = rq->clock;
}

/*初始化实时调度类实例*/
const struct sched_class rt_sched_class =
{
	/*实时调度类下一个调度类时完全公平调度类*/
	.next			= &fair_sched_class,
	/*将指定实时进程插入到其优先级对应的实时就绪队列的末尾，如果该优先级就绪队列中
	原来没有就绪进程，则需要同步置位实时优先级就绪队列位图中对应优先级位*/
	.enqueue_task		= enqueue_task_rt,
	/*将指定实时进程从就绪队列中删除，如果此时该优先级队列中没有进程，则需要清除该
	实时调度队列位图中对应位，以示该优先级就绪队列为空*/
	.dequeue_task		= dequeue_task_rt,
	/*进程由于等待资源或事件主动放弃或被调度被动放弃cpu控制权，重新插入对应优先级的
	实时就绪队列中排队，并且设置该进程需要重新调度标识*/
	.yield_task		= yield_task_rt,
	/*检查指定实时进程的优先级是否大于当前就绪队列中正在运行的进程，如果是则需要抢
	占当前正在运行的进程*/
	.check_preempt_curr	= check_preempt_curr_rt,
	/*从实时调度类优先级队列中选择优先级最高的进程*/
	.pick_next_task		= pick_next_task_rt,
	/*选择指定进程作为下一个运行的进程*/
	.put_prev_task		= put_prev_task_rt,

#ifdef CONFIG_SMP
	/*实现smp系统中负载均衡功能：根据就绪队列的负载权重，找出最忙的就绪队列，然后从
	该（多就绪进程的）队列中选择一个（没有运行或刚结束运行的）进程，迁移到该进程被允
	许运行的最空闲的cpu对应的就绪队列中*/
	.load_balance		= load_balance_rt,
	/*迁移一个进程到最空闲cpu对应的就绪队列上*/
	.move_one_task		= move_one_task_rt,
#endif

	/*获取指定就绪队列上正在运行的进程，并设置该进程的开始运行时间为当前就绪队列的
	时钟时间*/
	.set_curr_task          = set_curr_task_rt,
	/*更新SCHED_RR调度策略进程的时间片，如果该进程的时间片用完了，则重置位默认时间
	片，如果该优先级对应的队列中还有其它进程，则将该进程插入到该队列末尾，并设置重
	调标识*/
	.task_tick		= task_tick_rt,
};
