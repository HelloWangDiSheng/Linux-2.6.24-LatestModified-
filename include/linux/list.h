#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#ifdef __KERNEL__

#include <linux/stddef.h>
#include <linux/poison.h>
#include <linux/prefetch.h>
#include <asm/system.h>

/*�򵥵�˫����ʵ�֡�����������������ǵ������ʱһЩ�ڲ����������ã���Ϊ��ʱ�����Ѿ�
ֱ��ǰ����㣬ͨ��ֱ��ʹ�����ǿ������ɸ��õĴ���*/
/*��˫������*/
struct list_head
{
	/*������ָ��*/
	struct list_head *next;
	/*ǰ����ָ��*/
	struct list_head *prev;
};

/*��̬�귽ʽ��ʼ��һ�����*/
#define LIST_HEAD_INIT(name) { &(name), &(name) }
/*���岢��ʼ��һ�����*/
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)
/*������ʽ��ʼ��һ����㣬ǰ�����ָ��ָ������*/
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	/*��������ָ������Ϊ����*/
	list->next = list;
	/*���ǰ����ָ������Ϊ����*/
	list->prev = list;
}

/*����֪�������������֮�����һ���½��*/
#ifndef CONFIG_DEBUG_LIST
static inline void __list_add(struct list_head *new, struct list_head *prev,
			      				struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
#else
extern void __list_add(struct list_head *new, struct list_head *prev,
			      			struct list_head *next);
#endif

/*��һ��ָ�����֮�����һ���½�㣬�ú���ʵ�ֵ�ͷ�巨��������½�������ͷ���֮��
��ʵ��ջ������*/
#ifndef CONFIG_DEBUG_LIST
static inline void list_add(struct list_head *new, struct list_head *head)
{
	/*ͷ�巨����������½��*/
	__list_add(new, head, head->next);
}
#else
extern void list_add(struct list_head *new, struct list_head *head);
#endif


/*��һ���ض����֮ǰ����һ���½�㣬�ú���ʵ�ֵ�β�巨��������½�������β���֮��
��ʵ�ֶ��к�����*/
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	/*β�巨����������½��*/
	__list_add(new, head->prev, head);
}


/*����֪�������������֮�����һ���½�㣬�����øý���ǰ��ָ�����������ڴ�
���ϣ�������øý���ǰ���������next��prev��ָ����*/
static inline void __list_add_rcu(struct list_head * new, struct list_head * prev,
									struct list_head * next)
{
	/*��ȷ���½���ǰ��ָ��������*/
	new->next = next;
	new->prev = prev;
	/*�����ڴ��Ż����ϣ��ڴ˴�֮���д������ʼ֮ǰ���˴�֮ǰ���е�д�������Ѿ����*/
	smp_wmb();
	/*ȷ���½���ǰ���������next��prev��ָ��������*/
	next->prev = new;
	prev->next = new;
}

/*����rcu����������ͷ���֮�����һ���½�㣬�����ʵ��ջ�����á����ú��������ȡ��Ҫ
�Ĵ�ʩ������к��ʵ���������ͬʱ������ͬһ�������һ����ı�����ԭʼ���ݵĲ�������
list_add_head_rcu()��list_del_rcu()��������̬������Ȼ�����ú�������ʹ��_rcu��׺������
list_for_each_entry()������ԭʼ����*/
static inline void list_add_rcu(struct list_head *new, struct list_head *head)
{
	/*ͷ�巨����RCU��������������½��*/
	__list_add_rcu(new, head, head->next);
}

/*��rcu�����������ͷ�󣨽��ڱ�ͷ�����һ���½�㣬�÷�����ʵ�ֶ��к����á�
���ú��������ȡ��Ҫ�Ĵ�ʩ������к��ʵ���������ͬʱ������ͬһ�������һ����ı�����
ԭʼ���ݵĲ�������list_add_head_rcu()��list_del_rcu()��������̬������Ȼ�����ú�������
ʹ��_rcu��׺������list_for_each_entry()������ԭʼ����*/
static inline void list_add_tail_rcu(struct list_head *new, struct list_head *head)
{
	/*β�巨����RCU��������������½��*/
	__list_add_rcu(new, head->prev, head);
}
 
/*ɾ����֪��ǰ�������֮��Ľ�㣬����ɾ�������㣬����֪����ǰ��ָ��������*/
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	/*ֱ������ǰ�����ָ��������*/
	next->prev = prev;
	prev->next = next;
}

/*��������ɾ��һ����㡣ɾ����Ľ�㴦��δ����״̬����ʱ�Ա�ɾ��������list_empty()
���᷵��true�����������з�����Щ��㽫����ȱҳ�쳣*/
#ifndef CONFIG_DEBUG_LIST
static inline void list_del(struct list_head *entry)
{
	/*ɾ��entry���*/
	__list_del(entry->prev, entry->next);
	/*��㱻ɾ���󣬽��������Ϊδ����״̬*/
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}
#else
extern void list_del(struct list_head *entry);
#endif

/*��ָ������������ɾ�������ҽ���ɾ������ǰ��ָ��������Ϊδ����״̬�������������ж�
�ý��ִ��list_empty()���᷵��true���ú����Ի�������RCU���������á��ر�أ����ܶ�������
��Ȼ�����������ʹ���е�ǰ��ָ�롣���ú��������ȡ��Ҫ�Ĵ�ʩ������к��ʵ���������ͬʱ
������ͬһ�������һ����ı�����ԭʼ���ݵĲ�������list_add_head_rcu()��list_del_rcu()��
������̬������Ȼ�����ú�������ʹ��_rcu��׺������list_for_each_entry()������ԭʼ����ע
�⣡���ú��������������ͷ���ɾ���Ľ�㣬�෴��ʹ��call_rcu()����synchronize_rcu()�Ƴ���
�ţ�ֱ�����˿�����*/
static inline void list_del_rcu(struct list_head *entry)
{
	/*ɾ��entry���*/
	__list_del(entry->prev, entry->next);
	/*����ɾ���Ľ��ǰ��ָ��������Ϊδ����״̬�����������·��ʸý�㽫����ȱҳ�쳣*/
	entry->prev = LIST_POISON2;
}

/*���½���滻�ɽ�㣬�������½����ɽ��ĺ�����֮�����ϵ���������½���оɽ��
ǰ����֮��Ĺ�ϵ��ע�⣡�ɽ�㲻��Ϊ�ա�������ɽ��Ϊ�գ���������д��*/
static inline void list_replace(struct list_head *old, struct list_head *new)
{
	/*�����½����ɽ�������֮�����ϵ*/
	new->next = old->next;
	new->next->prev = new;
	/*�����½����ɽ��ǰ����֮�������*/
	new->prev = old->prev;
	new->prev->next = new;
}

/*�½���滻�ɽ�㣬�����ɽ�����³�ʼ��*/
static inline void list_replace_init(struct list_head *old, struct list_head *new)
{
	/*���½���滻�ɽ��*/
	list_replace(old, new);
	/*���³�ʼ���ɽ��*/
	INIT_LIST_HEAD(old);
}

/*���½���滻�ɽ�㣬ע�⣡�ɽ�㲻��Ϊ�ա��Ƚ����½����ɽ��ǰ����֮�����ϵ��
��������ڴ�д�Ż����ϣ�Ȼ���ٽ����½��ǰ�������½��֮�����ϵ����󽫾ɽ���
ǰ��ָ��������Ϊδ����״̬���Ըý�����list_empty()���᷵��true�����������£���
�ʸý�㽫����ȱҳ�쳣*/
static inline void list_replace_rcu(struct list_head *old, struct list_head *new)
{
	/*�����½���ǰ��ָ����*/
	new->next = old->next;
	new->prev = old->prev;
	/*�����ڴ��Ż�д����*/
	smp_wmb();
	/*�����½��ǰ�������next��prev��ָ����*/
	new->next->prev = new;
	new->prev->next = new;
	/*���ɽ���ǰ��ָ��������Ϊδ����״̬*/
	old->prev = LIST_POISON2;
}

/*��������ɾ��һ��㣬�����ý�����³�ʼ��*/
static inline void list_del_init(struct list_head *entry)
{
	/*��������ɾ��entry���*/
	__list_del(entry->prev, entry->next);
	/*��ʼ��entry���*/
	INIT_LIST_HEAD(entry);
}

/*ɾ��������ָ���Ľ�㣬������ͷ�巨���ý����ӵ���һ����*/
static inline void list_move(struct list_head *list, struct list_head *head)
{
	/*ɾ��list���*/
	__list_del(list->prev, list->next);
	/*ͷ�巨��list�����뵽head����*/
	list_add(list, head);
}

/*��������ɾ��ָ����㣬����β�巨���ý����뵽��һ������*/
static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
	/*ɾ��������list���*/
	__list_del(list->prev, list->next);
	/*β�巨��list�����ӵ�head����*/
	list_add_tail(list, head);
}

/*���Խ���Ƿ��������β��㡣����nextָ�����Ƿ�Ϊͷ���*/
static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
	return list->next == head;
}

/*���������Ƿ�Ϊ�գ���������ͷ����next���Ƿ�������*/
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/*���������Ƿ�Ϊ�գ����Ҽ������cpuû�������޸����Ա����ǰ��ָ����ע�⣡ʹ�÷�
ͬ����list_empty_carful()��������ִ��list_del_init()ʱ�ǰ�ȫ�ģ���һ��cpu����ִ��
list_add()ʱ����ʹ�øú���*/
static inline int list_empty_careful(const struct list_head *head)
{
	/*��ȡ�����һ�����*/
	struct list_head *next = head->next;
	/*ֻ�е������ͷ���ǰ��ָ����ָ������ʱ�������Ϊ��*/
	return (next == head) && (next == head->prev);
}

/*�ϲ�����������ָ�������г�ͷ���֮������н����ӵ���һ�������β��*/
static inline void __list_splice(struct list_head *list, struct list_head *head)
{
	/*��ȡlist����ĵ�һ�����*/
	struct list_head *first = list->next;
	/*��ȡlist��������һ�����*/
	struct list_head *last = list->prev;
	/*��ȡhead����ĵ�һ�����*/
	struct list_head *at = head->next;

	/*����list����ĵ�һ�����Ϊhead����ĵ�һ�����*/
	first->prev = head;
	head->next = first;
	/*����list��������һ�����Ϊheadǰ��һ������ǰ���*/
	last->next = at;
	at->prev = last;
}

/*�ϲ�������Դ����ǿ�ʱ������ͷ�巨��Դ�����г�ͷ���֮������н������ƽ�Ʋ���
��Ŀ������*/
static inline void list_splice(struct list_head *list, struct list_head *head)
{
	/*list����ǿ�ʱ������ͷ�巨��list�����з�ͷ������head����*/
	if (!list_empty(list))
		__list_splice(list, head);
}

/*�ϲ�����������ͷ�巨���ǿ�Դ�����з�ͷ�������ƽ�Ʋ���Ŀ�������У�Ȼ��Դ�����ʼ��*/
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
	/*list����ǿ�ʱ�ϲ�*/
	if (!list_empty(list))
	{
		/*ͷ�巨��list�����з�ͷ����������head����*/
		__list_splice(list, head);
		/*��ʼ��list����*/
		INIT_LIST_HEAD(list);
	}
}

/*��һ����rcu������Դ����ϲ���һ���Ѵ��ڵ�Ŀ������Ŀ�������ܱ�rcu_read�͸ú���
ͬʱ��������Ҫ˵�������ú��������ȡ��Ҫ��ʩ��ֹĿ��������κ��������¡�ԭ���У���
sync()��ʼ����ʱ�����޸ĸ�����ʱ���ܵģ������Щ��Ϊ��Ҫ��һ������call_rcu()�Ŀ�ѡ
�汾���Ա���������������Ҫʱ������û��RCU API��Ա�ļ򻯰汾*/
static inline void list_splice_init_rcu(struct list_head *list, struct list_head *head,
											void (*sync)(void))
{
	/*��ȡԤ��������ĵ�һ�����*/
	struct list_head *first = list->next;
	/*��ȡԤ������������һ�����*/
	struct list_head *last = list->prev;
	/*��ȡĿ������ĵ�һ�����*/
	struct list_head *at = head->next;
	/*Ŀ������Ϊ��ʱֱ���˳�*/
	if (list_empty(head))
		return;
	/*Ԥ���������һ�������һ�������֪��Ϳ��Ա���������˿������³�ʼ��������*/
	INIT_LIST_HEAD(list);

	/*��ʱ������������Ȼָ��Դ�����ڸ�����ϲ���Ŀ������֮ǰ�ȴ���һ���߽���ʹ��
	�������κ��¶��߿����Ķ��ǿ�����*/
	/*���߽�����Դ����ķ��ʺ󣬿�ʼִ�кϲ��ϲ���������Ŀ��������rcu����ͬʱ���ʵ�
	ȫ������ʱ˳��ͱ�ú���Ҫ��ע�⣡û�иú�����rcu���߲��������ǰ��ָ��*/
	sync();
	/*Դ������������һ���������ΪĿ���������һ�����*/
	last->next = at;
	/*ִ���ڴ�д���ϣ���֤�ڸõ�֮���д������ʼ֮ǰ��ǰ�����е�д�������Ѿ�ִ�����*/
	smp_wmb();
	/*Ŀ������ĵ�һ���������Ϊ֮ǰԴ����ĵ�һ�����*/
	head->next = first;
	first->prev = head;
	/*ǰĿ�������ǰһ���������ΪǰԴ��������һ�����*/
	at->prev = last;
}

/*���ݣ�type���ṹ��Ա�����������member�����ַptr����ȡ�û��������ĵ�ַ*/
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/*���ݣ�type���ṹ��Ա�����������member�����ַptr����ȡ�ýṹ�������е�һ������
��ַ�����棡������Ϊ��*/ 
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)

/*��������ͷ��㣬��Ԥȡ���ܵ�˳��������������ڼ�Ӧ�ò�ȡ��Ҫ�ı�����ʩ�������
�����ڼ����list_del(pos)�������ں�panic���������list_del_init(pos)������ѭ��*/
#define list_for_each(pos, head) \
	for (pos = (head)->next; prefetch(pos->next), pos != (head); pos = pos->next)

 /*��������ͷ��㣬��Ԥȡ���ܵ�˳�����������ʹ�øú�����ǰ������֪����̣ܶ�
 �󲿷�ʱ��Ϊ�ջ���ֻ��һ�����*/
 
#define __list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

/*��Ԥȡ���ܵ������������*/
#define list_for_each_prev(pos, head) 											\
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); 		\
        	pos = pos->prev)


/*��ɾ��������ȫ�ı�������*/
#define list_for_each_safe(pos, n, head) 										\
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/*��ɾ��������ȫ�������������*/
#define list_for_each_prev_safe(pos, n, head) 													\
	for (pos = (head)->prev, n = pos->prev; prefetch(pos->prev), pos != (head); \
	     pos = n, n = pos->prev)

/*�����������͵��������ݽṹ��type(*pos)���г�Ա���������ͣ�member��������ͷ���
��head�������������͵����б���*/
#define list_for_each_entry(pos, head, member)									\
	for (pos = list_entry((head)->next, typeof(*pos), member);					\
	     prefetch(pos->member.next), &pos->member != (head);				 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/*��������������͵��������ݽṹ��typeof(*pos)����Ա�����������member������
ͷ��㣨head��������������нṹ����*/
#define list_for_each_entry_reverse(pos, head, member)							\
	for (pos = list_entry((head)->prev, typeof(*pos), member);					\
	     prefetch(pos->member.prev), &pos->member != (head); 					\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))
 
 /*��ȡΪʹ��list_for_each_entry_continue()����ڽ��pos��pos�ǿ�ʱʹ��pos������pos
 ��ֵΪʹ��list_entry��ȡ��һ���ṹ������ַ*/
#define list_prepare_entry(pos, head, member) ((pos) ? : list_entry(head, typeof(*pos), member))

/*�ӵ�ǰ��λ�ã��ṹ����pos��ַ����ʼ�����������������ͣ�typeof(*pos)��������
ֱ���������*/
#define list_for_each_entry_continue(pos, head, member)							\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);		\
	     prefetch(pos->member.next), &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/*�ӵ�ǰλ�ã������ṹ�������ַpos����ʼ����������ýṹ��������ڵ�����ֱ��
������ͷ���Ϊֹ*/
#define list_for_each_entry_continue_reverse(pos, head, member)					\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);		\
	     prefetch(pos->member.prev), &pos->member != (head);					\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/*�ӵ�ǰλ������������һ����㿪ʼ��˳������������͵�����*/
#define list_for_each_entry_from(pos, head, member) 							\
	for (; prefetch(pos->member.next), &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))


/*��ֹɾ�������Ұ�ȫ�Ĵ�ͷ��ʼ����ָ�����͵�����*/
#define list_for_each_entry_safe(pos, n, head, member)							\
	for (pos = list_entry((head)->next, typeof(*pos), member)					\
		n = list_entry(pos->member.next, typeof(*pos), member);					\
	     &pos->member != (head); 												\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*�ӵ�ǰ������һ����㿪ʼ����ȫ��ɾ�������ı���ָ�����͵�����*/
#define list_for_each_entry_safe_continue(pos, n, head, member) 				\
	for (pos = list_entry(pos->member.next, typeof(*pos), member), 	\
		n = list_entry(pos->member.next, typeof(*pos), member);					\
	     &pos->member != (head);												\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*�ӵ�ǰ��㿪ʼ����ֹɾ����ȫ��˳�����ָ�����͵�����*/
#define list_for_each_entry_safe_from(pos, n, head, member) 					\
	for (n = list_entry(pos->member.next, typeof(*pos), member);				\
	     &pos->member != (head);												\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*�ӵ�ǰ��㿪ʼ����ȫ��ֹɾ���������������ָ����������*/
#define list_for_each_entry_safe_reverse(pos, n, head, member)					\
	for (pos = list_entry((head)->prev, typeof(*pos), member),					\
		n = list_entry(pos->member.prev, typeof(*pos), member);					\
	     &pos->member != (head); 												\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))


/*��ͷ��ʼ˳�������RCU����������ֻҪ�������ʱ��rcu_read_lock����������ԭʼ����
���԰�ȫ�غ�����list_add_rcu�ȸı�����ԭʼ���ݵ�_rcu��׺����ͬʱ����*/
#define list_for_each_rcu(pos, head) \
	for (pos = (head)->next; prefetch(rcu_dereference(pos)->next), pos != (head); \
        	pos = pos->next)

#define __list_for_each_rcu(pos, head) \
	for (pos = (head)->next; rcu_dereference(pos) != (head); pos = pos->next)

/*��ȫ��ֹɾ���ı�����RCU��������ֻҪ�������ʱ��rcu_read_lock����������ԭʼ����
���԰�ȫ�غ�����list_add_rcu�ȸı�����ԭʼ���ݵ�_rcu��׺����ͬʱ����*/
#define list_for_each_safe_rcu(pos, n, head) \
	for (pos = (head)->next; n = rcu_dereference(pos)->next, pos != (head); pos = n)

/*������RCU������ָ�����͵�����ֻҪ�������ʱ��rcu_read_lock����������ԭʼ����
���԰�ȫ�غ�����list_add_rcu�ȸı�����ԭʼ���ݵ�_rcu��׺����ͬʱ����*/
#define list_for_each_entry_rcu(pos, head, member) 											\
	for (pos = list_entry((head)->next, typeof(*pos), member); 								\
		prefetch(rcu_dereference(pos)->member.next), &pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member))

/*�ӵ�ǰ������һ����㿪ʼ��������RCU����������ֻҪ�������ʱ��rcu_read_lock����
������ԭʼ������԰�ȫ�غ�����list_add_rcu�ȸı�����ԭʼ���ݵ�_rcu��׺����ͬʱ����*/
#define list_for_each_continue_rcu(pos, head) 				\
	for ((pos) = (pos)->next; 								\
		prefetch(rcu_dereference((pos))->next), (pos) != (head); (pos) = (pos)->next)

 
/*˫������ͷ������һ��ָ�롣��ɢ�б�����ã�ɢ�б���ʹ������ͷָ��̫�˷��ˡ�
û��ǰ����ָ�룬Ҳ��ʧȥ��O(1)����β���*/
struct hlist_head
{
	/*ͷ���ָ��ɢ�б��һ�����*/
	struct hlist_node *first;
};

/*ɢ�н��ṹ��*/
struct hlist_node
{
	/*��һ��ɢ�н���ָ��*/
	struct hlist_node *next;
	/*�ý��ǰһ����next��ָ��*/
	struct hlist_node **pprev;
};
/*��ʼ��ɢ�б�ͷ*/
#define HLIST_HEAD_INIT { .first = NULL }
/*����һ��name���Ƶ�ɢ�б�ͷ*/
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
/*��ʼ��ɢ�б�ͷָ��*/
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

/*��ʼ��ɢ�н��*/
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	/*�����ĺ�����ָ������Ϊ��*/
	h->next = NULL;
	/*������ǰ�����next��ָ������Ϊ��*/
	h->pprev = NULL;
}

/*����ɢ�н���Ƿ���ɢ�б��У�������ǰ�����nextָ�����Ƿ�Ϊ��*/
static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

/*����ɢ�б��Ƿ�Ϊ��*/
static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

/*ɾ��ָ����ɢ�н��*/
static inline void __hlist_del(struct hlist_node *n)
{
	/*��ȡָ�����ĺ�����*/
	struct hlist_node *next = n->next;
	/*��ȡָ������ǰ�����next��ָ��*/
	struct hlist_node **pprev = n->pprev;
	/*����ǰ�����next��ָ��ֵΪ��һ���*/
	*pprev = next;
	/*������ǿ������ú������ǰ����next��ָ��Ϊǰ�����next��ָ��*/
	if (next)
		next->pprev = pprev;
}

/*ɾ��ָ����ɢ�н�㣬��ɾ����ɢ�н�㴦��δ����״̬*/
static inline void hlist_del(struct hlist_node *n)
{
	/*��ɢ�б���ɾ�����*/
	__hlist_del(n);
	/*����ɾ�����ĺ���ָ������Ϊδ��״̬������ͨ������ָ�����ɢ�б�*/
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

/*��ɢ�б���ɾ��һ����㣬��ɾ�����δ���³�ʼ�����ý�㴦��δ����״̬����˶������
hlist_unhashed()���᷵��true���Ի���rcu�������������á��ر�أ�����ζ�Ų���ͨ��ǰ��
ָ�����ɢ�б����ú��������ȡ��Ҫ�Ĵ�ʩ������к��ʵ���������ͬʱ������ͬһ�����
��һ����ı�����ԭʼ���ݵĲ�������hlist_add_head_rcu()��hlist_del_rcu()��������̬����
��Ȼ�����ú�������ʹ��_rcu��׺������hlist_for_each_entry()������ԭʼ����*/
static inline void hlist_del_rcu(struct hlist_node *n)
{
	/*ɾ���ý��*/
	__hlist_del(n);
	/*���ý��ǰ�����next��ָ������Ϊδ����״̬������ͨ����ָ�����ȱ���ɢ�б�*/
	n->pprev = LIST_POISON2;
}

/*ɾ��ɢ�б���ָ���Ľ�㣬�����ý�����³�ʼ��*/
static inline void hlist_del_init(struct hlist_node *n)
{
	/*��������ĳ��ɢ�б���ʱ�����ý���ɢ�б���ɾ��*/
	if (!hlist_unhashed(n))
	{
		/*�������ɢ�б���ɾ��*/
		__hlist_del(n);
		/*��ʼ����ɾ���Ľ��*/
		INIT_HLIST_NODE(n);
	}
}

/*���½���滻ָ�����*/
static inline void hlist_replace_rcu(struct hlist_node *old, struct hlist_node *new)
{
	/*��ȡ���滻������һ�����*/
	struct hlist_node *next = old->next;
	/*�����½��ĺ��������滻���ĺ�����*/
	new->next = next;
	/*�����滻����ǰ����Ϊ���滻����ǰ����*/
	new->pprev = old->pprev;
	/*�����ڴ�����*/
	smp_wmb();
	/*����滻���ĺ�����ǿգ������ú������ǰ����next��ָ��Ϊ�滻����next���ַ*/
	if (next)
		new->next->pprev = &new->next;
	/*�����滻����ǰ�����next���ֵΪ�滻���*/
	*new->pprev = new;
	/*�����滻����ǰ����nextָ��������Ϊδ����״̬�����ý�㲻���κ�ɢ�б���*/
	old->pprev = LIST_POISON2;
}

/*��ɢ�б������һ��ɢ�е�*/
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	/*��ȡɢ�б��һ�����*/
	struct hlist_node *first = h->first;
	/*�����²�����ĺ�����Ϊǰɢ�б����һ�����*/
	n->next = first;
	/*���ɢ�зǿգ��򽫺������ǰ����next��ָ������Ϊ������ĵ�ַ*/
	if (first)
		first->pprev = &n->next;
	/*�����²�����Ϊɢ�б��һ�����*/
	h->first = n;
	/*�����²������ǰ������ɢ�б��ͷ��ַ*/
	n->pprev = &h->first;
}

/*ͷ�巨��ɢ�б������һ����㡣������̬����ʱ�����ر�ɢ�б�������ر��㡣
���ú��������ȡ�κα�Ҫ��Ԥ����ʩ����������ʵ�����������ͬʱ������һ������
�ϵ���һ���ɸı�����ԭʼ���ݵ�����hlist_add_head_rcu()��hlist_del_rcu()�Ĳ���
������̬��Ȼ���������������ĺ�_rci��׺������hlist_for_each_entry_rcu()����ԭʼ
�������ͬʱ���У�����������Alpha�ദ������ֹ�ڴ�һ�����⣬����cpu���ͣ�ԭʼ����
�ı�������Ҫ��rcu_read_lock()����*/
static inline void hlist_add_head_rcu(struct hlist_node *n, struct hlist_head *h)
{
	/*��ȡɢ�б�ĵ�һ�����*/
	struct hlist_node *first = h->first;
	/*����n����next��ָ��ָfirst*/
	n->next = first;
	/*����n��ǰ����next��Ϊɢ�б�ͷ���ĵ�ַ*/
	n->pprev = &h->first;
	/*�����ڴ�д����*/
	smp_wmb();
	/*֮ǰ���б�ǿ�ʱ���ڶ������firstǰ����next��ָ��ָ��n����next��ָ��ĵ�ַ*/
	if (first)
		first->pprev = &n->next;
	/*����ɢ�б�ĵ�һ�����Ϊn*/
	h->first = n;
}

/*��ɢ�е�n���뵽ɢ�е�next֮ǰ��next�����ǿս��*/
static inline void hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{
	/*����n����ǰ����next��ָ��Ϊnext���ǰ����nextָ����*/
	n->pprev = next->pprev;
	/*����n���ĺ���ָ����Ϊnext*/
	n->next = next;
	/*����nextǰ��ָ���next�����ֵΪn����nextָ����ĵ�ַ*/
	next->pprev = &n->next;
	/*����n���ǰ�����nextָ����ָ��n����next��*/
	*(n->pprev) = n;
}

/*��ɢ�е�next���뵽ɢ�е�n֮��*/
static inline void hlist_add_after(struct hlist_node *n, struct hlist_node *next)
{
	/*����next������Ϊn������*/
	next->next = n->next;
	/*����n�ĺ�����Ϊnext*/
	n->next = next;
	/*����nextǰ��n����next��ָ��Ϊnext���next��ָ��ĵ�ַ*/
	next->pprev = &n->next;
	/*next������ǿ�ʱ����next�������pprev����Ϊnext����nextָ�����ַ*/
	if(next->next)
		next->next->pprev  = &next->next;
}

/*��ɢ�б���ָ�����ǰ���һ���½�㡣���ú��������ȡ�κα�Ҫ��Ԥ����ʩ���������
�ʵ�����������ͬʱ������һ�������ϵ���һ���ɸı�����ԭʼ���ݵ�����hlist_add_head_rcu()
��hlist_del_rcu()�Ĳ���������̬��Ȼ���������������ĵĺ�_rci��׺������
hlist_for_each_entry_rcu()����ԭʼ�������ͬʱ���У�����������Alpha�ദ������ֹ�ڴ�
һ�����⣬����cpu���ͣ�ԭʼ����ı�������Ҫ��rcu_read_lock()����*/
/*��n�����뵽next���֮ǰ*/
static inline void hlist_add_before_rcu(struct hlist_node *n, struct hlist_node *next)
{
	/*����n��ǰ�����next��ָ��Ϊnextǰ�����next��ָ��*/
	n->pprev = next->pprev;
	/*����n�ĺ�����Ϊnext*/
	n->next = next;
	/*�ڴ�д����*/
	smp_wmb();
	/*nextǰ�����next��ָ��ָ��next����next���ַ*/
	next->pprev = &n->next;
	/*����n��ǰ�����next��Ϊn��next��ָ��*/
	*(n->pprev) = n;
}

/**
 * hlist_add_after_rcu
 * @prev: the existing element to add the new element after.
 * @n: the new element to add to the hash list.
 *
 * Description:
 * Adds the specified element to the specified hlist
 * after the specified node while permitting racing traversals.
 *
 */
 /*��ɢ�б���ָ�����֮�����һ���½��.���ú��������ȡ�κα�Ҫ��Ԥ����ʩ�������
 ���ʵ�����������ͬʱ������һ�������ϵ���һ���ɸı�����ԭʼ���ݵ�����hlist_add_head_rcu()
 ��hlist_del_rcu()�Ĳ���������̬��Ȼ���������������Ϸ��ĺ�_rci��׺������
 hlist_for_each_entry_rcu()����ԭʼ�������ͬʱ���У�����������Alpha�ദ������ֹ�ڴ�
 һ�����⣬����cpu���ͣ�ԭʼ����ı�������Ҫ��rcu_read_lock()����*/
static inline void hlist_add_after_rcu(struct hlist_node *prev, struct hlist_node *n)
{
	/*����n�ĺ�������prev�ĺ�����*/
	n->next = prev->next;
	/*����n��ǰ�����next����prev����next���ַ*/
	n->pprev = &prev->next;
	/*�����ڴ�д����*/
	smp_wmb();
	/*����prev��������n*/
	prev->next = n;
	if (n->next)
		n->next->pprev = &n->next;
}

/*����type�ṹ��Ա���������ͱ�����member�����Ӧ�ĵ�ַ��ptr��������container_of
���ƻ�ȡtype�ṹ������ַ*/
#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

/*����Ԥȡ���ܣ�˳�����ɢ�б�pos&&({prefetch(pos->next); 1; )��ʾ��ֻҪpos�ǿգ�
ѭ����һֱ����*/
#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos && ({ prefetch(pos->next); 1; }); pos = pos->next)

/**/
#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); pos = n)

/*��Ԥȡ���ܵ�˳�����ָ�����͵�ɢ�б�ֱ��ɢ�б����*/
#define hlist_for_each_entry(tpos, pos, head, member)			 								\
	for (pos = (head)->first;		pos && ({ prefetch(pos->next); 1;}) &&			 			\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

/*�ӵ�ǰ�����һ��㿪ʼ��������ɢ�б�*/
#define hlist_for_each_entry_continue(tpos, pos, member)		 					\
	for (pos = (pos)->next;	pos && ({ prefetch(pos->next); 1;}) &&			\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

/*�ӵ�ǰ��㿪ʼ����ɢ�б�*/
#define hlist_for_each_entry_from(tpos, pos, member)			\
	for (; pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

/*��ָ�����͵�ɢ�б��һ����㿪ʼ��ѭ������ɢ�б�*/
#define hlist_for_each_entry_safe(tpos, pos, n, head, member) 		 				\
	for (pos = (head)->first;	pos && ({ n = pos->next; 1; }) && 				 	\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = n)

/*ֻҪ��rcu_read_lock()�����ı����£�ԭʼ���ݵ�����������԰�ȫ����_rcu��׺�ɸı�ԭʼ
���ݵģ�����hlist_add_head_rcu������ͬʱ����*/
/*����RCU���Ƽ�Ԥȡ���ܣ��ӵ�һ����㿪ʼ��˳�����ָ������ɢ�б�*/
#define hlist_for_each_entry_rcu(tpos, pos, head, member)		 \
	for (pos = (head)->first;	rcu_dereference(pos) && ({ prefetch(pos->next); 1;}) &&	 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

#else
#warning "don't include kernel headers in userspace"
#endif /* __KERNEL__ */
#endif
