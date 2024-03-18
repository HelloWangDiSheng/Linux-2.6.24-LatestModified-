/*Derived from arch/ppc/mm/extable.c and arch/i386/mm/extable.c*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sort.h>
#include <asm/uaccess.h>

#ifndef ARCH_HAS_SORT_EXTABLE
/*使用二分查找法查找已排序的异常表，该方法适用于内核异常表或已加载模块异常表。函数
根据输入的两个异常表地址，判断两地址指向的异常表可能发生错误或异常的地址之间的位置
关系*/
static int cmp_ex(const void *a, const void *b)
{
	const struct exception_table_entry *x = a, *y = b;

	/*避免溢出*/
	if (x->insn > y->insn)
		return 1;
	if (x->insn < y->insn)
		return -1;
	return 0;
}

/*启用堆排序来按正序排序异常表*/
void sort_extable(struct exception_table_entry *start,
			struct exception_table_entry *finish)
{
	sort(start, finish - start, sizeof(struct exception_table_entry), cmp_ex, NULL);
}
#endif

#ifndef ARCH_HAS_SEARCH_EXTABLE
/*根据给定的指令地址，使用二分法查询异常表（假定该表已经排序过了），获取对应的异常表
中错误或异常地址。查找成功则返回对应的异常或错误地址，否则返回NULL*/
const struct exception_table_entry *search_extable(const struct exception_table_entry *first,
	       const struct exception_table_entry *last,	unsigned long value)
{
	/*while-do循环中启用已排序表的二分法查询*/
	while (first <= last)
	{
		const struct exception_table_entry *mid;
		/*获取异常表中指定起止地址的中间地址*/
		mid = (last - first) / 2 + first;
		/*注意！两个查询点的距离间隔可能大于2G		 ，查询地址大于中间地址，则查询后半部分*/
		if (mid->insn < value)
			first = mid + 1;
		/*如果查询地址小于中间地址，则查询前半部分*/
		else if (mid->insn > value)
			last = mid - 1;
		/*成功查到该地址对应的异常或错误信息，则返回该异常表中对应的异常或错误信息*/
		else
			return mid;
    }
	/*没找到，返回NULL*/
    return NULL;
}
#endif
