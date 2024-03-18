#ifndef _LINUX_CTYPE_H
#define _LINUX_CTYPE_H

/*注意，类型转换不能像标准C库那样处理EOF(End Of File，一般设置为-1)*/
/*低8位有效位中每一位代表一种类型*/
/*大些字符Upper*/
#define _U	0x01
/*小写字符Lower*/
#define _L	0x02
/*数字字符Digit*/
#define _D	0x04
/*控制字符Control。从0（NULL）到0x1F（US）之间的字符以及字符0x7F（DEL）*/
#define _C	0x08
/*可打印字符。从0x20空格（' '）到0x7E（'~'）之间的字符*/
#define _P	0x10
/*空白字符White space。包含空格' '、回车'\r'、换行'\n'、换页'\f'、水平制表'\t'、
垂直制表'\v'*/
#define _S	0x20
/*十六进制字符*/
#define _X	0x40
/*空格符，特指0x20*/
#define _SP	0x80

/*声明一个与ASCII控制字符对照表和扩展字符对照表相对应的类型标识数组，数组中每项表示为
ASCII字符对应如上所示的类型标识*/
extern unsigned char _ctype[];

/*获取ASCII表中x字符在上述类型标识数组项的类型*/
#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

/*测试c字符是否是字母或数字*/
#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
/*测试字符c是否是字符*/
#define isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
/*测试字符c是否是控制字符*/
#define iscntrl(c)	((__ismask(c)&(_C)) != 0)
/*测试字符c是否是数字*/
#define isdigit(c)	((__ismask(c)&(_D)) != 0)
/*除空格之外的可打印字符*/
#define isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
/*测试字符c是否是小写字母*/
#define islower(c)	((__ismask(c)&(_L)) != 0)
/*测试字符c是否是包含空格在内可打印字符*/
#define isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
/*测试字符c是否是除空格、字母和数字外的可打印字符*/
#define ispunct(c)	((__ismask(c)&(_P)) != 0)
/*测试字符c是否是空格、换行、换页、回车、水平制表、垂直制表符*/
#define isspace(c)	((__ismask(c)&(_S)) != 0)
/*测试字符c是否是大写字母*/
#define isupper(c)	((__ismask(c)&(_U)) != 0)
/*测试字符c是否是十六进制字符*/
#define isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)
/*测试字符c是否是ascii字符（值是否小于0x7f）*/
#define isascii(c) (((unsigned char)(c))<=0x7f)
/*将字符c转换为ascii字符，只取低7位*/
#define toascii(c) (((unsigned char)(c))&0x7f)

/*如果字符是大写字母，则将其转换为小写字母*/
static inline unsigned char __tolower(unsigned char c)
{
	/*'A'=0x41，'a'=0x61，两者之间相差0x20*/
	if (isupper(c))
		c -= 'A'-'a';
	return c;
}

/*如果字符是大写字母，则将其转换为大写字母*/
static inline unsigned char __toupper(unsigned char c)
{
	if (islower(c))
		c -= 'a'-'A';
	return c;
}

/*将上述转换为大小写字母的内联函数转换为对应的宏*/
#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

#endif
