#ifndef __LINUX_SEQLOCK_H
#define __LINUX_SEQLOCK_H
 /*   ˳����seqlock_t�ǶԶ�д����һ���Ż�������˶�����д���Ķ����ԣ�д�����ᱻ��������������
 �����ж�������ɺ��д��������Ҳ���ᱻд���������ٽ���������д�ٽ����Ա�˳�������������Դ
 ����д������ͬʱ��Ȼ���Լ���������д���ᱻд��������д����֮��ʱ����ģ������дִ�е�Ԫ����
 ����д������������дִ�е�Ԫ���������ȴ���ֱ��дִ�е�Ԫ�ͷ�˳����Ϊֹ�������ִ�е�Ԫ�ڶ���
 ���ڼ䣬дִ�е�Ԫ�Ѿ�������д��������ô����ִ�е�Ԫ��������ȥ�����ݣ��Ա�ȷ������������ʱ��
 ���ģ��������ڶ�д����ͬʱ���еĸ��ʱȽ�С������ʱ�ǳ��õģ������������д����ͬʱ���У����
 ���������˲����ԡ�
      ˳������һ�����ƣ�������Ҫ�󱻱����Ĺ�����Դ�в��ܺ���ָ�룬��Ϊдִ�е�Ԫ���ܻ�ʹָ��ʧ
 Ч������ִ�е�Ԫ�����Ҫ���ʸ�ָ��ʱ��ϵͳ�ͻ����*/
 

#include <linux/spinlock.h>
#include <linux/preempt.h>
/*˳�����ṹ����*/
typedef struct
{
	/*������ţ�һֱ����*/
	unsigned sequence;
	/*��װһ������������*/
	spinlock_t lock;
} seqlock_t;

/*��ʼ��һ��˳����Ϊδ����״̬����Щ����gcc-3.x����ʱ�������⣬���ڿ����ˣ���ʱע��*/
#define __SEQLOCK_UNLOCKED(lockname) \
		 { 0, __SPIN_LOCK_UNLOCKED(lockname) }

#define SEQLOCK_UNLOCKED \
		 __SEQLOCK_UNLOCKED(old_style_seqlock_init)
/*��ʼ��˳��������x*/
#define seqlock_init(x)					\
	do {						\
		(x)->sequence = 0;			\
		spin_lock_init(&(x)->lock);		\
	} while (0)
/*���岢��ʼ��һ������������x*/
#define DEFINE_SEQLOCK(x) \
		seqlock_t x = __SEQLOCK_UNLOCKED(x)

/*������д���ų�����֮�Ⲣ���¼���������������������������������Ϊ�Ѿ���ȡ����������
��˲���Ҫ������ռ*/
static inline void write_seqlock(seqlock_t *sl)
{
	spin_lock(&sl->lock);
	++sl->sequence;
	/*�ڴ��Ż����ϣ��˵�֮���д������ʼ֮ǰ���˵�֮ǰ������д�������Ѿ����*/
	smp_wmb();
}
/*д�߽��˳����*/
static inline void write_sequnlock(seqlock_t *sl)
{
	smp_wmb();
	sl->sequence++;
	spin_unlock(&sl->lock);
}

/*������ʽ����������������ɹ�ʱ���¼���*/
static inline int write_tryseqlock(seqlock_t *sl)
{
	int ret = spin_trylock(&sl->lock);
	/*�����������ɹ�����¼���*/
	if (ret) {
		++sl->sequence;
		smp_wmb();
	}
	return ret;
}

/*��ʼ��ͳ�ƣ���ȡ���һ������ɵ�д������ļ���ֵ*/
static __always_inline unsigned read_seqbegin(const seqlock_t *sl)
{
	unsigned ret = sl->sequence;
	smp_rmb();
	return ret;
}

/*���Զ��߽����Ƿ��зǷ����ݡ������ڷ����걻˳����s1�����Ĺ�����Դ����Ҫ���øú�������飬��
�������ڼ��Ƿ���д�߷����˸ù�����Դ���ü��ʱͨ���жϵ�ǰ˳����s1��˳������ʼ˳���start��
�����ʵ�ֵģ��������ȣ�����߷����ڼ���д�߷����˹�����Դ�����߾���Ҫ���½��ж�����������
���߳ɹ���ɶ�����*/
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
/*����ʹ��˳�����ֵ��*/
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
