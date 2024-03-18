/*���ļ��еİ�������ͨ���������������ں������к�ģ��ѡ�GNU�������ļ���ʽѡ���ǣ�
 -kr -i8 -npsl -pcs*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

/*�����get_option�з������ַ��������������ֵķ�ΧM-N����չ��Ϊ[M, M+1, ..., N]������
����get_options�е���������*/
static int get_range(char **str, int *pint)
{
	int x, inc_counter, upper_range;
	/*��ȡstr����ָ��ָ�����һ�����ִ�*/
	(*str)++;
	/*��strָ����ַ���ת��Ϊʮ������*/
	upper_range = simple_strtol((*str), NULL, 0);
	/*��ȡ����[*pint, upper_range]��Χ��long������������Ŀ*/
	inc_counter = upper_range - *pint;
	/*��pintָ���inc_counter��long������������ݱ���Ϊ*pint++*/
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
/*��һ����ѡ�ַ����н������֣�strΪ��ѡ����pint�����str�н�����long���ͽ��*/
int get_option (char **str, int *pint)
{
	char *cur = *str;

	/*strΪchar���͵Ķ���ָ�룬���strΪ�ջ���ָ�������Ϊ0��
	�򷵻�0�˳�*/
	if (!cur || !(*cur))
		return 0;
	/*������cur����long���ͽ�����ظ�*pint*/
	*pint = simple_strtol (cur, str, 0);
	/*�����ִ���û������*/
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

/*����һ�������ַ���Ϊһ���޷��ų����������֡����ִ��洢��ptr�У�Ǳ�ڵĴ洢��ʽ��
���ִ�����׺%K��Kilobytes����1024bytes����%M(Megabytes����1048576bytes)��%G��
gigabytews����1073741824bytes������������������K��M��GΪ��׺����ô����ֵ�ͱ�ʾ����
����1K��1M��1G*/
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
