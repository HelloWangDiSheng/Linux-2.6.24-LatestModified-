#include <linux/rbtree.h>
#include <linux/module.h>
/*左旋：以某个结点作为支点（旋转结点），其右子结点变位旋转结点的父结点，右子结点的左
子结点变为旋转结点的右子结点，左子结点保持不变*/
static void __rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
	/*获取旋转结点的右子结点*/
	struct rb_node *right = node->rb_right;
	/*获取旋转结点的父结点*/
	struct rb_node *parent = rb_parent(node);
	/*旋转结点的右子结点赋值为其右子结点的左子结点，如果其非空，则设置旋转结点右子结点
	的左子结点的父结点为旋转结点*/
	if ((node->rb_right = right->rb_left))
		rb_set_parent(right->rb_left, node);
	/*设置旋转结点右子结点为其父结点*/
	right->rb_left = node;
	/*设置旋转结点的祖父结点为其原父结点*/
	rb_set_parent(right, parent);
	/*如果旋转结点原父结点非空，则重新设置其父结点与原父结点之前的关系*/
	if (parent)
	{
		/*如果旋转结点之前是其父结点的左子结点，则设置其父结点为其原父结点的左子结点，
		否则，设置其父结点为其原父结点的右子结点*/
		if (node == parent->rb_left)
			parent->rb_left = right;
		else
			parent->rb_right = right;
	}
	/*如果旋转结点原父结点为空，即旋转结点是原红黑树的根结点，则设置旋转结点的原右子
	结点为当前的红黑树根结点*/
	else
		root->rb_node = right;
	/*设置旋转结点的父结点是其原右子结点*/
	rb_set_parent(node, right);
}

/*右旋：以某个结点作为支点（旋转结点），其左子结点变为旋转结点的父结点，左子结点的右
子结点作为旋转结点的左子结点，右子结点保持不变*/
static void __rb_rotate_right(struct rb_node *node, struct rb_root *root)
{
	/*获取旋转结点的左子结点*/
	struct rb_node *left = node->rb_left;
	/*获取旋转结点的父结点*/
	struct rb_node *parent = rb_parent(node);
	/*设置旋转结点的左子结点为其原左子结点的右子结点，如果其原左子结点的右子结点非空，
	则设置其父结点为旋转结点*/
	if ((node->rb_left = left->rb_right))
		rb_set_parent(left->rb_right, node);
	/*设置旋转结点为其原左子结点的右子结点*/
	left->rb_right = node;
	/*设置旋转结点的原左子结点的父结点为其原父结点*/
	rb_set_parent(left, parent);
	/*如果旋转结点的原父结点不是红黑树的根结点，则根据旋转结点与其父结点的关系（是其
	父结点的左或右子结点）来确定旋转后其父结点与其原父结点之间的关系*/
	if (parent)
	{
		/*如果旋转结点之前是其父结点的右子树，则设置旋转后的父结点是其原父结点的右子结
		点，否则，是其左子结点*/
		if (node == parent->rb_right)
			parent->rb_right = left;
		else
			parent->rb_left = left;
	}
	/*如果旋转结点之前是其所在红黑树的根结点，则设置旋转结点的左子树为红黑树的根结点*/
	else
		root->rb_node = left;
	/*设置旋转结点的父结点为其原左子结点*/
	rb_set_parent(node, left);
}

/*将一个结点插入到红黑树中，需要执行哪些步骤呢？首先，将红黑树当作一颗二叉查找树，将
结点插入；然后，将结点着色为红色；最后，通过旋转和重新着色等方法来修正该树，使之重新
成为一颗红黑树。详细描述如下：
第一步: 将红黑树当作一颗二叉查找树，将结点插入。红黑树本身就是一颗二叉查找树，将结点
插入后，该树仍然是一颗二叉查找树。也就意味着，树的键值仍然是有序的。此外，无论是左旋
还是右旋，若旋转之前这棵树是二叉查找树，旋转之后它一定还是二叉查找树。这也就意味着，
任何的旋转和重新着色操作，都不会改变它仍然是一颗二叉查找树的事实。
第二步：将插入的结点着色为"红色"。为什么着色成红色，而不是黑色呢？为什么呢？将插入的
结点着色为红色，不会违背"特性(5)"！少违背一条特性，就意味着我们需要处理的情况越少。接
下来，就要努力的让这棵树满足其它性质即可；满足了的话，它就又是一颗红黑树了。
第三步: 通过一系列的旋转或着色等操作，使之重新成为一颗红黑树。
第二步中，将插入结点着色为"红色"之后，不会违背"特性(5)"。那它到底会违背哪些特性呢？
对于"特性(1)"，显然不会违背了。因为我们已经将它涂成红色了。对于"特性(2)"，显然也不会
违背。在第一步中，我们是将红黑树当作二叉查找树，然后执行的插入操作。而根据二叉查找数
的特点，插入操作不会改变根结点。所以，根结点仍然是黑色。对于"特性(3)"，显然不会违背了
。这里的叶子结点是指的空叶子结点，插入非空结点并不会对它们造成影响。对于"特性(4)"，是
有可能违背的！那接下来，想办法使之"满足特性(4)"，就可以将树重新构造成红黑树了。
根据被插入结点的父结点的情况，可以将"当结点z被着色为红色结点，并插入二叉树"划分为三种
情况来处理：① 情况说明：被插入的结点是根结点。处理方法：直接把此结点涂为黑色。② 情况说
明：被插入的结点的父结点是黑色。处理方法：什么也不需要做。结点被插入后，仍然是红黑树。
③ 情况说明：被插入的结点的父结点是红色。处理方法：那么，该情况与红黑树的“特性(4)”相冲
突。这种情况下，被插入结点是一定存在非空祖父结点的；进一步的讲，被插入结点也一定存在叔
叔结点(即使叔叔结点为空，我们也视之为存在，空结点本身就是黑色结点)。理解这点之后，我们
依据"叔叔结点的情况"，将这种情况进一步划分为3种情况。三种情况处理问题的核心思路都是：
将红色的结点移到根结点；然后，将根结点设为黑色。下面对它们详细进行介绍：
第一种情况：叔叔是红色。现象说明当前结点(即，被插入结点)的父结点是红色，且当前结点的祖
父结点的另一个子结点（叔叔结点）也是红色。处理策略：(01) 将“父结点”设为黑色(02) 将“叔
叔结点”设为黑色(03) 将“祖父结点”设为“红色”(04) 将“祖父结点”设为“当前结点”(红色结点)；
即，之后继续对“当前结点”进行操作。操作解释：“当前结点”和“父结点”都是红色，违背“特性(4)
”。所以，将“父结点”设置“黑色”以解决这个问题。但是，将“父结点”由“红色”变成“黑色”之后，
违背了“特性(5)”：因为，包含“父结点”的分支的黑色结点的总数增加了1。 解决这个问题的办法
是：将“祖父结点”由“黑色”变成红色，同时，将“叔叔结点”由“红色”变成“黑色”。关于这里，说明
几点：第一，为什么“祖父结点”之前是黑色？这个应该很容易想明白，因为在变换操作之前，该树
是红黑树，“父结点”是红色，那么“祖父结点”一定是黑色。 第二，为什么将“祖父结点”由“黑色”
变成红色，同时，将“叔叔结点”由“红色”变成“黑色”；能解决“包含‘父结点’的分支的黑色结点的总
数增加了1”的问题。这个道理也很简单。“包含‘父结点’的分支的黑色结点的总数增加了1” 同时也
意味着 “包含‘祖父结点’的分支的黑色结点的总数增加了1”，既然这样，我们通过将“祖父结点”由
“黑色”变成“红色”以解决“包含‘祖父结点’的分支的黑色结点的总数增加了1”的问题； 但是，这样
处理之后又会引起另一个问题“包含‘叔叔’结点的分支的黑色结点的总数减少了1”，现在我们已知“
叔叔结点”是“红色”，将“叔叔结点”设为“黑色”就能解决这个问题。 所以，将“祖父结点”由“黑色”
变成红色，同时，将“叔叔结点”由“红色”变成“黑色”；就解决了该问题。按照上面的步骤处理之后
：当前结点、父结点、叔叔结点之间都不会违背红黑树特性，但祖父结点却不一定。若此时，祖父
结点是根结点，直接将祖父结点设为“黑色”，那就完全解决这个问题了；若祖父结点不是根结点，
那我们需要将“祖父结点”设为“新的当前结点”，接着对“新的当前结点”进行分析。
第二种情况：叔叔是黑色，且当前结点是右孩子。现象说明：前结点(即，被插入结点)的父结点是
红色，叔叔结点是黑色，且当前结点是其父结点的右孩子。处理策略：(01) 将“父结点”作为“新的
当前结点”。(02) 以“新的当前结点”为支点进行左旋。操作解释：首先，将“父结点”作为“新的当
前结点”；接着，以“新的当前结点”为支点进行左旋。 为了便于理解，我们先说明第(02)步，再说
明第(01)步；为了便于说明，我们设置“父结点”的代号为F(Father)，“当前结点”的代号为S(Son)。
为什么要“以F为支点进行左旋”呢？根据已知条件可知：S是F的右孩子。而之前我们说过，我们处
理红黑树的核心思想：将红色的结点移到根结点；然后，将根结点设为黑色。既然是“将红色的结
点移到根结点”，那就是说要不断的将破坏红黑树特性的红色结点上移(即向根方向移动)。 而S又
是一个右孩子，因此，我们可以通过“左旋”来将S上移！按照上面的步骤(以F为支点进行左旋)处理
之后：若S变成了根结点，那么直接将其设为“黑色”，就完全解决问题了；若S不是根结点，那我们
需要执行步骤(01)，即“将F设为‘新的当前结点’”。那为什么不继续以S为新的当前结点继续处理，
而需要以F为新的当前结点来进行处理呢？这是因为“左旋”之后，F变成了S的“子结点”，即S变成了
F的父结点；而我们处理问题的时候，需要从下至上(由叶到根)方向进行处理；也就是说，必须先
解决“孩子”的问题，再解决“父亲”的问题；所以，我们执行步骤(01)：将“父结点”作为“新的当前
结点”。
第三种情况：叔叔是黑色，且当前结点是左孩子。现象说明：当前结点(即，被插入结点)的父结点
是红色，叔叔结点是黑色，且当前结点是其父结点的左孩子。处理策略：(01) 将“父结点”设为“黑
色”(02) 将“祖父结点”设为“红色”(03) 以“祖父结点”为支点进行右旋。操作解释：为了便于说明，
我们设置“当前结点”为S(Original Son)，“兄弟结点”为B(Brother)，“叔叔结点”为U(Uncle)，“父
结点”为F(Father)，祖父结点为G(Grand-Father)。S和F都是红色，违背了红黑树的“特性(4)”，我
们可以将F由“红色”变为“黑色”，就解决了“违背‘特性(4)’”的问题；但却引起了其它问题：违背特
性(5)，因为将F由红色改为黑色之后，所有经过F的分支的黑色结点的个数增加了1。那我们如何解
决“所有经过F的分支的黑色结点的个数增加了1”的问题呢？ 我们可以通过“将G由黑色变成红色”，
同时“以G为支点进行右旋”来解决。
*/
void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *parent, *gparent;

	while ((parent = rb_parent(node)) && rb_is_red(parent))
	{
		gparent = rb_parent(parent);

		if (parent == gparent->rb_left)
		{
			{
				register struct rb_node *uncle = gparent->rb_right;
				if (uncle && rb_is_red(uncle))
				{
					rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_right == node)
			{
				register struct rb_node *tmp;
				__rb_rotate_left(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			rb_set_black(parent);
			rb_set_red(gparent);
			__rb_rotate_right(gparent, root);
		} else {
			{
				register struct rb_node *uncle = gparent->rb_left;
				if (uncle && rb_is_red(uncle))
				{
					rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_left == node)
			{
				register struct rb_node *tmp;
				__rb_rotate_right(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			rb_set_black(parent);
			rb_set_red(gparent);
			__rb_rotate_left(gparent, root);
		}
	}

	rb_set_black(root->rb_node);
}
EXPORT_SYMBOL(rb_insert_color);

static void __rb_erase_color(struct rb_node *node, struct rb_node *parent,
			     				struct rb_root *root)
{
	struct rb_node *other;

	while ((!node || rb_is_black(node)) && node != root->rb_node)
	{
		if (parent->rb_left == node)
		{
			other = parent->rb_right;
			if (rb_is_red(other))
			{
				rb_set_black(other);
				rb_set_red(parent);
				__rb_rotate_left(parent, root);
				other = parent->rb_right;
			}
			if ((!other->rb_left || rb_is_black(other->rb_left)) &&
			    (!other->rb_right || rb_is_black(other->rb_right)))
			{
				rb_set_red(other);
				node = parent;
				parent = rb_parent(node);
			}
			else
			{
				if (!other->rb_right || rb_is_black(other->rb_right))
				{
					struct rb_node *o_left;
					if ((o_left = other->rb_left))
						rb_set_black(o_left);
					rb_set_red(other);
					__rb_rotate_right(other, root);
					other = parent->rb_right;
				}
				rb_set_color(other, rb_color(parent));
				rb_set_black(parent);
				if (other->rb_right)
					rb_set_black(other->rb_right);
				__rb_rotate_left(parent, root);
				node = root->rb_node;
				break;
			}
		}
		else
		{
			other = parent->rb_left;
			if (rb_is_red(other))
			{
				rb_set_black(other);
				rb_set_red(parent);
				__rb_rotate_right(parent, root);
				other = parent->rb_left;
			}
			if ((!other->rb_left || rb_is_black(other->rb_left)) &&
			    (!other->rb_right || rb_is_black(other->rb_right)))
			{
				rb_set_red(other);
				node = parent;
				parent = rb_parent(node);
			}
			else
			{
				if (!other->rb_left || rb_is_black(other->rb_left))
				{
					register struct rb_node *o_right;
					if ((o_right = other->rb_right))
						rb_set_black(o_right);
					rb_set_red(other);
					__rb_rotate_left(other, root);
					other = parent->rb_left;
				}
				rb_set_color(other, rb_color(parent));
				rb_set_black(parent);
				if (other->rb_left)
					rb_set_black(other->rb_left);
				__rb_rotate_right(parent, root);
				node = root->rb_node;
				break;
			}
		}
	}
	if (node)
		rb_set_black(node);
}
/*删除红黑树中指定结点*/
void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *child, *parent;
	int color;

	/*如果待删结点的左子结点为空，则获取待删结点的右子结点*/
	if (!node->rb_left)
		child = node->rb_right;
	/*如果待删结点的右子结点为空，则获取待删结点的左子结点*/
	else if (!node->rb_right)
		child = node->rb_left;
	else
	{
		/*如果待删结点的左右子结点都不为空后果，则保存待删结点*/
		struct rb_node *old = node, *left;
		/*保存待删结点的右子结点*/
		node = node->rb_right;
		/*如果待删结点的右子结点中存在左子结点，则一直获取并保存左子结点*/
		while ((left = node->rb_left) != NULL)
			node = left;
		/*获取待删结点右子结点的左子树中最左子结点的右子结点*/
		child = node->rb_right;
		/*获取待删结点右子结点的左子树中最左子结点的父结点*/
		parent = rb_parent(node);
		/*获取待删结点右子结点的左子树中最左子树结点的颜色*/
		color = rb_color(node);
		/*如果待删结点右子结点的左子树中最左结点的右子结点存在，则设置其父结点为其原父
		结点的父结点，也即其祖父结点*/
		if (child)
			rb_set_parent(child, parent);
		/*如果待删结点右子结点没有左子结点*/
		if (parent == old)
		{
			/*设置待删结点的右子结点为待删结点右子结点的右子结点*/
			parent->rb_right = child;
			/*保存待删结点的右子结点*/
			parent = node;
		}
		/*如果待删结点的右子结点有左子结点，则设置待删结点右子结点的左子树的最左结点的
		父结点的左子结点为其原子结点的右子结点（右孙子结点）*/
		else
			parent->rb_left = child;
		/*此时的node结点为待删结点的右子结点（待删结点右子结点的左子结点为空时）或者为
		待删结点右子结点的左子树中最左结点（待删结点右子结点的左子结点非空时）。设置
		node结点的父结点指针及颜色、左右结点指针*/
		node->rb_parent_color = old->rb_parent_color;
		node->rb_right = old->rb_right;
		node->rb_left = old->rb_left;
		/*如果待删结点不是其所在红黑树的根结点，则根据待删结点与其父结点之间的关系，设
		置node结点与待删结点之间的关系。if语句块执行完后，mode结点已替换原待删结点*/
		if (rb_parent(old))
		{
			/*待删结点是其父结点的左子结点，则设置node结点为待删结点的左子结点，否则，为
			其右子结点*/
			if (rb_parent(old)->rb_left == old)
				rb_parent(old)->rb_left = node;
			else
				rb_parent(old)->rb_right = node;
		}
		/*如果待删结点是其所在红黑树的根结点，则设置node结点为根结点*/
		else
			root->rb_node = node;
		/*设置待删结点原左子结点的父结点为node结点*/
		rb_set_parent(old->rb_left, node);
		/*如果待删结点的右子结点存在，则设置待删结点右子结点的父结点为mode结点*/
		if (old->rb_right)
			rb_set_parent(old->rb_right, node);
		goto color;
	}
	/*获取待删结点的父结点*/
	parent = rb_parent(node);
	/*获取待删结点的颜色*/
	color = rb_color(node);
	/*如果待删结点有左或右子树，则设置待删结点子树的父结点为其父结点*/
	if (child)
		rb_set_parent(child, parent);
	/*如果待删结点的父结点存在，则设置结点*/
	if (parent)
	{
		/*如果待删结点是其父结点的左子树，则设置其子树为其父结点的左子树，否则，设置
		其子树结点为其父结点的右子树*/
		if (parent->rb_left == node)
			parent->rb_left = child;
		else
			parent->rb_right = child;
	}
	/*如果待删结点是红黑树的根结点，则设置其子树结点为树根结点*/
	else
		root->rb_node = child;

 color:
 	/*如果待删结点的颜色是黑色，则向上至根结点处理当前结点与父结点之间的衍射*/
	if (color == RB_BLACK)
		__rb_erase_color(child, parent, root);
}
EXPORT_SYMBOL(rb_erase);

/*获取已排序的红黑树的第一个结点（左子树的最左侧结点）*/
struct rb_node *rb_first(struct rb_root *root)
{
	struct rb_node	*n;
	/*获取红黑树的根结点*/
	n = root->rb_node;
	/*根结点为空，返回NULL*/
	if (!n)
		return NULL;
	/*获取红黑树左子树的最左侧结点*/
	while (n->rb_left)
		n = n->rb_left;
	return n;
}
EXPORT_SYMBOL(rb_first);
/*获取已排序的红黑树的最后一个结点（右子树的最右侧结点）*/
struct rb_node *rb_last(struct rb_root *root)
{
	struct rb_node	*n;
	/*获取红黑树根结点*/
	n = root->rb_node;
	/*根结点为NULL时返回NULL*/
	if (!n)
		return NULL;
	/*如果结点存在右子树，则一直遍历右子树*/
	while (n->rb_right)
		n = n->rb_right;
	/*返回红黑树中最右侧的结点*/
	return n;
}
EXPORT_SYMBOL(rb_last);

/*获取结点的后一个逻辑结点*/
struct rb_node *rb_next(struct rb_node *node)
{
	struct rb_node *parent;
	/*如果结点是根结点，则返回空*/
	if (rb_parent(node) == node)
		return NULL;

	/*如果结点有右子树，一直向下遍历该结点的左子树，直到左子树为空*/
	if (node->rb_right)
	{
		/*获取结点的右子树*/
		node = node->rb_right;
		/*如果结点存在左子树，则一直遍历结点的左子树，直到左子树为空*/
		while (node->rb_left)
			node=node->rb_left;
		return node;
	}

	/* No right-hand children.  Everything down and left is
	   smaller than us, so any 'next' node must be in the general
	   direction of our parent. Go up the tree; any time the
	   ancestor is a right-hand child of its parent, keep going
	   up. First time it's a left-hand child of its parent, said
	   parent is our 'next' node. */
	/*这种情况下说明结点没有右子树，如果结点是其父结点的右子树，则一直向上遍历，直到
	结点不是父结点的右子树为止，然后返回结点的父结点*/
	while ((parent = rb_parent(node)) && node == parent->rb_right)
		node = parent;

	return parent;
}
EXPORT_SYMBOL(rb_next);

/*获取结点的前一个逻辑结点*/
struct rb_node *rb_prev(struct rb_node *node)
{
	struct rb_node *parent;
	/*如果结点为根结点，则返回NULL*/
	if (rb_parent(node) == node)
		return NULL;

	/* If we have a left-hand child, go down and then right as far
	   as we can. */
	/*如果结点有左子树，则一直遍历左子树的右子树结点*/
	if (node->rb_left)
	{
		/*获取结点的左子树*/
		node = node->rb_left;
		/*如果结点有右子树，则一直遍历右子树，获取最右侧的右子树结点*/
		while (node->rb_right)
			node=node->rb_right;
		return node;
	}

	/*如果结点的父结点存在，且该结点是父结点的左子树，则一直获取结点的父结点*/
	while ((parent = rb_parent(node)) && node == parent->rb_left)
		node = parent;

	return parent;
}
EXPORT_SYMBOL(rb_prev);

/*替换红黑树中结点。new结点替换红黑树root中victim结点*/
void rb_replace_node(struct rb_node *victim, struct rb_node *new, struct rb_root *root)
{
	/*获取结点的父结点*/
	struct rb_node *parent = rb_parent(victim);

	/*如果替换结点父结点存在 Set the surrounding nodes to point to the replacement */
	if (parent)
	{
		/*如果替换结点是其父结点的左子树，则将新结点作为其父结点的左子树结点，否则，
		将新结点作为其父结点的右子树结点*/
		if (victim == parent->rb_left)
			parent->rb_left = new;
		else
			parent->rb_right = new;
	}
	/*替换结点的父结点不存在，说明该结点是根结点，则设置新结点为根结点*/
	else
	{
		root->rb_node = new;
	}

	/*如果替换结点有左子树，则设置其左子树的父结点为新结点*/
	if (victim->rb_left)
		rb_set_parent(victim->rb_left, new);
	/*如果替换结点有右子树，则设置其右子树的父结点是新结点*/
	if (victim->rb_right)
		rb_set_parent(victim->rb_right, new);

	/*设置新结点的（指针和颜色）值为替换结点的值*/
	*new = *victim;
}
EXPORT_SYMBOL(rb_replace_node);
