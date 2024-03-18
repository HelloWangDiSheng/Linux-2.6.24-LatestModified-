#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

#include <asm/linkage.h>

/*宏CPP_ASMLINKAGE/asmlinkage/FASTCALL在IA-32以外的所有体系结构上都定义为空串*/

/*CPP_ASMLINKAGE关键字通常扩展为一个空串（仅当使用C++编译器编译内核时才插入extern C
关键字），这通知编译器使用C调用约定（第一个参数最后入栈），而不是C++调用约定（第一个
参数首先入栈）*/
#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif

/*标识函数从汇编代码内被调用。因为在这种情况下参数传递必须必须手工编码（因而编译器
无法访问），与通过栈能够传递的参数数目相比，通过寄存器传递的参数数目显然不会给我们
带来惊讶，这也是通过寄存器传递参数的选项必须显式启用的原因*/
#ifndef asmlinkage
#define asmlinkage CPP_ASMLINKAGE
#endif

#ifndef prevent_tail_call
#define prevent_tail_call(ret) do { } while (0)
#endif

#ifndef __ALIGN
#define __ALIGN		.align 4,0x90
#define __ALIGN_STR	".align 4,0x90"
#endif

#ifdef __ASSEMBLY__

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:
#endif

#ifndef WEAK
/*弱链接 别名*/
#define WEAK(name)	   \
	.weak name;	   \
	name:
#endif

#define KPROBE_ENTRY(name) \
  .pushsection .kprobes.text, "ax"; \
  ENTRY(name)

#define KPROBE_END(name) \
  END(name);		 \
  .popsection

#ifndef END
#define END(name) \
  .size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
  .type name, @function; \
  END(name)
#endif

#endif
/*noreturn属性用于指定被调函数并不返回到调用者。优化将导致较佳的代码（当然因为函数不
返回通常会导致程勋异常终止，代码较佳就无意义）。使用该属性，主要是为了防止针对相应代
码中出现的未初始化变量的编译器警告。在内核中，该关键字适用于触发内核恐慌的函数，或者
在结束后通常会导致关机的函数*/
#define NORET_TYPE
#define ATTRIB_NORET  __attribute__((noreturn))
#define NORET_AND     noreturn,

/*FASTCALL用于快速调用一个函数*/
#ifndef FASTCALL
#define FASTCALL(x)	x
#define fastcall
#endif

#endif
