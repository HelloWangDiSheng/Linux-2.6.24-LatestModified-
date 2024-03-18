#ifndef __TASK_IO_ACCOUNTING_OPS_INCLUDED
#define __TASK_IO_ACCOUNTING_OPS_INCLUDED

#include <linux/sched.h>

#ifdef CONFIG_TASK_IO_ACCOUNTING
/*累计当前进程从块设备上读取的字节数目*/
static inline void task_io_account_read(size_t bytes)
{
	current->ioac.read_bytes += bytes;
}

/*获取进程大约读取的块（块大小512字节）数目，*/
static inline unsigned long task_io_get_inblock(const struct task_struct *p)
{
	return p->ioac.read_bytes >> 9;
}

/*累计当前进程向块设备写入字节的数目*/
static inline void task_io_account_write(size_t bytes)
{
	current->ioac.write_bytes += bytes;
}

/*获取进程向块设备写入的块（每块512字节）数目*/
static inline unsigned long task_io_get_oublock(const struct task_struct *p)
{
	return p->ioac.write_bytes >> 9;
}

/*累计当前进程已取消的预期向块设备写数据的字节数目*/
static inline void task_io_account_cancelled_write(size_t bytes)
{
	current->ioac.cancelled_write_bytes += bytes;
}

/*初始化进程输入输出统计值*/
static inline void task_io_accounting_init(struct task_struct *tsk)
{
	memset(&tsk->ioac, 0, sizeof(tsk->ioac));
}

#else

static inline void task_io_account_read(size_t bytes)
{
}

static inline unsigned long task_io_get_inblock(const struct task_struct *p)
{
	return 0;
}

static inline void task_io_account_write(size_t bytes)
{
}

static inline unsigned long task_io_get_oublock(const struct task_struct *p)
{
	return 0;
}

static inline void task_io_account_cancelled_write(size_t bytes)
{
}

static inline void task_io_accounting_init(struct task_struct *tsk)
{
}

#endif		/* CONFIG_TASK_IO_ACCOUNTING */
#endif		/* __TASK_IO_ACCOUNTING_OPS_INCLUDED */
