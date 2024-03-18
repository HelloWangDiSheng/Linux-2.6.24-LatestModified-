#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/sections.h>

/*�����쳣�����Ŀ�ʼ�ͽ�����ַ*/
extern struct exception_table_entry __start___ex_table[];
extern struct exception_table_entry __stop___ex_table[];

/*�����ں������쳣��*/
void __init sort_main_extable(void)
{
	sort_extable(__start___ex_table, __stop___ex_table);
}

/*���ں��쳣���в�ѯ�����ĵ�ַ*/
const struct exception_table_entry *search_exception_tables(unsigned long addr)
{
	const struct exception_table_entry *e;
	/*���ݸ����ĵ�ַ�����ַ���ѯ��������쳣��*/
	e = search_extable(__start___ex_table, __stop___ex_table-1, addr);
	/*����ں��쳣����û���ҵ������ѯ�Ѽ��ص�ģ���쳣��*/
	if (!e)
		e = search_module_extables(addr);
	return e;
}

/*����ָ����ַ�Ƿ����ں˴���λ��ʼ������ε���ֹ��Χ�ڣ����򷵻�1�����򷵻�0*/
int core_kernel_text(unsigned long addr)
{
	/*���ָ����ַ���ں˴������ֹ��Χ���򷵻�1*/
	if (addr >= (unsigned long)_stext && addr <= (unsigned long)_etext)
		return 1;
	/*���ָ����ַ�ڳ�ʼ������ε���ֹ��Χ���򷵻�1*/
	if (addr >= (unsigned long)_sinittext && addr <= (unsigned long)_einittext)
		return 1;
	return 0;
}

/*���ָ����ַ���ں˴���Ρ��ں˳�ʼ������Ρ�ģ���ʼ������λ�ģ����Ĵ�����У�
�򷵻�1�����򷵻�0*/
int __kernel_text_address(unsigned long addr)
{
	/*���ָ����ַ���ں˴���λ��ʼ������η�Χ�ڣ��򷵻�1*/
	if (core_kernel_text(addr))
		return 1;
	/*���ָ����ַ��ģ���ʼ������λ���Ĵ�����У��򷵻ط���1*/
	return __module_text_address(addr) != NULL;
}

/*���ָ����ַ���ں˴���Ρ��ں˳�ʼ������Ρ�ģ���ʼ������λ�ģ����Ĵ�����У�
�򷵻�1�����򷵻�0*/
int kernel_text_address(unsigned long addr)
{
	if (core_kernel_text(addr))
		return 1;
	return module_text_address(addr) != NULL;
}
