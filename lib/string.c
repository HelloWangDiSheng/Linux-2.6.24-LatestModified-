/*�޴��Ŀ⺯�����Ż���ѡ��ͨ����Ϊ�������������<asm-string.h>��*/
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/module.h>

#ifndef __HAVE_ARCH_STRNICMP

/*ָ�����Ⱥ��Դ�Сд�Ƚ������ַ���������Ƚϳ��ȷ��㣬�����do-whileѭ�����������ַ�
����ʼλ�ÿ�ʼ����ȡ�ַ����ַ�����Ǳ���Ϊ�޷����ַ����ͣ��������һ�ַ�Ϊ'/0'�ַ�,��
����ѭ�����������ַ���ֵ��Ϊ�ȽϽ����������ַ���ͬ�������ѭ�������򣬽����ַ���ת��
ΪСд�ַ��ڱȽϴ�С����������������ѭ�������ֵ������򽫱Ƚϳ��ȼ�һ��ȡ��һ���ַ�
����ѭ����ֱ���Ƚϴﵽ�Ƚϳ���Ϊֹ*/
int strnicmp(const char *s1, const char *s2, size_t len)
{
	/*����������޷��ŵ�*/
	unsigned char c1, c2;
	/*lenΪ0ʱ��if���������������Ҫ�ȳ�ʼ��*/
	c1 = c2 = 0;
	if (len)
	{
		/*do-whileѭ��ֱ���ﵽָ���Ƚϳ��Ȼ���һ�ַ������������ַ�ת�������ʱ����*/
		do
		{
			/*��ȡ���ַ������ַ�ֵ*/
			c1 = *s1;
			c2 = *s2;
			/*���±Ƚ�λ��*/
			s1++;
			s2++;
			/*��ȡ���ַ�����һ���ǿ��ַ���������ѭ��*/
			if (!c1)
				break;
			if (!c2)
				break;
			/*������Ƚ��ַ���ȣ�������Ƚ���һ���ַ�*/
			if (c1 == c2)
				continue;
			/*��Ϊ���Դ�Сд���������ת��ΪСд�ַ���Ȼ���ڱȽ�*/
			c1 = tolower(c1);
			c2 = tolower(c2);
			/*���ת�������ַ�����ȣ�������ѭ��*/
			if (c1 != c2)
				break;
		} while (--len);/*���±Ƚϳ���*/
	}
	/*�����ַ��Ĳ�ֵ��Ϊ�ȽϽ��*/
	return (int)c1 - (int)c2;
}
EXPORT_SYMBOL(strnicmp);
#endif

#ifndef __HAVE_ARCH_STRCASECMP
/*���Դ�Сд�Ƚ����ַ�����С��ֱ���ε�һ���ַ������������ؽ���ʱ���ַ��Ĳ�ֵ*/
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
/*���Դ�Сд�Ƚ�ָ�����ȵ����ַ���С�����ָ������Ϊ0������ǲ��Ե�*/
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
/*�ַ������ƣ�ֱ��Դ������*/
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
/*ָ�����Ƴ��ȵ��ַ������ơ����Դ������С�ڸ��Ƴ���ʱ������ĸ���������Ч���������
Դ�����ȴ��ڸ��Ƴ���ʱ��Ŀ�괮��Ԥ�ڽ�����λ�÷ǿգ����³������в���ȷ*/
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
		/*��ȡԴ�ַ����е��ַ���ֵ��Ŀ���ַ������ַ���Ȼ�����Ŀ���ַ�������һ���ַ�
		λ�ú�ͳ�Ƽ��������Դ������С�ڸ��Ƴ���ʱ������ĸ���������Ч���������Դ��
		���ȴ��ڸ��Ƴ���ʱ��Ŀ�괮��Ԥ�ڽ�����λ�÷ǿգ����³������в���ȷ*/
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
/*����һ���ַ�����ָ�����ȵĻ������У���ʵ���Ƶ��ַ�����size-1�����һ���ſ��ַ�*/
size_t strlcpy(char *dest, const char *src, size_t size)
{
	/*��ȡԴ�ַ����ĳ���*/
	size_t ret = strlen(src);
	/*���Ƴ�����Чʱ�Ÿ���*/
	if (size)
	{
		/*��ȡԤ������Ч�ַ�����Ŀ��������������ָ���ĳ��ȣ�*/
		size_t len = (ret >= size) ? size - 1 : ret;
		/*���Ʋ��ᳬ�����������ȵ��ַ�����������*/
		memcpy(dest, src, len);
		/*����Ŀ���ַ����Ľ�����*/
		dest[len] = '\0';
	}
	return ret;
}
EXPORT_SYMBOL(strlcpy);
#endif

#ifndef __HAVE_ARCH_STRCAT
/*���������ַ���*/
#undef strcat
char *strcat(char *dest, const char *src)
{
	char *tmp = dest;
	/*�ƶ���Ŀ�괮��β*/
	while (*dest)
		dest++;
	/*����Դ����Ŀ�괮β��*/
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}
EXPORT_SYMBOL(strcat);
#endif

#ifndef __HAVE_ARCH_STRNCAT
/*��һ�ַ������ӵ���һ���ַ�����β�������ӵ�����ַ���ĿΪָ����Ŀ*/
char *strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;
	/*���ӳ��ȷ���ʱ����Ч*/
	if (count)
	{
		/*�ƶ���Ŀ�괮ĩβ*/
		while (*dest)
			dest++;
		/*��Դ���ַ�������Ƶ�Ŀ�괮ĩβ*/
		while ((*dest++ = *src++) != 0)
		{
			/*�������ָ�������ַ�����Ŀ��������Ŀ�괮���������˳�*/
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
/*����һ���ַ����ٵ���һ��ָ����󻺳������ȵ��ַ�����ĩβ������ǰ�ȼ���Դ��Ŀ���ַ�
���ĳ��ȣ�Ŀ�괮���Ȳ�С�ڸû�����������bug��Ȼ��Ŀ�괮��ָ�붨λ����ĩβ�����㻺��
��ʣ��ʣ���ַ��ռ䣬�����Դ�����ȴ��ڻ�����ʣ��ռ䣬����ิ��ʣ��ռ��һ��Ϊ����
�����������ַ���Ŀ�괮ĩβ*/
size_t strlcat(char *dest, const char *src, size_t count)
{
	/*��ȡĿ�괮����*/
	size_t dsize = strlen(dest);
	/*��ȡԴ������*/
	size_t len = strlen(src);
	/*��ȡ���ַ����ϲ������󳤶�*/
	size_t res = dsize + len;

	/*countʱĿ�괮���������ȣ�Ŀ�괮���Ȳ�С�ڻ��������ȣ�������bug*/
	BUG_ON(dsize >= count);
	/*Ŀ�괮ָ������ΪĿ�괮ĩβ*/
	dest += dsize;
	/*��������ʣ����ַ���*/
	count -= dsize;
	/*���Դ�����ȴ��ڻ�����ʣ��ռ䣬������򻺳������Ƶ��ַ���������������ʣ��һ��
	���ַ��ռ䣩*/
	if (len >= count)
		len = count-1;
	memcpy(dest, src, len);
	dest[len] = 0;
	return res;
}
EXPORT_SYMBOL(strlcat);
#endif

#ifndef __HAVE_ARCH_STRCMP
/*�Ƚ����ַ���С*/
#undef strcmp
int strcmp(const char *cs, const char *ct)
{
	signed char __res;

	while (1)
	{
		/*�Ƚϵ����ַ�����Ȼ���һ�ַ�Ϊ���ַ�ʱ����ѭ���������ƶ���һ���Ƚ�ָ�루
		����һ��ָ���Ѿ��ƶ�����һ���ַ������������д���澭��*/
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}
	return __res;
}
EXPORT_SYMBOL(strcmp);
#endif

#ifndef __HAVE_ARCH_STRNCMP
/*�Ƚ����ַ�����ָ�����ȵĴ�С*/
int strncmp(const char *cs, const char *ct, size_t count)
{
	signed char __res = 0;

	while (count)
	{
		/*Դ����Ŀ�괮�бȽ��ַ�����Ȼ���һ�ַ����������ߵ���Ƚϳ���ʱ�˳�ѭ��*/
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}
	return __res;
}
EXPORT_SYMBOL(strncmp);
#endif

#ifndef __HAVE_ARCH_STRCHR
/*�����ַ����е�һ�γ���ָ���ַ���λ��*/
char *strchr(const char *s, int c)
{
	/*���ַ�����ʼ����ѯ������ַ���ָ���ַ�����ȣ������������һ���ַ���������˳�
	ѭ���������ַ����ַ����еĵ�ַ������һֱ��ѯ���ַ�����������ѯ��ĩβ��û�в鵽
	��ָ���ַ���ȵ��ַ�ʱ���ؿ�*/
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return NULL;
	return (char *)s;
}
EXPORT_SYMBOL(strchr);
#endif

#ifndef __HAVE_ARCH_STRRCHR

/*���ַ���β����ʼ��ѯ���г���ָ���ַ���λ��*/
char *strrchr(const char *s, int c)
{
	/*����ָ��ָ���ַ���ĩβ*/
	const char *p = s + strlen(s);
	do
	{
		/*����Ƚ��ַ���ָ�볣��������ȣ��򷵻��ַ�ָ�룬���򣬽�ָ��ǰ�ƣ�ֱ���ַ���
		��ʼλ��*/
		if (*p == (char)c)
        	return (char *)p;
    } while (--p >= s);
    return NULL;
}
EXPORT_SYMBOL(strrchr);
#endif

#ifndef __HAVE_ARCH_STRNCHR
/*���ַ�����ʼ����ѯָ�������ڳ���ָ��ָ���ַ���λ��*/
char *strnchr(const char *s, size_t count, int c)
{
	/*���ַ�����ʼ����ѯָ���ַ��Ƿ�ʹ��ıȽ��ַ���ȣ�����������������ѯ��һ��
	�Ƚ��ַ���ֱ��ָ����ѯ���Ȼ��ַ�������ʱ�˳�ѭ��*/
	for (; count-- && *s != '\0'; ++s)
		if (*s == (char)c)
			return (char *)s;
	return NULL;
}
EXPORT_SYMBOL(strnchr);
#endif

/*���ַ���ǰ��Ŀո�����ո񡢻س������С���ҳ��ˮƽ�Ʊ���ֱ�Ʊ�ȥ����ע�⣡
β���������һ���ո񱻿��ַ��滻���ײ���ʼ�Ŀո�û�䣬ֻ���ַ���ָ����Ƶ���һ��
�ǿո����λ��*/
char *strstrip(char *s)
{
	size_t size;
	char *end;
	/*��ȡ���봮����*/
	size = strlen(s);
	/*���봮����Ϊ��ֱ�ӷ���Դ��*/
	if (!size)
		return s;
	/*������봮���һ����Ч�ַ���ַ*/
	end = s + size - 1;
	/*ȥ��β���ո�*/
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';
	/*ȥ��ǰ���ո�*/
	while (*s && isspace(*s))
		s++;
	/*����ȥ��ǰ��ո���ַ���*/
	return s;
}
EXPORT_SYMBOL(strstrip);

#ifndef __HAVE_ARCH_STRLEN

/*��ȡ�ַ������ȡ�����һ��ָ�볣�������ַ���ͷ����ʼ�������ַ���������ʶ���ҵ���
����ָ��������õ��ַ�������*/
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
/*��ȡ���ָ�����ȵ��ַ���*/
size_t strnlen(const char *s, size_t count)
{
	const char *sc;
	/*���ַ���ͷ����ʼ�������ַ���������ʶ��ֱ������ָ�����Ȼ��ַ�������Ϊֹ*/
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
/*˳�����ָ�����е��ַ�ֻ����accept�е��ַ�����*/
size_t strspn(const char *s, const char *accept)
{
	const char *p;
	const char *a;
	size_t count = 0;
	/*��ָ����ͷ����ʼ�����ң�ֱ�����ַ�������*/
	for (p = s; *p != '\0'; ++p)
	{
		/*��ָ���Ӵ���ʼ�����ң�ֱ���ַ�������*/
		for (a = accept; *a != '\0'; ++a) 
		{
			/*���Դ����ǰ�ַ���ָ������ǰ�ַ���ͬ�����˳��ڲ�ѭ��*/
			if (*p == *a)
				break;
		}
		/*����Ӵ��������򷵻��ִ���Դ���е�����*/
		if (*a == '\0')
			return count;
		/*��Դ����һ��λ�ÿ�ʼ�����ִ�*/
		++count;
	}
	/*û�ҵ�ʱ�����ַ������ȼ�1*/
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
/*����ct�ַ����������ַ�������cs�ַ����е��Ӵ�*/
char *strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;
	for (sc1 = cs; *sc1 != '\0'; ++sc1)
	{
		for (sc2 = ct; *sc2 != '\0'; ++sc2)
		{
			/*���ct���еķ�������cs�У��򷵻�cs���ִ�*/
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
/*ʹ�ø���ֵ���һ���ڴ��򡣽�ָ���ַ���s��ǰcount���ַ���ǿ��ת��cΪchar�͵��ַ�
�滻��Ҫʹ��memset_io()����memset()����ȥ����IO�ռ䣬*/
void *memset(void *s, int c, size_t count)
{
	char *xs = s;
	/*��s��ͷ��count���ַ���(char)c���*/
	while (count--)
		*xs++ = c;
	return s;
}
EXPORT_SYMBOL(memset);
#endif

#ifndef __HAVE_ARCH_MEMCPY
/*��һ���ڴ����е�ֵ��ֵΪ��һ���ڴ����е�ֵ��ʹ��memcpy_toio()����memcpy_fromio()
�������Ǹú���ȥ����IO�ռ�*/
void *memcpy(void *dest, const void *src, size_t count)
{
	/*��ȡdest�׵�ַ*/
	char *tmp = dest;
	/*��ȡsrc�׵�ַ*/
	const char *s = src;
	/*destǰcount��ֵ��src��ǰcount��ֵ���*/
	while (count--)
		*tmp++ = *s++;
	return dest;
}
EXPORT_SYMBOL(memcpy);
#endif

#ifndef __HAVE_ARCH_MEMMOVE
/*��һ���ڴ�����ָ����Ŀ��ֵ���Ƶ���һ���ڴ����С���ͬ��memcpy()��memmove()�ܸ���
�ص�����*/
void *memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;
	/*���Ŀ��������׵�ַ��ַ�����ڣ�����ʱ�����ø��Ʋ�����Դ������׵�ַ�����ͷ
	��ʼ���ֽڸ��ơ����ڶ��Ǵ�ͷ��ʼ����ַ�ص�Ҳ������ȷִ�С����򣬾�Ҫ��β����ʼ
	���ֽڸ��ƣ���ͷ��ʼ�����и��������´���*/
	if (dest <= src)
	{
		/*��ȡĿ�������׵�ַ*/
		tmp = dest;
		/*��ȡԴ�����׵�ַ*/
		s = src;
		/*while-doѭ����ͷ��ʼ����ָ����Ŀ������*/
		while (count--)
			/*��Ŀ�������Դ������׵�ַ��ʼ�����ֽڸ�������*/
			*tmp++ = *s++;
	}
	else
	{
		/*��ȡĿ��������׵�ַ*/
		tmp = dest;
		/*��ȡ����˳������Ŀ���Ӧ��Ŀ�������ַ*/
		tmp += count;
		/*��ȡԴ�����׵�ַ*/
		s = src;
		/*��ȡ����˳���Ʋ������Ӧ��Դ�����ַ*/
		s += count;
		/*���������Դ����ָ����ַ��ʼ���Ƶ�Ŀ�������ַ����֤���Ḳ��*/
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}
EXPORT_SYMBOL(memmove);
#endif

#ifndef __HAVE_ARCH_MEMCMP
/*�Ƚ������ڴ����е�ֵ�Ƿ����*/
#undef memcmp
int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;
	/*�����޷����ַ����ͳ���ָ�룬����˳��ͬ����Դ��Ŀ���ڴ�����ʼ��ַ��ʼ���Ƚ���
	���ݣ������ͬ������ѭ������������֮��Ĳ�ֵ�����򣬼�����һ���ַ��Ƚϣ�ֱ����
	��Ԥ���ıȽ���ĿΪֹ*/
	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}
EXPORT_SYMBOL(memcmp);
#endif

#ifndef __HAVE_ARCH_MEMSCAN
/*����˳�����һ���ڴ����в���ָ���ַ���һ�γ��ֵĵ�ַ��û�ҵ�ʱ���ظ��ڴ������һ
���ֽڵ�ַ*/
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
/*����һ�ַ�������һ�ַ����г��ֵ�λ��*/
char *strstr(const char *s1, const char *s2)
{
	int l1, l2;
	/*��ȡs2����*/
	l2 = strlen(s2);
	/*s2Ϊ�մ�ʱֱ�ӷ���s1�׵�ַ*/
	if (!l2)
		return (char *)s1;
	/*��ȡs1����*/
	l1 = strlen(s1);
	/*��s1�Ŀ�ʼλ�ñȽ�s2�Ƿ���s1���ִ������򷵻أ����򣬽�s2��s1����һ���ַ���ʼ��
	�ִ����бȽϣ�ֱ��s1ʣ���Ӵ��ĳ���С��s2�ĳ���Ϊֹ*/
	while (l1 >= l2)
	{
		l1--;
		/*���s1��s2��ǰs2���ȸ��ַ���ͬ���򷵻�s1�ıȽ�λ�ã�����s1�׵�ַ��Ųһλ
		�����ȼ�һ��ѭ����ʼ�Ƚϣ�ֱ��s1��ʣ�೤�Ȳ�����s2����Ϊֹ*/
		if (!memcmp(s1, s2, l2))
			return (char *)s1;
		/*��s1����һ��λ�ÿ�ʼ�����Ƚ�*/
		s1++;
	}
	return NULL;
}
EXPORT_SYMBOL(strstr);
#endif

#ifndef __HAVE_ARCH_MEMCHR
/*���ض��ڴ����в�ѯָ���ַ����ֵĵ�ַ��û���ҵ�ʱ����NULL*/
void *memchr(const void *s, int c, size_t n)
{
	/*��λ�ڴ���ʼλ��*/
	const unsigned char *p = s;
	/*������ǰn���ַ�*/
	while (n-- != 0)
	{
		/*����ҵ�ָ�����ַ����򷵻ظ��ַ����ֵ�λ��*/
       	if ((unsigned char)c == *p++)
		{
			return (void *)(p - 1);
		}
	}
	return NULL;
}
EXPORT_SYMBOL(memchr);
#endif
