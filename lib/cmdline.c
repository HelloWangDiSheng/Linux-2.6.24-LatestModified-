/*该文件中的帮助函数通常被用于来解析内核命令行和模块选项。GNU缩进的文件格式选项是：
 -kr -i8 -npsl -pcs*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

/*如果在get_option中发现连字符，它将处理数字的范围M-N，将展开为[M, M+1, ..., N]并插入
到在get_options中的整形数组*/
static int get_range(char **str, int *pint)
{
	int x, inc_counter, upper_range;
	/*获取str二级指针指向的下一个数字串*/
	(*str)++;
	/*将str指向的字符串转换为十进制数*/
	upper_range = simple_strtol((*str), NULL, 0);
	/*获取区间[*pint, upper_range]范围内long类型数据项数目*/
	inc_counter = upper_range - *pint;
	/*将pint指向的inc_counter个long型数组项的内容保存为*pint++*/
	for (x = *pint; x < upper_range; x++)
		*pint++ = x;
	return inc_counter;
}

/**
 *	get_option - Parse integer from an option string
 *	@str: option string
 *	@pint: (output) integer value parsed from @str
 *
 *	Read an int from an option string; if available accept a subsequent
 *	comma as well.
 *
 *	Return values:
 *	0 - no int in string
 *	1 - int found, no subsequent comma
 *	2 - int found including a subsequent comma
 *	3 - hyphen found to denote a range
 */
/*从一个可选字符串中解析数字，str为可选串，pint保存从str中解析的long类型结果*/
int get_option (char **str, int *pint)
{
	char *cur = *str;

	/*str为char类型的二级指针，如果str为空或者指向的数字为0，
	则返回0退出*/
	if (!cur || !(*cur))
		return 0;
	/*将解析cur串的long类型结果返回给*pint*/
	*pint = simple_strtol (cur, str, 0);
	/*解析字串中没有数字*/
	if (cur == *str)
		return 0;
	/**/
	if (**str == ',')
	{
		(*str)++;
		return 2;
	}
	/**/
	if (**str == '-')
		return 3;

	return 1;
}

/**
 *	get_options - Parse a string into a list of integers
 *	@str: String to be parsed
 *	@nints: size of integer array
 *	@ints: integer array
 *
 *	This function parses a string containing a comma-separated
 *	list of integers, a hyphen-separated range of _positive_ integers,
 *	or a combination of both.  The parse halts when the array is
 *	full, or when no more numbers can be retrieved from the
 *	string.
 *
 *	Return value is the character in the string which caused
 *	the parse to end (typically a null terminator, if @str is
 *	completely parseable).
 */
 
char *get_options(const char *str, int nints, int *ints)
{
	int res, i = 1;

	while (i < nints) {
		res = get_option ((char **)&str, ints + i);
		if (res == 0)
			break;
		if (res == 3) {
			int range_nums;
			range_nums = get_range((char **)&str, ints + i);
			if (range_nums < 0)
				break;
			/*
			 * Decrement the result by one to leave out the
			 * last number in the range.  The next iteration
			 * will handle the upper number in the range
			 */
			i += (range_nums - 1);
		}
		i++;
		if (res == 1)
			break;
	}
	ints[0] = i - 1;
	return (char *)str;
}

/*解析一个数字字符串为一个无符号长长整形数字。数字串存储在ptr中，潜在的存储格式是
数字串带后缀%K（Kilobytes或者1024bytes）、%M(Megabytes或者1048576bytes)、%G（
gigabytews或者1073741824bytes）。如果这个数字是以K、M、G为后缀，那么返回值就表示数字
乘以1K、1M、1G*/
unsigned long long memparse (char *ptr, char **retptr)
{	
	unsigned long long ret = simple_strtoull (ptr, retptr, 0);

	switch (**retptr)
	{
		case 'G':
		case 'g':
			ret <<= 10;
		case 'M':
		case 'm':
			ret <<= 10;
		case 'K':
		case 'k':
			ret <<= 10;
			(*retptr)++;
		default:
			break;
	}
	return ret;
}


EXPORT_SYMBOL(memparse);
EXPORT_SYMBOL(get_option);
EXPORT_SYMBOL(get_options);
