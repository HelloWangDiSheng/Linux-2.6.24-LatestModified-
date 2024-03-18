#ifndef _ARCH_X86_CACHE_H
#define _ARCH_X86_CACHE_H

/*����L1�����д�С*/
#define L1_CACHE_SHIFT	(CONFIG_X86_L1_CACHE_SHIFT)
#define L1_CACHE_BYTES	(1 << L1_CACHE_SHIFT)

/*���ú궨��ı���������".data.read_mostly"���ݶ���*/
#define __read_mostly __attribute__((__section__(".data.read_mostly")))

#ifdef CONFIG_X86_VSMP
/*vSMP�ڲ���㻺���д�СΪ4K*/
#define INTERNODE_CACHE_SHIFT (12)
#ifdef CONFIG_SMP
/*�����ͱ���4k��С�����ұ���������".data.page_aligned"���ݶ���*/
#define __cacheline_aligned_in_smp					\
	__attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))	\
	__attribute__((__section__(".data.page_aligned")))
#endif
#endif

#endif
