#ifndef	_LINUX_RBTREE_H
#define	_LINUX_RBTREE_H

#include <linux/kernel.h>
#include <linux/stddef.h>

/*一般会要求二叉树所有的节点按照一定的顺序排列，这样我们进行插入、删除、查找时效率就
会非常高，我们把这样的树叫做二叉搜索树或者二叉查找树。它的具体定义是这样的，二叉搜索
树，要么是个空树，要么符合以下几个条件：1.左子树如果存在的话，左子树所有节点的键值都
要小于根节点的键值，2.右子树如果存在的话，右子树所有节点的键值都要大于根节点的键值，
3.它的所有子树也都要符合前面的两个条件。经过这样定义之后，二叉树就变成了二叉搜索树，
它的插入、删除、查找效率一般情况下都是O(logn)。
如果我们对不等叉树的节点键值数和插入、删除逻辑添加一些特殊的要求，使其能达到绝对平衡
的效果，我们就把这种树叫做B树。B树全称Balance Tree，是一种自平衡树。它和等叉树最大的
不同首先表现在存储结构上，等叉树上每个节点的键值数和分叉数都是相同的，而B树则不是。如
果某个B树上所有节点的分叉数最大值是m，则把这个B数叫做m阶B树。B树的具体定义:
（1）所有节点最多有m个子节点（2）非根非叶子节点至少有m/2(向上取整)个子节点（3）根节点
至少有两个子节点(除非总结点数都不够3个)（4）所有叶子节点都在同一层（5）任意节点如果有
k个键值，则有k+1个子节点指针，键值要按照从小到大排列，子节点树上所有的键值都要在对应的
两个键值之间。

*/
/*红黑树又称RBTree（Red-Black Tree），它是一种特殊的二叉查找树，红黑树的每个节点上都有
存储位表示节点的颜色，可以是红或黑。红黑树具有以下五个特性：
1）每个节点或者是黑色，或者是红色。2）根节点是黑色。3）每个叶子结点（NIL，这里的叶子结
点不是传统的叶子结点，是指为空的叶子结点）是黑色。4）如果一个结点是红色的，则它的子结
点必须是黑色的。5）从一个结点到该结点的子孙结点的所有路径上包含相同数目的黑结点。
根据特性5，可以确保没有一条路径会比其他路径长处两倍，因此，红黑树是相对接近平衡的二叉
树。因为操作比如插入、删除和查找某个值的最坏情况时间都要求与树的高度成比例，这个在高度
上的理论上限允许红黑树在最坏情况下都是高效的，而不同于普通的二叉查找树。为什么说红黑树
有一条路径会比其他路径长处两倍呢，注意到性质4导致了路径不能有两个毗连的红色节点就足够了
。最短的可能路径都是黑色节点，最长的可能路径有交替的红色和黑色节点。因为根据性质5所有最
长的路径都有相同数目的黑色节点，这就表明了没有路径能多于任何其他路径的两倍长。
红黑树的应用比较广泛，主要是用它来存储有序的数据，它的时间复杂度是O(lgn)，效率非常之高
一棵含有n个节点的红黑树的高度至多为2log(n+1)
*/

/*红黑树结点*/
struct rb_node
{
	/*父结点指针（四字节对齐）与本结点颜色（最低位）组合*/
	unsigned long  rb_parent_color;
	/*红色*/
#define	RB_RED		0
	/*黑色*/
#define	RB_BLACK	1
	/*结点右子树*/
	struct rb_node *rb_right;
	/*结点左子树*/
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));/*结构体四字节对齐*/
/*红黑树根结点*/
struct rb_root
{	/*红黑树根结点指针*/
	struct rb_node *rb_node;
};

/*获取父结点指针，因为最低位为颜色，直接屏蔽最低二位即可*/
#define rb_parent(r) 		((struct rb_node *)((r)->rb_parent_color & ~3))
/*获取结点颜色*/
#define rb_color(r)			((r)->rb_parent_color & 1)
/*结点是否为红色*/
#define rb_is_red(r)   		(!rb_color(r))
/*结点是否为黑色*/
#define rb_is_black(r) 		rb_color(r)
/*设置结点为红色*/
#define rb_set_red(r)  		do { (r)->rb_parent_color &= ~1; } while (0)
/*设置结点为黑色*/
#define rb_set_black(r)		do { (r)->rb_parent_color |= 1; } while (0)

/*设置结点父结点*/
static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (unsigned long)p;
}

/*设置父结点颜色*/
static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

/*初始化根结点*/
#define RB_ROOT	(struct rb_root) { NULL, }
/*根据结构体中成员member的指针获取该结构体变量的指针*/
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)
/*红黑树是否为空*/
#define RB_EMPTY_ROOT(root)	((root)->rb_node == NULL)
/*结点是否为根结点*/
#define RB_EMPTY_NODE(node)	(rb_parent(node) == node)
/*设置结点为根结点*/
#define RB_CLEAR_NODE(node)	(rb_set_parent(node, node))

extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

/*树中查询前后逻辑结点*/
extern struct rb_node *rb_next(struct rb_node *);
extern struct rb_node *rb_prev(struct rb_node *);
extern struct rb_node *rb_first(struct rb_root *);
extern struct rb_node *rb_last(struct rb_root *);

/*无需删除/平衡/添加/再平衡快速替换一个结点*/
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new,
			    				struct rb_root *root);

/*将node作为parent的子结点添加到树中*/
static inline void rb_link_node(struct rb_node * node, struct rb_node * parent,
									struct rb_node ** rb_link)
{
	/*设置node的父节点为parent*/
	node->rb_parent_color = (unsigned long )parent;
	/*node左右子树为空*/
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#endif	/* _LINUX_RBTREE_H */
