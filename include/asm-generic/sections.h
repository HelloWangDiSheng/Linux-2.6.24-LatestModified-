#ifndef _ASM_GENERIC_SECTIONS_H_
#define _ASM_GENERIC_SECTIONS_H_

/*内核占据的内存分为几个段，其边界包含在变量中_text和_etext是代码段的起始和结束地址，
包含了编译后的内核代码。数据段位于_etext和_edata之间，保存了大部分内核变量。初始化数
据在内核启动过程结束后不在需要（例如，包含初始化为0的所有静态全局变量的bss段），保存
在最后一段，从_edata到_end。在内核初始化完成后，其中的大部分数据都可以从内存中删除，
给应用程序流出更多空间。这一段内存区划分为更小的子区间，以控制哪些可以删除，哪些不能
删除*/
/*声明代码段起始、终止位置，包含了编译后的内核代码*/
extern char _text[], _stext[], _etext[];
/*声明数据段起始、终止位置，保存了大部分内核变量*/
extern char _data[], _sdata[], _edata[];
/*BSS(Block Started by Symbol)段中通常存放程序中以下符号：未初始化的全局变量和静态局
部变量、初始值为0的全局变量和静态局部变量(依赖于编译器实现)、未定义且初值不为0的符号
(该初值即common block的大小)C语言中，未显式初始化的静态分配变量被初始化为0(算术类型)
或空指针(指针类型)。由于程序加载时，BSS会被操作系统清零，所以未赋初值或初值为0的全局
变量都在BSS中。BSS段仅为未初始化的静态分配变量预留位置，在目标文件中并不占据空间，这
样可减少目标文件体积。但程序运行时需为变量分配内存空间，故目标文件必须记录所有未初始
化的静态分配变量大小总和(通过start_bss和end_bss地址写入机器代码)。当加载器(loader)加
载程序时，将为BSS段分配的内存初始化为0。在嵌入式软件中，进入main()函数之前BSS段被C运
行时系统映射到初始化为全零的内存(效率较高)。     注意，尽管均放置于BSS段，但初值为0的全局
变量是强符号，而未初始化的全局变量是弱符号。若其他地方已定义同名的强符号(初值可能非0
)，则弱符号与之链接时不会引起重定义错误，但运行时的初值可能并非期望值(会被强符号覆盖
)。因此，定义全局变量时，若只有本文件使用，则尽量使用static关键字修饰；否则需要为全
局变量定义赋初值(哪怕0值)，保证该变量为强符号，以便链接时发现变量名冲突，而不是被未
知值覆盖。某些编译器将未初始化的全局变量保存在common段，链接时再将其放入BSS段。在编
译阶段可通过-fno-common选项来禁止将未初始化的全局变量放入common段。*/
/*未初始化数据起始、终止位置*/
extern char __bss_start[], __bss_stop[];
/*".init"数据段的起始、终止地址*/
extern char __init_begin[], __init_end[];
/*".init.text"代码段的起始、终止地址*/
extern char _sinittext[], _einittext[];
/*扩展代码段的起始、终止地址*/
extern char _sextratext[] __attribute__((weak));
extern char _eextratext[] __attribute__((weak));
/*初始化期间所需数据和代码的结束地址*/
extern char _end[];
/*percpu变量的起始、终止地址*/
extern char __per_cpu_start[], __per_cpu_end[];
/*内核自动检测代码段的起始、终止地址*/
extern char __kprobes_text_start[], __kprobes_text_end[];
/*".init.data"数据段的起始、终止地址*/
extern char __initdata_begin[], __initdata_end[];
/*只读数据段的起始、终止地址*/
extern char __start_rodata[], __end_rodata[];

#endif /* _ASM_GENERIC_SECTIONS_H_ */
