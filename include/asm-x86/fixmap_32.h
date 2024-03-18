/*编译时虚拟内存分配*/
#ifndef _ASM_FIXMAP_H
#define _ASM_FIXMAP_H

/*vmallocz.c和vsyscall.ld.s使用。在VMALLOC内存域和固定映射内存域起始处之间预留一个空页*/
extern unsigned long __FIXADDR_TOP;
/*   Linux中的vdso（Virtual Dynamic Shared Object）是一种特殊的动态共享对象，它在用户空
间和内核空间之间提供了一种高效的接口。vdso机制的目的是减少用户空间程序与内核之间频繁的
上下文切换开销，提高系统性能。
	 “vDSO”（虚拟动态共享对象）是一个小型的共享库，内核会自动将其映射到所有用户空间应用
程序的地址空间中。应用程序通常无需关注这些细节，因为vDSO最常由C库调用。这样，可以以正常
方式编码，使用标准函数，而C库会负责使用通过vDSO可用的任何功能。
	 vDSO的存在是为什么？内核提供了一些系统调用，用户空间代码经常使用这些调用，以至于这
些调用可能主导整体性能。这既是由于调用的频率，又是由于从用户空间退出并进入内核所产生的
上下文切换开销。
	 vdso包含一组特定的函数，这些函数在用户空间中执行，但其实现是由内核提供的。用户空间
程序可以通过调用这些函数来访问一些系统功能，而无需陷入内核态。vdso的一个重要用途是实现
系统调用的快速路径。当用户空间程序执行系统调用时，通常需要进行一次上下文切换，将控制权
从用户态切换到内核态。然而，某些系统调用是非常频繁且开销较小的，这种上下文切换的开销可
能会成为性能瓶颈。vdso提供了一个快速路径，通过在用户空间中执行特定的系统调用函数，避免
了不必要的上下文切换，从而提高了系统调用的性能。
	 在Linux中，vdso通常以linux-vdso.so.X的形式存在于/proc/self/maps中，并且被映射到每个
进程的地址空间中。这样，用户空间程序可以直接调用vdso中的函数，而无需显式加载和链接vdso
库。*/
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
/*对每个固定映射地址都会创建一个常数，加入到fixed_addresses枚举值列表中*/
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
/*建立固定映射常数和物理内存页之间的映射关系*/
#define set_fixmap(idx, phys) __set_fixmap(idx, phys, PAGE_KERNEL)
/*有些硬件想要不依靠缓存而直接设置固定映射关系*/
#define set_fixmap_nocache(idx, phys) __set_fixmap(idx, phys, PAGE_KERNEL_NOCACHE)
/*解除固定映射区域中指定固定映射常数与物理内存页之间的映射关系*/
#define clear_fixmap(idx) __set_fixmap(idx, 0, __pgprot(0))
/*固定映射内存域的结束地址*/
#define FIXADDR_TOP	((unsigned long)__FIXADDR_TOP)
/*固定映射内存域的长度*/
#define __FIXADDR_SIZE	(__end_of_permanent_fixed_addresses << PAGE_SHIFT)
/**/
#define __FIXADDR_BOOT_SIZE	(__end_of_fixed_addresses << PAGE_SHIFT)
/*固定映射内存域的起始地址。该内存域中的地址指向物理内存中的随机位置，。相对于内核
空间起始处的线性映射，在该映射区域内部的虚拟地址和物理地址之间的关联不是预设的，而是
可以自由定义，当定义之后不能改变。固定映射内存域会一直延伸到虚拟地址空间顶端（最后一
个页不属于该区域）。固定映射地址的优点在于，在编译时对此类地址的处理类似于常数，内核
一启动即为其分配了物理地址。此类地址的解引比普通指针要快。内核会确保在上下文切换期间
，对应于固定映射的页表项不会从TLB刷出，因此在访问固定映射的内存时，总是通过TLB高速缓
存取得对应的物理地址。
     在固定映射区中的虚拟内存地址可以自由映射到物理内存的高端地址上，但是与动态映射区
以及永久映射区不同的是，在固定映射区中虚拟地址是固定的，而被映射的物理地址是可以改变
的。也就是说，有些虚拟地址在编译的时候就固定下来了，是在内核启动过程中被确定的，而这
些虚拟地址对应的物理地址不是固定的。采用固定虚拟地址的好处是它相当于一个指针常量（常
量的值在编译时确定），指向物理地址，如果虚拟地址不固定，则相当于一个指针变量。
	 那为什么会有固定映射这个概念呢 ?  比如：在内核的启动过程中，有些模块需要使用虚拟内
存并映射到指定的物理地址上，而且这些模块也没有办法等待完整的内存管理模块初始化之后再
进行地址映射。因此，内核固定分配了一些虚拟地址，这些地址有固定的用途，使用该地址的模
块在初始化的时候，将这些固定分配的虚拟地址映射到指定的物理地址上去
*/
#define FIXADDR_START		(FIXADDR_TOP - __FIXADDR_SIZE)
/**/
#define FIXADDR_BOOT_START	(FIXADDR_TOP - __FIXADDR_BOOT_SIZE)
/*获取固定映射内存域中指定的固定映射常数对应的虚拟内存地址*/
#define __fix_to_virt(x)	(FIXADDR_TOP - ((x) << PAGE_SHIFT))
/*获取固定映射内存域中的虚拟地址对应的固定映射常数（enum fixed_addresses枚举值）*/
#define __virt_to_fix(x)	((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)
/*没有定义的伪函数，在内核链接期间，如果出现固定映射常数超出enum fixed_addresses
时，机会调用该函数，导致编译错误*/
extern void __this_fixmap_does_not_exist(void);

/*
 * 'index to address' translation. If anyone tries to use the idx
 * directly without tranlation, we catch the bug with a NULL-deference
 * kernel oops. Illegal ranges of incoming indices are caught too.
 */
/*计算固定映射常数（enum fixed_addresses枚举值）的虚拟地址*/
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
	/*编译器优化机制会完全消除if语句，因为该函数定义为内联函数，而且其参数都是常数。
	这样的优化时必要的，否则固定映射地址实际上并不优于普通指针。形式上的检查确保了
	所需的固定映射地址在有效区域内。__end_of_fixed_addresses是fixed_addresses的最后
	一个成员，定义了最大的可能数字。如果内核访问的是无效地址，则调用伪函数
	__this_fixmap_does_not_exist（没有定义）。在内核链接期间，这回导致错误的信息，表
	明由于存在未定义符号而无法生成映像文件。因此，此种内核故障在编译时即可检测，而不
	会在运行时出现*/
	if (idx >= __end_of_fixed_addresses)
		__this_fixmap_does_not_exist();

        return __fix_to_virt(idx);
}

/*获取固定映射内存域中指定虚拟地址对应的固定映射常数（enum fixed_addresses枚举值）*/
static inline unsigned long virt_to_fix(const unsigned long vaddr)
{
	BUG_ON(vaddr >= FIXADDR_TOP || vaddr < FIXADDR_START);
	return __virt_to_fix(vaddr);
}

#endif /* !__ASSEMBLY__ */
#endif
