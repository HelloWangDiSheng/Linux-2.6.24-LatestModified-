#ifndef __LINUX_CPUMASK_H
#define __LINUX_CPUMASK_H

/*
 * Cpumasks provide a bitmap suitable for representing the
 * set of CPU's in a system, one bit position per CPU number.
 *
 * See detailed comments in the file linux/bitmap.h describing the
 * data type on which these cpumasks are based.
 *
 * For details of cpumask_scnprintf() and cpumask_parse_user(),
 * see bitmap_scnprintf() and bitmap_parse_user() in lib/bitmap.c.
 * For details of cpulist_scnprintf() and cpulist_parse(), see
 * bitmap_scnlistprintf() and bitmap_parselist(), also in bitmap.c.
 * For details of cpu_remap(), see bitmap_bitremap in lib/bitmap.c
 * For details of cpus_remap(), see bitmap_remap in lib/bitmap.c.
 *
 * The available cpumask operations are:
 *
 * void cpu_set(cpu, mask)		turn on bit 'cpu' in mask
 * void cpu_clear(cpu, mask)		turn off bit 'cpu' in mask
 * void cpus_setall(mask)		set all bits
 * void cpus_clear(mask)		clear all bits
 * int cpu_isset(cpu, mask)		true iff bit 'cpu' set in mask
 * int cpu_test_and_set(cpu, mask)	test and set bit 'cpu' in mask
 *
 * void cpus_and(dst, src1, src2)	dst = src1 & src2  [intersection]
 * void cpus_or(dst, src1, src2)	dst = src1 | src2  [union]
 * void cpus_xor(dst, src1, src2)	dst = src1 ^ src2
 * void cpus_andnot(dst, src1, src2)	dst = src1 & ~src2
 * void cpus_complement(dst, src)	dst = ~src
 *
 * int cpus_equal(mask1, mask2)		Does mask1 == mask2?
 * int cpus_intersects(mask1, mask2)	Do mask1 and mask2 intersect?
 * int cpus_subset(mask1, mask2)	Is mask1 a subset of mask2?
 * int cpus_empty(mask)			Is mask empty (no bits sets)?
 * int cpus_full(mask)			Is mask full (all bits sets)?
 * int cpus_weight(mask)		Hamming weigh - number of set bits
 *
 * void cpus_shift_right(dst, src, n)	Shift right
 * void cpus_shift_left(dst, src, n)	Shift left
 *
 * int first_cpu(mask)			Number lowest set bit, or NR_CPUS
 * int next_cpu(cpu, mask)		Next cpu past 'cpu', or NR_CPUS
 *
 * cpumask_t cpumask_of_cpu(cpu)	Return cpumask with bit 'cpu' set
 * CPU_MASK_ALL				Initializer - all bits set
 * CPU_MASK_NONE			Initializer - no bits set
 * unsigned long *cpus_addr(mask)	Array of unsigned long's in mask
 *
 * int cpumask_scnprintf(buf, len, mask) Format cpumask for printing
 * int cpumask_parse_user(ubuf, ulen, mask)	Parse ascii string as cpumask
 * int cpulist_scnprintf(buf, len, mask) Format cpumask as list for printing
 * int cpulist_parse(buf, map)		Parse ascii string as cpulist
 * int cpu_remap(oldbit, old, new)	newbit = map(old, new)(oldbit)
 * int cpus_remap(dst, src, old, new)	*dst = map(old, new)(src)
 *
 * for_each_cpu_mask(cpu, mask)		for-loop cpu over mask
 *
 * int num_online_cpus()		Number of online CPUs
 * int num_possible_cpus()		Number of all possible CPUs
 * int num_present_cpus()		Number of present CPUs
 *
 * int cpu_online(cpu)			Is some cpu online?
 * int cpu_possible(cpu)		Is some cpu possible?
 * int cpu_present(cpu)			Is some cpu present (can schedule)?
 *
 * int any_online_cpu(mask)		First online cpu in mask
 *
 * for_each_possible_cpu(cpu)		for-loop cpu over cpu_possible_map
 * for_each_online_cpu(cpu)		for-loop cpu over cpu_online_map
 * for_each_present_cpu(cpu)		for-loop cpu over cpu_present_map
 *
 * Subtlety:
 * 1) The 'type-checked' form of cpu_isset() causes gcc (3.3.2, anyway)
 *    to generate slightly worse code.  Note for example the additional
 *    40 lines of assembly code compiling the "for each possible cpu"
 *    loops buried in the disk_stat_read() macros calls when compiling
 *    drivers/block/genhd.c (arch i386, CONFIG_SMP=y).  So use a simple
 *    one-line #define for cpu_isset(), instead of wrapping an inline
 *    inside a macro, the way we do the other calls.
 */

#include <linux/kernel.h>
#include <linux/threads.h>
#include <linux/bitmap.h>
/*����һ������NR_CPUS��Чλ��λͼ*/
typedef struct
{
	DECLARE_BITMAP(bits, NR_CPUS);
}cpumask_t;
extern cpumask_t _unused_cpumask_arg_;

/*��λͼ�ж�Ӧ��cpu���λ��λ*/
#define cpu_set(cpu, dst) __cpu_set((cpu), &(dst))
static inline void __cpu_set(int cpu, volatile cpumask_t *dstp)
{
	set_bit(cpu, dstp->bits);
}
/*��λͼ�ж�Ӧ��cpu���Ϊ��0*/
#define cpu_clear(cpu, dst) __cpu_clear((cpu), &(dst))
static inline void __cpu_clear(int cpu, volatile cpumask_t *dstp)
{
	clear_bit(cpu, dstp->bits);
}
/*��λͼ����Ч��NR_CPUSλȫ����λ*/
#define cpus_setall(dst) __cpus_setall(&(dst), NR_CPUS)
static inline void __cpus_setall(cpumask_t *dstp, int nbits)
{
	bitmap_fill(dstp->bits, nbits);
}
/*��λͼ����Ч��NR_CPUSλȫ����0*/
#define cpus_clear(dst) __cpus_clear(&(dst), NR_CPUS)
static inline void __cpus_clear(cpumask_t *dstp, int nbits)
{
	bitmap_zero(dstp->bits, nbits);
}

/*����λͼ�ж�Ӧcpu��ŵ�λ�Ƿ���λ���Ǿ�̬�������ͼ��*/
#define cpu_isset(cpu, cpumask) test_bit((cpu), (cpumask).bits)
/*���Բ�����λͼ�ж�Ӧcpu��ŵ�λ�����ظ�λ֮ǰ��״̬*/
#define cpu_test_and_set(cpu, cpumask) __cpu_test_and_set((cpu), &(cpumask))
static inline int __cpu_test_and_set(int cpu, cpumask_t *addr)
{
	return test_and_set_bit(cpu, addr->bits);
}
/*��src1��src2���ߵ�ǰNR_CPUSλ�Ľ���������dst��*/
#define cpus_and(dst, src1, src2) __cpus_and(&(dst), &(src1), &(src2), NR_CPUS)
static inline void __cpus_and(cpumask_t *dstp, const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	bitmap_and(dstp->bits, src1p->bits, src2p->bits, nbits);
}
/*��src1��src2���ߵ�ǰNR_CPUSλ�Ĳ���������dst��*/
#define cpus_or(dst, src1, src2) __cpus_or(&(dst), &(src1), &(src2), NR_CPUS)
static inline void __cpus_or(cpumask_t *dstp, const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	bitmap_or(dstp->bits, src1p->bits, src2p->bits, nbits);
}
/*��src1��src2���ߵ�ǰNR_CPUSλ����򱣴���dst��*/
#define cpus_xor(dst, src1, src2) __cpus_xor(&(dst), &(src1), &(src2), NR_CPUS)
static inline void __cpus_xor(cpumask_t *dstp, const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	bitmap_xor(dstp->bits, src1p->bits, src2p->bits, nbits);
}
/*��src1�зǳ�����src2�е�ǰNR_CPUSλ��src1���Ӽ�������dst��*/

#define cpus_andnot(dst, src1, src2) \
				__cpus_andnot(&(dst), &(src1), &(src2), NR_CPUS)
static inline void __cpus_andnot(cpumask_t *dstp, const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	bitmap_andnot(dstp->bits, src1p->bits, src2p->bits, nbits);
}

/*����src1��dst��ǰNR_CPUS�󲹼�*/
#define cpus_complement(dst, src) __cpus_complement(&(dst), &(src), NR_CPUS)
static inline void __cpus_complement(cpumask_t *dstp,
					const cpumask_t *srcp, int nbits)
{
	bitmap_complement(dstp->bits, srcp->bits, nbits);
}

/*λͼsrc1��src2���ߵ�ǰNR_CPUSλ�Ƿ���ͬ*/
#define cpus_equal(src1, src2) __cpus_equal(&(src1), &(src2), NR_CPUS)
static inline int __cpus_equal(const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	return bitmap_equal(src1p->bits, src2p->bits, nbits);
}
/*λͼsrc1��src2���ߵ�ǰNR_CPUSλ�Ƿ��н�����ͬλֵͬ��*/
#define cpus_intersects(src1, src2) __cpus_intersects(&(src1), &(src2), NR_CPUS)
static inline int __cpus_intersects(const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	return bitmap_intersects(src1p->bits, src2p->bits, nbits);
}
/*λͼsrc1�Ƿ���src2�Ĳ������Ƚ�����ǰNR_CPUSλ��*/
#define cpus_subset(src1, src2) __cpus_subset(&(src1), &(src2), NR_CPUS)
static inline int __cpus_subset(const cpumask_t *src1p,
					const cpumask_t *src2p, int nbits)
{
	return bitmap_subset(src1p->bits, src2p->bits, nbits);
}
/*λͼsrcǰNR_CPUSλ�Ƿ�δ��λ*/
#define cpus_empty(src) __cpus_empty(&(src), NR_CPUS)
static inline int __cpus_empty(const cpumask_t *srcp, int nbits)
{
	return bitmap_empty(srcp->bits, nbits);
}
/*λͼcpumaskǰNR_CPUSλ�Ƿ�����λ*/
#define cpus_full(cpumask) __cpus_full(&(cpumask), NR_CPUS)
static inline int __cpus_full(const cpumask_t *srcp, int nbits)
{
	return bitmap_full(srcp->bits, nbits);
}

/*λͼcpumaskǰNR_CPUSλ������λ����Ŀ*/
#define cpus_weight(cpumask) __cpus_weight(&(cpumask), NR_CPUS)
static inline int __cpus_weight(const cpumask_t *srcp, int nbits)
{
	return bitmap_weight(srcp->bits, nbits);
}

/**/
#define cpus_shift_right(dst, src, n) \
			__cpus_shift_right(&(dst), &(src), (n), NR_CPUS)
static inline void __cpus_shift_right(cpumask_t *dstp,
					const cpumask_t *srcp, int n, int nbits)
{
	bitmap_shift_right(dstp->bits, srcp->bits, n, nbits);
}
/**/
#define cpus_shift_left(dst, src, n) \
			__cpus_shift_left(&(dst), &(src), (n), NR_CPUS)
static inline void __cpus_shift_left(cpumask_t *dstp,
					const cpumask_t *srcp, int n, int nbits)
{
	bitmap_shift_left(dstp->bits, srcp->bits, n, nbits);
}

#ifdef CONFIG_SMP
int __first_cpu(const cpumask_t *srcp);
/*��ȡcpuλͼ�е�һ������λ��cpu���*/
#define first_cpu(src) __first_cpu(&(src))
int __next_cpu(int n, const cpumask_t *srcp);
/*��ȡcpuλͼ�е�n������Ժ��һ������λ��cpu�ı��*/
#define next_cpu(n, src) __next_cpu((n), &(src))
#else
#define first_cpu(src)		0
#define next_cpu(n, src)	1
#endif
/*��λͼ��cpu��ŵ�λ��λ�����λͼ����Чλ���Ȳ�����һ���ֳ��Ļ�����ֱ�Ӱ��޷������δ���
��Ӧλ�����������λͼ���ٽ�λͼ�ж�Ӧ��cpu���λ��λ*/
#define cpumask_of_cpu(cpu)						\
({												\
	typeof(_unused_cpumask_arg_) m;				\/*����һ��λͼm*/\
	if (sizeof(m) == sizeof(unsigned long)) 	\
	{											\
		m.bits[0] = 1UL<<(cpu);					\/*λͼ����Чλ������һ���ֳ�ʱֱ�Ӱ�unsigned long���ʹ���*/\
	} 											\
	else {										\
		cpus_clear(m);							\/*�������λͼ*/\
		cpu_set((cpu), m);						\/*��λͼ��cpu��Ŷ�Ӧ��λ��λ*/\
	}											\
	m;											\
})
/*��NR_CPUS������ֳ�����Чλ��λ*/
#define CPU_MASK_LAST_WORD BITMAP_LAST_WORD_MASK(NR_CPUS)

/**/

#if NR_CPUS <= BITS_PER_LONG
/*λͼ��Чλ���Ȳ������ֳ�ʱ�����޷������δ���*/
#define CPU_MASK_ALL											\
(cpumask_t) { {													\
	[BITS_TO_LONGS(NR_CPUS)-1] = CPU_MASK_LAST_WORD				\
} }
#else
/*λͼ��Чλ���ȴ����ֳ�ʱ����ǰ���ֵ����޷��ų����δ�������β��������λ����*/
#define CPU_MASK_ALL											\
(cpumask_t) { {													\
	[0 ... BITS_TO_LONGS(NR_CPUS)-2] = ~0UL,					\/*����ǰ�������޷����������鴦��*/
	[BITS_TO_LONGS(NR_CPUS)-1] = CPU_MASK_LAST_WORD				\/*���²���λ����*/
} }
#endif
/*cpuλͼ��0*/
#define CPU_MASK_NONE								\
(cpumask_t) { {										\
	[0 ... BITS_TO_LONGS(NR_CPUS)-1] =  0UL			\
} }
/*cpuλͼ�е�һ��cpu��Ӧλ��λ*/
#define CPU_MASK_CPU0		\
(cpumask_t) { {				\
	[0] =  1UL				\
} }
/*��ȡcpuλͼ*/
#define cpus_addr(src) ((src).bits)

#define cpumask_scnprintf(buf, len, src) \
			__cpumask_scnprintf((buf), (len), &(src), NR_CPUS)
static inline int __cpumask_scnprintf(char *buf, int len,
					const cpumask_t *srcp, int nbits)
{
	return bitmap_scnprintf(buf, len, srcp->bits, nbits);
}

#define cpumask_parse_user(ubuf, ulen, dst) \
			__cpumask_parse_user((ubuf), (ulen), &(dst), NR_CPUS)
static inline int __cpumask_parse_user(const char __user *buf, int len,
					cpumask_t *dstp, int nbits)
{
	return bitmap_parse_user(buf, len, dstp->bits, nbits);
}

#define cpulist_scnprintf(buf, len, src) \
			__cpulist_scnprintf((buf), (len), &(src), NR_CPUS)
static inline int __cpulist_scnprintf(char *buf, int len,
					const cpumask_t *srcp, int nbits)
{
	return bitmap_scnlistprintf(buf, len, srcp->bits, nbits);
}

#define cpulist_parse(buf, dst) __cpulist_parse((buf), &(dst), NR_CPUS)
static inline int __cpulist_parse(const char *buf, cpumask_t *dstp, int nbits)
{
	return bitmap_parselist(buf, dstp->bits, nbits);
}

#define cpu_remap(oldbit, old, new) \
		__cpu_remap((oldbit), &(old), &(new), NR_CPUS)
static inline int __cpu_remap(int oldbit,
		const cpumask_t *oldp, const cpumask_t *newp, int nbits)
{
	return bitmap_bitremap(oldbit, oldp->bits, newp->bits, nbits);
}

#define cpus_remap(dst, src, old, new) \
		__cpus_remap(&(dst), &(src), &(old), &(new), NR_CPUS)
static inline void __cpus_remap(cpumask_t *dstp, const cpumask_t *srcp,
		const cpumask_t *oldp, const cpumask_t *newp, int nbits)
{
	bitmap_remap(dstp->bits, srcp->bits, oldp->bits, newp->bits, nbits);
}

/*����cpuλͼ������cpu*/
#if NR_CPUS > 1
#define for_each_cpu_mask(cpu, mask)\
	for ((cpu) = first_cpu(mask); (cpu) < NR_CPUS; (cpu) = next_cpu((cpu), (mask)))
#else /* NR_CPUS == 1 */
#define for_each_cpu_mask(cpu, mask) for ((cpu) = 0; (cpu) < 1; (cpu)++, (void)mask)
#endif /* NR_CPUS */

/*
 * The following particular system cpumasks and operations manage
 * possible, present and online cpus.  Each of them is a fixed size
 * bitmap of size NR_CPUS.
 *
 *  #ifdef CONFIG_HOTPLUG_CPU
 *     cpu_possible_map - has bit 'cpu' set iff cpu is populatable
 *     cpu_present_map  - has bit 'cpu' set iff cpu is populated
 *     cpu_online_map   - has bit 'cpu' set iff cpu available to scheduler
 *  #else
 *     cpu_possible_map - has bit 'cpu' set iff cpu is populated
 *     cpu_present_map  - copy of cpu_possible_map
 *     cpu_online_map   - has bit 'cpu' set iff cpu available to scheduler
 *  #endif
 *
 *  In either case, NR_CPUS is fixed at compile time, as the static
 *  size of these bitmaps.  The cpu_possible_map is fixed at boot
 *  time, as the set of CPU id's that it is possible might ever
 *  be plugged in at anytime during the life of that system boot.
 *  The cpu_present_map is dynamic(*), representing which CPUs
 *  are currently plugged in.  And cpu_online_map is the dynamic
 *  subset of cpu_present_map, indicating those CPUs available
 *  for scheduling.
 *
 *  If HOTPLUG is enabled, then cpu_possible_map is forced to have
 *  all NR_CPUS bits set, otherwise it is just the set of CPUs that
 *  ACPI reports present at boot.
 *
 *  If HOTPLUG is enabled, then cpu_present_map varies dynamically,
 *  depending on what ACPI reports as currently plugged in, otherwise
 *  cpu_present_map is just a copy of cpu_possible_map.
 *
 *  (*) Well, cpu_present_map is dynamic in the hotplug case.  If not
 *      hotplug, it's a copy of cpu_possible_map, hence fixed at boot.
 *
 * Subtleties:
 * 1) UP arch's (NR_CPUS == 1, CONFIG_SMP not defined) hardcode
 *    assumption that their single CPU is online.  The UP
 *    cpu_{online,possible,present}_maps are placebos.  Changing them
 *    will have no useful affect on the following num_*_cpus()
 *    and cpu_*() macros in the UP case.  This ugliness is a UP
 *    optimization - don't waste any instructions or memory references
 *    asking if you're online or how many CPUs there are if there is
 *    only one CPU.
 * 2) Most SMP arch's #define some of these maps to be some
 *    other map specific to that arch.  Therefore, the following
 *    must be #define macros, not inlines.  To see why, examine
 *    the assembly code produced by the following.  Note that
 *    set1() writes phys_x_map, but set2() writes x_map:
 *        int x_map, phys_x_map;
 *        #define set1(a) x_map = a
 *        inline void set2(int a) { x_map = a; }
 *        #define x_map phys_x_map
 *        main(){ set1(3); set2(5); }
 */
/*cpu�Ȳ�ι���*/
/*cpu���ڵ���û�����뵽�ں˹����cpuλͼ*/
extern cpumask_t cpu_possible_map;
/*���Ա����ȵ�cpu��Ӧ��λͼ*/
extern cpumask_t cpu_online_map;
/*�Ѿ����ں˹����cpu��Ӧ��λͼ*/
extern cpumask_t cpu_present_map;

#if NR_CPUS > 1
/*�ɵ��ȵ�cpu��Ŀ*/
#define num_online_cpus()	cpus_weight(cpu_online_map)
/*���ڵ���û�б��ں˹����cpu��Ŀ*/
#define num_possible_cpus()	cpus_weight(cpu_possible_map)
/*�ѱ��ں˹����cpu��Ŀ*/
#define num_present_cpus()	cpus_weight(cpu_present_map)
/*ָ����cpu�Ƿ�ɹ�����*/
#define cpu_online(cpu)		cpu_isset((cpu), cpu_online_map)
/*ָ����cpu�Ƿ�û�б��ں˹���*/
#define cpu_possible(cpu)	cpu_isset((cpu), cpu_possible_map)
/*ָ����cpu�Ƿ��ѱ��ں˹���*/
#define cpu_present(cpu)	cpu_isset((cpu), cpu_present_map)
#else
#define num_online_cpus()	1
#define num_possible_cpus()	1
#define num_present_cpus()	1
#define cpu_online(cpu)		((cpu) == 0)
#define cpu_possible(cpu)	((cpu) == 0)
#define cpu_present(cpu)	((cpu) == 0)
#endif
/*ָ����cpu�Ƿ񻹲��ܱ�����*/
#define cpu_is_offline(cpu)	unlikely(!cpu_online(cpu))

#ifdef CONFIG_SMP
extern int nr_cpu_ids;
#define any_online_cpu(mask) __any_online_cpu(&(mask))
int __any_online_cpu(const cpumask_t *mask);
#else
#define nr_cpu_ids			1
#define any_online_cpu(mask)		0
#endif
/*�������л�û�б��ں˹����cpu*/
#define for_each_possible_cpu(cpu)  for_each_cpu_mask((cpu), cpu_possible_map)
/*�������пɱ����ȵ�cpu*/
#define for_each_online_cpu(cpu)  for_each_cpu_mask((cpu), cpu_online_map)
/*���������ѱ��ں˹����cpu*/
#define for_each_present_cpu(cpu) for_each_cpu_mask((cpu), cpu_present_map)

#endif /* __LINUX_CPUMASK_H */
