#ifndef __ASM_PARAVIRT_H
#define __ASM_PARAVIRT_H

/*x86上运行的许多指令需要被替换为虚拟参数：这些钩子函数在此定义*/

#ifdef CONFIG_PARAVIRT
#include <asm/page.h>

/* Bitmask of what can be clobbered: usually at least eax. */
#define CLBR_NONE 0x0
#define CLBR_EAX 0x1
#define CLBR_ECX 0x2
#define CLBR_EDX 0x4
#define CLBR_ANY 0x7

#ifndef __ASSEMBLY__
#include <linux/types.h>
#include <linux/cpumask.h>
#include <asm/kmap_types.h>

struct page;
struct thread_struct;
struct Xgt_desc_struct;
struct tss_struct;
struct mm_struct;
struct desc_struct;

/*通用信息*/
struct pv_info
{
	unsigned int kernel_rpl;
	int shared_kernel_pmd;
	int paravirt_enabled;
	const char *name;
};

struct pv_init_ops
{
	/*
	 * Patch may replace one of the defined code sequences with
	 * arbitrary code, subject to the same register constraints.
	 * This generally means the code is not free to clobber any
	 * registers other than EAX.  The patch function should return
	 * the number of bytes of code generated, as we nop pad the
	 * rest in generic code.
	 */
	unsigned (*patch)(u8 type, u16 clobber, void *insnbuf,
			  unsigned long addr, unsigned len);

	/* Basic arch-specific setup */
	void (*arch_setup)(void);
	char *(*memory_setup)(void);
	void (*post_allocator_init)(void);

	/* Print a banner to identify the environment */
	void (*banner)(void);
};


struct pv_lazy_ops
{
	/* Set deferred update mode, used for batching operations. */
	void (*enter)(void);
	void (*leave)(void);
};

struct pv_time_ops
{
	void (*time_init)(void);

	/* Set and set time of day */
	unsigned long (*get_wallclock)(void);
	int (*set_wallclock)(unsigned long);

	unsigned long long (*sched_clock)(void);
	unsigned long (*get_cpu_khz)(void);
};

struct pv_cpu_ops
{
	/*不同权限指令的钩子函数*/
	unsigned long (*get_debugreg)(int regno);
	void (*set_debugreg)(int regno, unsigned long value);
	void (*clts)(void);
	unsigned long (*read_cr0)(void);
	void (*write_cr0)(unsigned long);
	unsigned long (*read_cr4_safe)(void);
	unsigned long (*read_cr4)(void);
	void (*write_cr4)(unsigned long);
	/*段描述符处理函数*/
	void (*load_tr_desc)(void);
	void (*load_gdt)(const struct Xgt_desc_struct *);
	void (*load_idt)(const struct Xgt_desc_struct *);
	void (*store_gdt)(struct Xgt_desc_struct *);
	void (*store_idt)(struct Xgt_desc_struct *);
	void (*set_ldt)(const void *desc, unsigned entries);
	unsigned long (*store_tr)(void);
	void (*load_tls)(struct thread_struct *t, unsigned int cpu);
	void (*write_ldt_entry)(struct desc_struct *,	int entrynum, u32 low, u32 high);
	void (*write_gdt_entry)(struct desc_struct *,	int entrynum, u32 low, u32 high);
	void (*write_idt_entry)(struct desc_struct *,	int entrynum, u32 low, u32 high);
	void (*load_esp0)(struct tss_struct *tss, struct thread_struct *t);
	void (*set_iopl_mask)(unsigned mask);
	void (*wbinvd)(void);
	void (*io_delay)(void);
	/* cpuid emulation, mostly so that caps bits can be disabled */
	void (*cpuid)(unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
	/* MSR, PMC and TSR operations. err = 0/-EFAULT.  wrmsr returns 0/-EFAULT. */
	u64 (*read_msr)(unsigned int msr, int *err);
	int (*write_msr)(unsigned int msr, u64 val);
	u64 (*read_tsc)(void);
	u64 (*read_pmc)(void);
	/* These two are jmp to, not actually called. */
	void (*irq_enable_sysexit)(void);
	void (*iret)(void);
	struct pv_lazy_ops lazy_mode;
};
/**/
struct pv_irq_ops
{
	void (*init_IRQ)(void);

	/*
	 * Get/set interrupt state.  save_fl and restore_fl are only
	 * expected to use X86_EFLAGS_IF; all other bits
	 * returned from save_fl are undefined, and may be ignored by
	 * restore_fl.
	 */
	unsigned long (*save_fl)(void);
	void (*restore_fl)(unsigned long);
	void (*irq_disable)(void);
	void (*irq_enable)(void);
	void (*safe_halt)(void);
	void (*halt)(void);
};

struct pv_apic_ops
{
#ifdef CONFIG_X86_LOCAL_APIC
	/*
	 * Direct APIC operations, principally for VMI.  Ideally
	 * these shouldn't be in this interface.
	 */
	void (*apic_write)(unsigned long reg, unsigned long v);
	void (*apic_write_atomic)(unsigned long reg, unsigned long v);
	unsigned long (*apic_read)(unsigned long reg);
	void (*setup_boot_clock)(void);
	void (*setup_secondary_clock)(void);
	void (*startup_ipi_hook)(int phys_apicid, unsigned long start_eip, unsigned long start_esp);
#endif
};

struct pv_mmu_ops
{
	/*
	 * Called before/after init_mm pagetable setup. setup_start
	 * may reset %cr3, and may pre-install parts of the pagetable;
	 * pagetable setup is expected to preserve any existing
	 * mapping.
	 */
	void (*pagetable_setup_start)(pgd_t *pgd_base);
	void (*pagetable_setup_done)(pgd_t *pgd_base);
	unsigned long (*read_cr2)(void);
	void (*write_cr2)(unsigned long);
	unsigned long (*read_cr3)(void);
	void (*write_cr3)(unsigned long);
	/*Hooks for intercepting the creation/use/destruction of an mm_struct*/
	void (*activate_mm)(struct mm_struct *prev,	struct mm_struct *next);
	void (*dup_mmap)(struct mm_struct *oldmm, struct mm_struct *mm);
	void (*exit_mmap)(struct mm_struct *mm);
	/* TLB operations */
	void (*flush_tlb_user)(void);
	void (*flush_tlb_kernel)(void);
	void (*flush_tlb_single)(unsigned long addr);
	void (*flush_tlb_others)(const cpumask_t *cpus, struct mm_struct *mm, unsigned long va);
	/* Hooks for allocating/releasing pagetable pages */
	void (*alloc_pt)(struct mm_struct *mm, u32 pfn);
	void (*alloc_pd)(u32 pfn);
	void (*alloc_pd_clone)(u32 pfn, u32 clonepfn, u32 start, u32 count);
	void (*release_pt)(u32 pfn);
	void (*release_pd)(u32 pfn);
	/* Pagetable manipulation functions */
	void (*set_pte)(pte_t *ptep, pte_t pteval);
	void (*set_pte_at)(struct mm_struct *mm, unsigned long addr, pte_t *ptep, pte_t pteval);
	void (*set_pmd)(pmd_t *pmdp, pmd_t pmdval);
	void (*pte_update)(struct mm_struct *mm, unsigned long addr, pte_t *ptep);
	void (*pte_update_defer)(struct mm_struct *mm, unsigned long addr, pte_t *ptep);

#ifdef CONFIG_X86_PAE
	void (*set_pte_atomic)(pte_t *ptep, pte_t pteval);
	void (*set_pte_present)(struct mm_struct *mm, unsigned long addr,
				pte_t *ptep, pte_t pte);
	void (*set_pud)(pud_t *pudp, pud_t pudval);
	void (*pte_clear)(struct mm_struct *mm, unsigned long addr, pte_t *ptep);
	void (*pmd_clear)(pmd_t *pmdp);

	unsigned long long (*pte_val)(pte_t);
	unsigned long long (*pmd_val)(pmd_t);
	unsigned long long (*pgd_val)(pgd_t);

	pte_t (*make_pte)(unsigned long long pte);
	pmd_t (*make_pmd)(unsigned long long pmd);
	pgd_t (*make_pgd)(unsigned long long pgd);
#else
	unsigned long (*pte_val)(pte_t);
	unsigned long (*pgd_val)(pgd_t);
	pte_t (*make_pte)(unsigned long pte);
	pgd_t (*make_pgd)(unsigned long pgd);
#endif

#ifdef CONFIG_HIGHPTE
	void *(*kmap_atomic_pte)(struct page *page, enum km_type type);
#endif

	struct pv_lazy_ops lazy_mode;
};

/* This contains all the paravirt structures: we get a convenient
 * number for each function using the offset which we use to indicate
 * what to patch. */
struct paravirt_patch_template
{
	struct pv_init_ops pv_init_ops;
	struct pv_time_ops pv_time_ops;
	struct pv_cpu_ops pv_cpu_ops;
	struct pv_irq_ops pv_irq_ops;
	struct pv_apic_ops pv_apic_ops;
	struct pv_mmu_ops pv_mmu_ops;
};

extern struct pv_info pv_info;
extern struct pv_init_ops pv_init_ops;
extern struct pv_time_ops pv_time_ops;
extern struct pv_cpu_ops pv_cpu_ops;
extern struct pv_irq_ops pv_irq_ops;
extern struct pv_apic_ops pv_apic_ops;
extern struct pv_mmu_ops pv_mmu_ops;

#define PARAVIRT_PATCH(x)	(offsetof(struct paravirt_patch_template, x) / sizeof(void *))
#define paravirt_type(op) [paravirt_typenum] "i" (PARAVIRT_PATCH(op)),	[paravirt_opptr] "m" (op)
#define paravirt_clobber(clobber)		[paravirt_clobber] "i" (clobber)

/*
 * Generate some code, and mark it as patchable by the
 * apply_paravirt() alternate instruction patcher.
 */
#define _paravirt_alt(insn_string, type, clobber)	\
	"771:\n\t" insn_string "\n" "772:\n"		\
	".pushsection .parainstructions,\"a\"\n"	\
	"  .long 771b\n"				\
	"  .byte " type "\n"				\
	"  .byte 772b-771b\n"				\
	"  .short " clobber "\n"			\
	".popsection\n"

/* Generate patchable code, with the default asm parameters. */
#define paravirt_alt(insn_string)\	
		_paravirt_alt(insn_string, "%c[paravirt_typenum]", "%c[paravirt_clobber]")

unsigned paravirt_patch_nop(void);
unsigned paravirt_patch_ignore(unsigned len);
unsigned paravirt_patch_call(void *insnbuf, const void *target, u16 tgt_clobbers,
			     				unsigned long addr, u16 site_clobbers, unsigned len);
unsigned paravirt_patch_jmp(void *insnbuf, const void *target, unsigned long addr, unsigned len);
unsigned paravirt_patch_default(u8 type, u16 clobbers, void *insnbuf, unsigned long addr,
									unsigned len);
unsigned paravirt_patch_insns(void *insnbuf, unsigned len, const char *start, const char *end);
int paravirt_disable_iospace(void);

/*
 * This generates an indirect call based on the operation type number.
 * The type number, computed in PARAVIRT_PATCH, is derived from the
 * offset into the paravirt_patch_template structure, and can therefore be
 * freely converted back into a structure offset.
 */
#define PARAVIRT_CALL	"call *%[paravirt_opptr];"

/*
 * These macros are intended to wrap calls through one of the paravirt
 * ops structs, so that they can be later identified and patched at
 * runtime.
 *
 * Normally, a call to a pv_op function is a simple indirect call:
 * (paravirt_ops.operations)(args...).
 *
 * Unfortunately, this is a relatively slow operation for modern CPUs,
 * because it cannot necessarily determine what the destination
 * address is.  In this case, the address is a runtime constant, so at
 * the very least we can patch the call to e a simple direct call, or
 * ideally, patch an inline implementation into the callsite.  (Direct
 * calls are essentially free, because the call and return addresses
 * are completely predictable.)
 *
 * These macros rely on the standard gcc "regparm(3)" calling
 * convention, in which the first three arguments are placed in %eax,
 * %edx, %ecx (in that order), and the remaining arguments are placed
 * on the stack.  All caller-save registers (eax,edx,ecx) are expected
 * to be modified (either clobbered or used for return values).
 *
 * The call instruction itself is marked by placing its start address
 * and size into the .parainstructions section, so that
 * apply_paravirt() in arch/i386/kernel/alternative.c can do the
 * appropriate patching under the control of the backend pv_init_ops
 * implementation.
 *
 * Unfortunately there's no way to get gcc to generate the args setup
 * for the call, and then allow the call itself to be generated by an
 * inline asm.  Because of this, we must do the complete arg setup and
 * return value handling from within these macros.  This is fairly
 * cumbersome.
 *
 * There are 5 sets of PVOP_* macros for dealing with 0-4 arguments.
 * It could be extended to more arguments, but there would be little
 * to be gained from that.  For each number of arguments, there are
 * the two VCALL and CALL variants for void and non-void functions.
 *
 * When there is a return value, the invoker of the macro must specify
 * the return type.  The macro then uses sizeof() on that type to
 * determine whether its a 32 or 64 bit value, and places the return
 * in the right register(s) (just %eax for 32-bit, and %edx:%eax for
 * 64-bit).
 *
 * 64-bit arguments are passed as a pair of adjacent 32-bit arguments
 * in low,high order.
 *
 * Small structures are passed and returned in registers.  The macro
 * calling convention can't directly deal with this, so the wrapper
 * functions must do this.
 *
 * These PVOP_* macros are only defined within this header.  This
 * means that all uses must be wrapped in inline functions.  This also
 * makes sure the incoming and outgoing types are always correct.
 */
#define __PVOP_CALL(rettype, op, pre, post, ...)			\
	({								\
		rettype __ret;						\
		unsigned long __eax, __edx, __ecx;			\
		if (sizeof(rettype) > sizeof(unsigned long)) {		\
			asm volatile(pre				\
				     paravirt_alt(PARAVIRT_CALL)	\
				     post				\
				     : "=a" (__eax), "=d" (__edx),	\
				       "=c" (__ecx)			\
				     : paravirt_type(op),		\
				       paravirt_clobber(CLBR_ANY),	\
				       ##__VA_ARGS__			\
				     : "memory", "cc");			\
			__ret = (rettype)((((u64)__edx) << 32) | __eax); \
		} else {						\
			asm volatile(pre				\
				     paravirt_alt(PARAVIRT_CALL)	\
				     post				\
				     : "=a" (__eax), "=d" (__edx),	\
				       "=c" (__ecx)			\
				     : paravirt_type(op),		\
				       paravirt_clobber(CLBR_ANY),	\
				       ##__VA_ARGS__			\
				     : "memory", "cc");			\
			__ret = (rettype)__eax;				\
		}							\
		__ret;							\
	})
#define __PVOP_VCALL(op, pre, post, ...)				\
	({								\
		unsigned long __eax, __edx, __ecx;			\
		asm volatile(pre					\
			     paravirt_alt(PARAVIRT_CALL)		\
			     post					\
			     : "=a" (__eax), "=d" (__edx), "=c" (__ecx) \
			     : paravirt_type(op),			\
			       paravirt_clobber(CLBR_ANY),		\
			       ##__VA_ARGS__				\
			     : "memory", "cc");				\
	})

#define PVOP_CALL0(rettype, op)						\
	__PVOP_CALL(rettype, op, "", "")
#define PVOP_VCALL0(op)							\
	__PVOP_VCALL(op, "", "")

#define PVOP_CALL1(rettype, op, arg1)					\
	__PVOP_CALL(rettype, op, "", "", "0" ((u32)(arg1)))
#define PVOP_VCALL1(op, arg1)	__PVOP_VCALL(op, "", "", "0" ((u32)(arg1)))

#define PVOP_CALL2(rettype, op, arg1, arg2)				\
	__PVOP_CALL(rettype, op, "", "", "0" ((u32)(arg1)), "1" ((u32)(arg2)))
#define PVOP_VCALL2(op, arg1, arg2)					\
	__PVOP_VCALL(op, "", "", "0" ((u32)(arg1)), "1" ((u32)(arg2)))

#define PVOP_CALL3(rettype, op, arg1, arg2, arg3)			\
	__PVOP_CALL(rettype, op, "", "", "0" ((u32)(arg1)),		\
		    "1"((u32)(arg2)), "2"((u32)(arg3)))
#define PVOP_VCALL3(op, arg1, arg2, arg3)				\
	__PVOP_VCALL(op, "", "", "0" ((u32)(arg1)), "1"((u32)(arg2)),	\
		     "2"((u32)(arg3)))

#define PVOP_CALL4(rettype, op, arg1, arg2, arg3, arg4)			\
	__PVOP_CALL(rettype, op,					\
		    "push %[_arg4];", "lea 4(%%esp),%%esp;",		\
		    "0" ((u32)(arg1)), "1" ((u32)(arg2)),		\
		    "2" ((u32)(arg3)), [_arg4] "mr" ((u32)(arg4)))
#define PVOP_VCALL4(op, arg1, arg2, arg3, arg4)				\
	__PVOP_VCALL(op,						\
		    "push %[_arg4];", "lea 4(%%esp),%%esp;",		\
		    "0" ((u32)(arg1)), "1" ((u32)(arg2)),		\
		    "2" ((u32)(arg3)), [_arg4] "mr" ((u32)(arg4)))

static inline int paravirt_enabled(void)
{
	return pv_info.paravirt_enabled;
}

static inline void load_esp0(struct tss_struct *tss,
			     struct thread_struct *thread)
{
	PVOP_VCALL2(pv_cpu_ops.load_esp0, tss, thread);
}

#define ARCH_SETUP			pv_init_ops.arch_setup();
static inline unsigned long get_wallclock(void)
{
	return PVOP_CALL0(unsigned long, pv_time_ops.get_wallclock);
}

static inline int set_wallclock(unsigned long nowtime)
{
	return PVOP_CALL1(int, pv_time_ops.set_wallclock, nowtime);
}

static inline void (*choose_time_init(void))(void)
{
	return pv_time_ops.time_init;
}

/* The paravirtualized CPUID instruction. */
static inline void __cpuid(unsigned int *eax, unsigned int *ebx,
			   unsigned int *ecx, unsigned int *edx)
{
	PVOP_VCALL4(pv_cpu_ops.cpuid, eax, ebx, ecx, edx);
}

/*
 * These special macros can be used to get or set a debugging register
 */
static inline unsigned long paravirt_get_debugreg(int reg)
{
	return PVOP_CALL1(unsigned long, pv_cpu_ops.get_debugreg, reg);
}
#define get_debugreg(var, reg) var = paravirt_get_debugreg(reg)
static inline void set_debugreg(unsigned long val, int reg)
{
	PVOP_VCALL2(pv_cpu_ops.set_debugreg, reg, val);
}

static inline void clts(void)
{
	PVOP_VCALL0(pv_cpu_ops.clts);
}

static inline unsigned long read_cr0(void)
{
	return PVOP_CALL0(unsigned long, pv_cpu_ops.read_cr0);
}

static inline void write_cr0(unsigned long x)
{
	PVOP_VCALL1(pv_cpu_ops.write_cr0, x);
}

static inline unsigned long read_cr2(void)
{
	return PVOP_CALL0(unsigned long, pv_mmu_ops.read_cr2);
}

static inline void write_cr2(unsigned long x)
{
	PVOP_VCALL1(pv_mmu_ops.write_cr2, x);
}

static inline unsigned long read_cr3(void)
{
	return PVOP_CALL0(unsigned long, pv_mmu_ops.read_cr3);
}

static inline void write_cr3(unsigned long x)
{
	PVOP_VCALL1(pv_mmu_ops.write_cr3, x);
}

static inline unsigned long read_cr4(void)
{
	return PVOP_CALL0(unsigned long, pv_cpu_ops.read_cr4);
}
static inline unsigned long read_cr4_safe(void)
{
	return PVOP_CALL0(unsigned long, pv_cpu_ops.read_cr4_safe);
}

static inline void write_cr4(unsigned long x)
{
	PVOP_VCALL1(pv_cpu_ops.write_cr4, x);
}

static inline void raw_safe_halt(void)
{
	PVOP_VCALL0(pv_irq_ops.safe_halt);
}

static inline void halt(void)
{
	PVOP_VCALL0(pv_irq_ops.safe_halt);
}

static inline void wbinvd(void)
{
	PVOP_VCALL0(pv_cpu_ops.wbinvd);
}

#define get_kernel_rpl()  (pv_info.kernel_rpl)

static inline u64 paravirt_read_msr(unsigned msr, int *err)
{
	return PVOP_CALL2(u64, pv_cpu_ops.read_msr, msr, err);
}
static inline int paravirt_write_msr(unsigned msr, unsigned low, unsigned high)
{
	return PVOP_CALL3(int, pv_cpu_ops.write_msr, msr, low, high);
}

/* These should all do BUG_ON(_err), but our headers are too tangled. */
#define rdmsr(msr,val1,val2) do {		\
	int _err;				\
	u64 _l = paravirt_read_msr(msr, &_err);	\
	val1 = (u32)_l;				\
	val2 = _l >> 32;			\
} while(0)

#define wrmsr(msr,val1,val2) do {		\
	paravirt_write_msr(msr, val1, val2);	\
} while(0)

#define rdmsrl(msr,val) do {			\
	int _err;				\
	val = paravirt_read_msr(msr, &_err);	\
} while(0)

#define wrmsrl(msr,val)		wrmsr(msr, (u32)((u64)(val)), ((u64)(val))>>32)
#define wrmsr_safe(msr,a,b)	paravirt_write_msr(msr, a, b)

/* rdmsr with exception handling */
#define rdmsr_safe(msr,a,b) ({			\
	int _err;				\
	u64 _l = paravirt_read_msr(msr, &_err);	\
	(*a) = (u32)_l;				\
	(*b) = _l >> 32;			\
	_err; })


static inline u64 paravirt_read_tsc(void)
{
	return PVOP_CALL0(u64, pv_cpu_ops.read_tsc);
}

#define rdtscl(low) do {			\
	u64 _l = paravirt_read_tsc();		\
	low = (int)_l;				\
} while(0)

#define rdtscll(val) (val = paravirt_read_tsc())

static inline unsigned long long paravirt_sched_clock(void)
{
	return PVOP_CALL0(unsigned long long, pv_time_ops.sched_clock);
}
#define calculate_cpu_khz() (pv_time_ops.get_cpu_khz())

#define write_tsc(val1,val2) wrmsr(0x10, val1, val2)

static inline unsigned long long paravirt_read_pmc(int counter)
{
	return PVOP_CALL1(u64, pv_cpu_ops.read_pmc, counter);
}

#define rdpmc(counter,low,high) do {		\
	u64 _l = paravirt_read_pmc(counter);	\
	low = (u32)_l;				\
	high = _l >> 32;			\
} while(0)

static inline void load_TR_desc(void)
{
	PVOP_VCALL0(pv_cpu_ops.load_tr_desc);
}
static inline void load_gdt(const struct Xgt_desc_struct *dtr)
{
	PVOP_VCALL1(pv_cpu_ops.load_gdt, dtr);
}
static inline void load_idt(const struct Xgt_desc_struct *dtr)
{
	PVOP_VCALL1(pv_cpu_ops.load_idt, dtr);
}
static inline void set_ldt(const void *addr, unsigned entries)
{
	PVOP_VCALL2(pv_cpu_ops.set_ldt, addr, entries);
}
static inline void store_gdt(struct Xgt_desc_struct *dtr)
{
	PVOP_VCALL1(pv_cpu_ops.store_gdt, dtr);
}
static inline void store_idt(struct Xgt_desc_struct *dtr)
{
	PVOP_VCALL1(pv_cpu_ops.store_idt, dtr);
}
static inline unsigned long paravirt_store_tr(void)
{
	return PVOP_CALL0(unsigned long, pv_cpu_ops.store_tr);
}
#define store_tr(tr)	((tr) = paravirt_store_tr())
static inline void load_TLS(struct thread_struct *t, unsigned cpu)
{
	PVOP_VCALL2(pv_cpu_ops.load_tls, t, cpu);
}
static inline void write_ldt_entry(void *dt, int entry, u32 low, u32 high)
{
	PVOP_VCALL4(pv_cpu_ops.write_ldt_entry, dt, entry, low, high);
}
static inline void write_gdt_entry(void *dt, int entry, u32 low, u32 high)
{
	PVOP_VCALL4(pv_cpu_ops.write_gdt_entry, dt, entry, low, high);
}
static inline void write_idt_entry(void *dt, int entry, u32 low, u32 high)
{
	PVOP_VCALL4(pv_cpu_ops.write_idt_entry, dt, entry, low, high);
}
static inline void set_iopl_mask(unsigned mask)
{
	PVOP_VCALL1(pv_cpu_ops.set_iopl_mask, mask);
}

/* The paravirtualized I/O functions */
static inline void slow_down_io(void) {
	pv_cpu_ops.io_delay();
#ifdef REALLY_SLOW_IO
	pv_cpu_ops.io_delay();
	pv_cpu_ops.io_delay();
	pv_cpu_ops.io_delay();
#endif
}

#ifdef CONFIG_X86_LOCAL_APIC
/*
 * Basic functions accessing APICs.
 */
static inline void apic_write(unsigned long reg, unsigned long v)
{
	PVOP_VCALL2(pv_apic_ops.apic_write, reg, v);
}

static inline void apic_write_atomic(unsigned long reg, unsigned long v)
{
	PVOP_VCALL2(pv_apic_ops.apic_write_atomic, reg, v);
}

static inline unsigned long apic_read(unsigned long reg)
{
	return PVOP_CALL1(unsigned long, pv_apic_ops.apic_read, reg);
}

static inline void setup_boot_clock(void)
{
	PVOP_VCALL0(pv_apic_ops.setup_boot_clock);
}

static inline void setup_secondary_clock(void)
{
	PVOP_VCALL0(pv_apic_ops.setup_secondary_clock);
}
#endif

static inline void paravirt_post_allocator_init(void)
{
	if (pv_init_ops.post_allocator_init)
		(*pv_init_ops.post_allocator_init)();
}

static inline void paravirt_pagetable_setup_start(pgd_t *base)
{
	(*pv_mmu_ops.pagetable_setup_start)(base);
}

static inline void paravirt_pagetable_setup_done(pgd_t *base)
{
	(*pv_mmu_ops.pagetable_setup_done)(base);
}

#ifdef CONFIG_SMP
static inline void startup_ipi_hook(int phys_apicid, unsigned long start_eip,
				    unsigned long start_esp)
{
	PVOP_VCALL3(pv_apic_ops.startup_ipi_hook,
		    phys_apicid, start_eip, start_esp);
}
#endif

static inline void paravirt_activate_mm(struct mm_struct *prev,
					struct mm_struct *next)
{
	PVOP_VCALL2(pv_mmu_ops.activate_mm, prev, next);
}

static inline void arch_dup_mmap(struct mm_struct *oldmm,
				 struct mm_struct *mm)
{
	PVOP_VCALL2(pv_mmu_ops.dup_mmap, oldmm, mm);
}

static inline void arch_exit_mmap(struct mm_struct *mm)
{
	PVOP_VCALL1(pv_mmu_ops.exit_mmap, mm);
}

static inline void __flush_tlb(void)
{
	PVOP_VCALL0(pv_mmu_ops.flush_tlb_user);
}
static inline void __flush_tlb_global(void)
{
	PVOP_VCALL0(pv_mmu_ops.flush_tlb_kernel);
}
static inline void __flush_tlb_single(unsigned long addr)
{
	PVOP_VCALL1(pv_mmu_ops.flush_tlb_single, addr);
}

static inline void flush_tlb_others(cpumask_t cpumask, struct mm_struct *mm,
				    unsigned long va)
{
	PVOP_VCALL3(pv_mmu_ops.flush_tlb_others, &cpumask, mm, va);
}

static inline void paravirt_alloc_pt(struct mm_struct *mm, unsigned pfn)
{
	PVOP_VCALL2(pv_mmu_ops.alloc_pt, mm, pfn);
}
static inline void paravirt_release_pt(unsigned pfn)
{
	PVOP_VCALL1(pv_mmu_ops.release_pt, pfn);
}

static inline void paravirt_alloc_pd(unsigned pfn)
{
	PVOP_VCALL1(pv_mmu_ops.alloc_pd, pfn);
}

static inline void paravirt_alloc_pd_clone(unsigned pfn, unsigned clonepfn,
					   unsigned start, unsigned count)
{
	PVOP_VCALL4(pv_mmu_ops.alloc_pd_clone, pfn, clonepfn, start, count);
}
static inline void paravirt_release_pd(unsigned pfn)
{
	PVOP_VCALL1(pv_mmu_ops.release_pd, pfn);
}

#ifdef CONFIG_HIGHPTE
static inline void *kmap_atomic_pte(struct page *page, enum km_type type)
{
	unsigned long ret;
	ret = PVOP_CALL2(unsigned long, pv_mmu_ops.kmap_atomic_pte, page, type);
	return (void *)ret;
}
#endif

static inline void pte_update(struct mm_struct *mm, unsigned long addr,
			      pte_t *ptep)
{
	PVOP_VCALL3(pv_mmu_ops.pte_update, mm, addr, ptep);
}

static inline void pte_update_defer(struct mm_struct *mm, unsigned long addr,
				    pte_t *ptep)
{
	PVOP_VCALL3(pv_mmu_ops.pte_update_defer, mm, addr, ptep);
}

#ifdef CONFIG_X86_PAE
static inline pte_t __pte(unsigned long long val)
{
	unsigned long long ret = PVOP_CALL2(unsigned long long,
					    pv_mmu_ops.make_pte,
					    val, val >> 32);
	return (pte_t) { ret, ret >> 32 };
}

static inline pmd_t __pmd(unsigned long long val)
{
	return (pmd_t) { PVOP_CALL2(unsigned long long, pv_mmu_ops.make_pmd,
				    val, val >> 32) };
}

static inline pgd_t __pgd(unsigned long long val)
{
	return (pgd_t) { PVOP_CALL2(unsigned long long, pv_mmu_ops.make_pgd,
				    val, val >> 32) };
}

static inline unsigned long long pte_val(pte_t x)
{
	return PVOP_CALL2(unsigned long long, pv_mmu_ops.pte_val,
			  x.pte_low, x.pte_high);
}

static inline unsigned long long pmd_val(pmd_t x)
{
	return PVOP_CALL2(unsigned long long, pv_mmu_ops.pmd_val,
			  x.pmd, x.pmd >> 32);
}

static inline unsigned long long pgd_val(pgd_t x)
{
	return PVOP_CALL2(unsigned long long, pv_mmu_ops.pgd_val,
			  x.pgd, x.pgd >> 32);
}

static inline void set_pte(pte_t *ptep, pte_t pteval)
{
	PVOP_VCALL3(pv_mmu_ops.set_pte, ptep, pteval.pte_low, pteval.pte_high);
}

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
			      pte_t *ptep, pte_t pteval)
{
	/* 5 arg words */
	pv_mmu_ops.set_pte_at(mm, addr, ptep, pteval);
}

static inline void set_pte_atomic(pte_t *ptep, pte_t pteval)
{
	PVOP_VCALL3(pv_mmu_ops.set_pte_atomic, ptep,
		    pteval.pte_low, pteval.pte_high);
}

static inline void set_pte_present(struct mm_struct *mm, unsigned long addr,
				   pte_t *ptep, pte_t pte)
{
	/* 5 arg words */
	pv_mmu_ops.set_pte_present(mm, addr, ptep, pte);
}

static inline void set_pmd(pmd_t *pmdp, pmd_t pmdval)
{
	PVOP_VCALL3(pv_mmu_ops.set_pmd, pmdp,
		    pmdval.pmd, pmdval.pmd >> 32);
}

static inline void set_pud(pud_t *pudp, pud_t pudval)
{
	PVOP_VCALL3(pv_mmu_ops.set_pud, pudp,
		    pudval.pgd.pgd, pudval.pgd.pgd >> 32);
}

static inline void pte_clear(struct mm_struct *mm, unsigned long addr, pte_t *ptep)
{
	PVOP_VCALL3(pv_mmu_ops.pte_clear, mm, addr, ptep);
}

static inline void pmd_clear(pmd_t *pmdp)
{
	PVOP_VCALL1(pv_mmu_ops.pmd_clear, pmdp);
}

#else  /* !CONFIG_X86_PAE */

static inline pte_t __pte(unsigned long val)
{
	return (pte_t) { PVOP_CALL1(unsigned long, pv_mmu_ops.make_pte, val) };
}

static inline pgd_t __pgd(unsigned long val)
{
	return (pgd_t) { PVOP_CALL1(unsigned long, pv_mmu_ops.make_pgd, val) };
}

static inline unsigned long pte_val(pte_t x)
{
	return PVOP_CALL1(unsigned long, pv_mmu_ops.pte_val, x.pte_low);
}

static inline unsigned long pgd_val(pgd_t x)
{
	return PVOP_CALL1(unsigned long, pv_mmu_ops.pgd_val, x.pgd);
}

static inline void set_pte(pte_t *ptep, pte_t pteval)
{
	PVOP_VCALL2(pv_mmu_ops.set_pte, ptep, pteval.pte_low);
}

static inline void set_pte_at(struct mm_struct *mm, unsigned long addr,
			      pte_t *ptep, pte_t pteval)
{
	PVOP_VCALL4(pv_mmu_ops.set_pte_at, mm, addr, ptep, pteval.pte_low);
}

static inline void set_pmd(pmd_t *pmdp, pmd_t pmdval)
{
	PVOP_VCALL2(pv_mmu_ops.set_pmd, pmdp, pmdval.pud.pgd.pgd);
}
#endif	/* CONFIG_X86_PAE */

/* Lazy mode for batching updates / context switch */
enum paravirt_lazy_mode {
	PARAVIRT_LAZY_NONE,
	PARAVIRT_LAZY_MMU,
	PARAVIRT_LAZY_CPU,
};

enum paravirt_lazy_mode paravirt_get_lazy_mode(void);
void paravirt_enter_lazy_cpu(void);
void paravirt_leave_lazy_cpu(void);
void paravirt_enter_lazy_mmu(void);
void paravirt_leave_lazy_mmu(void);
void paravirt_leave_lazy(enum paravirt_lazy_mode mode);

#define  __HAVE_ARCH_ENTER_LAZY_CPU_MODE
static inline void arch_enter_lazy_cpu_mode(void)
{
	PVOP_VCALL0(pv_cpu_ops.lazy_mode.enter);
}

static inline void arch_leave_lazy_cpu_mode(void)
{
	PVOP_VCALL0(pv_cpu_ops.lazy_mode.leave);
}

static inline void arch_flush_lazy_cpu_mode(void)
{
	if (unlikely(paravirt_get_lazy_mode() == PARAVIRT_LAZY_CPU)) {
		arch_leave_lazy_cpu_mode();
		arch_enter_lazy_cpu_mode();
	}
}


#define  __HAVE_ARCH_ENTER_LAZY_MMU_MODE
static inline void arch_enter_lazy_mmu_mode(void)
{
	PVOP_VCALL0(pv_mmu_ops.lazy_mode.enter);
}

static inline void arch_leave_lazy_mmu_mode(void)
{
	PVOP_VCALL0(pv_mmu_ops.lazy_mode.leave);
}

static inline void arch_flush_lazy_mmu_mode(void)
{
	if (unlikely(paravirt_get_lazy_mode() == PARAVIRT_LAZY_MMU)) {
		arch_leave_lazy_mmu_mode();
		arch_enter_lazy_mmu_mode();
	}
}

void _paravirt_nop(void);
#define paravirt_nop	((void *)_paravirt_nop)

/* These all sit in the .parainstructions section to tell us what to patch. */
struct paravirt_patch_site {
	u8 *instr; 		/* original instructions */
	u8 instrtype;		/* type of this instruction */
	u8 len;			/* length of original instruction */
	u16 clobbers;		/* what registers you may clobber */
};

extern struct paravirt_patch_site __parainstructions[],
	__parainstructions_end[];

static inline unsigned long __raw_local_save_flags(void)
{
	unsigned long f;

	asm volatile(paravirt_alt("pushl %%ecx; pushl %%edx;"
				  PARAVIRT_CALL
				  "popl %%edx; popl %%ecx")
		     : "=a"(f)
		     : paravirt_type(pv_irq_ops.save_fl),
		       paravirt_clobber(CLBR_EAX)
		     : "memory", "cc");
	return f;
}

static inline void raw_local_irq_restore(unsigned long f)
{
	asm volatile(paravirt_alt("pushl %%ecx; pushl %%edx;"
				  PARAVIRT_CALL
				  "popl %%edx; popl %%ecx")
		     : "=a"(f)
		     : "0"(f),
		       paravirt_type(pv_irq_ops.restore_fl),
		       paravirt_clobber(CLBR_EAX)
		     : "memory", "cc");
}

static inline void raw_local_irq_disable(void)
{
	asm volatile(paravirt_alt("pushl %%ecx; pushl %%edx;"
				  PARAVIRT_CALL
				  "popl %%edx; popl %%ecx")
		     :
		     : paravirt_type(pv_irq_ops.irq_disable),
		       paravirt_clobber(CLBR_EAX)
		     : "memory", "eax", "cc");
}

static inline void raw_local_irq_enable(void)
{
	asm volatile(paravirt_alt("pushl %%ecx; pushl %%edx;"
				  PARAVIRT_CALL
				  "popl %%edx; popl %%ecx")
		     :
		     : paravirt_type(pv_irq_ops.irq_enable),
		       paravirt_clobber(CLBR_EAX)
		     : "memory", "eax", "cc");
}

static inline unsigned long __raw_local_irq_save(void)
{
	unsigned long f;

	f = __raw_local_save_flags();
	raw_local_irq_disable();
	return f;
}

#define CLI_STRING							\
	_paravirt_alt("pushl %%ecx; pushl %%edx;"			\
		      "call *%[paravirt_cli_opptr];"			\
		      "popl %%edx; popl %%ecx",				\
		      "%c[paravirt_cli_type]", "%c[paravirt_clobber]")

#define STI_STRING							\
	_paravirt_alt("pushl %%ecx; pushl %%edx;"			\
		      "call *%[paravirt_sti_opptr];"			\
		      "popl %%edx; popl %%ecx",				\
		      "%c[paravirt_sti_type]", "%c[paravirt_clobber]")

#define CLI_STI_CLOBBERS , "%eax"
#define CLI_STI_INPUT_ARGS						\
	,								\
	[paravirt_cli_type] "i" (PARAVIRT_PATCH(pv_irq_ops.irq_disable)),		\
	[paravirt_cli_opptr] "m" (pv_irq_ops.irq_disable),		\
	[paravirt_sti_type] "i" (PARAVIRT_PATCH(pv_irq_ops.irq_enable)),		\
	[paravirt_sti_opptr] "m" (pv_irq_ops.irq_enable),		\
	paravirt_clobber(CLBR_EAX)

/* Make sure as little as possible of this mess escapes. */
#undef PARAVIRT_CALL
#undef __PVOP_CALL
#undef __PVOP_VCALL
#undef PVOP_VCALL0
#undef PVOP_CALL0
#undef PVOP_VCALL1
#undef PVOP_CALL1
#undef PVOP_VCALL2
#undef PVOP_CALL2
#undef PVOP_VCALL3
#undef PVOP_CALL3
#undef PVOP_VCALL4
#undef PVOP_CALL4

#else  /* __ASSEMBLY__ */

#define PARA_PATCH(struct, off)	((PARAVIRT_PATCH_##struct + (off)) / 4)

#define PARA_SITE(ptype, clobbers, ops)		\
771:;						\
	ops;					\
772:;						\
	.pushsection .parainstructions,"a";	\
	 .long 771b;				\
	 .byte ptype;				\
	 .byte 772b-771b;			\
	 .short clobbers;			\
	.popsection

#define INTERRUPT_RETURN						\
	PARA_SITE(PARA_PATCH(pv_cpu_ops, PV_CPU_iret), CLBR_NONE,	\
		  jmp *%cs:pv_cpu_ops+PV_CPU_iret)

#define DISABLE_INTERRUPTS(clobbers)					\
	PARA_SITE(PARA_PATCH(pv_irq_ops, PV_IRQ_irq_disable), clobbers, \
		  pushl %eax; pushl %ecx; pushl %edx;			\
		  call *%cs:pv_irq_ops+PV_IRQ_irq_disable;		\
		  popl %edx; popl %ecx; popl %eax)			\

#define ENABLE_INTERRUPTS(clobbers)					\
	PARA_SITE(PARA_PATCH(pv_irq_ops, PV_IRQ_irq_enable), clobbers,	\
		  pushl %eax; pushl %ecx; pushl %edx;			\
		  call *%cs:pv_irq_ops+PV_IRQ_irq_enable;		\
		  popl %edx; popl %ecx; popl %eax)

#define ENABLE_INTERRUPTS_SYSEXIT					       \
	PARA_SITE(PARA_PATCH(pv_cpu_ops, PV_CPU_irq_enable_sysexit), CLBR_NONE,\
		  jmp *%cs:pv_cpu_ops+PV_CPU_irq_enable_sysexit)

#define GET_CR0_INTO_EAX			\
	push %ecx; push %edx;			\
	call *pv_cpu_ops+PV_CPU_read_cr0;	\
	pop %edx; pop %ecx

#endif /* __ASSEMBLY__ */
#endif /* CONFIG_PARAVIRT */
#endif	/* __ASM_PARAVIRT_H */
