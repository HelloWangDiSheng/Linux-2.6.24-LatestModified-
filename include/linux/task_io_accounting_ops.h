#ifndef __TASK_IO_ACCOUNTING_OPS_INCLUDED
#define __TASK_IO_ACCOUNTING_OPS_INCLUDED

#include <linux/sched.h>

#ifdef CONFIG_TASK_IO_ACCOUNTING
/*�ۼƵ�ǰ���̴ӿ��豸�϶�ȡ���ֽ���Ŀ*/
static inline void task_io_account_read(size_t bytes)
{
	current->ioac.read_bytes += bytes;
}

/*��ȡ���̴�Լ��ȡ�Ŀ飨���С512�ֽڣ���Ŀ��*/
static inline unsigned long task_io_get_inblock(const struct task_struct *p)
{
	return p->ioac.read_bytes >> 9;
}

/*�ۼƵ�ǰ��������豸д���ֽڵ���Ŀ*/
static inline void task_io_account_write(size_t bytes)
{
	current->ioac.write_bytes += bytes;
}

/*��ȡ��������豸д��Ŀ飨ÿ��512�ֽڣ���Ŀ*/
static inline unsigned long task_io_get_oublock(const struct task_struct *p)
{
	return p->ioac.write_bytes >> 9;
}

/*�ۼƵ�ǰ������ȡ����Ԥ������豸д���ݵ��ֽ���Ŀ*/
static inline void task_io_account_cancelled_write(size_t bytes)
{
	current->ioac.cancelled_write_bytes += bytes;
}

/*��ʼ�������������ͳ��ֵ*/
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
