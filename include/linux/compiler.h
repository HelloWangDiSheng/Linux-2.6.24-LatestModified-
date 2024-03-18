#ifndef __LINUX_COMPILER_H
#define __LINUX_COMPILER_H

#ifndef __ASSEMBLY__

#ifdef __CHECKER__
/*�ں˲��ܼ򵥵������û��ռ��ָ�룬����������ض��ĺ�����ȷ��Ŀ���ڴ����Ѿ��������ڴ��У�Ϊȷ
���ں�����������Լ�����û��ռ�ָ��ͨ��__user���Ա�ǣ���֧��C check tools��Դ������Զ������*/
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

 /*intel������������__GNUC__��������¸���ǰ��compilerͷ�ļ�*/
#ifdef __INTEL_COMPILER
#include <linux/compiler-intel.h>
#endif

/*�����Ż����󲿷������x��ֵ�Ƿ�0��*/
#define likely(x)	__builtin_expect(!!(x), 1)
/*�����Ż����󲿷������x��ֵ��0*/
#define unlikely(x)	__builtin_expect(!!(x), 0)
/**/

#ifndef barrier
/*����һ���Ż����ϣ���ָ���֪��������������cpu�Ĵ����У�������֮ǰ��Ч�������ڴ��ַ��������
֮�󶼽�ʧЧ�������ϣ�����ζ�ű�����������֮ǰ�����Ķ�д�������֮ǰ�����ᴦ������֮����κ�
��д���󣬵�cpu��Ȼ��������ʱ��*/
#define barrier() __memory_barrier()
#endif
/*��ȡptr�������ǵ�ַ��ƫ��off����Ϣ*/
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

/*���Ƕ���Ϊ�����������ؼ���inline�ڱ���ʱ����һ���Ż�������ܳ�Ϊ��������*/
#ifndef __always_inline
#define __always_inline inline
#endif

#endif /* __KERNEL__ */

#ifndef __attribute_const__
# define __attribute_const__	/* unimplemented */
#endif

/*��֪gcc���������ú�������ʹ��*/
#ifndef __cold
#define __cold
#endif

#endif /* __LINUX_COMPILER_H */
