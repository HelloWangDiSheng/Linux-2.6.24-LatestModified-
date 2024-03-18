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
/*����IO����ͳ��*/
struct task_io_accounting
{
	/*���̴Ӵ洢�豸�϶�ȡ���ַ���*/
	u64 read_bytes;
	/*�ɽ���������������д����ַ���*/
	u64 write_bytes;
	/*����ȡ��д����ַ���*/
	u64 cancelled_write_bytes;
};
#else
struct task_io_accounting {
};
#endif
