#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#ifdef __KERNEL__

#include <stdarg.h>
#include <linux/linkage.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/bitops.h>
#include <linux/log2.h>
#include <asm/byteorder.h>
#include <asm/bug.h>

extern const char linux_banner[];
extern const char linux_proc_banner[];
/*int变量值上限，除了符号位全是1*/
#define INT_MAX			((int)(~0U>>1))
/*int变量取值下限，两者溢出时INT_MAX+1==INT_MIN INT_MIN-1==INT_MAX*/
#define INT_MIN			(-INT_MAX - 1)
/*unsigned int变量取值上限*/
#define UINT_MAX		(~0U)
/*long类型变量取值上限*/
#define LONG_MAX		((long)(~0UL>>1))
/*long类型变量取值下限*/
#define LONG_MIN		(-LONG_MAX - 1)
/*unsigned long变量取值上限*/
#define ULONG_MAX		(~0UL)
/*long long类型变量取值上限*/
#define LLONG_MAX		((long long)(~0ULL>>1))
/*long long类型变量取值下限*/
#define LLONG_MIN		(-LLONG_MAX - 1)
/*unsigned long long类型变量取值上限*/
#define ULLONG_MAX		(~0ULL)

#define STACK_MAGIC		0xdeadbeef
/*x对齐到mask*/
#define __ALIGN_MASK(x, mask)		(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)					__ALIGN_MASK(x, (typeof(x))(a)-1)

/*指针p对齐到a*/
#define PTR_ALIGN(p, a)			((typeof(p))ALIGN((unsigned long)(p), (a)))
/*x是否已对齐到a*/
#define IS_ALIGNED(x,a)			(((x) % ((typeof(x))(a))) == 0)
/*arr数组中数组项数目*/
#define ARRAY_SIZE(arr)			(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
/*获得结构体t中成员f所占内存空间大小*/
#define FIELD_SIZEOF(t, f)		(sizeof(((t*)0)->f))
/*按对齐要求向上对齐，如DIV_ROUND_UP(18，4)=5*/
#define DIV_ROUND_UP(n,d) 		(((n) + (d) - 1) / (d))
/*计算x向上对齐y后的y倍*/
#define roundup(x, y)	 		((((x) + ((y) - 1)) / (y)) * (y))

#ifdef CONFIG_LBD
#include <asm/div64.h>
#define sector_div(a, b) 		do_div(a, b)
#else
#define sector_div(n, b)	\
( 							\
	{ 						\
		int _res; 			\
		_res = (n) % (b); \
		(n) /= (b); 		\
		_res; 				\
	} 						\
)
#endif

/*返回32至63比特的数值，右移位数大于类型宽度时警告*/
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
/*系统不可用，*/
#define	KERN_EMERG			"<0>"
/*需要立即采取处理*/
#define	KERN_ALERT			"<1>"
/*关键信息，一般用来显示硬件和软件错误*/
#define	KERN_CRIT			"<2>"
/*错误信息*/
#define	KERN_ERR			"<3>"
/*警告信息*/
#define	KERN_WARNING		"<4>"
/*需要引起注意的信息*/
#define	KERN_NOTICE			"<5>"
/*一般信息*/
#define	KERN_INFO			"<6>"
/*调试信息*/
#define	KERN_DEBUG			"<7>"

/*
 * Annotation for a "continued" line of log printout (only done after a
 * line that had no enclosing \n). Only to be used by core/arch code
 * during early bootup (a continued line is not SMP-safe otherwise).
 */
#define	KERN_CONT		""

extern int console_printk[];

#define console_loglevel (console_printk[0])
#define default_message_loglevel (console_printk[1])
#define minimum_console_loglevel (console_printk[2])
#define default_console_loglevel (console_printk[3])

struct completion;
struct pt_regs;
struct user;

/*might_sleep函数表明该函数可能会进入睡眠状态。如果执行一个原子上下文（如自旋锁，中断请求
 例程）时，该宏将打印栈追踪信息，这对调试很有用，它可以及早捕获预期非睡眠函数进入睡眠问题*/
#ifdef CONFIG_PREEMPT_VOLUNTARY
extern int cond_resched(void);
#define might_resched() cond_resched()
#else
#define might_resched() do { } while (0)
#endif

#ifdef CONFIG_DEBUG_SPINLOCK_SLEEP
void __might_sleep(char *file, int line);
#define might_sleep() do { __might_sleep(__FILE__, __LINE__); might_resched(); } while (0)
#else
#define might_sleep() do { might_resched(); } while (0)
#endif
#define might_sleep_if(cond) do { if (cond) might_sleep(); } while (0)

/*取绝对值*/
#define abs(x) ({ int __x = (x);	(__x < 0) ? -__x : __x;	})

extern struct atomic_notifier_head panic_notifier_list;
extern long (*panic_blink)(long time);
NORET_TYPE void panic(const char * fmt, ...) __attribute__ ((NORET_AND format (printf, 1, 2))) __cold;
extern void oops_enter(void);
extern void oops_exit(void);
extern int oops_may_print(void);
fastcall NORET_TYPE void do_exit(long error_code) ATTRIB_NORET;
NORET_TYPE void complete_and_exit(struct completion *, long)	ATTRIB_NORET;
extern unsigned long simple_strtoul(const char *,char **,unsigned int);
extern long simple_strtol(const char *,char **,unsigned int);
extern unsigned long long simple_strtoull(const char *,char **,unsigned int);
extern long long simple_strtoll(const char *,char **,unsigned int);
extern int sprintf(char * buf, const char * fmt, ...)	__attribute__ ((format (printf, 2, 3)));
extern int vsprintf(char *buf, const char *, va_list)	__attribute__ ((format (printf, 2, 0)));
extern int snprintf(char * buf, size_t size, const char * fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
	__attribute__ ((format (printf, 3, 0)));
extern int scnprintf(char * buf, size_t size, const char * fmt, ...)
	__attribute__ ((format (printf, 3, 4)));
extern int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
	__attribute__ ((format (printf, 3, 0)));
extern char *kasprintf(gfp_t gfp, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
extern char *kvasprintf(gfp_t gfp, const char *fmt, va_list args);

extern int sscanf(const char *, const char *, ...)	__attribute__ ((format (scanf, 2, 3)));
extern int vsscanf(const char *, const char *, va_list)	__attribute__ ((format (scanf, 2, 0)));

extern int get_option(char **str, int *pint);
extern char *get_options(const char *str, int nints, int *ints);
extern unsigned long long memparse(char *ptr, char **retptr);

extern int core_kernel_text(unsigned long addr);
extern int __kernel_text_address(unsigned long addr);
extern int kernel_text_address(unsigned long addr);
struct pid;
extern struct pid *session_of_pgrp(struct pid *pgrp);

extern void dump_thread(struct pt_regs *regs, struct user *dump);

#ifdef CONFIG_PRINTK
asmlinkage int vprintk(const char *fmt, va_list args) __attribute__ ((format (printf, 1, 0)));
asmlinkage int printk(const char * fmt, ...)	__attribute__ ((format (printf, 1, 2))) __cold;
extern int log_buf_get_len(void);
extern int log_buf_read(int idx);
extern int log_buf_copy(char *dest, int idx, int len);
#else
static inline int vprintk(const char *s, va_list args)	__attribute__ ((format (printf, 1, 0)));
static inline int vprintk(const char *s, va_list args) { return 0; }
static inline int printk(const char *s, ...)	__attribute__ ((format (printf, 1, 2)));
static inline int __cold printk(const char *s, ...) { return 0; }
static inline int log_buf_get_len(void) { return 0; }
static inline int log_buf_read(int idx) { return 0; }
static inline int log_buf_copy(char *dest, int idx, int len) { return 0; }
#endif

unsigned long int_sqrt(unsigned long);

extern int printk_ratelimit(void);
extern int __printk_ratelimit(int ratelimit_jiffies, int ratelimit_burst);
extern bool printk_timed_ratelimit(unsigned long *caller_jiffies, unsigned int interval_msec);

static inline void console_silent(void)
{
	console_loglevel = 0;
}

static inline void console_verbose(void)
{
	if (console_loglevel)
		console_loglevel = 15;
}

extern void bust_spinlocks(int yes);
extern void wake_up_klogd(void);
extern int oops_in_progress;		/* If set, an oops, panic(), BUG() or die() is in progress */
extern int panic_timeout;
extern int panic_on_oops;
extern int panic_on_unrecovered_nmi;
extern int tainted;
extern const char *print_tainted(void);
extern void add_taint(unsigned);

/*系统状态*/
extern enum system_states
{
	/*启动*/
	SYSTEM_BOOTING,
	/*运行*/
	SYSTEM_RUNNING,
	/*挂起*/
	SYSTEM_HALT,
	/*关机*/
	SYSTEM_POWER_OFF,
	/*重启*/
	SYSTEM_RESTART,
	/*暂停硬盘*/
	SYSTEM_SUSPEND_DISK,
} system_state;

/*模块的许可证时专有的或不兼容GPL*/
#define TAINT_PROPRIETARY_MODULE		(1<<0)
/*模块是强制装载的*/
#define TAINT_FORCED_MODULE				(1<<1)
/*SMP运行的处理器不支持SMP*/
#define TAINT_UNSAFE_SMP				(1<<2)
/*强制移除内核*/
#define TAINT_FORCED_RMMOD				(1<<3)
/*System experienced a machine check exception*/
#define TAINT_MACHINE_CHECK				(1<<4)
/*系统遇到坏页*/
#define TAINT_BAD_PAGE					(1<<5)
/*Userspace-defined naughtiness*/
#define TAINT_USER						(1<<6)
/**/
#define TAINT_DIE			(1<<7)

extern void dump_stack(void) __cold;

/*内核转储时添加的信息前缀*/
enum
{
	/*转储信息无前缀*/
	DUMP_PREFIX_NONE,
	/*转储信息前添加地址前缀*/
	DUMP_PREFIX_ADDRESS,
	/*转储信息前添加偏移前缀*/
	DUMP_PREFIX_OFFSET
};

extern void hex_dump_to_buffer(const void *buf, size_t len, int rowsize, int groupsize,
		char *linebuf, size_t linebuflen, bool ascii);
extern void print_hex_dump(const char *level, const char *prefix_str, int prefix_type,
		int rowsize, int groupsize, const void *buf, size_t len, bool ascii);
extern void print_hex_dump_bytes(const char *prefix_str, int prefix_type, const void *buf, size_t len);
#define hex_asc(x)		"0123456789abcdef"[x]

#define pr_emerg(fmt, arg...) 			printk(KERN_EMERG fmt, ##arg)
#define pr_alert(fmt, arg...) 			printk(KERN_ALERT fmt, ##arg)
#define pr_crit(fmt, arg...) 			printk(KERN_CRIT fmt, ##arg)
#define pr_err(fmt, arg...) 			printk(KERN_ERR fmt, ##arg)
#define pr_warning(fmt, arg...)			printk(KERN_WARNING fmt, ##arg)
#define pr_notice(fmt, arg...) 			printk(KERN_NOTICE fmt, ##arg)
#define pr_info(fmt, arg...)			printk(KERN_INFO fmt, ##arg)

#ifdef DEBUG
/*写驱动时用dev_dbg替代该函数*/
#define pr_debug(fmt, arg...) 			printk(KERN_DEBUG fmt, ##arg)
#else
static inline int __attribute__ ((format (printf, 1, 2))) pr_debug(const char * fmt, ...)
{
	return 0;
}
#endif

/*将32位IPv4地址显示为可读格式，如addr=0xFCA80101，转换后的格式为
"192.168.1.1"*/
#define NIPQUAD(addr) 				\
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT 	"%u.%u.%u.%u"

/*将128位IPv6地址转换成可读格式*/
#define NIP6(addr) 					\
	ntohs((addr).s6_addr16[0]), 	\
	ntohs((addr).s6_addr16[1]), 	\
	ntohs((addr).s6_addr16[2]), 	\
	ntohs((addr).s6_addr16[3]), 	\
	ntohs((addr).s6_addr16[4]), 	\
	ntohs((addr).s6_addr16[5]), 	\
	ntohs((addr).s6_addr16[6]), 	\
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT 		"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define NIP6_SEQFMT 	"%04x%04x%04x%04x%04x%04x%04x%04x"

#if defined(__LITTLE_ENDIAN)
#define HIPQUAD(addr) \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[0]
#elif defined(__BIG_ENDIAN)
#define HIPQUAD	NIPQUAD
#else
#error "Please fix asm/byteorder.h"
#endif /* __LITTLE_ENDIAN */

/*执行严格类型检查的min()宏。分别定义和两个比较参数类型相同的变量，并赋值为比较参
数，然后检测两变量的类型是否一致，不一致编译时抛出警告，void表示忽略得到的结果，不
加void会提示代码无意义*/
#define min(x,y) 				\
	({ 							\
		typeof(x) _x = (x);		\
		typeof(y) _y = (y);		\
		(void) (&_x == &_y);	\
		_x < _y ? _x : _y; 		\
	})
/*严格类型检查的max()宏*/
#define max(x,y) 				\
	({ 							\
		typeof(x) _x = (x);		\
		typeof(y) _y = (y);		\
		(void) (&_x == &_y);	\
		_x > _y ? _x : _y; 		\
	})

 /*不使用严格类型的min和max宏定义*/
#define min_t(type,x,y) ({ type __x = (x); type __y = (y); __x < __y ? __x : __y; })
#define max_t(type,x,y) ({ type __x = (x); type __y = (y); __x > __y ? __x : __y; })


/*根据type结构体中变量member的指针ptr，获取该结构体type类型的变量*/
#define container_of(ptr, type, member) 							\
	({																\
		const typeof( ((type *)0)->member ) *__mptr = (ptr);		\
		(type *)( (char *)__mptr - offsetof(type,member) );\
	})

/*编译时检查x是否是type类型，经常返回1*/
#define typecheck(type, x)				\
({										\
	type __dummy; 						\
	typeof(x) __dummy2; 				\
	(void)(&__dummy == &__dummy2); 		\
	1; 									\
})

/*编译时检测function是否是特定类型，或者是那种类型的指针（需要用typedef定义这
个函数类型）*/
#define typecheck_fn(type,function) ({	typeof(type) __tmp = function; (void)__tmp;})

struct sysinfo;
extern int do_sysinfo(struct sysinfo *info);

#endif /* __KERNEL__ */

#define SI_LOAD_SHIFT	16
struct sysinfo
{
	/*启动以来经过的秒数*/
	long uptime;
	/*1/5/15分钟系统的平均负载*/
	unsigned long loads[3];
	/*总计可用主存大小*/
	unsigned long totalram;
	/*空闲内存大小*/
	unsigned long freeram;
	/*共享内存大小*/
	unsigned long sharedram;
	/*用作缓冲的内存大小*/
	unsigned long bufferram;
	/*总计的交换空间大小*/
	unsigned long totalswap;
	/*仍然可用的交换空间大小*/
	unsigned long freeswap;
	/*当前进程的数目*/
	unsigned short procs;
	/*m68k的显式填充信息*/
	unsigned short pad;
	/*高端内存总计大小*/
	unsigned long totalhigh;
	/*可用的高端内存大小*/
	unsigned long freehigh;
	/*用bytes为单位表示的内存单元大小*/
	unsigned int mem_unit;
	/*lib5使用的填充信息*/
	char _f[20-2*sizeof(long)-sizeof(int)];
};

/*条件为真时（sizeof参数为char[-1]）强制编译错误*/
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

/*内核中一个编译期检查宏，用于在编译器检测并防止编译错误。该宏将一个（通常是关系或
逻辑）表达式作为参数进行运算，如果表达式的值不为0，则触发编译器错误而终止编译过程。
该机制通常用于过程性任务定义，避免出现类似于无限循环、计算错误等非预期情况。通过在
编译期间检查这些边界条件和错误情况，可以提高代码性能和可靠性，减少后续运行时错误和
异常情况*/
#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)

/*编译时预编译器将用所在的函数名，返回值时字符串*/
#define __FUNCTION__ (__func__)

/*避免条件编译 CONFIG_NUMA*/
#ifdef CONFIG_NUMA
#define NUMA_BUILD 1
#else
#define NUMA_BUILD 0
#endif

#endif
