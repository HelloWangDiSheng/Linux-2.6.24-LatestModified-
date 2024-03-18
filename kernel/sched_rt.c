/*ӳ��SCHED_FIFO��SCHED_RR���Ȳ��Ե�ʵʱ������*/

/*����ָ�����������е�ǰ������Ϊʵʱ�����̵�����ʱ��ͳ����Ϣ*/
static void update_curr_rt(struct rq *rq)
{
	/*��ȡ�����������������еĽ���*/
	struct task_struct *curr = rq->curr;
	u64 delta_exec;
	/*�����ǰ���̲���ʵʱ���Ȳ��ԵĽ��̣���ֱ���˳�*/
	if (!task_has_rt_policy(curr))
		return;
	/*��ȡ��ǰ���̱��ν���Ŀǰ�Ѿ����е�ʱ��*/
	delta_exec = rq->clock - curr->se.exec_start;
	/*ʱ����ƣ�*/
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;
	/*���ý�������ʱ�������ʱ��*/
	schedstat_set(curr->se.exec_max, max(curr->se.exec_max, delta_exec));
	/*���½������ۼƾ����е�ʱ��*/
	curr->se.sum_exec_runtime += delta_exec;
	/*�������õ�ǰ���̵Ŀ�ʼ����ʱ��*/
	curr->se.exec_start = rq->clock;
	/**/
	cpuacct_charge(curr, delta_exec);
}

/*��ָ��ʵʱ������ӵ���Ӧ���ȼ��ľ�������ĩβ������ʵʱ���ȼ�����λͼ�ж�Ӧλ��λ*/
static void enqueue_task_rt(struct rq *rq, struct task_struct *p, int wakeup)
{
	/*��ȡ����������ʵʱ���������ȼ����ݽṹ*/
	struct rt_prio_array *array = &rq->rt.active;
	/*��ָ��ʵʱ������ӵ���Ӧ���ȼ����е�ĩβ*/
	list_add_tail(&p->run_list, array->queue + p->prio);
	/*����Ӧ���ȼ�λͼ�е�λ��λ*/
	__set_bit(p->prio, array->bitmap);
}

/*��ָ����ʵʱ������ʵʱ���ȼ�������ɾ��*/
static void dequeue_task_rt(struct rq *rq, struct task_struct *p, int sleep)
{
	/*��ȡ����������ʵʱ���������ȼ�����*/
	struct rt_prio_array *array = &rq->rt.active;
	/*���µ�ǰ�����������������е�ʵʱ���̵�����ʱ��ͳ����Ϣ*/
	update_curr_rt(rq);
	/*�����̴Ӿ���������ɾ��*/
	list_del(&p->run_list);
	/*�����ɾ�����̵�ʵʱ���ȼ���Ӧ�ľ���������û�о������̣���������Ӧ��ʵʱ����
	�����ȼ�λͼ�е�λ*/
	if (list_empty(array->queue + p->prio))
		__clear_bit(p->prio, array->bitmap);
}

/*��ָ��ʵʱ������ӵ������ȼ���Ӧ�����ȼ�����ĩβ*/
static void requeue_task_rt(struct rq *rq, struct task_struct *p)
{
	/*��ȡʵʱ����������ȼ����������Ϣ*/
	struct rt_prio_array *array = &rq->rt.active;
	/*��ָ��������ӵ���Ӧʵʱ������ľ������е�ĩβ*/
	list_move_tail(&p->run_list, array->queue + p->prio);
}

/*��ǰ���н��̷���cpu�����·ŵ��������е�ĩβ*/
static void yield_task_rt(struct rq *rq)
{
	requeue_task_rt(rq, rq->curr);
}

/*����»��ѽ��̵����ȼ��ȵ�ǰ�������еĽ������ȼ��ߣ�����ռ��ǰ��������*/
static void check_preempt_curr_rt(struct rq *rq, struct task_struct *p)
{
	/*���ָ�����̵����ȼ����ڵ�ǰ�������еĽ��̣���ռ��ǰ�������еĽ��̣������õ�ǰ
	�����ص��ȱ�ʶ*/
	if (p->prio < rq->curr->prio)
		resched_task(rq->curr);
}

/*��ʵʱ��������������л�ȡ�Ѵ��ڵ����ȼ���ߵĽ���*/
static struct task_struct *pick_next_task_rt(struct rq *rq)
{
	/*��ȡʵʱ������������������Ϣ*/
	struct rt_prio_array *array = &rq->rt.active;
	struct task_struct *next;
	struct list_head *queue;
	int idx;

	/*�ӵ�λ��ʼ����ʵʱ������λͼ�е�һ����λ�ı���λ��Ҳ���ǲ���ʵʱ���ȶ�����
	���ȼ���ߵĵ�һ������*/
	idx = sched_find_first_bit(array->bitmap);
	/*���ʵʱ���ȶ���Ϊ�գ�Ҳ����û��ʵʱ���̣���ֱ�ӷ���NULL*/
	if (idx >= MAX_RT_PRIO)
		return NULL;

	/*�����ȼ��ɴ�С��ȡʵʱ���������д��ڵȴ����̵ľ�������*/
	queue = array->queue + idx;
	/*��ȡ�Ѵ��ڵ�������ȼ��Ľ���*/
	next = list_entry(queue->next, struct task_struct, run_list);
	/*�����ѻ�ȡ�ȴ����еĽ��̵Ŀ�ʼ����ʱ��*/
	next->se.exec_start = rq->clock;

	return next;
}

/*���µ�ǰ���е�ʵʱ���̵�ͳ��ʱ����Ϣ��������ָ���Ľ��̵Ŀ�ʼ����ʱ��Ϊ��*/
static void put_prev_task_rt(struct rq *rq, struct task_struct *p)
{
	/*���µ�ǰʵʱ������ľ����������������н��̵�����ʱ��ͳ����Ϣ*/
	update_curr_rt(rq);
	/*����ָ�����̵Ŀ�ʼ����ʱ��Ϊ��*/
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
	/*��ȡ��ǰ��������*/
	struct rq *rq = arg;
	/*��ȡʵʱ���������ȼ����������Ϣ*/
	struct rt_prio_array *array = &rq->rt.active;
	struct list_head *head, *curr;
	struct task_struct *p;
	int idx;

	/*�������ȼ��ɴ�С����ȡʵʱ���Ⱦ������̵�ʵʱ���ȼ�*/
	idx = sched_find_first_bit(array->bitmap);
	/*���ʵʱ����������û�о������̣���ֱ���˳�*/
	if (idx >= MAX_RT_PRIO)
		return NULL;

	/*��ȡ�Ѵ��ڵ�������ȼ���ʵʱ��������*/
	head = array->queue + idx;
	/*��ȡ�þ��������ж�β����*/
	curr = head->prev;
	p = list_entry(curr, struct task_struct, run_list);
	/*��ȡ����������ԭ���ĵ����ڶ������̶�Ӧ�Ľ�㣨������ڣ�*/
	curr = curr->prev;
	/*���ø��ؾ�����Ϣ�еĵ�ǰ���ȼ�*/
	rq->rt.rt_load_balance_idx = idx;
	/*���ø��ؾ�����Ϣ�еĵ�ǰ���ȼ���Ӧ�Ķ�ͷ���*/
	rq->rt.rt_load_balance_head = head;
	/*���õ�ǰ���ؾ�����Ϣ�е�ǰ���ȼ���β���*/
	rq->rt.rt_load_balance_curr = curr;

	return p;
}

/**/
static struct task_struct *load_balance_next_rt(void *arg)
{
	/*��ȡ��ǰ��������*/
	struct rq *rq = arg;
	/*��ȡʵʱ���������ȼ����������Ϣ*/
	struct rt_prio_array *array = &rq->rt.active;
	struct list_head *head, *curr;
	struct task_struct *p;
	int idx;

	/*��ȡ���ؾ������ȼ�*/
	idx = rq->rt.rt_load_balance_idx;
	/*��ȡ���ؾ������ȼ���Ӧ�Ķ��е�һ�����*/
	head = rq->rt.rt_load_balance_head;
	/*��ȡ���ؾ������ȼ���Ӧ���������һ�����*/
	curr = rq->rt.rt_load_balance_curr;

	/*
	 * If we arrived back to the head again then
	 * iterate to the next queue (if any):
	 */
	/*���ؾ������ȼ�������ֻ��һ����������*/
	if (unlikely(head == curr))
	{
		/*������һ�������������̵����ȼ����ж�Ӧ�����ȼ�*/
		int next_idx = find_next_bit(array->bitmap, MAX_RT_PRIO, idx+1);
		/*��ǰ���ȼ��������ȼ���һ���ľ���������û�о������̣�����NULL*/
		if (next_idx >= MAX_RT_PRIO)
			return NULL;
		/*�ҵ���һ�������������̵����ȼ����ж�Ӧ�����ȼ�*/
		idx = next_idx;
		/*��ȡ��ʵʱ�������еĵ�һ�����*/
		head = array->queue + idx;
		/*��ȡ��ʵʱ�������е����һ�����*/
		curr = head->prev;
		/*���ø��ؾ��������ȼ���Ϣ*/
		rq->rt.rt_load_balance_idx = idx;
		/*���ø��ؾ������ȼ����е�һ�������Ϣ*/
		rq->rt.rt_load_balance_head = head;
	}
	/*��ȡʵʱ�������������һ������*/
	p = list_entry(curr, struct task_struct, run_list);
	/*��ȡʵʱ���������е����ڶ������̶��еĽ��*/
	curr = curr->prev;
	/*���ø��ؾ�������е�ǰ���*/
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

/*���£��Լ�1����ǰSCHED_RR���Ȳ��ԵĽ���ʱ��Ƭ��Ϣ���������ʱ��Ƭû�����꣬���
�����У��������ʱ��Ƭ��������Ҫ���øý��̵�ʱ��ƬΪĬ��ʱ��Ƭ������ý��̵����ȼ�
��Ӧ�ľ��������л����������̣�����Ҫ���ý��̷��ö�β�����Ŷӣ��������øý��̵��ص�
�ȱ�ʶ*/
static void task_tick_rt(struct rq *rq, struct task_struct *p)
{
	/*����ָ�������������������е�ʵʱ��������ʱ��ͳ����Ϣ*/
	update_curr_rt(rq);

	/*ʵʱ��ת���Ƚ�����ʱ��Ƭ����ʵʱ�����߷������û��ʱ��Ƭ*/
	if (p->policy != SCHED_RR)
		return;

	/*ʱ��Ƭ�Լ�1���������˵�����ֵ��Ȼ�û�н�������������*/
	if (--p->time_slice)
		return;

	/*ʱ��Ƭ���꣬���ֵ��Ƚ���������ʱ��ƬΪĬ��ʱ��Ƭ100ms*/
	p->time_slice = DEF_TIMESLICE;

	/*���ָ�����̲��ǵ�ǰʵʱ���ȼ�������Ψһ�ľ������̣������½��ý��̷��õ���β��
	�������øý��̵��ص��ȱ�ʶ*/
	if (p->run_list.prev != p->run_list.next)
	{
		requeue_task_rt(rq, p);
		set_tsk_need_resched(p);
	}
}

/*��ȡָ�������������������еĽ��̣������ý��̵Ŀ�ʼ����ʱ��Ϊ�������е�ʱ��ʱ��*/
static void set_curr_task_rt(struct rq *rq)
{
	/*��ȡָ�����������е�ǰ�������еĽ���*/
	struct task_struct *p = rq->curr;
	/*���õ�ǰ���̵Ŀ�ʼ����ʱ��Ϊ�������е�ʱ��ʱ��*/
	p->se.exec_start = rq->clock;
}

/*��ʼ��ʵʱ������ʵ��*/
const struct sched_class rt_sched_class =
{
	/*ʵʱ��������һ��������ʱ��ȫ��ƽ������*/
	.next			= &fair_sched_class,
	/*��ָ��ʵʱ���̲��뵽�����ȼ���Ӧ��ʵʱ�������е�ĩβ����������ȼ�����������
	ԭ��û�о������̣�����Ҫͬ����λʵʱ���ȼ���������λͼ�ж�Ӧ���ȼ�λ*/
	.enqueue_task		= enqueue_task_rt,
	/*��ָ��ʵʱ���̴Ӿ���������ɾ���������ʱ�����ȼ�������û�н��̣�����Ҫ�����
	ʵʱ���ȶ���λͼ�ж�Ӧλ����ʾ�����ȼ���������Ϊ��*/
	.dequeue_task		= dequeue_task_rt,
	/*�������ڵȴ���Դ���¼����������򱻵��ȱ�������cpu����Ȩ�����²����Ӧ���ȼ���
	ʵʱ�����������Ŷӣ��������øý�����Ҫ���µ��ȱ�ʶ*/
	.yield_task		= yield_task_rt,
	/*���ָ��ʵʱ���̵����ȼ��Ƿ���ڵ�ǰ�����������������еĽ��̣����������Ҫ��
	ռ��ǰ�������еĽ���*/
	.check_preempt_curr	= check_preempt_curr_rt,
	/*��ʵʱ���������ȼ�������ѡ�����ȼ���ߵĽ���*/
	.pick_next_task		= pick_next_task_rt,
	/*ѡ��ָ��������Ϊ��һ�����еĽ���*/
	.put_prev_task		= put_prev_task_rt,

#ifdef CONFIG_SMP
	/*ʵ��smpϵͳ�и��ؾ��⹦�ܣ����ݾ������еĸ���Ȩ�أ��ҳ���æ�ľ������У�Ȼ���
	�ã���������̵ģ�������ѡ��һ����û�����л�ս������еģ����̣�Ǩ�Ƶ��ý��̱���
	�����е�����е�cpu��Ӧ�ľ���������*/
	.load_balance		= load_balance_rt,
	/*Ǩ��һ�����̵������cpu��Ӧ�ľ���������*/
	.move_one_task		= move_one_task_rt,
#endif

	/*��ȡָ�������������������еĽ��̣������øý��̵Ŀ�ʼ����ʱ��Ϊ��ǰ�������е�
	ʱ��ʱ��*/
	.set_curr_task          = set_curr_task_rt,
	/*����SCHED_RR���Ȳ��Խ��̵�ʱ��Ƭ������ý��̵�ʱ��Ƭ�����ˣ�������λĬ��ʱ��
	Ƭ����������ȼ���Ӧ�Ķ����л����������̣��򽫸ý��̲��뵽�ö���ĩβ����������
	����ʶ*/
	.task_tick		= task_tick_rt,
};
