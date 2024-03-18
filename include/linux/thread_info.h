/*普通低层的线程信息*/

#ifndef _LINUX_THREAD_INFO_H
#define _LINUX_THREAD_INFO_H

#include <linux/types.h>

/*实现信号机制，重启系统调用*/
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

/*设置低层线程TIF_*标识*/
static inline void set_ti_thread_flag(struct thread_info *ti, int flag)
{
	set_bit(flag, &ti->flags);
}
/*清除低层的线程TIF_*标识*/
static inline void clear_ti_thread_flag(struct thread_info *ti, int flag)
{
	clear_bit(flag, &ti->flags);
}
/*测试并设置低层线程TIF_*标识，返回此标识为之前的状态*/
static inline int test_and_set_ti_thread_flag(struct thread_info *ti, int flag)
{
	return test_and_set_bit(flag, &ti->flags);
}
/*测试并清除低层线程TIF_*信息，返回此标识位此前的状态*/
static inline int test_and_clear_ti_thread_flag(struct thread_info *ti, int flag)
{
	return test_and_clear_bit(flag, &ti->flags);
}
/*测试并返回低层线程TIF_*标识状态*/
static inline int test_ti_thread_flag(struct thread_info *ti, int flag)
{
	return test_bit(flag, &ti->flags);
}
/*设置当前进程的低层TIF_*标识*/
#define set_thread_flag(flag) \
	set_ti_thread_flag(current_thread_info(), flag)
/*清除当前进程的低层TIF_*标识*/
#define clear_thread_flag(flag) \
	clear_ti_thread_flag(current_thread_info(), flag)
/*测试并设置当前进程低层TIF_*标识，返回此标识为之前的状态*/
#define test_and_set_thread_flag(flag) \
	test_and_set_ti_thread_flag(current_thread_info(), flag)
/*测试并清除当前进程低层TIF_*标识，返回此标识为之前的状态*/
#define test_and_clear_thread_flag(flag) \
	test_and_clear_ti_thread_flag(current_thread_info(), flag)
/*测试并返回当前进程的低层TIF_*标识状态*/
#define test_thread_flag(flag) \
	test_ti_thread_flag(current_thread_info(), flag)

/*设置当前进程的低层TIF_NEED_REACHED标识*/
#define set_need_resched()	set_thread_flag(TIF_NEED_RESCHED)
/*清除当前进程的低层TIF_NEED_REACHED标识*/
#define clear_need_resched()	clear_thread_flag(TIF_NEED_RESCHED)

#endif

#endif /* _LINUX_THREAD_INFO_H */
