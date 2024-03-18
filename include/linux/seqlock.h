#ifndef __LINUX_SEQLOCK_H
#define __LINUX_SEQLOCK_H
 /*   顺序锁seqlock_t是对读写锁的一种优化，提高了读锁和写锁的独立性，写锁不会被读锁阻塞（不必
 等所有读操作完成后才写），读锁也不会被写锁阻塞（临界区可以在写临界区对被顺序锁保护额共享资源
 进行写操作的同时仍然可以继续读），写锁会被写锁阻塞，写操作之间时互斥的，如果有写执行单元正在
 进行写操作，其它的写执行单元必须自旋等待，直到写执行单元释放顺序锁为止。如果读执行单元在读操
 作期间，写执行单元已经发生了写操作，那么，读执行单元必须重新去读数据，以便确保读到的数据时完
 整的，这种锁在读写操作同时运行的概率比较小，性能时非常好的，而且它允许读写操作同时进行，因而
 更大的提高了并发性。
      顺序锁有一个限制：它必须要求被保护的共享资源中不能含有指针，因为写执行单元可能会使指针失
 效，当读执行单元如果正要访问该指针时，系统就会崩溃*/
 

#include <linux/spinlock.h>
#include <linux/preempt.h>
/*顺序锁结构定义*/
typedef struct
{
	/*计数编号，一直累增*/
	unsigned sequence;
	/*封装一个自旋锁变量*/
	spinlock_t lock;
} seqlock_t;

/*初始化一个顺序锁为未锁定状态，这些宏在gcc-3.x编译时触发问题，现在可以了，用时注意*/
#define __SEQLOCK_UNLOCKED(lockname) \
		 { 0, __SPIN_LOCK_UNLOCKED(lockname) }

#define SEQLOCK_UNLOCKED \
		 __SEQLOCK_UNLOCKED(old_style_seqlock_init)
/*初始化顺序锁变量x*/
#define seqlock_init(x)					\
	do {						\
		(x)->sequence = 0;			\
		spin_lock_init(&(x)->lock);		\
	} while (0)
/*定义并初始化一个自旋锁变量x*/
#define DEFINE_SEQLOCK(x) \
		seqlock_t x = __SEQLOCK_UNLOCKED(x)

/*将其它写者排除在锁之外并更新计数，操作就像正常的自旋锁操作，因为已经获取了自旋锁，
因此不需要禁用抢占*/
static inline void write_seqlock(seqlock_t *sl)
{
	spin_lock(&sl->lock);
	++sl->sequence;
	/*内存优化屏障，此点之后的写操作开始之前，此点之前的所有写操作都已经完成*/
	smp_wmb();
}
/*写者解除顺序锁*/
static inline void write_sequnlock(seqlock_t *sl)
{
	smp_wmb();
	sl->sequence++;
	spin_unlock(&sl->lock);
}

/*非阻塞式申请自旋锁，申请成功时更新计数*/
static inline int write_tryseqlock(seqlock_t *sl)
{
	int ret = spin_trylock(&sl->lock);
	/*申请自旋锁成功后更新计数*/
	if (ret) {
		++sl->sequence;
		smp_wmb();
	}
	return ret;
}

/*开始读统计，获取最后一个已完成的写操作后的计数值*/
static __always_inline unsigned read_seqbegin(const seqlock_t *sl)
{
	unsigned ret = sl->sequence;
	smp_rmb();
	return ret;
}

/*测试读者进程是否有非法数据。读者在访问完被顺序锁s1保护的共享资源后需要调用该函数来检查，在
读访问期间是否有写者访问了该共享资源，该检查时通过判断当前顺序锁s1的顺序号与初始顺序号start是
否相等实现的，如果不相等，则读者访问期间有写者访问了共享资源，读者就需要从新进行读操作，否则，
读者成功完成读操作*/
static __always_inline int read_seqretry(const seqlock_t *sl, unsigned iv)
{
	smp_rmb();
	return (iv & 1) | (sl->sequence ^ iv);
}


/*
 * Version using sequence counter only.
 * This can be used when code has its own mutex protecting the
 * updating starting before the write_seqcountbeqin() and ending
 * after the write_seqcount_end().
 */
/*仅仅使用顺序计数值，*/
typedef struct seqcount {
	unsigned sequence;
} seqcount_t;

#define SEQCNT_ZERO { 0 }
#define seqcount_init(x)	do { *(x) = (seqcount_t) SEQCNT_ZERO; } while (0)

/* Start of read using pointer to a sequence counter only.  */
static inline unsigned read_seqcount_begin(const seqcount_t *s)
{
	unsigned ret = s->sequence;
	smp_rmb();
	return ret;
}

/* Test if reader processed invalid data.
 * Equivalent to: iv is odd or sequence number has changed.
 *                (iv & 1) || (*s != iv)
 * Using xor saves one conditional branch.
 */
static inline int read_seqcount_retry(const seqcount_t *s, unsigned iv)
{
	smp_rmb();
	return (iv & 1) | (s->sequence ^ iv);
}


/*
 * Sequence counter only version assumes that callers are using their
 * own mutexing.
 */
static inline void write_seqcount_begin(seqcount_t *s)
{
	s->sequence++;
	smp_wmb();
}

static inline void write_seqcount_end(seqcount_t *s)
{
	smp_wmb();
	s->sequence++;
}

/*
 * Possible sw/hw IRQ protected versions of the interfaces.
 */
#define write_seqlock_irqsave(lock, flags)				\
	do { local_irq_save(flags); write_seqlock(lock); } while (0)
#define write_seqlock_irq(lock)						\
	do { local_irq_disable();   write_seqlock(lock); } while (0)
#define write_seqlock_bh(lock)						\
        do { local_bh_disable();    write_seqlock(lock); } while (0)

#define write_sequnlock_irqrestore(lock, flags)				\
	do { write_sequnlock(lock); local_irq_restore(flags); } while(0)
#define write_sequnlock_irq(lock)					\
	do { write_sequnlock(lock); local_irq_enable(); } while(0)
#define write_sequnlock_bh(lock)					\
	do { write_sequnlock(lock); local_bh_enable(); } while(0)

#define read_seqbegin_irqsave(lock, flags)				\
	({ local_irq_save(flags);   read_seqbegin(lock); })

#define read_seqretry_irqrestore(lock, iv, flags)			\
	({								\
		int ret = read_seqretry(lock, iv);			\
		local_irq_restore(flags);				\
		ret;							\
	})

#endif /* __LINUX_SEQLOCK_H */
