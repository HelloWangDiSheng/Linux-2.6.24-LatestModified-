#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

/*
 * include/linux/spinlock.h - generic spinlock/rwlock declarations
 *
 * here's the role of the various spinlock/rwlock related include files:
 *
 * on SMP builds:
 *  asm/spinlock_types.h: contains the raw_spinlock_t/raw_rwlock_t and the initializers
 *  linux/spinlock_types.h: defines the generic type and initializers
 *  asm/spinlock.h:     contains the __raw_spin_*()/etc. lowlevel       implementations, mostly 
 inline assembly code(also included on UP-debug builds:)
 *  linux/spinlock_api_smp.h:       contains the prototypes for the _spin_*() APIs.
 *  linux/spinlock.h:     builds the final spin_*() APIs. on UP builds:
 *  linux/spinlock_type_up.h:      contains the generic, simplified UP spinlock type.(which is
 an empty structure on non-debug builds)
 *  linux/spinlock_up.h: contains the __raw_spin_*()/etc. version of UP builds. (which 
 are NOPs on non-debug, non-preempt builds) (included on UP-non-debug builds:)
 linux/spinlock_api_up.h:      builds the _spin_*() APIs.
 */

#include <linux/preempt.h>
#include <linux/linkage.h>
#include <linux/compiler.h>
#include <linux/thread_info.h>
#include <linux/kernel.h>
#include <linux/stringify.h>
#include <linux/bottom_half.h>

#include <asm/system.h>

 /*内联函数需要的信息，必须要在引用其他文件前定义*/
#define LOCK_SECTION_NAME ".text.lock."KBUILD_BASENAME

#define LOCK_SECTION_START(extra)               \
        ".subsection 1\n\t"                     \
        extra                                   \
        ".ifndef " LOCK_SECTION_NAME "\n\t"     \
        LOCK_SECTION_NAME ":\n\t"               \
        ".endif\n"

#define LOCK_SECTION_END                        \
        ".previous\n\t"
/*锁函数标识宏，此类前缀函数的保存在".spinlock.txt"段中*/
#define __lockfunc fastcall __attribute__((section(".spinlock.text")))

/*
 * Pull the raw_spinlock_t and raw_rwlock_t definitions:
 */
#include <linux/spinlock_types.h>

extern int __lockfunc generic__raw_read_trylock(raw_rwlock_t *lock);

/*
 * Pull the __raw*() functions/declarations (UP-nondebug doesnt need them):
 */
#ifdef CONFIG_SMP
# include <asm/spinlock.h>
#else
# include <linux/spinlock_up.h>
#endif

#ifdef CONFIG_DEBUG_SPINLOCK
  extern void __spin_lock_init(spinlock_t *lock, const char *name,
			       struct lock_class_key *key);
# define spin_lock_init(lock)					\
do {								\
	static struct lock_class_key __key;			\
								\
	__spin_lock_init((lock), #lock, &__key);		\
} while (0)

#else
/*    初始化自旋锁为未锁定状态，spin_lock会考虑下面两种情况：(1)如果内核中其它地方尚未获得
锁，则由当前处理器获取，其它处理器不能在进入锁保护的代码范围。(2)如果锁已经有由其它处理器
获得，spin_lock进入一个无线循环，重复地检查锁是否已经有spin_unlock释放，如果已经释放，则获
得锁，并进入临界区。spin_lock定义为一个原子操作，在获得自旋锁的情况下可防止竟态条件出现。内
核还提供了spin_trylock和spin_trylock_bh两种方法，它们尝试获取锁，但在锁无法立即获取时不会阻
塞，在操作成功时，它们返回非零值（代码由自旋锁保护），否则返回0，后一种情况下，代码没有被锁
保护。
     使用自旋锁时必须要注意以下两点:(1)如果获得锁之后不释放，系统将变得不可用。所有处理器，
（包括获得锁的在内），迟早需要进入临界区。它们会进入不限循环等待锁释放，但等不到，就产生死锁
。(2)自旋锁不应该长期持有，因为所有等待锁释放的处理器都处于不可能状态，无法用于其它的工作。
     
*/
#define spin_lock_init(lock)					\
	do { *(lock) = SPIN_LOCK_UNLOCKED; } while (0)/*初始化自旋锁为未锁定状态*/\
#endif

#ifdef CONFIG_DEBUG_SPINLOCK
  extern void __rwlock_init(rwlock_t *lock, const char *name,
			    struct lock_class_key *key);
#define rwlock_init(lock)					\
do {								\
	static struct lock_class_key __key;			\
								\
	__rwlock_init((lock), #lock, &__key);			\
} while (0)
#else
/*初始化读写锁为未锁定状态*/
#define rwlock_init(lock)					\
	do { *(lock) = RW_LOCK_UNLOCKED; } while (0)
#endif
/*测试spinlock是否处于锁定状态*/
#define spin_is_locked(lock)	__raw_spin_is_locked(&(lock)->raw_lock)

/*等待spinlock解除锁定*/
#define spin_unlock_wait(lock)	__raw_spin_unlock_wait(&(lock)->raw_lock)

/*
 * Pull the _spin_*()/_read_*()/_write_*() functions/declarations:
 */
#if defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)
#include <linux/spinlock_api_smp.h>
#else
#include <linux/spinlock_api_up.h>
#endif

#ifdef CONFIG_DEBUG_SPINLOCK
 extern void _raw_spin_lock(spinlock_t *lock);
#define _raw_spin_lock_flags(lock, flags) _raw_spin_lock(lock)
 extern int _raw_spin_trylock(spinlock_t *lock);
 extern void _raw_spin_unlock(spinlock_t *lock);
 extern void _raw_read_lock(rwlock_t *lock);
 extern int _raw_read_trylock(rwlock_t *lock);
 extern void _raw_read_unlock(rwlock_t *lock);
 extern void _raw_write_lock(rwlock_t *lock);
 extern int _raw_write_trylock(rwlock_t *lock);
 extern void _raw_write_unlock(rwlock_t *lock);
#else
#define _raw_spin_lock(lock)		__raw_spin_lock(&(lock)->raw_lock)
#define _raw_spin_lock_flags(lock, flags) \
		__raw_spin_lock_flags(&(lock)->raw_lock, *(flags))
#define _raw_spin_trylock(lock)	__raw_spin_trylock(&(lock)->raw_lock)
#define _raw_spin_unlock(lock)		__raw_spin_unlock(&(lock)->raw_lock)
#define _raw_read_lock(rwlock)		__raw_read_lock(&(rwlock)->raw_lock)
#define _raw_read_trylock(rwlock)	__raw_read_trylock(&(rwlock)->raw_lock)
#define _raw_read_unlock(rwlock)	__raw_read_unlock(&(rwlock)->raw_lock)
#define _raw_write_lock(rwlock)	__raw_write_lock(&(rwlock)->raw_lock)
#define _raw_write_trylock(rwlock)	__raw_write_trylock(&(rwlock)->raw_lock)
#define _raw_write_unlock(rwlock)	__raw_write_unlock(&(rwlock)->raw_lock)
#endif

#define read_can_lock(rwlock)		__raw_read_can_lock(&(rwlock)->raw_lock)
#define write_can_lock(rwlock)		__raw_write_can_lock(&(rwlock)->raw_lock)

/*
 * Define the various spin_lock and rw_lock methods.  Note we define these
 * regardless of whether CONFIG_SMP or CONFIG_PREEMPT are set. The various
 * methods are defined as nops in the case they are not required.
 */
 /*申请锁不成功时直接返回，不会阻塞*/
#define spin_trylock(lock)		__cond_lock(lock, _spin_trylock(lock))
#define read_trylock(lock)		__cond_lock(lock, _read_trylock(lock))
#define write_trylock(lock)		__cond_lock(lock, _write_trylock(lock))

#define spin_lock(lock)			_spin_lock(lock)

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define spin_lock_nested(lock, subclass) _spin_lock_nested(lock, subclass)
#else
# define spin_lock_nested(lock, subclass) _spin_lock(lock)
#endif
/*获得读写锁*/
#define write_lock(lock)		_write_lock(lock)
#define read_lock(lock)			_read_lock(lock)

#if defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)
/*获得自旋锁，停用本地处理器中断*/
#define spin_lock_irqsave(lock, flags)	flags = _spin_lock_irqsave(lock)
#define read_lock_irqsave(lock, flags)	flags = _read_lock_irqsave(lock)
#define write_lock_irqsave(lock, flags)	flags = _write_lock_irqsave(lock)

#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define spin_lock_irqsave_nested(lock, flags, subclass) \
	flags = _spin_lock_irqsave_nested(lock, subclass)
#else
#define spin_lock_irqsave_nested(lock, flags, subclass) \
	flags = _spin_lock_irqsave(lock)
#endif

#else

#define spin_lock_irqsave(lock, flags)	_spin_lock_irqsave(lock, flags)
#define read_lock_irqsave(lock, flags)	_read_lock_irqsave(lock, flags)
#define write_lock_irqsave(lock, flags)	_write_lock_irqsave(lock, flags)
#define spin_lock_irqsave_nested(lock, flags, subclass)	\
	spin_lock_irqsave(lock, flags)

#endif
/*获得锁，停用本地处理器中断(_bh结尾的时停用软中断)*/
#define spin_lock_irq(lock)		_spin_lock_irq(lock)
#define spin_lock_bh(lock)		_spin_lock_bh(lock)

#define read_lock_irq(lock)		_read_lock_irq(lock)
#define read_lock_bh(lock)		_read_lock_bh(lock)

#define write_lock_irq(lock)		_write_lock_irq(lock)
#define write_lock_bh(lock)		_write_lock_bh(lock)

/*
 * We inline the unlock functions in the nondebug case:
 */
#if defined(CONFIG_DEBUG_SPINLOCK) || defined(CONFIG_PREEMPT) || \
	!defined(CONFIG_SMP)
#define spin_unlock(lock)		_spin_unlock(lock)
#define read_unlock(lock)		_read_unlock(lock)
#define write_unlock(lock)		_write_unlock(lock)
#define spin_unlock_irq(lock)		_spin_unlock_irq(lock)
#define read_unlock_irq(lock)		_read_unlock_irq(lock)
#define write_unlock_irq(lock)		_write_unlock_irq(lock)
#else
/*解锁自旋锁*/
#define spin_unlock(lock) \
    do {__raw_spin_unlock(&(lock)->raw_lock); __release(lock); } while (0)
/*解除读自旋锁*/
#define read_unlock(lock) \
    do {__raw_read_unlock(&(lock)->raw_lock); __release(lock); } while (0)
/*解除写自旋锁*/
#define write_unlock(lock) \
    do {__raw_write_unlock(&(lock)->raw_lock); __release(lock); } while (0)
/*解锁自旋锁并启用本地cpu中断*/
#define spin_unlock_irq(lock)			\
do {						\
	__raw_spin_unlock(&(lock)->raw_lock);	\
	__release(lock);			\
	local_irq_enable();			\
} while (0)
/*解除读自旋锁并启用本地cpu中断*/
#define read_unlock_irq(lock)			\
do {						\
	__raw_read_unlock(&(lock)->raw_lock);	\
	__release(lock);			\
	local_irq_enable();			\
} while (0)
/*解锁写自旋锁，并启用本地cpu中断*/
#define write_unlock_irq(lock)			\
do {						\
	__raw_write_unlock(&(lock)->raw_lock);	\
	__release(lock);			\
	local_irq_enable();			\
} while (0)
#endif
/*解除自旋锁，并启用本地cpu中断*/
#define spin_unlock_irqrestore(lock, flags) _spin_unlock_irqrestore(lock, flags)
/*解除自旋锁，并启用本地cpu软中断*/
#define spin_unlock_bh(lock)		_spin_unlock_bh(lock)
/*解除读锁，并启用本地cpu中断*/
#define read_unlock_irqrestore(lock, flags) 		_read_unlock_irqrestore(lock, flags)
/*解除读锁，并启用本地cpu软中断*/
#define read_unlock_bh(lock)		_read_unlock_bh(lock)
/*解除写锁，并启用本地cpu中断*/
#define write_unlock_irqrestore(lock, flags) _write_unlock_irqrestore(lock, flags)
/*解除写锁，并启用本地cpu软中断*/
#define write_unlock_bh(lock)		_write_unlock_bh(lock)
/*非阻塞式申请自旋锁，申请成功后，禁用本地cpu软中断，失败则直接返回*/
#define spin_trylock_bh(lock)	__cond_lock(lock, _spin_trylock_bh(lock))
/*非阻塞式申请自旋锁，申请成功后禁用本地cpu中断，失败后直接返回*/
#define spin_trylock_irq(lock) \
({ \
	local_irq_disable(); \
	spin_trylock(lock) ? 1 : ({ local_irq_enable(); 0;  }); \
})
/*非阻塞式申请自旋锁，申请成功后禁用本地cpu中断，失败直接返回*/
#define spin_trylock_irqsave(lock, flags) \
({ \
	local_irq_save(flags); \
	spin_trylock(lock) ?	1 : ({ local_irq_restore(flags); 0; }); \
})
/*非阻塞式申请写自旋锁，申请成功后禁用本地cpu中断，失败直接返回*/
#define write_trylock_irqsave(lock, flags) \
({ \
	local_irq_save(flags); \
	write_trylock(lock) ?	1 : ({ local_irq_restore(flags); 0; }); \
})

/*申请两个自旋锁，如果其中一把锁应该先申请，那就先申请，释放时顺序刚好相反*/
static inline void double_spin_lock(spinlock_t *l1, spinlock_t *l2,
				    bool l1_first)
	__acquires(l1)
	__acquires(l2)
{
	if (l1_first)
	{
		spin_lock(l1);
		spin_lock(l2);
	} else 
	{
		spin_lock(l2);
		spin_lock(l1);
	}
}

/*释放两把自旋锁，释放顺序与申请时相反*/
static inline void double_spin_unlock(spinlock_t *l1, spinlock_t *l2,
				      bool l1_taken_first)
	__releases(l1)
	__releases(l2)
{
	if (l1_taken_first) {
		spin_unlock(l2);
		spin_unlock(l1);
	} else {
		spin_unlock(l1);
		spin_unlock(l2);
	}
}

/*
 * Pull the atomic_t declaration:
 * (asm-mips/atomic.h needs above definitions)
 */
#include <asm/atomic.h>
/**
 * atomic_dec_and_lock - lock on reaching reference count zero
 * @atomic: the atomic counter
 * @lock: the spinlock in question
 */
extern int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock);
#define atomic_dec_and_lock(atomic, lock) \
		__cond_lock(lock, _atomic_dec_and_lock(atomic, lock))

/**
 * spin_can_lock - would spin_trylock() succeed?
 * @lock: the spinlock in question.
 */
#define spin_can_lock(lock)	(!spin_is_locked(lock))

#endif /* __LINUX_SPINLOCK_H */
