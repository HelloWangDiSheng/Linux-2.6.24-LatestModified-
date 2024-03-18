#ifndef __LINUX_COMPILER_H
#error "Please don't include <linux/compiler-gcc.h> directly, include <linux/compiler.h> instead."
#endif

/*
 * Common definitions for all gcc versions go here.
 */


/*优化屏障*/
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
如果必须避免填充，例如，可能一个结构用于与外部设备交换数据，必须按数据结构的原定义
接收数据，就可以在结构的定义中指定__packed属性，防止编译器加入填充字节。因此，这样的
结构中，那些可能未对齐的部分只能使用下列函数访问：
（1）get_unaligned(ptr)读取非对齐指针
（2）put_aligned(val, ptr)将值val写入到非对齐的内存地址ptr处
比较旧的体系结构（如IA-32）可以透明地处理非对齐访问，但大多数RISC机器不是这样。因而对
所有非对齐访问都必须使用这两个函数，以保证可移植性。
考虑下列结构：
struct align
{
	char *ptr1;
	char c;
	char *ptr2;
};
在64位系统上，一个指针需要8个字节，而一个char类型变量需要1个字节。尽管结构中只保存17
字节数据，但sizeof报告的该长度将是24。这是因为编译器需要保证第二个指针ptr2正确对齐，
需要在c之后放置7个填充字节，这些字节并不使用，内存中存储信息如下
基址处0到7偏移处存ptr1，8到15处存储一个c和7个填充字节，16到23处存储ptr2。
结构中的填充字节也可以填充有用信息，因此，应该根据对齐的要求来设法安排结构中的各个数
据成员
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

/*align指定了数据对齐的最低要求，即数据在内存中对齐的特征位置，该属性需要一个整数参数，
数据所在的内存地址必须能够被该整数值整除，其单位是字节。该属性很重要，因为它通过将结构
的关键部分放置到内存中最恰当的位置，来最大限度读发挥CPU高速缓存的作用*/
#define __aligned(x)			__attribute__((aligned(x)))
#define __printf(a,b)			__attribute__((format(printf,a,b)))
#define  noinline			__attribute__((noinline))
#define __attribute_const__		__attribute__((__const__))
#define __maybe_unused			__attribute__((unused))
