#ifndef _LINUX_MMAN_H
#define _LINUX_MMAN_H

#include <asm/mman.h>

/**/
#define MREMAP_MAYMOVE			1
/**/
#define MREMAP_FIXED			2

/*�ں˶�������������overcommit���ԣ������commit��˼����Ҫ����������ڴ棬overcommit
����˼�����ں���������ģ�ԶԶ���������ڴ������������ڴ档OVERCOMMIT_GUESS���ں˵�Ĭ
��overcommit���ԡ�������ģʽ�£��ر𼤽��ģ������������ڴ����뽫�ᱻ�ܾ����ں˻����
���ڴ��ܹ����������������һ�������ƣ����ֲ��ԼȲ�����Ҳ�����أ��Ƚ���ӹ*/
#define OVERCOMMIT_GUESS		0
/*OVERCOMMIT_ALWAYS����Ϊ������overcommit���ԣ����۽���������������ڴ棬ֻҪ������
�������������ڴ�ռ�Ĵ�С���ں��ܻ�ʹ��Ĵ�Ӧ���������ֲ����£������ڴ��������Ȼ����
�����ǵ���������ȱҳ���ں�Ϊ����������ڴ��ʱ�򣬻�ǳ��������OOM */
#define OVERCOMMIT_ALWAYS		1
/*OVERCOMMIT_NEVER����Ϊ�ϸ��һ�ֿ��������ڴ�overcommit�Ĳ��ԣ�������ģʽ�£��ں˻���
��Ĺ涨�����ڴ����������*/
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

/*���½�����ʹ�ó���ͳ����Ϣ*/
static inline void vm_unacct_memory(long pages)
{
	vm_acct_memory(-pages);
}

/*�Ż��ꡣ�ȼ���(x & bit1) ? bit2 : 0����������汾���죡bit1��bit2�����ǵ�����λ*/
#define _calc_vm_trans(x, bit1, bit2) 								\
  ((bit1) <= (bit2) ? ((x) & (bit1)) * ((bit2) / (bit1)) \
   : ((x) & (bit1)) / ((bit1) / (bit2)))

/*��mmap->prot�ϲ�Ϊ�ڲ�ʹ�õ�vm_flags*/
static inline unsigned long calc_vm_prot_bits(unsigned long prot)
{
	return _calc_vm_trans(prot, PROT_READ,  VM_READ ) |
	 			_calc_vm_trans(prot, PROT_WRITE, VM_WRITE) |
	 			_calc_vm_trans(prot, PROT_EXEC, VM_EXEC);
}

/*��mmap->flags���Ϊ�ڲ�ʹ�õ�vm_flags*/
static inline unsigned long calc_vm_flag_bits(unsigned long flags)
{
	return _calc_vm_trans(flags, MAP_GROWSDOWN,  VM_GROWSDOWN ) |
	       		_calc_vm_trans(flags, MAP_DENYWRITE,  VM_DENYWRITE ) |
	       		_calc_vm_trans(flags, MAP_EXECUTABLE, VM_EXECUTABLE) |
	      		_calc_vm_trans(flags, MAP_LOCKED,     VM_LOCKED    );
}
#endif /* __KERNEL__ */
#endif /* _LINUX_MMAN_H */
