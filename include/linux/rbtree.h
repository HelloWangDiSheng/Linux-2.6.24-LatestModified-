#ifndef	_LINUX_RBTREE_H
#define	_LINUX_RBTREE_H

#include <linux/kernel.h>
#include <linux/stddef.h>

/*һ���Ҫ����������еĽڵ㰴��һ����˳�����У��������ǽ��в��롢ɾ��������ʱЧ�ʾ�
��ǳ��ߣ����ǰ��������������������������߶�������������ľ��嶨���������ģ���������
����Ҫô�Ǹ�������Ҫô�������¼���������1.������������ڵĻ������������нڵ�ļ�ֵ��
ҪС�ڸ��ڵ�ļ�ֵ��2.������������ڵĻ������������нڵ�ļ�ֵ��Ҫ���ڸ��ڵ�ļ�ֵ��
3.������������Ҳ��Ҫ����ǰ�������������������������֮�󣬶������ͱ���˶�����������
���Ĳ��롢ɾ��������Ч��һ������¶���O(logn)��
������ǶԲ��Ȳ����Ľڵ��ֵ���Ͳ��롢ɾ���߼����һЩ�����Ҫ��ʹ���ܴﵽ����ƽ��
��Ч�������ǾͰ�����������B����B��ȫ��Balance Tree����һ����ƽ���������͵Ȳ�������
��ͬ���ȱ����ڴ洢�ṹ�ϣ��Ȳ�����ÿ���ڵ�ļ�ֵ���ͷֲ���������ͬ�ģ���B�����ǡ���
��ĳ��B�������нڵ�ķֲ������ֵ��m��������B������m��B����B���ľ��嶨��:
��1�����нڵ������m���ӽڵ㣨2���Ǹ���Ҷ�ӽڵ�������m/2(����ȡ��)���ӽڵ㣨3�����ڵ�
�����������ӽڵ�(�����ܽ����������3��)��4������Ҷ�ӽڵ㶼��ͬһ�㣨5������ڵ������
k����ֵ������k+1���ӽڵ�ָ�룬��ֵҪ���մ�С�������У��ӽڵ��������еļ�ֵ��Ҫ�ڶ�Ӧ��
������ֵ֮�䡣

*/
/*������ֳ�RBTree��Red-Black Tree��������һ������Ķ�����������������ÿ���ڵ��϶���
�洢λ��ʾ�ڵ����ɫ�������Ǻ��ڡ��������������������ԣ�
1��ÿ���ڵ�����Ǻ�ɫ�������Ǻ�ɫ��2�����ڵ��Ǻ�ɫ��3��ÿ��Ҷ�ӽ�㣨NIL�������Ҷ�ӽ�
�㲻�Ǵ�ͳ��Ҷ�ӽ�㣬��ָΪ�յ�Ҷ�ӽ�㣩�Ǻ�ɫ��4�����һ������Ǻ�ɫ�ģ��������ӽ�
������Ǻ�ɫ�ġ�5����һ����㵽�ý��������������·���ϰ�����ͬ��Ŀ�ĺڽ�㡣
��������5������ȷ��û��һ��·���������·��������������ˣ����������Խӽ�ƽ��Ķ���
������Ϊ����������롢ɾ���Ͳ���ĳ��ֵ������ʱ�䶼Ҫ�������ĸ߶ȳɱ���������ڸ߶�
�ϵ������������������������¶��Ǹ�Ч�ģ�����ͬ����ͨ�Ķ����������Ϊʲô˵�����
��һ��·���������·�����������أ�ע�⵽����4������·�����������������ĺ�ɫ�ڵ���㹻��
����̵Ŀ���·�����Ǻ�ɫ�ڵ㣬��Ŀ���·���н���ĺ�ɫ�ͺ�ɫ�ڵ㡣��Ϊ��������5������
����·��������ͬ��Ŀ�ĺ�ɫ�ڵ㣬��ͱ�����û��·���ܶ����κ�����·������������
�������Ӧ�ñȽϹ㷺����Ҫ���������洢��������ݣ�����ʱ�临�Ӷ���O(lgn)��Ч�ʷǳ�֮��
һ�ú���n���ڵ�ĺ�����ĸ߶�����Ϊ2log(n+1)
*/

/*��������*/
struct rb_node
{
	/*�����ָ�루���ֽڶ��룩�뱾�����ɫ�����λ�����*/
	unsigned long  rb_parent_color;
	/*��ɫ*/
#define	RB_RED		0
	/*��ɫ*/
#define	RB_BLACK	1
	/*���������*/
	struct rb_node *rb_right;
	/*���������*/
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));/*�ṹ�����ֽڶ���*/
/*����������*/
struct rb_root
{	/*����������ָ��*/
	struct rb_node *rb_node;
};

/*��ȡ�����ָ�룬��Ϊ���λΪ��ɫ��ֱ��������Ͷ�λ����*/
#define rb_parent(r) 		((struct rb_node *)((r)->rb_parent_color & ~3))
/*��ȡ�����ɫ*/
#define rb_color(r)			((r)->rb_parent_color & 1)
/*����Ƿ�Ϊ��ɫ*/
#define rb_is_red(r)   		(!rb_color(r))
/*����Ƿ�Ϊ��ɫ*/
#define rb_is_black(r) 		rb_color(r)
/*���ý��Ϊ��ɫ*/
#define rb_set_red(r)  		do { (r)->rb_parent_color &= ~1; } while (0)
/*���ý��Ϊ��ɫ*/
#define rb_set_black(r)		do { (r)->rb_parent_color |= 1; } while (0)

/*���ý�㸸���*/
static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->rb_parent_color = (rb->rb_parent_color & 3) | (unsigned long)p;
}

/*���ø������ɫ*/
static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->rb_parent_color = (rb->rb_parent_color & ~1) | color;
}

/*��ʼ�������*/
#define RB_ROOT	(struct rb_root) { NULL, }
/*���ݽṹ���г�Աmember��ָ���ȡ�ýṹ�������ָ��*/
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)
/*������Ƿ�Ϊ��*/
#define RB_EMPTY_ROOT(root)	((root)->rb_node == NULL)
/*����Ƿ�Ϊ�����*/
#define RB_EMPTY_NODE(node)	(rb_parent(node) == node)
/*���ý��Ϊ�����*/
#define RB_CLEAR_NODE(node)	(rb_set_parent(node, node))

extern void rb_insert_color(struct rb_node *, struct rb_root *);
extern void rb_erase(struct rb_node *, struct rb_root *);

/*���в�ѯǰ���߼����*/
extern struct rb_node *rb_next(struct rb_node *);
extern struct rb_node *rb_prev(struct rb_node *);
extern struct rb_node *rb_first(struct rb_root *);
extern struct rb_node *rb_last(struct rb_root *);

/*����ɾ��/ƽ��/���/��ƽ������滻һ�����*/
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new,
			    				struct rb_root *root);

/*��node��Ϊparent���ӽ����ӵ�����*/
static inline void rb_link_node(struct rb_node * node, struct rb_node * parent,
									struct rb_node ** rb_link)
{
	/*����node�ĸ��ڵ�Ϊparent*/
	node->rb_parent_color = (unsigned long )parent;
	/*node��������Ϊ��*/
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#endif	/* _LINUX_RBTREE_H */
