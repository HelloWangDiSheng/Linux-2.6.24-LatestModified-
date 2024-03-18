#ifndef __ASM_SPINLOCK_H
#define __ASM_SPINLOCK_H

#include <asm/atomic.h>
#include <asm/rwlock.h>
#include <asm/page.h>
#include <asm/processor.h>
#include <linux/compiler.h>

#ifdef CONFIG_PARAVIRT
#include <asm/paravirt.h>
#else
/*关中断字串*/
#define CLI_STRING	"cli"
/*开中断字串*/
#define STI_STRING	"sti"
#define CLI_STI_CLOBBERS
#define CLI_STI_INPUT_ARGS
#endif /* CONFIG_PARAVIRT */

/*
 * Your basic SMP spinlocks, allowing only a single CPU anywhere
 *
 * Simple spin lock operations.  There are two variants, one clears IRQ's
 * on the local processor, one does not.
 *
 * We make no fairness assumptions. They have a cost.
 *
 * (the type definitions are in asm/spinlock_types.h)
 */
 
/*自旋锁是否已被锁定*/
static inline int __raw_spin_is_locked(raw_spinlock_t *x)
{
	return *(volatile signed char *)(&(x)->slock) <= 0;
}

static inline void __raw_spin_lock(raw_spinlock_t *lock)
{
	asm volatile("\n1:\t"
		     LOCK_PREFIX " ; decb %0\n\t"
		     "jns 3f\n"
		     "2:\t"
		     "rep;nop\n\t"
		     "cmpb $0,%0\n\t"
		     "jle 2b\n\t"
		     "jmp 1b\n"
		     "3:\n\t"
		     : "+m" (lock->slock) : : "memory");
}

/*
 * It is easier for the lock validator if interrupts are not re-enabled
 * in the middle of a lock-acquire. This is a performance feature anyway
 * so we turn it off:
 *
 * NOTE: there's an irqs-on section here, which normally would have to be
 * irq-traced, but on CONFIG_TRACE_IRQFLAGS we never use this variant.
 */
#ifndef CONFIG_PROVE_LOCKING
static inline void __raw_spin_lock_flags(raw_spinlock_t *lock, unsigned long flags)
{
	asm volatile(
		"\n1:\t"
		LOCK_PREFIX " ; decb %[slock]\n\t"
		"jns 5f\n"
		"2:\t"
		"testl $0x200, %[flags]\n\t"
		"jz 4f\n\t"
		STI_STRING "\n"
		"3:\t"
		"rep;nop\n\t"
		"cmpb $0, %[slock]\n\t"
		"jle 3b\n\t"
		CLI_STRING "\n\t"
		"jmp 1b\n"
		"4:\t"
		"rep;nop\n\t"
		"cmpb $0, %[slock]\n\t"
		"jg 1b\n\t"
		"jmp 4b\n"
		"5:\n\t"
		: [slock] "+m" (lock->slock)
		: [flags] "r" (flags)
	 	  CLI_STI_INPUT_ARGS
		: "memory" CLI_STI_CLOBBERS);
}
#endif

static inline int __raw_spin_trylock(raw_spinlock_t *lock)
{
	char oldval;
	asm volatile(
		"xchgb %b0,%1"
		:"=q" (oldval), "+m" (lock->slock)
		:"0" (0) : "memory");
	return oldval > 0;
}

/*
 * __raw_spin_unlock based on writing $1 to the low byte.
 * This method works. Despite all the confusion.
 * (except on PPro SMP or if we are using OOSTORE, so we use xchgb there)
 * (PPro errata 66, 92)
 */

#if !defined(CONFIG_X86_OOSTORE) && !defined(CONFIG_X86_PPRO_FENCE)

static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
	asm volatile("movb $1,%0" : "+m" (lock->slock) :: "memory");
}

#else

static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
	char oldval = 1;

	asm volatile("xchgb %b0, %1"
		     : "=q" (oldval), "+m" (lock->slock)
		     : "0" (oldval) : "memory");
}

#endif

static inline void __raw_spin_unlock_wait(raw_spinlock_t *lock)
{
	while (__raw_spin_is_locked(lock))
		cpu_relax();
}

/*
 * Read-write spinlocks, allowing multiple readers
 * but only one writer.
 *
 * NOTE! it is quite common to have readers in interrupts
 * but no interrupt writers. For those circumstances we
 * can "mix" irq-safe locks - any writer needs to get a
 * irq-safe write-lock, but readers can get non-irqsafe
 * read-locks.
 *
 * On x86, we implement read-write locks as a 32-bit counter
 * with the high bit (sign) being the "contended" bit.
 *
 * The inline assembly is non-obvious. Think about it.
 *
 * Changed to use the same technique as rw semaphores.  See
 * semaphore.h for details.  -ben
 *
 * the helpers are in arch/i386/kernel/semaphore.c
 */

/*read_trylock()函数调用是否能成功*/
static inline int __raw_read_can_lock(raw_rwlock_t *x)
{
	return (int)(x)->lock > 0;
}

/*write_trylock()函数调用是否能成功*/
static inline int __raw_write_can_lock(raw_rwlock_t *x)
{
	return (x)->lock == RW_LOCK_BIAS;
}

static inline void __raw_read_lock(raw_rwlock_t *rw)
{
	asm volatile(LOCK_PREFIX " subl $1,(%0)\n\t"
		     "jns 1f\n"
		     "call __read_lock_failed\n\t"
		     "1:\n"
		     ::"a" (rw) : "memory");
}

static inline void __raw_write_lock(raw_rwlock_t *rw)
{
	asm volatile(LOCK_PREFIX " subl $" RW_LOCK_BIAS_STR ",(%0)\n\t"
		     "jz 1f\n"
		     "call __write_lock_failed\n\t"
		     "1:\n"
		     ::"a" (rw) : "memory");
}

/*原子操作，先预申请一个读锁，还有剩余可用锁，则申请成功返回，否则，说明无锁可用*/
static inline int __raw_read_trylock(raw_rwlock_t *lock)
{
	/*获取读写锁数目*/
	atomic_t *count = (atomic_t *)lock;
	/*预申请一个读锁*/
	atomic_dec(count);
	/*还有可用读锁，直接成功返回*/
	if (atomic_read(count) >= 0)
		return 1;
	/*无可用读锁，退还申请*/
	atomic_inc(count);
	return 0;
}

static inline int __raw_write_trylock(raw_rwlock_t *lock)
{
	atomic_t *count = (atomic_t *)lock;
	if (atomic_sub_and_test(RW_LOCK_BIAS, count))
		return 1;
	atomic_add(RW_LOCK_BIAS, count);
	return 0;
}

static inline void __raw_read_unlock(raw_rwlock_t *rw)
{
	asm volatile(LOCK_PREFIX "incl %0" :"+m" (rw->lock) : : "memory");
}

static inline void __raw_write_unlock(raw_rwlock_t *rw)
{
	asm volatile(LOCK_PREFIX "addl $" RW_LOCK_BIAS_STR ", %0"
				 : "+m" (rw->lock) : : "memory");
}

#define _raw_spin_relax(lock)	cpu_relax()
#define _raw_read_relax(lock)	cpu_relax()
#define _raw_write_relax(lock)	cpu_relax()

#endif /* __ASM_SPINLOCK_H */
