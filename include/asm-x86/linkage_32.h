#ifndef __ASM_LINKAGE_H
#define __ASM_LINKAGE_H

/*CPP_ASMLINKAGE关键字通常扩展为一个空串（仅当使用C++编译器编译内核时才插入extern C
关键字），这通知编译器使用C调用约定（第一个参数最后入栈），而不是C++调用约定（第一个
参数首先入栈）*/
/*标识为asmlinkage函数从汇编代码内被调用。因为在这种情况下参数传递必须必须手工编码
（因而编译器无法访问），与通过栈能够传递的参数数目相比，通过寄存器传递的参数数目显然
不会给我们带来惊讶，这也是通过寄存器传递参数的选项必须显式启用的原因*/
#define asmlinkage CPP_ASMLINKAGE __attribute__((regparm(0)))
#define FASTCALL(x)	x __attribute__((regparm(3)))
#define fastcall	__attribute__((regparm(3)))
/*上述宏在IA-32以外的体系结构上都定义为空串*/

#define prevent_tail_call(ret) __asm__ ("" : "=r" (ret) : "0" (ret))

#ifdef CONFIG_X86_ALIGNMENT_16
#define __ALIGN .align 16,0x90
#define __ALIGN_STR ".align 16,0x90"
#endif

#endif
