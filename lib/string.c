/*愚蠢的库函数。优化可选版通常作为内联代码出现在<asm-string.h>中*/
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/module.h>

#ifndef __HAVE_ARCH_STRNICMP

/*指定长度忽略大小写比较两个字符串。如果比较长度非零，则进入do-while循环，从两个字符
串开始位置开始，获取字符（字符最好是保存为无符号字符类型），如果任一字符为'/0'字符,则
跳出循环，计算两字符差值作为比较结果。如果两字符相同，则继续循环，否则，将两字符都转换
为小写字符在比较大小，如果不相等则跳出循环计算差值，相等则将比较长度减一，取下一个字符
继续循环，直到比较达到比较长度为止*/
int strnicmp(const char *s1, const char *s2, size_t len)
{
	/*变量最好是无符号的*/
	unsigned char c1, c2;
	/*len为0时，if条件不成立，因此要先初始化*/
	c1 = c2 = 0;
	if (len)
	{
		/*do-while循环直到达到指定比较长度或任一字符串结束或两字符转换后不相等时结束*/
		do
		{
			/*获取两字符串中字符值*/
			c1 = *s1;
			c2 = *s2;
			/*更新比较位置*/
			s1++;
			s2++;
			/*获取的字符中任一个是空字符，就跳出循环*/
			if (!c1)
				break;
			if (!c2)
				break;
			/*如果两比较字符相等，则继续比较下一个字符*/
			if (c1 == c2)
				continue;
			/*因为忽略大小写，不相等则都转换为小写字符，然后在比较*/
			c1 = tolower(c1);
			c2 = tolower(c2);
			/*如果转换后两字符不相等，则跳出循环*/
			if (c1 != c2)
				break;
		} while (--len);/*更新比较长度*/
	}
	/*将两字符的差值作为比较结果*/
	return (int)c1 - (int)c2;
}
EXPORT_SYMBOL(strnicmp);
#endif

#ifndef __HAVE_ARCH_STRCASECMP
/*忽略大小写比较两字符串大小，直到任第一个字符串结束，返回结束时两字符的差值*/
int strcasecmp(const char *s1, const char *s2)
{
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 == c2 && c1 != 0);
	return c1 - c2;
}
EXPORT_SYMBOL(strcasecmp);
#endif

#ifndef __HAVE_ARCH_STRNCASECMP
/*忽略大小写比较指定长度的两字符大小。如果指定长度为0，结果是不对的*/
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	int c1, c2;
	/*
	if(0 == n)
	{
		return -EINVAl;
	}
	*/
	do
	{
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while ((--n > 0) && c1 == c2 && c1 != 0);
	return c1 - c2;
}
EXPORT_SYMBOL(strncasecmp);
#endif

#ifndef __HAVE_ARCH_STRCPY
/*字符串复制，直到源串结束*/
#undef strcpy
char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;
	/*while(*tmp++ = *src++);*/
	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}
EXPORT_SYMBOL(strcpy);
#endif

#ifndef __HAVE_ARCH_STRNCPY
/*指定复制长度的字符串复制。如果源串长度小于复制长度时，后面的更新属于无效操作，如果
源串长度大于复制长度时，目标串的预期结束符位置非空，导致程序运行不正确*/
char *strncpy(char *dest, const char *src, size_t count)
{
	char *tmp = dest;
	/*
	if(!count)
		return NULL;
	while(n-- && *dest++ = *src++);
	*dest = '\0';
	*/
	while (count)
	{
		/*获取源字符串中的字符赋值给目标字符串中字符，然后更新目标字符串中下一个字符
		位置和统计计数，如果源串长度小于复制长度时，后面的更新属于无效操作，如果源串
		长度大于复制长度时，目标串的预期结束符位置非空，导致程序运行不正确*/
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}
EXPORT_SYMBOL(strncpy);
#endif

#ifndef __HAVE_ARCH_STRLCPY
/*复制一个字符串到指定长度的缓冲区中，真实复制的字符数是size-1，最后一个放空字符*/
size_t strlcpy(char *dest, const char *src, size_t size)
{
	/*获取源字符串的长度*/
	size_t ret = strlen(src);
	/*复制长度有效时才复制*/
	if (size)
	{
		/*获取预复制有效字符的数目（不超过缓冲区指定的长度）*/
		size_t len = (ret >= size) ? size - 1 : ret;
		/*复制不会超过缓冲区长度的字符串到缓冲区*/
		memcpy(dest, src, len);
		/*设置目标字符串的结束符*/
		dest[len] = '\0';
	}
	return ret;
}
EXPORT_SYMBOL(strlcpy);
#endif

#ifndef __HAVE_ARCH_STRCAT
/*连接两个字符串*/
#undef strcat
char *strcat(char *dest, const char *src)
{
	char *tmp = dest;
	/*移动到目标串结尾*/
	while (*dest)
		dest++;
	/*复制源串到目标串尾部*/
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}
EXPORT_SYMBOL(strcat);
#endif

#ifndef __HAVE_ARCH_STRNCAT
/*将一字符串连接到另一个字符串的尾部，连接的最大字符数目为指定数目*/
char *strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;
	/*连接长度非零时才有效*/
	if (count)
	{
		/*移动到目标串末尾*/
		while (*dest)
			dest++;
		/*将源串字符逐个复制到目标串末尾*/
		while ((*dest++ = *src++) != 0)
		{
			/*如果到达指定复制字符的数目，则设置目标串结束符并退出*/
			if (--count == 0)
			{
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}
EXPORT_SYMBOL(strncat);
#endif

#ifndef __HAVE_ARCH_STRLCAT
/*连接一个字符串操到另一个指定最大缓冲区长度的字符串的末尾。操作前先计算源和目标字符
串的长度，目标串长度不小于该缓冲区长度是bug，然后将目标串的指针定位到其末尾，计算缓冲
区剩余剩余字符空间，入如果源串长度大于缓冲区剩余空间，则最多复制剩余空间减一（为结束
符保留）个字符到目标串末尾*/
size_t strlcat(char *dest, const char *src, size_t count)
{
	/*获取目标串长度*/
	size_t dsize = strlen(dest);
	/*获取源串长度*/
	size_t len = strlen(src);
	/*获取两字符串合并后的最大长度*/
	size_t res = dsize + len;

	/*count时目标串缓冲区长度，目标串长度不小于缓冲区长度，可能是bug*/
	BUG_ON(dsize >= count);
	/*目标串指针这是为目标串末尾*/
	dest += dsize;
	/*缓冲区中剩余的字符间*/
	count -= dsize;
	/*如果源串长度大于缓冲区剩余空间，则最多向缓冲区复制的字符将缓冲区填满（剩余一个
	空字符空间）*/
	if (len >= count)
		len = count-1;
	memcpy(dest, src, len);
	dest[len] = 0;
	return res;
}
EXPORT_SYMBOL(strlcat);
#endif

#ifndef __HAVE_ARCH_STRCMP
/*比较两字符大小*/
#undef strcmp
int strcmp(const char *cs, const char *ct)
{
	signed char __res;

	while (1)
	{
		/*比较的两字符不相等或任一字符为空字符时结束循环，否则，移动另一个比较指针（
		其中一个指针已经移动至下一个字符），这个条件写的真经典*/
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}
	return __res;
}
EXPORT_SYMBOL(strcmp);
#endif

#ifndef __HAVE_ARCH_STRNCMP
/*比较两字符串中指定长度的大小*/
int strncmp(const char *cs, const char *ct, size_t count)
{
	signed char __res = 0;

	while (count)
	{
		/*源串和目标串中比较字符不相等或任一字符串结束或者到达比较长度时退出循环*/
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}
	return __res;
}
EXPORT_SYMBOL(strncmp);
#endif

#ifndef __HAVE_ARCH_STRCHR
/*返回字符串中第一次出现指定字符的位置*/
char *strchr(const char *s, int c)
{
	/*从字符串开始处查询，如果字符与指定字符不相等，则继续查找下一个字符，相等则退出
	循环，返回字符在字符串中的地址，否则，一直查询至字符串结束，查询到末尾还没有查到
	与指定字符相等的字符时返回空*/
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return NULL;
	return (char *)s;
}
EXPORT_SYMBOL(strchr);
#endif

#ifndef __HAVE_ARCH_STRRCHR

/*从字符串尾部开始查询串中出现指定字符的位置*/
char *strrchr(const char *s, int c)
{
	/*设置指针指向字符串末尾*/
	const char *p = s + strlen(s);
	do
	{
		/*如果比较字符和指针常量内容相等，则返回字符指针，否则，将指针前移，直到字符串
		开始位置*/
		if (*p == (char)c)
        	return (char *)p;
    } while (--p >= s);
    return NULL;
}
EXPORT_SYMBOL(strrchr);
#endif

#ifndef __HAVE_ARCH_STRNCHR
/*从字符串开始处查询指定长度内出现指定指定字符的位置*/
char *strnchr(const char *s, size_t count, int c)
{
	/*从字符串开始处查询指定字符是否和串的比较字符相等，相等则跳出，否则查询下一个
	比较字符，直到指定查询长度或字符串结束时退出循环*/
	for (; count-- && *s != '\0'; ++s)
		if (*s == (char)c)
			return (char *)s;
	return NULL;
}
EXPORT_SYMBOL(strnchr);
#endif

/*将字符串前后的空格符（空格、回车、换行、换页、水平制表、垂直制表）去掉。注意！
尾部逆序最后一个空格被空字符替换，首部起始的空格没变，只是字符串指针后移到第一个
非空格符的位置*/
char *strstrip(char *s)
{
	size_t size;
	char *end;
	/*获取输入串长度*/
	size = strlen(s);
	/*输入串长度为零直接返回源串*/
	if (!size)
		return s;
	/*获得输入串最后一个有效字符地址*/
	end = s + size - 1;
	/*去除尾部空格*/
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';
	/*去除前部空格*/
	while (*s && isspace(*s))
		s++;
	/*返回去除前后空格的字符串*/
	return s;
}
EXPORT_SYMBOL(strstrip);

#ifndef __HAVE_ARCH_STRLEN

/*获取字符串长度。利用一个指针常量，从字符串头部开始向后查找字符串结束标识，找到后
利用指针相减即得到字符串长度*/
size_t strlen(const char *s)
{
	const char *sc;
	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}
EXPORT_SYMBOL(strlen);
#endif

#ifndef __HAVE_ARCH_STRNLEN
/*获取最多指定长度的字符串*/
size_t strnlen(const char *s, size_t count)
{
	const char *sc;
	/*从字符串头部开始向后查找字符串结束标识，直到到达指定长度或字符串结束为止*/
	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}
EXPORT_SYMBOL(strnlen);
#endif

#ifndef __HAVE_ARCH_STRSPN
/**
 * strspn - Calculate the length of the initial substring of @s which only contain letters in @accept
 * @s: The string to be searched
 * @accept: The string to search for
 */
/*顺序查找指定串中的字符只属于accept中的字符个数*/
size_t strspn(const char *s, const char *accept)
{
	const char *p;
	const char *a;
	size_t count = 0;
	/*从指定串头部开始向后查找，直到该字符串结束*/
	for (p = s; *p != '\0'; ++p)
	{
		/*从指定子串开始向后查找，直到字符串结束*/
		for (a = accept; *a != '\0'; ++a) 
		{
			/*如果源串当前字符和指定串当前字符相同，则退出内部循环*/
			if (*p == *a)
				break;
		}
		/*如果子串结束，则返回字串在源串中的索引*/
		if (*a == '\0')
			return count;
		/*从源串下一个位置开始查找字串*/
		++count;
	}
	/*没找到时返回字符串长度加1*/
	return count;
}

EXPORT_SYMBOL(strspn);
#endif

#ifndef __HAVE_ARCH_STRCSPN
/**
 * strcspn - Calculate the length of the initial substring of @s which does not contain letters in @reject
 * @s: The string to be searched
 * @reject: The string to avoid
 */
size_t strcspn(const char *s, const char *reject)
{
	const char *p;
	const char *r;
	size_t count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (r = reject; *r != '\0'; ++r) {
			if (*p == *r)
				return count;
		}
		++count;
	}
	return count;
}
EXPORT_SYMBOL(strcspn);
#endif

#ifndef __HAVE_ARCH_STRPBRK
/*查找ct字符串中任意字符出现在cs字符串中的子串*/
char *strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;
	for (sc1 = cs; *sc1 != '\0'; ++sc1)
	{
		for (sc2 = ct; *sc2 != '\0'; ++sc2)
		{
			/*如果ct串中的符出现在cs中，则返回cs的字串*/
			if (*sc1 == *sc2)
				return (char *)sc1;
		}
	}
	return NULL;
}
EXPORT_SYMBOL(strpbrk);
#endif

#ifndef __HAVE_ARCH_STRSEP
/**
 * strsep - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 *
 * strsep() updates @s to point after the token, ready for the next call.
 *
 * It returns empty tokens, too, behaving exactly like the libc function
 * of that name. In fact, it was stolen from glibc2 and de-fancy-fied.
 * Same semantics, slimmer shape. ;)
 */
char *strsep(char **s, const char *ct)
{
	char *sbegin = *s;
	char *end;

	if (sbegin == NULL)
		return NULL;

	end = strpbrk(sbegin, ct);
	if (end)
		*end++ = '\0';
	*s = end;
	return sbegin;
}
EXPORT_SYMBOL(strsep);
#endif

#ifndef __HAVE_ARCH_MEMSET
/*使用给定值填充一个内存域。将指定字符串s的前count个字符用强制转换c为char型的字符
替换，要使用memset_io()而非memset()函数去访问IO空间，*/
void *memset(void *s, int c, size_t count)
{
	char *xs = s;
	/*将s开头的count个字符用(char)c填充*/
	while (count--)
		*xs++ = c;
	return s;
}
EXPORT_SYMBOL(memset);
#endif

#ifndef __HAVE_ARCH_MEMCPY
/*将一个内存域中的值赋值为另一个内存域中的值。使用memcpy_toio()或者memcpy_fromio()
函数而非该函数去访问IO空间*/
void *memcpy(void *dest, const void *src, size_t count)
{
	/*获取dest首地址*/
	char *tmp = dest;
	/*获取src首地址*/
	const char *s = src;
	/*dest前count个值用src中前count个值填充*/
	while (count--)
		*tmp++ = *s++;
	return dest;
}
EXPORT_SYMBOL(memcpy);
#endif

#ifndef __HAVE_ARCH_MEMMOVE
/*将一个内存域中指定数目的值复制到另一个内存域中。不同于memcpy()，memmove()能复制
重叠区域*/
void *memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;
	/*如果目标区域的首地址地址不大于（等于时都不用复制操作）源区域的首地址，则从头
	开始逐字节复制。由于都是从头开始，地址重叠也可以正确执行。否则，就要从尾部开始
	逐字节复制，从头开始可能有覆盖现象导致错误*/
	if (dest <= src)
	{
		/*获取目标区域首地址*/
		tmp = dest;
		/*获取源区域首地址*/
		s = src;
		/*while-do循环从头开始复制指定数目的内容*/
		while (count--)
			/*从目标区域和源区域的首地址开始，逐字节复制内容*/
			*tmp++ = *s++;
	}
	else
	{
		/*获取目标区域的首地址*/
		tmp = dest;
		/*获取正向顺序复制数目后对应的目标区域地址*/
		tmp += count;
		/*获取源区域首地址*/
		s = src;
		/*获取正向顺序复制操作后对应的源区域地址*/
		s += count;
		/*反向逆序从源区域指定地址开始复制到目标区域地址，保证不会覆盖*/
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}
EXPORT_SYMBOL(memmove);
#endif

#ifndef __HAVE_ARCH_MEMCMP
/*比较两个内存域中的值是否相等*/
#undef memcmp
int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;
	/*两个无符号字符类型常量指针，正向顺序同步从源和目标内存域起始地址开始，比较其
	内容，如果不同则跳出循环，返回两者之间的差值，否则，继续下一个字符比较，直至达
	到预定的比较数目为止*/
	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}
EXPORT_SYMBOL(memcmp);
#endif

#ifndef __HAVE_ARCH_MEMSCAN
/*正向顺序地在一个内存域中查找指定字符第一次出现的地址。没找到时返回该内存域的下一
个字节地址*/
void *memscan(void *addr, int c, size_t size)
{
	unsigned char *p = addr;

	while (size)
	{
		if (*p == c)
			return (void *)p;
		p++;
		size--;
	}
  	return (void *)p;
}
EXPORT_SYMBOL(memscan);
#endif

#ifndef __HAVE_ARCH_STRSTR
/*查找一字符串在另一字符串中出现的位置*/
char *strstr(const char *s1, const char *s2)
{
	int l1, l2;
	/*获取s2长度*/
	l2 = strlen(s2);
	/*s2为空串时直接返回s1首地址*/
	if (!l2)
		return (char *)s1;
	/*获取s1长度*/
	l1 = strlen(s1);
	/*从s1的开始位置比较s2是否是s1的字串，是则返回，否则，将s2与s1的下一个字符开始的
	字串进行比较，直到s1剩余子串的长度小于s2的长度为止*/
	while (l1 >= l2)
	{
		l1--;
		/*如果s1和s2的前s2长度个字符相同，则返回s1的比较位置，否则将s1首地址后挪一位
		，长度减一，循环开始比较，直到s1的剩余长度不大于s2长度为止*/
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		/*从s1的下一个位置开始继续比较*/
		s1++;
	}
	return NULL;
}
EXPORT_SYMBOL(strstr);
#endif

#ifndef __HAVE_ARCH_MEMCHR
/*从特定内存域中查询指定字符出现的地址，没有找到时返回NULL*/
void *memchr(const void *s, int c, size_t n)
{
	/*定位内存域开始位置*/
	const unsigned char *p = s;
	/*最多查找前n个字符*/
	while (n-- != 0)
	{
		/*如果找到指定的字符，则返回该字符出现的位置*/
       	if ((unsigned char)c == *p++)
		{
			return (void *)(p - 1);
		}
	}
	return NULL;
}
EXPORT_SYMBOL(memchr);
#endif
