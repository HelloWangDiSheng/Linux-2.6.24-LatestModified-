#include <linux/rbtree.h>
#include <linux/module.h>
/*��������ĳ�������Ϊ֧�㣨��ת��㣩�������ӽ���λ��ת���ĸ���㣬���ӽ�����
�ӽ���Ϊ��ת�������ӽ�㣬���ӽ�㱣�ֲ���*/
static void __rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
	/*��ȡ��ת�������ӽ��*/
	struct rb_node *right = node->rb_right;
	/*��ȡ��ת���ĸ����*/
	struct rb_node *parent = rb_parent(node);
	/*��ת�������ӽ�㸳ֵΪ�����ӽ������ӽ�㣬�����ǿգ���������ת������ӽ��
	�����ӽ��ĸ����Ϊ��ת���*/
	if ((node->rb_right = right->rb_left))
		rb_set_parent(right->rb_left, node);
	/*������ת������ӽ��Ϊ�丸���*/
	right->rb_left = node;
	/*������ת�����游���Ϊ��ԭ�����*/
	rb_set_parent(right, parent);
	/*�����ת���ԭ�����ǿգ������������丸�����ԭ�����֮ǰ�Ĺ�ϵ*/
	if (parent)
	{
		/*�����ת���֮ǰ���丸�������ӽ�㣬�������丸���Ϊ��ԭ���������ӽ�㣬
		���������丸���Ϊ��ԭ���������ӽ��*/
		if (node == parent->rb_left)
			parent->rb_left = right;
		else
			parent->rb_right = right;
	}
	/*�����ת���ԭ�����Ϊ�գ�����ת�����ԭ������ĸ���㣬��������ת����ԭ����
	���Ϊ��ǰ�ĺ���������*/
	else
		root->rb_node = right;
	/*������ת���ĸ��������ԭ���ӽ��*/
	rb_set_parent(node, right);
}

/*��������ĳ�������Ϊ֧�㣨��ת��㣩�������ӽ���Ϊ��ת���ĸ���㣬���ӽ�����
�ӽ����Ϊ��ת�������ӽ�㣬���ӽ�㱣�ֲ���*/
static void __rb_rotate_right(struct rb_node *node, struct rb_root *root)
{
	/*��ȡ��ת�������ӽ��*/
	struct rb_node *left = node->rb_left;
	/*��ȡ��ת���ĸ����*/
	struct rb_node *parent = rb_parent(node);
	/*������ת�������ӽ��Ϊ��ԭ���ӽ������ӽ�㣬�����ԭ���ӽ������ӽ��ǿգ�
	�������丸���Ϊ��ת���*/
	if ((node->rb_left = left->rb_right))
		rb_set_parent(left->rb_right, node);
	/*������ת���Ϊ��ԭ���ӽ������ӽ��*/
	left->rb_right = node;
	/*������ת����ԭ���ӽ��ĸ����Ϊ��ԭ�����*/
	rb_set_parent(left, parent);
	/*�����ת����ԭ����㲻�Ǻ�����ĸ���㣬�������ת������丸���Ĺ�ϵ������
	������������ӽ�㣩��ȷ����ת���丸�������ԭ�����֮��Ĺ�ϵ*/
	if (parent)
	{
		/*�����ת���֮ǰ���丸��������������������ת��ĸ��������ԭ���������ӽ�
		�㣬�����������ӽ��*/
		if (node == parent->rb_right)
			parent->rb_right = left;
		else
			parent->rb_left = left;
	}
	/*�����ת���֮ǰ�������ں�����ĸ���㣬��������ת����������Ϊ������ĸ����*/
	else
		root->rb_node = left;
	/*������ת���ĸ����Ϊ��ԭ���ӽ��*/
	rb_set_parent(node, left);
}

/*��һ�������뵽������У���Ҫִ����Щ�����أ����ȣ������������һ�Ŷ������������
�����룻Ȼ�󣬽������ɫΪ��ɫ�����ͨ����ת��������ɫ�ȷ���������������ʹ֮����
��Ϊһ�ź��������ϸ�������£�
��һ��: �����������һ�Ŷ�����������������롣������������һ�Ŷ���������������
����󣬸�����Ȼ��һ�Ŷ����������Ҳ����ζ�ţ����ļ�ֵ��Ȼ������ġ����⣬����������
��������������ת֮ǰ������Ƕ������������ת֮����һ�����Ƕ������������Ҳ����ζ�ţ�
�κε���ת��������ɫ������������ı�����Ȼ��һ�Ŷ������������ʵ��
�ڶ�����������Ľ����ɫΪ"��ɫ"��Ϊʲô��ɫ�ɺ�ɫ�������Ǻ�ɫ�أ�Ϊʲô�أ��������
�����ɫΪ��ɫ������Υ��"����(5)"����Υ��һ�����ԣ�����ζ��������Ҫ��������Խ�١���
��������ҪŬ����������������������ʼ��ɣ������˵Ļ�����������һ�ź�����ˡ�
������: ͨ��һϵ�е���ת����ɫ�Ȳ�����ʹ֮���³�Ϊһ�ź������
�ڶ����У�����������ɫΪ"��ɫ"֮�󣬲���Υ��"����(5)"���������׻�Υ����Щ�����أ�
����"����(1)"����Ȼ����Υ���ˡ���Ϊ�����Ѿ�����Ϳ�ɺ�ɫ�ˡ�����"����(2)"����ȻҲ����
Υ�����ڵ�һ���У������ǽ���������������������Ȼ��ִ�еĲ�������������ݶ��������
���ص㣬�����������ı����㡣���ԣ��������Ȼ�Ǻ�ɫ������"����(3)"����Ȼ����Υ����
�������Ҷ�ӽ����ָ�Ŀ�Ҷ�ӽ�㣬����ǿս�㲢������������Ӱ�졣����"����(4)"����
�п���Υ���ģ��ǽ���������취ʹ֮"��������(4)"���Ϳ��Խ������¹���ɺ�����ˡ�
���ݱ�������ĸ�������������Խ�"�����z����ɫΪ��ɫ��㣬�����������"����Ϊ����
����������� ���˵����������Ľ���Ǹ���㡣��������ֱ�ӰѴ˽��ͿΪ��ɫ���� ���˵
����������Ľ��ĸ�����Ǻ�ɫ����������ʲôҲ����Ҫ������㱻�������Ȼ�Ǻ������
�� ���˵����������Ľ��ĸ�����Ǻ�ɫ������������ô��������������ġ�����(4)�����
ͻ����������£�����������һ�����ڷǿ��游���ģ���һ���Ľ�����������Ҳһ��������
����(��ʹ������Ϊ�գ�����Ҳ��֮Ϊ���ڣ��ս�㱾����Ǻ�ɫ���)��������֮������
����"����������"�������������һ������Ϊ3����������������������ĺ���˼·���ǣ�
����ɫ�Ľ���Ƶ�����㣻Ȼ�󣬽��������Ϊ��ɫ�������������ϸ���н��ܣ�
��һ������������Ǻ�ɫ������˵����ǰ���(������������)�ĸ�����Ǻ�ɫ���ҵ�ǰ������
��������һ���ӽ�㣨�����㣩Ҳ�Ǻ�ɫ��������ԣ�(01) ��������㡱��Ϊ��ɫ(02) ������
���㡱��Ϊ��ɫ(03) �����游��㡱��Ϊ����ɫ��(04) �����游��㡱��Ϊ����ǰ��㡱(��ɫ���)��
����֮������ԡ���ǰ��㡱���в������������ͣ�����ǰ��㡱�͡�����㡱���Ǻ�ɫ��Υ��������(4)
�������ԣ���������㡱���á���ɫ���Խ��������⡣���ǣ���������㡱�ɡ���ɫ����ɡ���ɫ��֮��
Υ���ˡ�����(5)������Ϊ������������㡱�ķ�֧�ĺ�ɫ��������������1�� ����������İ취
�ǣ������游��㡱�ɡ���ɫ����ɺ�ɫ��ͬʱ�����������㡱�ɡ���ɫ����ɡ���ɫ�����������˵��
���㣺��һ��Ϊʲô���游��㡱֮ǰ�Ǻ�ɫ�����Ӧ�ú����������ף���Ϊ�ڱ任����֮ǰ������
�Ǻ������������㡱�Ǻ�ɫ����ô���游��㡱һ���Ǻ�ɫ�� �ڶ���Ϊʲô�����游��㡱�ɡ���ɫ��
��ɺ�ɫ��ͬʱ�����������㡱�ɡ���ɫ����ɡ���ɫ�����ܽ��������������㡯�ķ�֧�ĺ�ɫ������
��������1�������⡣�������Ҳ�ܼ򵥡�������������㡯�ķ�֧�ĺ�ɫ��������������1�� ͬʱҲ
��ζ�� ���������游��㡯�ķ�֧�ĺ�ɫ��������������1������Ȼ����������ͨ�������游��㡱��
����ɫ����ɡ���ɫ���Խ�����������游��㡯�ķ�֧�ĺ�ɫ��������������1�������⣻ ���ǣ�����
����֮���ֻ�������һ�����⡰���������塯���ķ�֧�ĺ�ɫ��������������1��������������֪��
�����㡱�ǡ���ɫ�������������㡱��Ϊ����ɫ�����ܽ��������⡣ ���ԣ������游��㡱�ɡ���ɫ��
��ɺ�ɫ��ͬʱ�����������㡱�ɡ���ɫ����ɡ���ɫ�����ͽ���˸����⡣��������Ĳ��账��֮��
����ǰ��㡢����㡢������֮�䶼����Υ����������ԣ����游���ȴ��һ��������ʱ���游
����Ǹ���㣬ֱ�ӽ��游�����Ϊ����ɫ�����Ǿ���ȫ�����������ˣ����游��㲻�Ǹ���㣬
��������Ҫ�����游��㡱��Ϊ���µĵ�ǰ��㡱�����Ŷԡ��µĵ�ǰ��㡱���з�����
�ڶ�������������Ǻ�ɫ���ҵ�ǰ������Һ��ӡ�����˵����ǰ���(������������)�ĸ������
��ɫ���������Ǻ�ɫ���ҵ�ǰ������丸�����Һ��ӡ�������ԣ�(01) ��������㡱��Ϊ���µ�
��ǰ��㡱��(02) �ԡ��µĵ�ǰ��㡱Ϊ֧������������������ͣ����ȣ���������㡱��Ϊ���µĵ�
ǰ��㡱�����ţ��ԡ��µĵ�ǰ��㡱Ϊ֧����������� Ϊ�˱�����⣬������˵����(02)������˵
����(01)����Ϊ�˱���˵�����������á�����㡱�Ĵ���ΪF(Father)������ǰ��㡱�Ĵ���ΪS(Son)��
ΪʲôҪ����FΪ֧������������أ�������֪������֪��S��F���Һ��ӡ���֮ǰ����˵�������Ǵ�
�������ĺ���˼�룺����ɫ�Ľ���Ƶ�����㣻Ȼ�󣬽��������Ϊ��ɫ����Ȼ�ǡ�����ɫ�Ľ�
���Ƶ�����㡱���Ǿ���˵Ҫ���ϵĽ��ƻ���������Եĺ�ɫ�������(����������ƶ�)�� ��S��
��һ���Һ��ӣ���ˣ����ǿ���ͨ��������������S���ƣ���������Ĳ���(��FΪ֧���������)����
֮����S����˸���㣬��ôֱ�ӽ�����Ϊ����ɫ��������ȫ��������ˣ���S���Ǹ���㣬������
��Ҫִ�в���(01)��������F��Ϊ���µĵ�ǰ��㡯������Ϊʲô��������SΪ�µĵ�ǰ����������
����Ҫ��FΪ�µĵ�ǰ��������д����أ�������Ϊ��������֮��F�����S�ġ��ӽ�㡱����S�����
F�ĸ���㣻�����Ǵ��������ʱ����Ҫ��������(��Ҷ����)������д���Ҳ����˵��������
��������ӡ������⣬�ٽ�������ס������⣻���ԣ�����ִ�в���(01)����������㡱��Ϊ���µĵ�ǰ
��㡱��
����������������Ǻ�ɫ���ҵ�ǰ��������ӡ�����˵������ǰ���(������������)�ĸ����
�Ǻ�ɫ���������Ǻ�ɫ���ҵ�ǰ������丸�������ӡ�������ԣ�(01) ��������㡱��Ϊ����
ɫ��(02) �����游��㡱��Ϊ����ɫ��(03) �ԡ��游��㡱Ϊ֧������������������ͣ�Ϊ�˱���˵����
�������á���ǰ��㡱ΪS(Original Son)�����ֵܽ�㡱ΪB(Brother)���������㡱ΪU(Uncle)������
��㡱ΪF(Father)���游���ΪG(Grand-Father)��S��F���Ǻ�ɫ��Υ���˺�����ġ�����(4)������
�ǿ��Խ�F�ɡ���ɫ����Ϊ����ɫ�����ͽ���ˡ�Υ��������(4)���������⣻��ȴ�������������⣺Υ����
��(5)����Ϊ��F�ɺ�ɫ��Ϊ��ɫ֮�����о���F�ķ�֧�ĺ�ɫ���ĸ���������1����������ν�
�������о���F�ķ�֧�ĺ�ɫ���ĸ���������1���������أ� ���ǿ���ͨ������G�ɺ�ɫ��ɺ�ɫ����
ͬʱ����GΪ֧������������������
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
/*ɾ���������ָ�����*/
void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *child, *parent;
	int color;

	/*�����ɾ�������ӽ��Ϊ�գ����ȡ��ɾ�������ӽ��*/
	if (!node->rb_left)
		child = node->rb_right;
	/*�����ɾ�������ӽ��Ϊ�գ����ȡ��ɾ�������ӽ��*/
	else if (!node->rb_right)
		child = node->rb_left;
	else
	{
		/*�����ɾ���������ӽ�㶼��Ϊ�պ�����򱣴��ɾ���*/
		struct rb_node *old = node, *left;
		/*�����ɾ�������ӽ��*/
		node = node->rb_right;
		/*�����ɾ�������ӽ���д������ӽ�㣬��һֱ��ȡ���������ӽ��*/
		while ((left = node->rb_left) != NULL)
			node = left;
		/*��ȡ��ɾ������ӽ����������������ӽ������ӽ��*/
		child = node->rb_right;
		/*��ȡ��ɾ������ӽ����������������ӽ��ĸ����*/
		parent = rb_parent(node);
		/*��ȡ��ɾ������ӽ�������������������������ɫ*/
		color = rb_color(node);
		/*�����ɾ������ӽ���������������������ӽ����ڣ��������丸���Ϊ��ԭ��
		���ĸ���㣬Ҳ�����游���*/
		if (child)
			rb_set_parent(child, parent);
		/*�����ɾ������ӽ��û�����ӽ��*/
		if (parent == old)
		{
			/*���ô�ɾ�������ӽ��Ϊ��ɾ������ӽ������ӽ��*/
			parent->rb_right = child;
			/*�����ɾ�������ӽ��*/
			parent = node;
		}
		/*�����ɾ�������ӽ�������ӽ�㣬�����ô�ɾ������ӽ������������������
		���������ӽ��Ϊ��ԭ�ӽ������ӽ�㣨�����ӽ�㣩*/
		else
			parent->rb_left = child;
		/*��ʱ��node���Ϊ��ɾ�������ӽ�㣨��ɾ������ӽ������ӽ��Ϊ��ʱ������Ϊ
		��ɾ������ӽ����������������㣨��ɾ������ӽ������ӽ��ǿ�ʱ��������
		node���ĸ����ָ�뼰��ɫ�����ҽ��ָ��*/
		node->rb_parent_color = old->rb_parent_color;
		node->rb_right = old->rb_right;
		node->rb_left = old->rb_left;
		/*�����ɾ��㲻�������ں�����ĸ���㣬����ݴ�ɾ������丸���֮��Ĺ�ϵ����
		��node������ɾ���֮��Ĺ�ϵ��if����ִ�����mode������滻ԭ��ɾ���*/
		if (rb_parent(old))
		{
			/*��ɾ������丸�������ӽ�㣬������node���Ϊ��ɾ�������ӽ�㣬����Ϊ
			�����ӽ��*/
			if (rb_parent(old)->rb_left == old)
				rb_parent(old)->rb_left = node;
			else
				rb_parent(old)->rb_right = node;
		}
		/*�����ɾ����������ں�����ĸ���㣬������node���Ϊ�����*/
		else
			root->rb_node = node;
		/*���ô�ɾ���ԭ���ӽ��ĸ����Ϊnode���*/
		rb_set_parent(old->rb_left, node);
		/*�����ɾ�������ӽ����ڣ������ô�ɾ������ӽ��ĸ����Ϊmode���*/
		if (old->rb_right)
			rb_set_parent(old->rb_right, node);
		goto color;
	}
	/*��ȡ��ɾ���ĸ����*/
	parent = rb_parent(node);
	/*��ȡ��ɾ������ɫ*/
	color = rb_color(node);
	/*�����ɾ���������������������ô�ɾ��������ĸ����Ϊ�丸���*/
	if (child)
		rb_set_parent(child, parent);
	/*�����ɾ���ĸ������ڣ������ý��*/
	if (parent)
	{
		/*�����ɾ������丸������������������������Ϊ�丸��������������������
		���������Ϊ�丸����������*/
		if (parent->rb_left == node)
			parent->rb_left = child;
		else
			parent->rb_right = child;
	}
	/*�����ɾ����Ǻ�����ĸ���㣬���������������Ϊ�������*/
	else
		root->rb_node = child;

 color:
 	/*�����ɾ������ɫ�Ǻ�ɫ��������������㴦��ǰ����븸���֮�������*/
	if (color == RB_BLACK)
		__rb_erase_color(child, parent, root);
}
EXPORT_SYMBOL(rb_erase);

/*��ȡ������ĺ�����ĵ�һ����㣨��������������㣩*/
struct rb_node *rb_first(struct rb_root *root)
{
	struct rb_node	*n;
	/*��ȡ������ĸ����*/
	n = root->rb_node;
	/*�����Ϊ�գ�����NULL*/
	if (!n)
		return NULL;
	/*��ȡ��������������������*/
	while (n->rb_left)
		n = n->rb_left;
	return n;
}
EXPORT_SYMBOL(rb_first);
/*��ȡ������ĺ���������һ����㣨�����������Ҳ��㣩*/
struct rb_node *rb_last(struct rb_root *root)
{
	struct rb_node	*n;
	/*��ȡ����������*/
	n = root->rb_node;
	/*�����ΪNULLʱ����NULL*/
	if (!n)
		return NULL;
	/*�������������������һֱ����������*/
	while (n->rb_right)
		n = n->rb_right;
	/*���غ���������Ҳ�Ľ��*/
	return n;
}
EXPORT_SYMBOL(rb_last);

/*��ȡ���ĺ�һ���߼����*/
struct rb_node *rb_next(struct rb_node *node)
{
	struct rb_node *parent;
	/*�������Ǹ���㣬�򷵻ؿ�*/
	if (rb_parent(node) == node)
		return NULL;

	/*����������������һֱ���±����ý�����������ֱ��������Ϊ��*/
	if (node->rb_right)
	{
		/*��ȡ����������*/
		node = node->rb_right;
		/*�������������������һֱ����������������ֱ��������Ϊ��*/
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
	/*���������˵�����û�������������������丸��������������һֱ���ϱ�����ֱ��
	��㲻�Ǹ�����������Ϊֹ��Ȼ�󷵻ؽ��ĸ����*/
	while ((parent = rb_parent(node)) && node == parent->rb_right)
		node = parent;

	return parent;
}
EXPORT_SYMBOL(rb_next);

/*��ȡ����ǰһ���߼����*/
struct rb_node *rb_prev(struct rb_node *node)
{
	struct rb_node *parent;
	/*������Ϊ����㣬�򷵻�NULL*/
	if (rb_parent(node) == node)
		return NULL;

	/* If we have a left-hand child, go down and then right as far
	   as we can. */
	/*������������������һֱ���������������������*/
	if (node->rb_left)
	{
		/*��ȡ����������*/
		node = node->rb_left;
		/*������������������һֱ��������������ȡ���Ҳ�����������*/
		while (node->rb_right)
			node=node->rb_right;
		return node;
	}

	/*������ĸ������ڣ��Ҹý���Ǹ���������������һֱ��ȡ���ĸ����*/
	while ((parent = rb_parent(node)) && node == parent->rb_left)
		node = parent;

	return parent;
}
EXPORT_SYMBOL(rb_prev);

/*�滻������н�㡣new����滻�����root��victim���*/
void rb_replace_node(struct rb_node *victim, struct rb_node *new, struct rb_root *root)
{
	/*��ȡ���ĸ����*/
	struct rb_node *parent = rb_parent(victim);

	/*����滻��㸸������ Set the surrounding nodes to point to the replacement */
	if (parent)
	{
		/*����滻������丸���������������½����Ϊ�丸������������㣬����
		���½����Ϊ�丸�������������*/
		if (victim == parent->rb_left)
			parent->rb_left = new;
		else
			parent->rb_right = new;
	}
	/*�滻���ĸ���㲻���ڣ�˵���ý���Ǹ���㣬�������½��Ϊ�����*/
	else
	{
		root->rb_node = new;
	}

	/*����滻����������������������������ĸ����Ϊ�½��*/
	if (victim->rb_left)
		rb_set_parent(victim->rb_left, new);
	/*����滻����������������������������ĸ�������½��*/
	if (victim->rb_right)
		rb_set_parent(victim->rb_right, new);

	/*�����½��ģ�ָ�����ɫ��ֵΪ�滻����ֵ*/
	*new = *victim;
}
EXPORT_SYMBOL(rb_replace_node);
