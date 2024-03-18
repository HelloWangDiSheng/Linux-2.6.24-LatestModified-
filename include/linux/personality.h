#ifndef _LINUX_PERSONALITY_H
#define _LINUX_PERSONALITY_H

#ifdef __KERNEL__

 /*处理不同ABI（personalities）*/
struct exec_domain;
struct pt_regs;

extern int register_exec_domain(struct exec_domain *);
extern int unregister_exec_domain(struct exec_domain *);
extern int __set_personality(unsigned long);

#endif /* __KERNEL__ */

/*错误模拟标识，这些占据高三位字节*/
enum
{
	/*禁用虚拟地址空间随机化*/
	ADDR_NO_RANDOMIZE = 		0x0040000,
	/*用户空间函数指针指向信号处理描述符signal handling*/
	FDPIC_FUNCPTRS =			0x0080000,
	MMAP_PAGE_ZERO =			0x0100000,
	/*使用经典内存布局*/
	ADDR_COMPAT_LAYOUT =		0x0200000,
	READ_IMPLIES_EXEC =			0x0400000,
	ADDR_LIMIT_32BIT =			0x0800000,
	SHORT_INODE =				0x1000000,
	WHOLE_SECONDS =				0x2000000,
	STICKY_TIMEOUTS	=			0x4000000,
	ADDR_LIMIT_3GB =		 	0x8000000,
};

/*安全相关的兼容性标识在运行setuid或setgid时必须清除*/
#define PER_CLEAR_ON_SETID (READ_IMPLIES_EXEC|ADDR_NO_RANDOMIZE)

/*个性化类型只使用低位字节，避免使用高位子节，它将与错误返回值有冲突*/
enum
{
	PER_LINUX =		0x0000,
	PER_LINUX_32BIT =	0x0000 | ADDR_LIMIT_32BIT,
	PER_LINUX_FDPIC =	0x0000 | FDPIC_FUNCPTRS,
	PER_SVR4 =		0x0001 | STICKY_TIMEOUTS | MMAP_PAGE_ZERO,
	PER_SVR3 =		0x0002 | STICKY_TIMEOUTS | SHORT_INODE,
	PER_SCOSVR3 =		0x0003 | STICKY_TIMEOUTS | WHOLE_SECONDS | SHORT_INODE,
	PER_OSR5 =		0x0003 | STICKY_TIMEOUTS | WHOLE_SECONDS,
	PER_WYSEV386 =		0x0004 | STICKY_TIMEOUTS | SHORT_INODE,
	PER_ISCR4 =		0x0005 | STICKY_TIMEOUTS,
	PER_BSD =		0x0006,
	PER_SUNOS =		0x0006 | STICKY_TIMEOUTS,
	PER_XENIX =		0x0007 | STICKY_TIMEOUTS | SHORT_INODE,
	PER_LINUX32 =		0x0008,
	PER_LINUX32_3GB =	0x0008 | ADDR_LIMIT_3GB,
	PER_IRIX32 =		0x0009 | STICKY_TIMEOUTS,/* IRIX5 32-bit */
	PER_IRIXN32 =		0x000a | STICKY_TIMEOUTS,/* IRIX6 new 32-bit */
	PER_IRIX64 =		0x000b | STICKY_TIMEOUTS,/* IRIX6 64-bit */
	PER_RISCOS =		0x000c,
	PER_SOLARIS =		0x000d | STICKY_TIMEOUTS,
	PER_UW7 =		0x000e | STICKY_TIMEOUTS | MMAP_PAGE_ZERO,
	PER_OSF4 =		0x000f,			 /* OSF/1 v4 */
	PER_HPUX =		0x0010,
	PER_MASK =		0x00ff,
};

#ifdef __KERNEL__

typedef void (*handler_t)(int, struct pt_regs *);
/*执行域描述符，前两个成员从汇编源文件中引用，除非显式需要，否则不动*/
struct exec_domain
{
	/*执行域名称*/
	const char		*name;
	/*系统调用函数*/
	handler_t		handler;
	/*personality最低位*/
	unsigned char		pers_low;
	/*personality最高位*/
	unsigned char		pers_high;
	/*信号位图*/
	unsigned long		*signal_map;
	/*翻转的信号位图*/
	unsigned long		*signal_invmap;
	/*错误位图*/
	struct map_segment	*err_map;
	/*套接字类型位图*/
	struct map_segment	*socktype_map;
	/*套接字选项位图*/
	struct map_segment	*sockopt_map;
	/*地址族位图*/
	struct map_segment	*af_map;
	/*执行域模块*/
	struct module		*module;
	/*内部使用单链表*/
	struct exec_domain	*next;
};

/*获取非标识的个性化基本类型，也就是personality的低字节*/
#define personality(pers)	(pers & PER_MASK)
/*当前运行进程的个性化信息*/
#define get_personality		(current->personality)

/*改变当前运行进程的个性化信息*/
#define set_personality(pers) \
	((current->personality == (pers)) ? 0 : __set_personality(pers))

#endif /* __KERNEL__ */

#endif /* _LINUX_PERSONALITY_H */
