/*Derived from arch/ppc/mm/extable.c and arch/i386/mm/extable.c*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sort.h>
#include <asm/uaccess.h>

#ifndef ARCH_HAS_SORT_EXTABLE
/*ʹ�ö��ֲ��ҷ�������������쳣���÷����������ں��쳣����Ѽ���ģ���쳣������
��������������쳣���ַ���ж�����ַָ����쳣����ܷ���������쳣�ĵ�ַ֮���λ��
��ϵ*/
static int cmp_ex(const void *a, const void *b)
{
	const struct exception_table_entry *x = a, *y = b;

	/*�������*/
	if (x->insn > y->insn)
		return 1;
	if (x->insn < y->insn)
		return -1;
	return 0;
}

/*���ö������������������쳣��*/
void sort_extable(struct exception_table_entry *start,
			struct exception_table_entry *finish)
{
	sort(start, finish - start, sizeof(struct exception_table_entry), cmp_ex, NULL);
}
#endif

#ifndef ARCH_HAS_SEARCH_EXTABLE
/*���ݸ�����ָ���ַ��ʹ�ö��ַ���ѯ�쳣���ٶ��ñ��Ѿ�������ˣ�����ȡ��Ӧ���쳣��
�д�����쳣��ַ�����ҳɹ��򷵻ض�Ӧ���쳣������ַ�����򷵻�NULL*/
const struct exception_table_entry *search_extable(const struct exception_table_entry *first,
	       const struct exception_table_entry *last,	unsigned long value)
{
	/*while-doѭ���������������Ķ��ַ���ѯ*/
	while (first <= last)
	{
		const struct exception_table_entry *mid;
		/*��ȡ�쳣����ָ����ֹ��ַ���м��ַ*/
		mid = (last - first) / 2 + first;
		/*ע�⣡������ѯ��ľ��������ܴ���2G		 ����ѯ��ַ�����м��ַ�����ѯ��벿��*/
		if (mid->insn < value)
			first = mid + 1;
		/*�����ѯ��ַС���м��ַ�����ѯǰ�벿��*/
		else if (mid->insn > value)
			last = mid - 1;
		/*�ɹ��鵽�õ�ַ��Ӧ���쳣�������Ϣ���򷵻ظ��쳣���ж�Ӧ���쳣�������Ϣ*/
		else
			return mid;
    }
	/*û�ҵ�������NULL*/
    return NULL;
}
#endif
