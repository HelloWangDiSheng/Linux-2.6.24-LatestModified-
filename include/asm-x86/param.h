#ifndef _ASM_X86_PARAM_H
#define _ASM_X86_PARAM_H

#ifdef __KERNEL__
/*内核内部时钟频率*/
#define HZ		CONFIG_HZ
/*一些用户接口使用的滴答*/
#define USER_HZ				100
/*每秒钟滴答次数*/
#define CLOCKS_PER_SEC		(USER_HZ)
#endif

/*无定义HZ的默认值*/
#ifndef HZ
#define HZ 100
#endif

#define EXEC_PAGESIZE		4096

#ifndef NOGROUP
#define NOGROUP				(-1)
#endif

/*主机名最大长度*/
#define MAXHOSTNAMELEN		64

#endif /* _ASM_X86_PARAM_H */
