#ifndef __LINUX_CACHE_H
#define __LINUX_CACHE_H

#include <linux/kernel.h>
#include <asm/cache.h>

/*SMP_CACHE_BYTES����뵽���ݣ�ʹ֮�ڴ������ϵ�ṹ���ܹ����������L1���ٻ�����
���������ֺ���SMP��������������ϵͳҲ�ᶨ��ó�����*/

#ifndef L1_CACHE_ALIGN
/*L1�����ж���*/
#define L1_CACHE_ALIGN(x) ALIGN(x, L1_CACHE_BYTES)
#endif

#ifndef SMP_CACHE_BYTES
 /*L1�����ж���*/
 #define SMP_CACHE_BYTES L1_CACHE_BYTES
#endif

#ifndef __read_mostly
#define __read_mostly
#endif

#ifndef ____cacheline_aligned
 /*SMP_CACHE_BYTES����*/
#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))
#endif

#ifndef ____cacheline_aligned_in_smp
#ifdef CONFIG_SMP
 /*SMP_CACHE_BYTES����*/
#define ____cacheline_aligned_in_smp ____cacheline_aligned
#else
#define ____cacheline_aligned_in_smp
#endif /* CONFIG_SMP */
#endif

#ifndef __cacheline_aligned
 /*SMP_CACHE_BYTES���룬�������ݱ�����".data.cacheline_aligned"���ݶ���*/
#define __cacheline_aligned	 __attribute__((__aligned__(SMP_CACHE_BYTES),			\
		 __section__(".data.cacheline_aligned")))
#endif /* __cacheline_aligned */

#ifndef __cacheline_aligned_in_smp
#ifdef CONFIG_SMP
#define __cacheline_aligned_in_smp __cacheline_aligned
#else
#define __cacheline_aligned_in_smp
#endif /* CONFIG_SMP */
#endif

/*һЩ�ؼ����ݽṹ��Ҫ���Ķ��뷽ʽ����Щ�������ڲ���㻺���д�С��L3������
 ��С�ȣ����ַ�ʽ����Ķ����ڸ��߻���ʹ�÷���ʱ��ѵģ����˷��˸����ʱ�䣬��
 �������Ե�ʹ����Ҫ�����ؿ��ǡ�����ϵ�ṹ��asm/cache.h�ж���*/
#ifndef INTERNODE_CACHE_SHIFT
/*����ڲ����Ļ����ж�Ӧ���ֽ�ƫ��*/
#define INTERNODE_CACHE_SHIFT L1_CACHE_SHIFT
#endif

#if !defined(____cacheline_internodealigned_in_smp)
#if defined(CONFIG_SMP)
/*SMPϵͳ��û�ж������ڲ�������ƫ�ƶ���ʱ������ö��볤��Ϊ1UL���ƽ���ڲ����Ļ�����
��Ӧ���ֽں�ĳ���*/
#define ____cacheline_internodealigned_in_smp \
	__attribute__((__aligned__(1 << (INTERNODE_CACHE_SHIFT))))
#else
#define ____cacheline_internodealigned_in_smp
#endif
#endif

#endif /* __LINUX_CACHE_H */
