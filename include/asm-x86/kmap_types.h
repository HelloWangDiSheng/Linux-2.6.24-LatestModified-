#ifndef _ASM_X86_KMAP_TYPES_H
#define _ASM_X86_KMAP_TYPES_H

/*映射类型*/
#if defined(CONFIG_X86_32) && defined(CONFIG_DEBUG_HIGHMEM)
#define D(n) __KM_FENCE_##n ,
#else
#define D(n)
#endif

/*固定映射机制，使得可以在内核地址空间中访问用于建立原子映射的内存。在FIX_KMAP_BEGIN
和FIX_KMAP_END之间建立一个用于映射高端内存页的区域，该区域位于fixed_addresses数组中。
准确的位置需要根据当前活动的CPU和所需映射类型计算。
idx = type + KM_TYPE_NR*smp_processor_id();
vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
在固定映射区域中，系统中的每个处理器都有一个对应的“窗口”。每个窗口中，每种映射类型都
对应于一项（KM_TYPE_NR不是一个独立的类型，只用于表示km_type中有多少项）。根据这种布局
，我们很清楚函数在使用kmap_atomic时不会阻塞。如果发生阻塞，那么另一个进程可能建立同样
类型的映射，覆盖现存的项。在使用上述公式计算出适当的索引，找到相关的固定映射地址之后，
内核只需相应地修改页表，并刷出TLB使页表修改生效*/
enum km_type
{
	D(0)	KM_BOUNCE_READ,
	D(1)	KM_SKB_SUNRPC_DATA,
	D(2)	KM_SKB_DATA_SOFTIRQ,
	D(3)	KM_USER0,
	D(4)	KM_USER1,
	D(5)	KM_BIO_SRC_IRQ,
	D(6)	KM_BIO_DST_IRQ,
	D(7)	KM_PTE0,
	D(8)	KM_PTE1,
	D(9)	KM_IRQ0,
	D(10)	KM_IRQ1,
	D(11)	KM_SOFTIRQ0,
	D(12)	KM_SOFTIRQ1,
	D(13)	KM_TYPE_NR
};

#undef D

#endif
