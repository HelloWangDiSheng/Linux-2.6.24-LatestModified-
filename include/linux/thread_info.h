/*��ͨ�Ͳ���߳���Ϣ*/

#ifndef _LINUX_THREAD_INFO_H
#define _LINUX_THREAD_INFO_H

#include <linux/types.h>

/*ʵ���źŻ��ƣ�����ϵͳ����*/
struct restart_block
{
	long (*fn)(struct restart_block *);
	union
	{
			struct
			{
				unsigned long arg0, arg1, arg2, arg3;
			};
		/* For futex_wait */
			struct
			{
				u32 *uaddr;
				u32 val;
				u32 flags;
				u64 time;
			} futex;
	};
};

extern long do_no_restart_syscall(struct restart_block *parm);

#include <linux/bitops.h>
#include <asm/thread_info.h>

#ifdef __KERNEL__

/*���õͲ��߳�TIF_*��ʶ*/
static inline void set_ti_thread_flag(struct thread_info *ti, int flag)
{
	set_bit(flag, &ti->flags);
}
/*����Ͳ���߳�TIF_*��ʶ*/
static inline void clear_ti_thread_flag(struct thread_info *ti, int flag)
{
	clear_bit(flag, &ti->flags);
}
/*���Բ����õͲ��߳�TIF_*��ʶ�����ش˱�ʶΪ֮ǰ��״̬*/
static inline int test_and_set_ti_thread_flag(struct thread_info *ti, int flag)
{
	return test_and_set_bit(flag, &ti->flags);
}
/*���Բ�����Ͳ��߳�TIF_*��Ϣ�����ش˱�ʶλ��ǰ��״̬*/
static inline int test_and_clear_ti_thread_flag(struct thread_info *ti, int flag)
{
	return test_and_clear_bit(flag, &ti->flags);
}
/*���Բ����صͲ��߳�TIF_*��ʶ״̬*/
static inline int test_ti_thread_flag(struct thread_info *ti, int flag)
{
	return test_bit(flag, &ti->flags);
}
/*���õ�ǰ���̵ĵͲ�TIF_*��ʶ*/
#define set_thread_flag(flag) \
	set_ti_thread_flag(current_thread_info(), flag)
/*�����ǰ���̵ĵͲ�TIF_*��ʶ*/
#define clear_thread_flag(flag) \
	clear_ti_thread_flag(current_thread_info(), flag)
/*���Բ����õ�ǰ���̵Ͳ�TIF_*��ʶ�����ش˱�ʶΪ֮ǰ��״̬*/
#define test_and_set_thread_flag(flag) \
	test_and_set_ti_thread_flag(current_thread_info(), flag)
/*���Բ������ǰ���̵Ͳ�TIF_*��ʶ�����ش˱�ʶΪ֮ǰ��״̬*/
#define test_and_clear_thread_flag(flag) \
	test_and_clear_ti_thread_flag(current_thread_info(), flag)
/*���Բ����ص�ǰ���̵ĵͲ�TIF_*��ʶ״̬*/
#define test_thread_flag(flag) \
	test_ti_thread_flag(current_thread_info(), flag)

/*���õ�ǰ���̵ĵͲ�TIF_NEED_REACHED��ʶ*/
#define set_need_resched()	set_thread_flag(TIF_NEED_RESCHED)
/*�����ǰ���̵ĵͲ�TIF_NEED_REACHED��ʶ*/
#define clear_need_resched()	clear_thread_flag(TIF_NEED_RESCHED)

#endif

#endif /* _LINUX_THREAD_INFO_H */
