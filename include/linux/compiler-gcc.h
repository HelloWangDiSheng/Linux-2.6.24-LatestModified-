#ifndef __LINUX_COMPILER_H
#error "Please don't include <linux/compiler-gcc.h> directly, include <linux/compiler.h> instead."
#endif

/*
 * Common definitions for all gcc versions go here.
 */


/*�Ż�����*/
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

/* This macro obfuscates arithmetic on a variable address so that gcc
   shouldn't recognize the original var, and make assumptions about it */
/*
 * Versions of the ppc64 compiler before 4.1 had a bug where use of
 * RELOC_HIDE could trash r30. The bug can be worked around by changing
 * the inline assembly constraint from =g to =r, in this particular
 * case either is valid.
 */
#define RELOC_HIDE(ptr, off)					\
  ({ unsigned long __ptr;						\
    __asm__ ("" : "=r"(__ptr) : "0"(ptr));		\
    (typeof(ptr)) (__ptr + (off)); })

/* &a[0] degrades to a pointer: a different type from an array */
#define __must_be_array(a) \
  BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

#define inline		inline		__attribute__((always_inline))
#define __inline__	__inline__	__attribute__((always_inline))
#define __inline	__inline	__attribute__((always_inline))
#define __deprecated			__attribute__((deprecated))
/*
������������䣬���磬����һ���ṹ�������ⲿ�豸�������ݣ����밴���ݽṹ��ԭ����
�������ݣ��Ϳ����ڽṹ�Ķ�����ָ��__packed���ԣ���ֹ��������������ֽڡ���ˣ�������
�ṹ�У���Щ����δ����Ĳ���ֻ��ʹ�����к������ʣ�
��1��get_unaligned(ptr)��ȡ�Ƕ���ָ��
��2��put_aligned(val, ptr)��ֵvalд�뵽�Ƕ�����ڴ��ַptr��
�ȽϾɵ���ϵ�ṹ����IA-32������͸���ش���Ƕ�����ʣ��������RISC�������������������
���зǶ�����ʶ�����ʹ���������������Ա�֤����ֲ�ԡ�
�������нṹ��
struct align
{
	char *ptr1;
	char c;
	char *ptr2;
};
��64λϵͳ�ϣ�һ��ָ����Ҫ8���ֽڣ���һ��char���ͱ�����Ҫ1���ֽڡ����ܽṹ��ֻ����17
�ֽ����ݣ���sizeof����ĸó��Ƚ���24��������Ϊ��������Ҫ��֤�ڶ���ָ��ptr2��ȷ���룬
��Ҫ��c֮�����7������ֽڣ���Щ�ֽڲ���ʹ�ã��ڴ��д洢��Ϣ����
��ַ��0��7ƫ�ƴ���ptr1��8��15���洢һ��c��7������ֽڣ�16��23���洢ptr2��
�ṹ�е�����ֽ�Ҳ�������������Ϣ����ˣ�Ӧ�ø��ݶ����Ҫ�����跨���Žṹ�еĸ�����
�ݳ�Ա
*/
#define __packed			__attribute__((packed))
#define __weak				__attribute__((weak))
#define __naked				__attribute__((naked))
#define __noreturn			__attribute__((noreturn))

/*
 * From the GCC manual:
 *
 * Many functions have no effects except the return value and their
 * return value depends only on the parameters and/or global
 * variables.  Such a function can be subject to common subexpression
 * elimination and loop optimization just as an arithmetic operator
 * would be.
 * [...]
 */
#define __pure				__attribute__((pure))

/*alignָ�������ݶ�������Ҫ�󣬼��������ڴ��ж��������λ�ã���������Ҫһ������������
�������ڵ��ڴ��ַ�����ܹ���������ֵ�������䵥λ���ֽڡ������Ժ���Ҫ����Ϊ��ͨ�����ṹ
�Ĺؼ����ַ��õ��ڴ�����ǡ����λ�ã�������޶ȶ�����CPU���ٻ��������*/
#define __aligned(x)			__attribute__((aligned(x)))
#define __printf(a,b)			__attribute__((format(printf,a,b)))
#define  noinline			__attribute__((noinline))
#define __attribute_const__		__attribute__((__const__))
#define __maybe_unused			__attribute__((unused))
