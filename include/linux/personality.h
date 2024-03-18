#ifndef _LINUX_PERSONALITY_H
#define _LINUX_PERSONALITY_H

#ifdef __KERNEL__

 /*����ͬABI��personalities��*/
struct exec_domain;
struct pt_regs;

extern int register_exec_domain(struct exec_domain *);
extern int unregister_exec_domain(struct exec_domain *);
extern int __set_personality(unsigned long);

#endif /* __KERNEL__ */

/*����ģ���ʶ����Щռ�ݸ���λ�ֽ�*/
enum
{
	/*���������ַ�ռ������*/
	ADDR_NO_RANDOMIZE = 		0x0040000,
	/*�û��ռ亯��ָ��ָ���źŴ���������signal handling*/
	FDPIC_FUNCPTRS =			0x0080000,
	MMAP_PAGE_ZERO =			0x0100000,
	/*ʹ�þ����ڴ沼��*/
	ADDR_COMPAT_LAYOUT =		0x0200000,
	READ_IMPLIES_EXEC =			0x0400000,
	ADDR_LIMIT_32BIT =			0x0800000,
	SHORT_INODE =				0x1000000,
	WHOLE_SECONDS =				0x2000000,
	STICKY_TIMEOUTS	=			0x4000000,
	ADDR_LIMIT_3GB =		 	0x8000000,
};

/*��ȫ��صļ����Ա�ʶ������setuid��setgidʱ�������*/
#define PER_CLEAR_ON_SETID (READ_IMPLIES_EXEC|ADDR_NO_RANDOMIZE)

/*���Ի�����ֻʹ�õ�λ�ֽڣ�����ʹ�ø�λ�ӽڣ���������󷵻�ֵ�г�ͻ*/
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
/*ִ������������ǰ������Ա�ӻ��Դ�ļ������ã�������ʽ��Ҫ�����򲻶�*/
struct exec_domain
{
	/*ִ��������*/
	const char		*name;
	/*ϵͳ���ú���*/
	handler_t		handler;
	/*personality���λ*/
	unsigned char		pers_low;
	/*personality���λ*/
	unsigned char		pers_high;
	/*�ź�λͼ*/
	unsigned long		*signal_map;
	/*��ת���ź�λͼ*/
	unsigned long		*signal_invmap;
	/*����λͼ*/
	struct map_segment	*err_map;
	/*�׽�������λͼ*/
	struct map_segment	*socktype_map;
	/*�׽���ѡ��λͼ*/
	struct map_segment	*sockopt_map;
	/*��ַ��λͼ*/
	struct map_segment	*af_map;
	/*ִ����ģ��*/
	struct module		*module;
	/*�ڲ�ʹ�õ�����*/
	struct exec_domain	*next;
};

/*��ȡ�Ǳ�ʶ�ĸ��Ի��������ͣ�Ҳ����personality�ĵ��ֽ�*/
#define personality(pers)	(pers & PER_MASK)
/*��ǰ���н��̵ĸ��Ի���Ϣ*/
#define get_personality		(current->personality)

/*�ı䵱ǰ���н��̵ĸ��Ի���Ϣ*/
#define set_personality(pers) \
	((current->personality == (pers)) ? 0 : __set_personality(pers))

#endif /* __KERNEL__ */

#endif /* _LINUX_PERSONALITY_H */
