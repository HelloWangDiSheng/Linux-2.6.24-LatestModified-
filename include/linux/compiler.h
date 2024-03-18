#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

#ifndef __ASSEMBLY__

#ifdef __CHECKER__
/*内核不能简单地引用用户空间的指针，而必须采用特定的函数，确保目标内存区已经在物理内存中，为确
保内核遵守了这种约定，用户空间指针通过__user属性标记，以支持C check tools对源代码的自动化检查*/
#define __user		__attribute__((noderef, address_space(1)))
#define __kernel	/* default address space */
#define __safe		__attribute__((safe))
#define __force	__attribute__((force))
#define __nocast	__attribute__((nocast))
#define __iomem	__attribute__((noderef, address_space(2)))
#define __acquires(x)	__attribute__((context(x,0,1)))
#define __releases(x)	__attribute__((context(x,1,0)))
#define __acquire(x)	__context__(x,1)
#define __release(x)	__context__(x,-1)
#define __cond_lock(x,c)	((c) ? ({ __acquire(x); 1; }) : 0)
extern void __chk_user_ptr(const volatile void __user *);
extern void __chk_io_ptr(const volatile void __iomem *);
#else
#define __user
#define __kernel
#define __safe
#define __force
#define __nocast
#define __iomem
#define __chk_user_ptr(x) (void)0
#define __chk_io_ptr(x) (void)0
#define __builtin_warning(x, y...) (1)
#define __acquires(x)
#define __releases(x)
#define __acquire(x) (void)0
#define __release(x) (void)0
#define __cond_lock(x,c) (c)
#endif

#ifdef __KERNEL__

#if __GNUC__ >= 4
#include <linux/compiler-gcc4.h>
#elif __GNUC__ == 3 && __GNUC_MINOR__ >= 2
#include <linux/compiler-gcc3.h>
#else
# error Sorry, your compiler is too old/not recognized.
#endif

 /*intel编译器定义了__GNUC__，因此重新覆盖前述compiler头文件*/
#ifdef __INTEL_COMPILER
#include <linux/compiler-intel.h>
#endif

/*编译优化，大部分情况下x的值是非0的*/
#define likely(x)	__builtin_expect(!!(x), 1)
/*编译优化，大部分情况下x的值是0*/
#define unlikely(x)	__builtin_expect(!!(x), 0)
/**/

#ifndef barrier
/*插入一个优化屏障，该指令告知编译器，保存在cpu寄存器中，在屏障之前有效的所有内存地址，在屏障
之后都将失效。本质上，这意味着编译器在屏障之前发出的读写请求完成之前，不会处理屏障之后的任何
读写请求，但cpu仍然可以重排时序*/
#define barrier() __memory_barrier()
#endif
/*获取ptr（可以是地址）偏移off的信息*/
#ifndef RELOC_HIDE
#define RELOC_HIDE(ptr, off)					\
  ({ unsigned long __ptr;					\
     __ptr = (unsigned long) (ptr);				\
    (typeof(ptr)) (__ptr + (off)); })
#endif

#endif /* __KERNEL__ */

#endif /* __ASSEMBLY__ */

#ifdef __KERNEL__

#ifndef __deprecated
#define __deprecated
#endif

#ifdef MODULE
#define __deprecated_for_modules __deprecated
#else
#define __deprecated_for_modules
#endif

#ifndef __must_check
#define __must_check
#endif

#ifndef CONFIG_ENABLE_MUST_CHECK
#undef __must_check
#define __must_check
#endif
#ifndef CONFIG_ENABLE_WARN_DEPRECATED
#undef __deprecated
#undef __deprecated_for_modules
#define __deprecated
#define __deprecated_for_modules
#endif

#ifndef __attribute_used__
#define __attribute_used__	/* deprecated */
#endif

#ifndef __used
#define __used			/* unimplemented */
#endif

#ifndef __maybe_unused
#define __maybe_unused		/* unimplemented */
#endif

#ifndef noinline
#define noinline
#endif

/*总是定义为内联函数，关键字inline在编译时启用一定优化级别才能成为内联函数*/
#ifndef __always_inline
#define __always_inline inline
#endif

#endif /* __KERNEL__ */

#ifndef __attribute_const__
# define __attribute_const__	/* unimplemented */
#endif

/*告知gcc编译器，该函数很少使用*/
#ifndef __cold
#define __cold
#endif

#endif /* __LINUX_COMPILER_H */
