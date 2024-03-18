#ifndef _LINUX_MMAN_H
#define _LINUX_MMAN_H

#include <asm/mman.h>

/**/
#define MREMAP_MAYMOVE			1
/**/
#define MREMAP_FIXED			2

/*内核定义了如下三个overcommit策略，这里的commit意思是需要申请的虚拟内存，overcommit
的意思是向内核申请过量的（远远超过物理内存容量）虚拟内存。OVERCOMMIT_GUESS是内核的默
认overcommit策略。在这种模式下，特别激进的，过量的虚拟内存申请将会被拒绝，内核会对虚
拟内存能够过量申请多少做出一定的限制，这种策略既不激进也不保守，比较中庸*/
#define OVERCOMMIT_GUESS		0
/*OVERCOMMIT_ALWAYS是最为激进的overcommit策略，无论进程申请多大的虚拟内存，只要不超过
整个进程虚拟内存空间的大小，内核总会痛快的答应。但是这种策略下，虚拟内存的申请虽然容易
，但是当进程遇到缺页，内核为其分配物理内存的时候，会非常容易造成OOM */
#define OVERCOMMIT_ALWAYS		1
/*OVERCOMMIT_NEVER是最为严格的一种控制虚拟内存overcommit的策略，在这种模式下，内核会严
格的规定虚拟内存的申请用量*/
#define OVERCOMMIT_NEVER		2

#ifdef __KERNEL__
#include <linux/mm.h>

#include <asm/atomic.h>

extern int sysctl_overcommit_memory;
extern int sysctl_overcommit_ratio;
extern atomic_t vm_committed_space;

#ifdef CONFIG_SMP
extern void vm_acct_memory(long pages);
#else
static inline void vm_acct_memory(long pages)
{
	atomic_add(pages, &vm_committed_space);
}
#endif

/*更新交换区使用长度统计信息*/
static inline void vm_unacct_memory(long pages)
{
	vm_acct_memory(-pages);
}

/*优化宏。等价与(x & bit1) ? bit2 : 0。但是这个版本更快！bit1和bit2必须是单独的位*/
#define _calc_vm_trans(x, bit1, bit2) 								\
  ((bit1) <= (bit2) ? ((x) & (bit1)) * ((bit2) / (bit1)) \
   : ((x) & (bit1)) / ((bit1) / (bit2)))

/*将mmap->prot合并为内部使用的vm_flags*/
static inline unsigned long calc_vm_prot_bits(unsigned long prot)
{
	return _calc_vm_trans(prot, PROT_READ,  VM_READ ) |
	 			_calc_vm_trans(prot, PROT_WRITE, VM_WRITE) |
	 			_calc_vm_trans(prot, PROT_EXEC, VM_EXEC);
}

/*将mmap->flags组合为内部使用的vm_flags*/
static inline unsigned long calc_vm_flag_bits(unsigned long flags)
{
	return _calc_vm_trans(flags, MAP_GROWSDOWN,  VM_GROWSDOWN ) |
	       		_calc_vm_trans(flags, MAP_DENYWRITE,  VM_DENYWRITE ) |
	       		_calc_vm_trans(flags, MAP_EXECUTABLE, VM_EXECUTABLE) |
	      		_calc_vm_trans(flags, MAP_LOCKED,     VM_LOCKED    );
}
#endif /* __KERNEL__ */
#endif /* _LINUX_MMAN_H */
