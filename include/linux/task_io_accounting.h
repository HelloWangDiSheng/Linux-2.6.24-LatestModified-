/*
 * task_io_accounting: a structure which is used for recording a single task's
 * IO statistics.
 *
 * Don't include this header file directly - it is designed to be dragged in via
 * sched.h.
 *
 * Blame akpm@osdl.org for all this.
 */

#ifdef CONFIG_TASK_IO_ACCOUNTING
/*进程IO操作统计*/
struct task_io_accounting
{
	/*进程从存储设备上读取的字符数*/
	u64 read_bytes;
	/*由进程引起的向磁盘上写入的字符数*/
	u64 write_bytes;
	/*进程取消写入的字符数*/
	u64 cancelled_write_bytes;
};
#else
struct task_io_accounting {
};
#endif
