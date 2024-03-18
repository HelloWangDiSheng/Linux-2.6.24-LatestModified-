#ifndef _LINUX_CTYPE_H
#define _LINUX_CTYPE_H

/*ע�⣬����ת���������׼C����������EOF(End Of File��һ������Ϊ-1)*/
/*��8λ��Чλ��ÿһλ����һ������*/
/*��Щ�ַ�Upper*/
#define _U	0x01
/*Сд�ַ�Lower*/
#define _L	0x02
/*�����ַ�Digit*/
#define _D	0x04
/*�����ַ�Control����0��NULL����0x1F��US��֮����ַ��Լ��ַ�0x7F��DEL��*/
#define _C	0x08
/*�ɴ�ӡ�ַ�����0x20�ո�' '����0x7E��'~'��֮����ַ�*/
#define _P	0x10
/*�հ��ַ�White space�������ո�' '���س�'\r'������'\n'����ҳ'\f'��ˮƽ�Ʊ�'\t'��
��ֱ�Ʊ�'\v'*/
#define _S	0x20
/*ʮ�������ַ�*/
#define _X	0x40
/*�ո������ָ0x20*/
#define _SP	0x80

/*����һ����ASCII�����ַ����ձ����չ�ַ����ձ����Ӧ�����ͱ�ʶ���飬������ÿ���ʾΪ
ASCII�ַ���Ӧ������ʾ�����ͱ�ʶ*/
extern unsigned char _ctype[];

/*��ȡASCII����x�ַ����������ͱ�ʶ�����������*/
#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

/*����c�ַ��Ƿ�����ĸ������*/
#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
/*�����ַ�c�Ƿ����ַ�*/
#define isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
/*�����ַ�c�Ƿ��ǿ����ַ�*/
#define iscntrl(c)	((__ismask(c)&(_C)) != 0)
/*�����ַ�c�Ƿ�������*/
#define isdigit(c)	((__ismask(c)&(_D)) != 0)
/*���ո�֮��Ŀɴ�ӡ�ַ�*/
#define isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
/*�����ַ�c�Ƿ���Сд��ĸ*/
#define islower(c)	((__ismask(c)&(_L)) != 0)
/*�����ַ�c�Ƿ��ǰ����ո����ڿɴ�ӡ�ַ�*/
#define isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
/*�����ַ�c�Ƿ��ǳ��ո���ĸ��������Ŀɴ�ӡ�ַ�*/
#define ispunct(c)	((__ismask(c)&(_P)) != 0)
/*�����ַ�c�Ƿ��ǿո񡢻��С���ҳ���س���ˮƽ�Ʊ���ֱ�Ʊ��*/
#define isspace(c)	((__ismask(c)&(_S)) != 0)
/*�����ַ�c�Ƿ��Ǵ�д��ĸ*/
#define isupper(c)	((__ismask(c)&(_U)) != 0)
/*�����ַ�c�Ƿ���ʮ�������ַ�*/
#define isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)
/*�����ַ�c�Ƿ���ascii�ַ���ֵ�Ƿ�С��0x7f��*/
#define isascii(c) (((unsigned char)(c))<=0x7f)
/*���ַ�cת��Ϊascii�ַ���ֻȡ��7λ*/
#define toascii(c) (((unsigned char)(c))&0x7f)

/*����ַ��Ǵ�д��ĸ������ת��ΪСд��ĸ*/
static inline unsigned char __tolower(unsigned char c)
{
	/*'A'=0x41��'a'=0x61������֮�����0x20*/
	if (isupper(c))
		c -= 'A'-'a';
	return c;
}

/*����ַ��Ǵ�д��ĸ������ת��Ϊ��д��ĸ*/
static inline unsigned char __toupper(unsigned char c)
{
	if (islower(c))
		c -= 'a'-'A';
	return c;
}

/*������ת��Ϊ��Сд��ĸ����������ת��Ϊ��Ӧ�ĺ�*/
#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

#endif
