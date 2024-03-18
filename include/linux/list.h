#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

#ifdef __KERNEL__

#include <linux/stddef.h>
#include <linux/poison.h>
#include <linux/prefetch.h>
#include <asm/system.h>

/*简单的双链表实现。当操作整个链表而非单个结点时一些内部函数很有用，因为有时我们已经
直到前或后结点，通过直接使用它们可以生成更好的代码*/
/*简单双链表定义*/
struct list_head
{
	/*后项结点指针*/
	struct list_head *next;
	/*前项结点指针*/
	struct list_head *prev;
};

/*静态宏方式初始化一个结点*/
#define LIST_HEAD_INIT(name) { &(name), &(name) }
/*定义并初始化一个结点*/
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)
/*函数方式初始化一个结点，前后结点的指针指向自身*/
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	/*结点后项结点指针设置为自身*/
	list->next = list;
	/*结点前项结点指针设置为自身*/
	list->prev = list;
}

/*在已知的两个连续结点之间插入一个新结点*/
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

/*在一个指定结点之后插入一个新结点，该函数实现的头插法（插入的新结点紧邻在头结点之后）
对实现栈很有用*/
#ifndef CONFIG_DEBUG_LIST
static inline void list_add(struct list_head *new, struct list_head *head)
{
	/*头插法向链表添加新结点*/
	__list_add(new, head, head->next);
}
#else
extern void list_add(struct list_head *new, struct list_head *head);
#endif


/*在一个特定结点之前插入一个新结点，该函数实现的尾插法（插入的新结点紧邻在尾结点之后）
对实现队列很有用*/
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	/*尾插法向链表添加新结点*/
	__list_add(new, head->prev, head);
}


/*在已知的两个连续结点之间插入一个新结点，先设置该结点的前后指针域，再设置内存
屏障，最后设置该结点的前（后）项结点的next（prev）指针域*/
static inline void __list_add_rcu(struct list_head * new, struct list_head * prev,
									struct list_head * next)
{
	/*向确定新结点的前后指针域内容*/
	new->next = next;
	new->prev = prev;
	/*设置内存优化屏障，在此处之后的写操作开始之前，此处之前所有的写操作都已经完成*/
	smp_wmb();
	/*确定新结点的前（后）项结点的next（prev）指针域内容*/
	next->prev = new;
	prev->next = new;
}

/*在受rcu保护的链表头结点之后添加一个新结点，这个对实现栈很有用。调用函数必须采取必要
的措施（如持有合适的锁）避免同时运行在同一链表的另一个会改变链表原始数据的操作（如
list_add_head_rcu()或list_del_rcu()）产生竟态条件。然而，该函数可与使用_rcu后缀（比如
list_for_each_entry()）遍历原始链表*/
static inline void list_add_rcu(struct list_head *new, struct list_head *head)
{
	/*头插法向受RCU保护的链表添加新结点*/
	__list_add_rcu(new, head, head->next);
}

/*向rcu保护的链表表头后（紧邻表头）添加一个新结点，该方法对实现队列很有用。
调用函数必须采取必要的措施（如持有合适的锁）避免同时运行在同一链表的另一个会改变链表
原始数据的操作（如list_add_head_rcu()或list_del_rcu()）产生竟态条件。然而，该函数可与
使用_rcu后缀（比如list_for_each_entry()）遍历原始链表*/
static inline void list_add_tail_rcu(struct list_head *new, struct list_head *head)
{
	/*尾插法向受RCU保护的链表添加新结点*/
	__list_add_rcu(new, head->prev, head);
}
 
/*删除已知的前后两结点之间的结点，可以删除多个结点，将已知结点的前后指针域相连*/
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	/*直接设置前后结点的指针域相连*/
	next->prev = prev;
	prev->next = next;
}

/*从链表中删除一个结点。删除后的结点处于未定义状态，此时对被删除结点调用list_empty()
不会返回true，正常环境中访问这些结点将导致缺页异常*/
#ifndef CONFIG_DEBUG_LIST
static inline void list_del(struct list_head *entry)
{
	/*删除entry结点*/
	__list_del(entry->prev, entry->next);
	/*结点被删除后，将结点设置为未定义状态*/
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}
#else
extern void list_del(struct list_head *entry);
#endif

/*将指定结点从链表中删除，并且将已删除结点的前项指针域设置为未定义状态。在正常环境中对
该结点执行list_empty()不会返回true，该函数对基于无锁RCU遍历很有用。特别地，不能毒化可能
仍然处于链表遍历使用中的前项指针。调用函数必须采取必要的措施（如持有合适的锁）避免同时
运行在同一链表的另一个会改变链表原始数据的操作（如list_add_head_rcu()或list_del_rcu()）
产生竟态条件。然而，该函数可与使用_rcu后缀（比如list_for_each_entry()）遍历原始链表。注
意！调用函数不允许立即释放新删除的结点，相反，使用call_rcu()或者synchronize_rcu()推迟释
放，直到过了宽限期*/
static inline void list_del_rcu(struct list_head *entry)
{
	/*删除entry结点*/
	__list_del(entry->prev, entry->next);
	/*将已删除的结点前项指针域设置为未定义状态，正常环境下访问该结点将导致缺页异常*/
	entry->prev = LIST_POISON2;
}

/*用新结点替换旧结点，先设置新结点与旧结点的后项结点之间的联系，再设置新结点有旧结点
前项结点之间的关系，注意！旧结点不能为空。？如果旧结点为空，它将被重写？*/
static inline void list_replace(struct list_head *old, struct list_head *new)
{
	/*建立新结点与旧结点后项结点之间的联系*/
	new->next = old->next;
	new->next->prev = new;
	/*建立新结点与旧结点前项结点之间的连接*/
	new->prev = old->prev;
	new->prev->next = new;
}

/*新结点替换旧结点，并将旧结点重新初始化*/
static inline void list_replace_init(struct list_head *old, struct list_head *new)
{
	/*用新结点替换旧结点*/
	list_replace(old, new);
	/*重新初始化旧结点*/
	INIT_LIST_HEAD(old);
}

/*用新结点替换旧结点，注意！旧结点不能为空。先建立新结点与旧结点前后项之间的联系，
随后启用内存写优化屏障，然后再建立新结点前后项与新结点之间的联系，最后将旧结点的
前项指针域设置为未定义状态，对该结点调用list_empty()不会返回true，正常环境下，访
问该结点将导致缺页异常*/
static inline void list_replace_rcu(struct list_head *old, struct list_head *new)
{
	/*设置新结点的前后指针域*/
	new->next = old->next;
	new->prev = old->prev;
	/*设置内存优化写屏障*/
	smp_wmb();
	/*设置新结点前（后）项的next（prev）指针域*/
	new->next->prev = new;
	new->prev->next = new;
	/*将旧结点的前项指针域设置为未定义状态*/
	old->prev = LIST_POISON2;
}

/*从链表上删除一结点，并将该结点重新初始化*/
static inline void list_del_init(struct list_head *entry)
{
	/*从链表中删除entry结点*/
	__list_del(entry->prev, entry->next);
	/*初始化entry结点*/
	INIT_LIST_HEAD(entry);
}

/*删除链表上指定的结点，并采用头插法将该结点添加到另一链表*/
static inline void list_move(struct list_head *list, struct list_head *head)
{
	/*删除list结点*/
	__list_del(list->prev, list->next);
	/*头插法将list结点插入到head链表*/
	list_add(list, head);
}

/*从链表上删除指定结点，采用尾插法将该结点插入到另一个链表*/
static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
	/*删除链表中list结点*/
	__list_del(list->prev, list->next);
	/*尾插法将list结点添加到head链表*/
	list_add_tail(list, head);
}

/*测试结点是否是链表的尾结点。结点的next指针域是否为头结点*/
static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
	return list->next == head;
}

/*测试链表是否为空，测试链表头结点的next域是否是自身*/
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/*测试链表是否为空，并且检查其它cpu没有正在修改其成员或其前后指针域。注意！使用非
同步的list_empty_carful()仅在链表执行list_del_init()时是安全的，另一个cpu重新执行
list_add()时不能使用该函数*/
static inline int list_empty_careful(const struct list_head *head)
{
	/*获取链表第一个结点*/
	struct list_head *next = head->next;
	/*只有当链表的头结点前后指针域都指向自身时，链表才为空*/
	return (next == head) && (next == head->prev);
}

/*合并两个链表。将指定链表中除头结点之外的所有结点添加到另一个链表的尾部*/
static inline void __list_splice(struct list_head *list, struct list_head *head)
{
	/*获取list链表的第一个结点*/
	struct list_head *first = list->next;
	/*获取list链表的最后一个结点*/
	struct list_head *last = list->prev;
	/*获取head链表的第一个结点*/
	struct list_head *at = head->next;

	/*设置list链表的第一个结点为head链表的第一个结点*/
	first->prev = head;
	head->next = first;
	/*设置list链表的最后一个结点为head前第一个结点的前结点*/
	last->next = at;
	at->prev = last;
}

/*合并两链表。源链表非空时，采用头插法将源链表中除头结点之外的所有结点整体平移插入
到目标链表*/
static inline void list_splice(struct list_head *list, struct list_head *head)
{
	/*list链表非空时，采用头插法将list链表中非头结点插入head链表*/
	if (!list_empty(list))
		__list_splice(list, head);
}

/*合并两链表，采用头插法将非空源链表中非头结点整体平移插入目标链表中，然后将源链表初始化*/
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
	/*list链表非空时合并*/
	if (!list_empty(list))
	{
		/*头插法将list链表中非头结点整体插入head链表*/
		__list_splice(list, head);
		/*初始化list链表*/
		INIT_LIST_HEAD(list);
	}
}

/*将一个受rcu保护的源链表合并到一个已存在的目标链表。目标链表能被rcu_read和该函数
同时遍历。重要说明：调用函数必须采取必要措施阻止目标链表的任何其它更新。原则中，当
sync()开始运行时尽快修改该链表时可能的，如果这些变为必要，一个基于call_rcu()的可选
版本可以被创建。但仅当必要时，这里没有RCU API成员的简化版本*/
static inline void list_splice_init_rcu(struct list_head *list, struct list_head *head,
											void (*sync)(void))
{
	/*获取预分裂链表的第一个结点*/
	struct list_head *first = list->next;
	/*获取预分裂链表的最后一个结点*/
	struct list_head *last = list->prev;
	/*获取目标链表的第一个结点*/
	struct list_head *at = head->next;
	/*目标链表为空时直接退出*/
	if (list_empty(head))
		return;
	/*预分裂链表第一个和最后一个结点已知候就可以遍历链表，因此可以重新初始化该链表*/
	INIT_LIST_HEAD(list);

	/*此时，链表主体依然指向源链表，在该链表合并到目标链表之前等待任一读者结束使用
	该链表任何新读者看到的都是空链表*/
	/*读者结束对源链表的访问后，开始执行合并合并操作，当目标链表是rcu读者同时访问的
	全局链表时顺序就变得很重要。注意！没有该函数的rcu读者不允许遍历前域指针*/
	sync();
	/*源链表最后结点的下一个结点设置为目标链表的是一个结点*/
	last->next = at;
	/*执行内存写屏障，保证在该点之后的写操作开始之前，前面所有的写操作都已经执行完毕*/
	smp_wmb();
	/*目标链表的第一个结点设置为之前源链表的第一个结点*/
	head->next = first;
	first->prev = head;
	/*前目标链表的前一个结点设置为前源链表的最后一个结点*/
	at->prev = last;
}

/*根据（type）结构成员（链表变量）member及其地址ptr，获取该机构变量的地址*/
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/*根据（type）结构成员（链表变量）member及其地址ptr，获取该结构变量表中第一个变量
地址。警告！链表不能为空*/ 
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)

/*根据链表头结点，带预取功能的顺序遍历链表，操作期间应该采取必要的保护措施。如果在
遍历期间调用list_del(pos)，导致内核panic，如果调用list_del_init(pos)则导致死循环*/
#define list_for_each(pos, head) \
	for (pos = (head)->next; prefetch(pos->next), pos != (head); pos = pos->next)

 /*根据链表头结点，无预取功能的顺序遍历该链表。使用该函数的前提是已知链表很短，
 大部分时间为空或者只有一个结点*/
 
#define __list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

/*带预取功能的逆序遍历链表*/
#define list_for_each_prev(pos, head) 											\
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); 		\
        	pos = pos->prev)


/*无删除操作安全的遍历链表*/
#define list_for_each_safe(pos, n, head) 										\
	for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/*无删除操作安全的逆序遍历链表*/
#define list_for_each_prev_safe(pos, n, head) 													\
	for (pos = (head)->prev, n = pos->prev; prefetch(pos->prev), pos != (head); \
	     pos = n, n = pos->prev)

/*遍历给定类型的链表。根据结构（type(*pos)）中成员（链表类型）member及该链表头结点
（head），遍历该类型的所有变量*/
#define list_for_each_entry(pos, head, member)									\
	for (pos = list_entry((head)->next, typeof(*pos), member);					\
	     prefetch(pos->member.next), &pos->member != (head);				 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/*逆序遍历给定类型的链表。根据结构（typeof(*pos)）成员（链表变量）member及链表
头结点（head），逆序遍历所有结构变量*/
#define list_for_each_entry_reverse(pos, head, member)							\
	for (pos = list_entry((head)->prev, typeof(*pos), member);					\
	     prefetch(pos->member.prev), &pos->member != (head); 					\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))
 
 /*获取为使用list_for_each_entry_continue()的入口结点pos，pos非空时使用pos，否则pos
 赋值为使用list_entry获取第一个结构变量地址*/
#define list_prepare_entry(pos, head, member) ((pos) ? : list_entry(head, typeof(*pos), member))

/*从当前的位置（结构变量pos地址）开始，继续遍历给定类型（typeof(*pos)）的链表，
直至链表结束*/
#define list_for_each_entry_continue(pos, head, member)							\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);		\
	     prefetch(pos->member.next), &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/*从当前位置（给定结构体变量地址pos）开始，逆序遍历该结构体变量所在的链表，直至
遍历到头结点为止*/
#define list_for_each_entry_continue_reverse(pos, head, member)					\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);		\
	     prefetch(pos->member.prev), &pos->member != (head);					\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/*从当前位置所属结点的下一个结点开始，顺序遍历给定类型的链表*/
#define list_for_each_entry_from(pos, head, member) 							\
	for (; prefetch(pos->member.next), &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))


/*禁止删除操作且安全的从头开始遍历指定类型的链表*/
#define list_for_each_entry_safe(pos, n, head, member)							\
	for (pos = list_entry((head)->next, typeof(*pos), member)					\
		n = list_entry(pos->member.next, typeof(*pos), member);					\
	     &pos->member != (head); 												\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*从当前结点的下一个结点开始，安全无删除操作的遍历指定类型的链表*/
#define list_for_each_entry_safe_continue(pos, n, head, member) 				\
	for (pos = list_entry(pos->member.next, typeof(*pos), member), 	\
		n = list_entry(pos->member.next, typeof(*pos), member);					\
	     &pos->member != (head);												\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*从当前结点开始，禁止删除安全的顺序遍历指定类型的链表*/
#define list_for_each_entry_safe_from(pos, n, head, member) 					\
	for (n = list_entry(pos->member.next, typeof(*pos), member);				\
	     &pos->member != (head);												\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*从当前结点开始，安全禁止删除操作的逆序遍历指定类型链表*/
#define list_for_each_entry_safe_reverse(pos, n, head, member)					\
	for (pos = list_entry((head)->prev, typeof(*pos), member),					\
		n = list_entry(pos->member.prev, typeof(*pos), member);					\
	     &pos->member != (head); 												\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))


/*从头开始顺序遍历受RCU保护的链表。只要链表遍历时有rcu_read_lock保护，遍历原始链表
可以安全地和诸如list_add_rcu等改变链表原始数据的_rcu后缀函数同时运行*/
#define list_for_each_rcu(pos, head) \
	for (pos = (head)->next; prefetch(rcu_dereference(pos)->next), pos != (head); \
        	pos = pos->next)

#define __list_for_each_rcu(pos, head) \
	for (pos = (head)->next; rcu_dereference(pos) != (head); pos = pos->next)

/*安全禁止删除的遍历受RCU保护链表。只要链表遍历时有rcu_read_lock保护，遍历原始链表
可以安全地和诸如list_add_rcu等改变链表原始数据的_rcu后缀函数同时运行*/
#define list_for_each_safe_rcu(pos, n, head) \
	for (pos = (head)->next; n = rcu_dereference(pos)->next, pos != (head); pos = n)

/*遍历受RCU保护的指定类型的链表。只要链表遍历时有rcu_read_lock保护，遍历原始链表
可以安全地和诸如list_add_rcu等改变链表原始数据的_rcu后缀函数同时运行*/
#define list_for_each_entry_rcu(pos, head, member) 											\
	for (pos = list_entry((head)->next, typeof(*pos), member); 								\
		prefetch(rcu_dereference(pos)->member.next), &pos->member != (head); \
		pos = list_entry(pos->member.next, typeof(*pos), member))

/*从当前结点的下一个结点开始，遍历受RCU保护的链表。只要链表遍历时有rcu_read_lock保护
，遍历原始链表可以安全地和诸如list_add_rcu等改变链表原始数据的_rcu后缀函数同时运行*/
#define list_for_each_continue_rcu(pos, head) 				\
	for ((pos) = (pos)->next; 								\
		prefetch(rcu_dereference((pos))->next), (pos) != (head); (pos) = (pos)->next)

 
/*双链表中头结点仅有一个指针。对散列表很有用，散列表中使用两个头指针太浪费了。
没有前项域指针，也就失去了O(1)访问尾结点*/
struct hlist_head
{
	/*头结点指向散列表第一个结点*/
	struct hlist_node *first;
};

/*散列结点结构体*/
struct hlist_node
{
	/*下一个散列结点的指针*/
	struct hlist_node *next;
	/*该结点前一结点的next域指针*/
	struct hlist_node **pprev;
};
/*初始化散列表头*/
#define HLIST_HEAD_INIT { .first = NULL }
/*定义一个name名称的散列表头*/
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
/*初始化散列表头指针*/
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

/*初始化散列结点*/
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	/*将结点的后项域指针设置为空*/
	h->next = NULL;
	/*将结点的前项结点的next域指针设置为空*/
	h->pprev = NULL;
}

/*测试散列结点是否在散列表中，即结点的前项结点的next指针域是否为空*/
static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

/*测试散列表是否为空*/
static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

/*删除指定的散列结点*/
static inline void __hlist_del(struct hlist_node *n)
{
	/*获取指定结点的后项结点*/
	struct hlist_node *next = n->next;
	/*获取指定结点的前项结点的next域指针*/
	struct hlist_node **pprev = n->pprev;
	/*设置前项结点的next域指针值为下一结点*/
	*pprev = next;
	/*后项结点非空则设置后项结点的前项结点next域指针为前项结点的next域指针*/
	if (next)
		next->pprev = pprev;
}

/*删除指定的散列结点，被删除的散列结点处于未定义状态*/
static inline void hlist_del(struct hlist_node *n)
{
	/*从散列表中删除结点*/
	__hlist_del(n);
	/*将被删除结点的后项指针设置为未定状态，不能通过后项指针遍历散列表*/
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

/*从散列表中删除一个结点，已删除结点未重新初始化，该结点处于未定义状态，因此对其调用
hlist_unhashed()不会返回true，对基于rcu无锁遍历很有用。特别地，这意味着不能通过前项
指针遍历散列表。调用函数必须采取必要的措施（如持有合适的锁）避免同时运行在同一链表的
另一个会改变链表原始数据的操作（如hlist_add_head_rcu()或hlist_del_rcu()）产生竟态条件
。然而，该函数可与使用_rcu后缀（比如hlist_for_each_entry()）遍历原始链表*/
static inline void hlist_del_rcu(struct hlist_node *n)
{
	/*删除该结点*/
	__hlist_del(n);
	/*将该结点前向结点的next域指针设置为未定义状态，不能通过该指针你先遍历散列表*/
	n->pprev = LIST_POISON2;
}

/*删除散列表中指定的结点，并将该结点重新初始化*/
static inline void hlist_del_init(struct hlist_node *n)
{
	/*如果结点在某个散列表中时，将该结点从散列表中删除*/
	if (!hlist_unhashed(n))
	{
		/*将结点重散列表中删除*/
		__hlist_del(n);
		/*初始化被删除的结点*/
		INIT_HLIST_NODE(n);
	}
}

/*用新结点替换指定结点*/
static inline void hlist_replace_rcu(struct hlist_node *old, struct hlist_node *new)
{
	/*获取被替换结点的下一个结点*/
	struct hlist_node *next = old->next;
	/*设置新结点的后项结点是替换结点的后项结点*/
	new->next = next;
	/*设置替换结点的前项结点为被替换结点的前项结点*/
	new->pprev = old->pprev;
	/*设置内存屏障*/
	smp_wmb();
	/*如果替换结点的后项结点非空，则设置后项结点的前项结点next于指针为替换结点的next域地址*/
	if (next)
		new->next->pprev = &new->next;
	/*设置替换结点的前项结点的next域的值为替换结点*/
	*new->pprev = new;
	/*将被替换结点的前项结点next指针域设置为未定义状态，即该结点不在任何散列表中*/
	old->pprev = LIST_POISON2;
}

/*向散列表中添加一个散列点*/
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	/*获取散列表第一个结点*/
	struct hlist_node *first = h->first;
	/*设置新插入结点的后项结点为前散列表的是一个结点*/
	n->next = first;
	/*如果散列非空，则将后项结点的前项结点next域指针设置为后项结点的地址*/
	if (first)
		first->pprev = &n->next;
	/*设置新插入结点为散列表第一个结点*/
	h->first = n;
	/*设置新插入结点的前项结点是散列表表头地址*/
	n->pprev = &h->first;
}

/*头插法向散列表中添加一个结点。当允许竟态遍历时，向特别散列表中添加特别结点。
调用函数必须采取任何必要的预防措施（比如持有适当的锁）避免同时运行在一个链表
上的另一个可改变链表原始内容的诸如hlist_add_head_rcu()或hlist_del_rcu()的操作
产生竟态，然而，它可以完美的和_rci后缀（诸如hlist_for_each_entry_rcu()）的原始
链表遍历同时运行，经常用于在Alpha多处理器阻止内存一致问题，忽略cpu类型，原始链表
的遍历必须要有rcu_read_lock()保护*/
static inline void hlist_add_head_rcu(struct hlist_node *n, struct hlist_head *h)
{
	/*获取散列表的第一个结点*/
	struct hlist_node *first = h->first;
	/*设置n结点的next域指针指first*/
	n->next = first;
	/*设置n的前项结点next域为散列表头结点的地址*/
	n->pprev = &h->first;
	/*设置内存写屏障*/
	smp_wmb();
	/*之前的列表非空时，第二个结点first前项结点next域指针指向n结点的next域指针的地址*/
	if (first)
		first->pprev = &n->next;
	/*设置散列表的第一个结点为n*/
	h->first = n;
}

/*将散列点n插入到散列点next之前，next不能是空结点*/
static inline void hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{
	/*设置n结点的前项结点next域指针为next结点前项结点next指针域*/
	n->pprev = next->pprev;
	/*设置n结点的后项指针域为next*/
	n->next = next;
	/*设置next前项指结点next针域的值为n结点的next指针域的地址*/
	next->pprev = &n->next;
	/*设置n结点前项结点的next指针域指向n结点的next域*/
	*(n->pprev) = n;
}

/*将散列点next插入到散列点n之后*/
static inline void hlist_add_after(struct hlist_node *n, struct hlist_node *next)
{
	/*设置next后项结点为n后项结点*/
	next->next = n->next;
	/*设置n的后项结点为next*/
	n->next = next;
	/*设置next前项n结点的next域指针为next结点next域指针的地址*/
	next->pprev = &n->next;
	/*next后项结点非空时，将next后项结点的pprev设置为next结点的next指针域地址*/
	if(next->next)
		next->next->pprev  = &next->next;
}

/*向散列表中指定结点前添加一个新结点。调用函数必须采取任何必要的预防措施（比如持有
适当的锁）避免同时运行在一个链表上的另一个可改变链表原始内容的诸如hlist_add_head_rcu()
或hlist_del_rcu()的操作产生竟态，然而，它可以完美的的和_rci后缀（诸如
hlist_for_each_entry_rcu()）的原始链表遍历同时运行，经常用于在Alpha多处理器阻止内存
一致问题，忽略cpu类型，原始链表的遍历必须要有rcu_read_lock()保护*/
/*将n结点插入到next结点之前*/
static inline void hlist_add_before_rcu(struct hlist_node *n, struct hlist_node *next)
{
	/*设置n的前项结点的next域指针为next前项结点的next域指针*/
	n->pprev = next->pprev;
	/*设置n的后项结点为next*/
	n->next = next;
	/*内存写屏障*/
	smp_wmb();
	/*next前项结点的next域指针指向next结点的next域地址*/
	next->pprev = &n->next;
	/*设置n的前项结点的next域为n的next域指针*/
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
 /*向散列表中指定结点之后添加一个新结点.调用函数必须采取任何必要的预防措施（比如持
 有适当的锁）避免同时运行在一个链表上的另一个可改变链表原始内容的诸如hlist_add_head_rcu()
 或hlist_del_rcu()的操作产生竟态，然而，它可以完美合法的和_rci后缀（诸如
 hlist_for_each_entry_rcu()）的原始链表遍历同时运行，经常用于在Alpha多处理器阻止内存
 一致问题，忽略cpu类型，原始链表的遍历必须要有rcu_read_lock()保护*/
static inline void hlist_add_after_rcu(struct hlist_node *prev, struct hlist_node *n)
{
	/*设置n的后项结点是prev的后项结点*/
	n->next = prev->next;
	/*设置n的前项结点的next域是prev结点的next域地址*/
	n->pprev = &prev->next;
	/*设置内存写屏障*/
	smp_wmb();
	/*设置prev后项结点是n*/
	prev->next = n;
	if (n->next)
		n->next->pprev = &n->next;
}

/*根据type结构成员（链表类型变量）member及其对应的地址（ptr），利用container_of
机制获取type结构变量地址*/
#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

/*利用预取功能，顺序遍历散列表，pos&&({prefetch(pos->next); 1; )表示，只要pos非空，
循环将一直进行*/
#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos && ({ prefetch(pos->next); 1; }); pos = pos->next)

/**/
#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); pos = n)

/*带预取功能的顺序遍历指定类型的散列表，直至散列表结束*/
#define hlist_for_each_entry(tpos, pos, head, member)			 								\
	for (pos = (head)->first;		pos && ({ prefetch(pos->next); 1;}) &&			 			\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

/*从当前结点下一结点开始继续遍历散列表*/
#define hlist_for_each_entry_continue(tpos, pos, member)		 					\
	for (pos = (pos)->next;	pos && ({ prefetch(pos->next); 1;}) &&			\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

/*从当前结点开始遍历散列表*/
#define hlist_for_each_entry_from(tpos, pos, member)			\
	for (; pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

/*从指定类型的散列表第一个结点开始，循环遍历散列表*/
#define hlist_for_each_entry_safe(tpos, pos, n, head, member) 		 				\
	for (pos = (head)->first;	pos && ({ n = pos->next; 1; }) && 				 	\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = n)

/*只要在rcu_read_lock()函数的保护下，原始数据的链表遍历可以安全的与_rcu后缀可改变原始
数据的（诸如hlist_add_head_rcu）函数同时运行*/
/*利用RCU机制及预取功能，从第一个结点开始，顺序遍历指定类型散列表*/
#define hlist_for_each_entry_rcu(tpos, pos, head, member)		 \
	for (pos = (head)->first;	rcu_dereference(pos) && ({ prefetch(pos->next); 1;}) &&	 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); pos = pos->next)

#else
#warning "don't include kernel headers in userspace"
#endif /* __KERNEL__ */
#endif
