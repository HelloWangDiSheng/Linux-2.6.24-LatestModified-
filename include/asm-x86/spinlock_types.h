#ifndef __ASM_SPINLOCK_TYPES_H
#define __ASM_SPINLOCK_TYPES_H
/*不能被直接引用该文件*/
#ifndef __LINUX_SPINLOCK_TYPES_H
# error "please don't include this file directly"
#endif
/*原始spinlock_t类型定义*/
typedef struct
{
	unsigned int slock;
} raw_spinlock_t;

#define __RAW_SPIN_LOCK_UNLOCKED	{ 1 }

/*原始读写锁定义*/
typedef struct
{
	unsigned int lock;
} raw_rwlock_t;

#define __RAW_RW_LOCK_UNLOCKED		{ RW_LOCK_BIAS }

#endif
