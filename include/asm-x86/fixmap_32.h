/*����ʱ�����ڴ����*/
#ifndef _ASM_FIXMAP_H
#define _ASM_FIXMAP_H

/*vmallocz.c��vsyscall.ld.sʹ�á���VMALLOC�ڴ���͹̶�ӳ���ڴ�����ʼ��֮��Ԥ��һ����ҳ*/
extern unsigned long __FIXADDR_TOP;
/*   Linux�е�vdso��Virtual Dynamic Shared Object����һ������Ķ�̬������������û���
����ں˿ռ�֮���ṩ��һ�ָ�Ч�Ľӿڡ�vdso���Ƶ�Ŀ���Ǽ����û��ռ�������ں�֮��Ƶ����
�������л����������ϵͳ���ܡ�
	 ��vDSO�������⶯̬���������һ��С�͵Ĺ���⣬�ں˻��Զ�����ӳ�䵽�����û��ռ�Ӧ��
����ĵ�ַ�ռ��С�Ӧ�ó���ͨ�������ע��Щϸ�ڣ���ΪvDSO���C����á�����������������
��ʽ���룬ʹ�ñ�׼��������C��Ḻ��ʹ��ͨ��vDSO���õ��κι��ܡ�
	 vDSO�Ĵ�����Ϊʲô���ں��ṩ��һЩϵͳ���ã��û��ռ���뾭��ʹ����Щ���ã���������
Щ���ÿ��������������ܡ���������ڵ��õ�Ƶ�ʣ��������ڴ��û��ռ��˳��������ں���������
�������л�������
	 vdso����һ���ض��ĺ�������Щ�������û��ռ���ִ�У�����ʵ�������ں��ṩ�ġ��û��ռ�
�������ͨ��������Щ����������һЩϵͳ���ܣ������������ں�̬��vdso��һ����Ҫ��;��ʵ��
ϵͳ���õĿ���·�������û��ռ����ִ��ϵͳ����ʱ��ͨ����Ҫ����һ���������л���������Ȩ
���û�̬�л����ں�̬��Ȼ����ĳЩϵͳ�����Ƿǳ�Ƶ���ҿ�����С�ģ������������л��Ŀ�����
�ܻ��Ϊ����ƿ����vdso�ṩ��һ������·����ͨ�����û��ռ���ִ���ض���ϵͳ���ú���������
�˲���Ҫ���������л����Ӷ������ϵͳ���õ����ܡ�
	 ��Linux�У�vdsoͨ����linux-vdso.so.X����ʽ������/proc/self/maps�У����ұ�ӳ�䵽ÿ��
���̵ĵ�ַ�ռ��С��������û��ռ�������ֱ�ӵ���vdso�еĺ�������������ʽ���غ�����vdso
�⡣*/
#define FIXADDR_USER_START     __fix_to_virt(FIX_VDSO)
#define FIXADDR_USER_END       __fix_to_virt(FIX_VDSO - 1)

#ifndef __ASSEMBLY__
#include <linux/kernel.h>
#include <asm/acpi.h>
#include <asm/apicdef.h>
#include <asm/page.h>
#ifdef CONFIG_HIGHMEM
#include <linux/threads.h>
#include <asm/kmap_types.h>
#endif

/**/
/*
 * Here we define all the compile-time 'special' virtual
 * addresses. The point is to have a constant address at
 * compile time, but to set the physical address only
 * in the boot process. We allocate these special addresses
 * from the end of virtual memory (0xfffff000) backwards.
 * Also this lets us do fail-safe vmalloc(), we
 * can guarantee that these special addresses and
 * vmalloc()-ed addresses never overlap.
 *
 * these 'compile-time allocated' memory buffers are
 * fixed-size 4k pages. (or larger if used with an increment
 * highger than 1) use fixmap_set(idx,phys) to associate
 * physical memory with fixmap indices.
 *
 * TLB entries of such buffers will not be flushed across
 * task switches.
 */
/*��ÿ���̶�ӳ���ַ���ᴴ��һ�����������뵽fixed_addressesö��ֵ�б���*/
enum fixed_addresses
{
	FIX_HOLE,
	/**/
	FIX_VDSO,
	FIX_DBGP_BASE,
	FIX_EARLYCON_MEM_BASE,
#ifdef CONFIG_X86_LOCAL_APIC
	FIX_APIC_BASE,	/* local (CPU) APIC) -- required for SMP or not */
#endif
#ifdef CONFIG_X86_IO_APIC
	FIX_IO_APIC_BASE_0,
	FIX_IO_APIC_BASE_END = FIX_IO_APIC_BASE_0 + MAX_IO_APICS-1,
#endif
#ifdef CONFIG_X86_VISWS_APIC
	FIX_CO_CPU,	/* Cobalt timer */
	FIX_CO_APIC,	/* Cobalt APIC Redirection Table */ 
	FIX_LI_PCIA,	/* Lithium PCI Bridge A */
	FIX_LI_PCIB,	/* Lithium PCI Bridge B */
#endif
#ifdef CONFIG_X86_F00F_BUG
	FIX_F00F_IDT,	/* Virtual mapping for IDT */
#endif
#ifdef CONFIG_X86_CYCLONE_TIMER
	FIX_CYCLONE_TIMER, /*cyclone timer register*/
#endif 
#ifdef CONFIG_HIGHMEM
	FIX_KMAP_BEGIN,	/* reserved pte's for temporary kernel mappings */
	FIX_KMAP_END = FIX_KMAP_BEGIN+(KM_TYPE_NR*NR_CPUS)-1,
#endif
#ifdef CONFIG_ACPI
	FIX_ACPI_BEGIN,
	FIX_ACPI_END = FIX_ACPI_BEGIN + FIX_ACPI_PAGES - 1,
#endif
#ifdef CONFIG_PCI_MMCONFIG
	FIX_PCIE_MCFG,
#endif
#ifdef CONFIG_PARAVIRT
	FIX_PARAVIRT_BOOTMAP,
#endif
	__end_of_permanent_fixed_addresses,
	/* temporary boot-time mappings, used before ioremap() is functional */
#define NR_FIX_BTMAPS	16
	FIX_BTMAP_END = __end_of_permanent_fixed_addresses,
	FIX_BTMAP_BEGIN = FIX_BTMAP_END + NR_FIX_BTMAPS - 1,
	FIX_WP_TEST,
	__end_of_fixed_addresses
};

extern void __set_fixmap (enum fixed_addresses idx,	unsigned long phys, pgprot_t flags);
extern void reserve_top_address(unsigned long reserve);
/*�����̶�ӳ�䳣���������ڴ�ҳ֮���ӳ���ϵ*/
#define set_fixmap(idx, phys) __set_fixmap(idx, phys, PAGE_KERNEL)
/*��ЩӲ����Ҫ�����������ֱ�����ù̶�ӳ���ϵ*/
#define set_fixmap_nocache(idx, phys) __set_fixmap(idx, phys, PAGE_KERNEL_NOCACHE)
/*����̶�ӳ��������ָ���̶�ӳ�䳣���������ڴ�ҳ֮���ӳ���ϵ*/
#define clear_fixmap(idx) __set_fixmap(idx, 0, __pgprot(0))
/*�̶�ӳ���ڴ���Ľ�����ַ*/
#define FIXADDR_TOP	((unsigned long)__FIXADDR_TOP)
/*�̶�ӳ���ڴ���ĳ���*/
#define __FIXADDR_SIZE	(__end_of_permanent_fixed_addresses << PAGE_SHIFT)
/**/
#define __FIXADDR_BOOT_SIZE	(__end_of_fixed_addresses << PAGE_SHIFT)
/*�̶�ӳ���ڴ������ʼ��ַ�����ڴ����еĵ�ַָ�������ڴ��е����λ�ã���������ں�
�ռ���ʼ��������ӳ�䣬�ڸ�ӳ�������ڲ��������ַ�������ַ֮��Ĺ�������Ԥ��ģ�����
�������ɶ��壬������֮���ܸı䡣�̶�ӳ���ڴ����һֱ���쵽�����ַ�ռ䶥�ˣ����һ
��ҳ�����ڸ����򣩡��̶�ӳ���ַ���ŵ����ڣ��ڱ���ʱ�Դ����ַ�Ĵ��������ڳ������ں�
һ������Ϊ������������ַ�������ַ�Ľ�������ָͨ��Ҫ�졣�ں˻�ȷ�����������л��ڼ�
����Ӧ�ڹ̶�ӳ���ҳ������TLBˢ��������ڷ��ʹ̶�ӳ����ڴ�ʱ������ͨ��TLB���ٻ�
��ȡ�ö�Ӧ�������ַ��
     �ڹ̶�ӳ�����е������ڴ��ַ��������ӳ�䵽�����ڴ�ĸ߶˵�ַ�ϣ������붯̬ӳ����
�Լ�����ӳ������ͬ���ǣ��ڹ̶�ӳ�����������ַ�ǹ̶��ģ�����ӳ��������ַ�ǿ��Ըı�
�ġ�Ҳ����˵����Щ�����ַ�ڱ����ʱ��͹̶������ˣ������ں����������б�ȷ���ģ�����
Щ�����ַ��Ӧ�������ַ���ǹ̶��ġ����ù̶������ַ�ĺô������൱��һ��ָ�볣������
����ֵ�ڱ���ʱȷ������ָ�������ַ����������ַ���̶������൱��һ��ָ�������
	 ��Ϊʲô���й̶�ӳ����������� ?  ���磺���ں˵����������У���Щģ����Ҫʹ��������
�沢ӳ�䵽ָ���������ַ�ϣ�������Щģ��Ҳû�а취�ȴ��������ڴ����ģ���ʼ��֮����
���е�ַӳ�䡣��ˣ��ں˹̶�������һЩ�����ַ����Щ��ַ�й̶�����;��ʹ�øõ�ַ��ģ
���ڳ�ʼ����ʱ�򣬽���Щ�̶�����������ַӳ�䵽ָ���������ַ��ȥ
*/
#define FIXADDR_START		(FIXADDR_TOP - __FIXADDR_SIZE)
/**/
#define FIXADDR_BOOT_START	(FIXADDR_TOP - __FIXADDR_BOOT_SIZE)
/*��ȡ�̶�ӳ���ڴ�����ָ���Ĺ̶�ӳ�䳣����Ӧ�������ڴ��ַ*/
#define __fix_to_virt(x)	(FIXADDR_TOP - ((x) << PAGE_SHIFT))
/*��ȡ�̶�ӳ���ڴ����е������ַ��Ӧ�Ĺ̶�ӳ�䳣����enum fixed_addressesö��ֵ��*/
#define __virt_to_fix(x)	((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)
/*û�ж����α���������ں������ڼ䣬������̶ֹ�ӳ�䳣������enum fixed_addresses
ʱ��������øú��������±������*/
extern void __this_fixmap_does_not_exist(void);

/*
 * 'index to address' translation. If anyone tries to use the idx
 * directly without tranlation, we catch the bug with a NULL-deference
 * kernel oops. Illegal ranges of incoming indices are caught too.
 */
/*����̶�ӳ�䳣����enum fixed_addressesö��ֵ���������ַ*/
static __always_inline unsigned long fix_to_virt(const unsigned int idx)
{
	/*
	 * this branch gets completely eliminated after inlining,
	 * except when someone tries to use fixaddr indices in an
	 * illegal way. (such as mixing up address types or using
	 * out-of-range indices).
	 *
	 * If it doesn't get removed, the linker will complain
	 * loudly with a reasonably clear error message..
	 */
	/*�������Ż����ƻ���ȫ����if��䣬��Ϊ�ú�������Ϊ����������������������ǳ�����
	�������Ż�ʱ��Ҫ�ģ�����̶�ӳ���ַʵ���ϲ���������ָͨ�롣��ʽ�ϵļ��ȷ����
	����Ĺ̶�ӳ���ַ����Ч�����ڡ�__end_of_fixed_addresses��fixed_addresses�����
	һ����Ա�����������Ŀ������֡�����ں˷��ʵ�����Ч��ַ�������α����
	__this_fixmap_does_not_exist��û�ж��壩�����ں������ڼ䣬��ص��´������Ϣ����
	�����ڴ���δ������Ŷ��޷�����ӳ���ļ�����ˣ������ں˹����ڱ���ʱ���ɼ�⣬����
	��������ʱ����*/
	if (idx >= __end_of_fixed_addresses)
		__this_fixmap_does_not_exist();

        return __fix_to_virt(idx);
}

/*��ȡ�̶�ӳ���ڴ�����ָ�������ַ��Ӧ�Ĺ̶�ӳ�䳣����enum fixed_addressesö��ֵ��*/
static inline unsigned long virt_to_fix(const unsigned long vaddr)
{
	BUG_ON(vaddr >= FIXADDR_TOP || vaddr < FIXADDR_START);
	return __virt_to_fix(vaddr);
}

#endif /* !__ASSEMBLY__ */
#endif
