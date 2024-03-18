#include <linux/kernel.h>
#include <linux/module.h>

/*体系架构没有实现自己的内核栈转储时提供的默认栈转储函数*/
void dump_stack(void)
{
	printk(KERN_NOTICE	"This architecture does not implement dump_stack()\n");
}

EXPORT_SYMBOL(dump_stack);
