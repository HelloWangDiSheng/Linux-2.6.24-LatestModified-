#ifndef _LINUX_THREADS_H
#define _LINUX_THREADS_H

/*默认的线程上限值目前可通过/proc/sys/kernel/threads-max查看，SMP系统支持的最大cpu数目，
这个值可以配置，最大数目默认为平台的字长比特位数目*/
#ifdef CONFIG_SMP
#define NR_CPUS		CONFIG_NR_CPUS
#else
#define NR_CPUS		1
#endif

/*根用户保留至少4个线程*/
#define MIN_THREADS_LEFT_FOR_ROOT 4

/*一个进程默认可以分配的最大线程数目是32768，小型系统内上（通常为嵌入式系统）可以
定义配置选项BASE_SMALL，此时支持的最大数目是4096*/
#define PID_MAX_DEFAULT (CONFIG_BASE_SMALL ? 0x1000 : 0x8000)

/*定义PID最大数目：小型系统上是32768，32位系统上是默认的最32768，字长大于4字节时是4M*/
/*		config						PID_MAX_DEFAULT			PID_MAX_LIIT
	CONFIG_BASE_SMALL						4K						32K
	BIT_PER_LONG == 32						32K						32K	
	BIT_PER_LONG > 32						32K						4M
*/
#define PID_MAX_LIMIT (CONFIG_BASE_SMALL ? PAGE_SIZE * 8 : \
	(sizeof(long) > 4 ? 4 * 1024 * 1024 : PID_MAX_DEFAULT))

#endif
