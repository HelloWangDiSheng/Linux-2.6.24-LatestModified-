#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/sections.h>

/*声明异常处理表的开始和结束地址*/
extern struct exception_table_entry __start___ex_table[];
extern struct exception_table_entry __stop___ex_table[];

/*排序内核内置异常表*/
void __init sort_main_extable(void)
{
	sort_extable(__start___ex_table, __stop___ex_table);
}

/*在内核异常表中查询给定的地址*/
const struct exception_table_entry *search_exception_tables(unsigned long addr)
{
	const struct exception_table_entry *e;
	/*根据给定的地址，二分法查询已排序的异常表*/
	e = search_extable(__start___ex_table, __stop___ex_table-1, addr);
	/*如果内核异常表中没有找到，则查询已加载的模块异常表*/
	if (!e)
		e = search_module_extables(addr);
	return e;
}

/*测试指定地址是否在内核代码段或初始化代码段的起止范围内，是则返回1，否则返回0*/
int core_kernel_text(unsigned long addr)
{
	/*如果指定地址在内核代码段起止范围，则返回1*/
	if (addr >= (unsigned long)_stext && addr <= (unsigned long)_etext)
		return 1;
	/*如果指定地址在初始化代码段的起止范围，则返回1*/
	if (addr >= (unsigned long)_sinittext && addr <= (unsigned long)_einittext)
		return 1;
	return 0;
}

/*如果指定地址在内核代码段、内核初始化代码段、模块初始化代码段或模块核心代码段中，
则返回1，否则返回0*/
int __kernel_text_address(unsigned long addr)
{
	/*如果指定地址在内核代码段或初始化代码段范围内，则返回1*/
	if (core_kernel_text(addr))
		return 1;
	/*如果指定地址在模块初始化代码段或核心代码段中，则返回返回1*/
	return __module_text_address(addr) != NULL;
}

/*如果指定地址在内核代码段、内核初始化代码段、模块初始化代码段或模块核心代码段中，
则返回1，否则返回0*/
int kernel_text_address(unsigned long addr)
{
	if (core_kernel_text(addr))
		return 1;
	return module_text_address(addr) != NULL;
}
