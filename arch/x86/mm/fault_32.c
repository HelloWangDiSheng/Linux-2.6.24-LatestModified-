#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/vt_kern.h>		/* For unblank_screen() */
#include <linux/highmem.h>
#include <linux/bootmem.h>		/* for max_low_pfn */
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/kdebug.h>
#include <linux/kprobes.h>

#include <asm/system.h>
#include <asm/desc.h>
#include <asm/segment.h>

extern void die(const char *,struct pt_regs *,long);

#ifdef CONFIG_KPROBES
/**/
static inline int notify_page_fault(struct pt_regs *regs)
{
	int ret = 0;

	/* kprobe_running() needs smp_processor_id() */
	if (!user_mode_vm(regs))
	{
		preempt_disable();
		if (kprobe_running() && kprobe_fault_handler(regs, 14))
			ret = 1;
		preempt_enable();
	}

	return ret;
}
#else
static inline int notify_page_fault(struct pt_regs *regs)
{
	return 0;
}
#endif

/*
 * Return EIP plus the CS segment base.  The segment limit is also
 * adjusted, clamped to the kernel/user address space (whichever is
 * appropriate), and returned in *eip_limit.
 *
 * The segment is checked, because it might have been changed by another
 * task between the original faulting instruction and here.
 *
 * If CS is no longer a valid code segment, or if EIP is beyond the
 * limit, or if it is a kernel address when CS is not a kernel segment,
 * then the returned value will be greater than *eip_limit.
 *
 * This is slow, but is very rarely executed.
 */
static inline unsigned long get_segment_eip(struct pt_regs *regs, unsigned long *eip_limit)
{
	unsigned long eip = regs->eip;
	unsigned seg = regs->xcs & 0xffff;
	u32 seg_ar, seg_limit, base, *desc;

	/* Unlikely, but must come before segment checks. */
	if (unlikely(regs->eflags & VM_MASK)) {
		base = seg << 4;
		*eip_limit = base + 0xffff;
		return base + (eip & 0xffff);
	}

	/* The standard kernel/user address space limit. */
	*eip_limit = user_mode(regs) ? USER_DS.seg : KERNEL_DS.seg;

	/* By far the most common cases. */
	if (likely(SEGMENT_IS_FLAT_CODE(seg)))
		return eip;

	/* Check the segment exists, is within the current LDT/GDT size,
	   that kernel/user (ring 0..3) has the appropriate privilege,
	   that it's a code segment, and get the limit. */
	__asm__ ("larl %3,%0; lsll %3,%1"
		 : "=&r" (seg_ar), "=r" (seg_limit) : "0" (0), "rm" (seg));
	if ((~seg_ar & 0x9800) || eip > seg_limit) {
		*eip_limit = 0;
		return 1;	 /* So that returned eip > *eip_limit. */
	}

	/* Get the GDT/LDT descriptor base.
	   When you look for races in this code remember that
	   LDT and other horrors are only used in user space. */
	if (seg & (1<<2)) {
		/* Must lock the LDT while reading it. */
		mutex_lock(&current->mm->context.lock);
		desc = current->mm->context.ldt;
		desc = (void *)desc + (seg & ~7);
	} else {
		/* Must disable preemption while reading the GDT. */
 		desc = (u32 *)get_cpu_gdt_table(get_cpu());
		desc = (void *)desc + (seg & ~7);
	}

	/* Decode the code segment base from the descriptor */
	base = get_desc_base((unsigned long *)desc);

	if (seg & (1<<2)) {
		mutex_unlock(&current->mm->context.lock);
	} else
		put_cpu();

	/* Adjust EIP and segment limit, and clamp at the kernel limit.
	   It's legitimate for segments to wrap at 0xffffffff. */
	seg_limit += base;
	if (seg_limit < *eip_limit && seg_limit >= base)
		*eip_limit = seg_limit;
	return eip + base;
}

/*
 * Sometimes AMD Athlon/Opteron CPUs report invalid exceptions on prefetch.
 * Check that here and ignore it.
 */
static int __is_prefetch(struct pt_regs *regs, unsigned long addr)
{
	unsigned long limit;
	unsigned char *instr = (unsigned char *)get_segment_eip (regs, &limit);
	int scan_more = 1;
	int prefetch = 0;
	int i;

	for (i = 0; scan_more && i < 15; i++) {
		unsigned char opcode;
		unsigned char instr_hi;
		unsigned char instr_lo;

		if (instr > (unsigned char *)limit)
			break;
		if (probe_kernel_address(instr, opcode))
			break;

		instr_hi = opcode & 0xf0;
		instr_lo = opcode & 0x0f;
		instr++;

		switch (instr_hi) {
		case 0x20:
		case 0x30:
			/* Values 0x26,0x2E,0x36,0x3E are valid x86 prefixes. */
			scan_more = ((instr_lo & 7) == 0x6);
			break;

		case 0x60:
			/* 0x64 thru 0x67 are valid prefixes in all modes. */
			scan_more = (instr_lo & 0xC) == 0x4;
			break;
		case 0xF0:
			/* 0xF0, 0xF2, and 0xF3 are valid prefixes */
			scan_more = !instr_lo || (instr_lo>>1) == 1;
			break;
		case 0x00:
			/* Prefetch instruction is 0x0F0D or 0x0F18 */
			scan_more = 0;
			if (instr > (unsigned char *)limit)
				break;
			if (probe_kernel_address(instr, opcode))
				break;
			prefetch = (instr_lo == 0xF) &&
				(opcode == 0x0D || opcode == 0x18);
			break;
		default:
			scan_more = 0;
			break;
		}
	}
	return prefetch;
}

static inline int is_prefetch(struct pt_regs *regs, unsigned long addr,
			      unsigned long error_code)
{
	if (unlikely(boot_cpu_data.x86_vendor == X86_VENDOR_AMD &&
		     boot_cpu_data.x86 >= 6)) {
		/* Catch an obscure case of prefetch inside an NX page. */
		if (nx_enabled && (error_code & 16))
			return 0;
		return __is_prefetch(regs, addr);
	}
	return 0;
}

static noinline void force_sig_info_fault(int si_signo, int si_code,
	unsigned long address, struct task_struct *tsk)
{
	siginfo_t info;

	info.si_signo = si_signo;
	info.si_errno = 0;
	info.si_code = si_code;
	info.si_addr = (void __user *)address;
	force_sig_info(si_signo, &info, tsk);
}

fastcall void do_invalid_op(struct pt_regs *, unsigned long);

/*获取进程页表中的中间页目录项地址*/
static inline pmd_t *vmalloc_sync_one(pgd_t *pgd, unsigned long address)
{
	/*获取pgd索引*/
	unsigned index = pgd_index(address);
	pgd_t *pgd_k;
	pud_t *pud, *pud_k;
	pmd_t *pmd, *pmd_k;

	/*获取进程页表的全局页目录项*/
	pgd += index;
	/*获取内核页表的全局页目录项，也即swapper_pg_dir[index]*/
	pgd_k = init_mm.pgd + index;
	/*进程的内核页表中全局页目录项不存在，返回NULL*/
	if (!pgd_present(*pgd_k))
		return NULL;

	/*
	 * set_pgd(pgd, *pgd_k); here would be useless on PAE
	 * and redundant with the set_pmd() on non-PAE. As would
	 * set_pud.
	 */
	/*获取进程页表中上层页目录项地址*/
	pud = pud_offset(pgd, address);
	/*获取内核页表中上层页目录项地址*/
	pud_k = pud_offset(pgd_k, address);
	/*内核页表中上层页目录项不存在时返回NULL*/
	if (!pud_present(*pud_k))
		return NULL;
	/*获取进程页表中的中间页目录项地址*/
	pmd = pmd_offset(pud, address);
	/*获取内核页表中的中间页目录项地址*/
	pmd_k = pmd_offset(pud_k, address);
	/*内核页表中中间页目录项地址不存在，返回NULL*/
	if (!pmd_present(*pmd_k))
		return NULL;
	/*进程页表中中间页目录项不存在时，同步中间页目录项*/
	if (!pmd_present(*pmd))
	{
		/*进程页表中中间页目录项和内核页表中中间页目录项同步*/
		set_pmd(pmd, *pmd_k);
		/**/
		arch_flush_lazy_mmu_mode();
	}
	else
		/*内核页表中的中间页目录项应该和进程页表中的中间页目录项相等*/
		BUG_ON(pmd_page(*pmd) != pmd_page(*pmd_k));
	return pmd_k;
}

/*处理VMALLOC或者模块映射区中的缺页中断，该函数假定处理区中没有大页。如果缺页地址的
页表在内存中，返回0，否则返回-1*/
static inline int vmalloc_fault(unsigned long address)
{
	unsigned long pgd_paddr;
	pmd_t *pmd_k;
	pte_t *pte_k;

	/*通过引用页表，同步获取进程的顶层页表，此处不能使用current，因为可能处于进程
	切换中的中断状态。该函数获取全局页目录项swapper_pg_dir*/
	pgd_paddr = read_cr3();
	/*获取进程页表中的中间页目录项地址*/
	pmd_k = vmalloc_sync_one(__va(pgd_paddr), address);
	/*进程中间页目录项为空则返回-1*/
	if (!pmd_k)
		return -1;
	/*获取页表项地址*/
	pte_k = pte_offset_kernel(pmd_k, address);
	/*页表项不在内存中，返回-1*/
	if (!pte_present(*pte_k))
		return -1;
	return 0;
}

int show_unhandled_signals = 1;

/*
 * This routine handles page faults.  It determines the address,
 * and the problem, and then passes it off to one of the appropriate
 * routines.
 *
 * error_code:
 *	bit 0 == 0 means no page found, 1 means protection fault
 *	bit 1 == 0 means read, 1 means write
 *	bit 2 == 0 means kernel, 1 means user-mode
 *	bit 3 == 1 means use of reserved bit detected
 *	bit 4 == 1 means fault was an instruction fetch
 */
/*缺页中断产生的根本原因是由于CPU访问的这段虚拟内存背后没有物理内存与之映射，表现的具
体形式主要有三种：（1）虚拟内存对应在进程页表体系中的相关各级页目录或者页表是空的，也
就是说这段虚拟内存完全没有被映射过（2）虚拟内存之前被映射过，其在进程页表的各级页目录
以及页表中均有对应的页目录项和页表项，但是其对应的物理内存被内核交换到磁盘上了（3）虚
拟内存虽然背后映射着物理内存，但是由于对物理内存的访问权限不够而导致的保护类型的缺页中
断。比如，尝试去写一个只读的物理内存页*/
/*事实上，内核空间里vmalloc映射区中发生的缺页中断与用户空间里文件映射与匿名映射区以及
堆中发生的缺页中断是不一样的。进程在用户空间中无论是通过brk系统调用在堆中申请内存还是
通过mmap系统调用在文件与匿名映射区中申请内存，内核都只是在相应的虚拟内存空间中划分出一
段虚拟内存来给进程使用。当进程真正访问到这段虚拟内存地址的时候，才会产生缺页中断，近而
才会分配物理内存，最后将引起本次缺页的虚拟地址在进程页表中对应的全局页目录项pgd，上层
页目录项pud，中间页目录pmd，页表项pte都创建好，然后在pte中将虚拟内存地址与物理内存地址
映射起来。而内核通过vmalloc内存分配接口在vmalloc映射区申请内存的时候，首先会在vmalloc
映射区中划分出一段未被使用的虚拟内存区域出来，暂且叫这段虚拟内存区域为vmalloc区，这一
点和前面文章介绍的mmap非常相似，只不过mmap工作在用户空间的文件与匿名映射区，vmalloc工
作在内核空间的vmalloc映射区。内核空间中的vmalloc映射区就是由这样一段一段的vmalloc区组
成的，每调用一次vmalloc内存分配接口，就会在vmalloc映射区中映射出一段vmalloc虚拟内存区
域，而且每个vmalloc区之间隔着一个4K大小的guardpage（虚拟内存），用于防止内存越界，将这
些非连续的物理内存区域隔离起来。和mmap不同的是，vmalloc在分配完虚拟内存之后，会马上为
这段虚拟内存分配物理内存，内核会首先计算出由vmalloc内存分配接口映射出的这一段虚拟内存
区域vmalloc区中包含的虚拟内存页数，然后调用伙伴系统依次为这些虚拟内存页分配物理内存页
*/
/**/
/**/
/**/
/**/
/**/
/**/
/*P(0)如果error_code第0个比特位置为0，表示该缺页异常是由于CPU访问的这个虚拟内存地址
背后并没有一个物理内存页与之映射而引起的，站在进程页的角度来说，就是CPU访问的这个虚
拟内存地址在进程四级页表体系中对应的各级页目录项或者页表项是空（页目录项或者页表项中
的P位为0）。如果error_code第0个比特位置为1，表示CPU访问的这个虚拟内存地址背后虽然有
物理内存页与之映射，是由于访问权限不够而引起的缺页异常（保护异常），比如，进程尝试对
一个只读的物理内存页进行写操作，那么就会引起写保护类型的缺页异常。
R/W(1):如果error_code第1个比特位置为0，表示是由于读访问引起的。置为1表示是由于写访问
引起的。注意：该标志位只是为了描述是哪种访问类型造成了本次缺页异常，这个和前面提到的
访问权限没有关系。比如，进程尝试对一个可写的虚拟内存页进行写入，访问权限没有问题，但
是该虚拟内存页背后并未有物理内存与之关联，也会导致缺页异常。这种情况下，error_code的
P位就会设置为0，R/W位就会设置为1。
U/S(2)：表示缺页异常发生在用户态还是内核态，error_code第2个比特位设置为0表示CPU访问内
核空间的地址引起的缺页异常，设置为1表示CPU访问用户空间的地址引起的缺页异常。
RSVD(3)：这里用于检测页表项中的保留位（Reserved相关的比特位）是否设置，这些页表项中的
保留位都是预留给内核的相关功能使用的，所以在缺页的时候需要检查这些保留位是否设置，从
而决定近一步的扩展处理。设置为1表示页表项中预留的这些比特位被使用了。设置为0表示页表
项中预留的这些比特位还没有被使用。
I/D(4)：设置为1，表示本次缺页异常是在CPU获取指令的时候引起的。
PK(5)：设置为1，表示引起缺页异常的虚拟内存地址对应页表项中的Protection相关的比特位被
设置了*/
/*
					IA-32上缺页异常错误代码的语义
比特位			置位（1）										未置位（0）
0  			缺页  										保护异常（没有足够的访问权限）
1  			读访问  										写访问
2  			核心态											用户状态
3  			表示检测到使用了保留位
4  			表示缺页异常是在取指令时出现的

*/
fastcall void __kprobes do_page_fault(struct pt_regs *regs, unsigned long error_code)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	struct vm_area_struct * vma;
	unsigned long address;
	int write, si_code;
	int fault;

	/*
	 * We can fault from pretty much anywhere, with unknown IRQ state.
	 */
	trace_hardirqs_fixup();

	/*获取异常地址*/
	address = read_cr2();
	/*获取当前进程*/
	tsk = current;
	/*预设错误码为对象未映射*/
	si_code = SEGV_MAPERR;
	/*因异常而进入到内核虚拟内存空间。参考页表为init_mm.pgd。要注意！对这种情况不能
	获取任何锁。可能是在中断或临界区中，只应当从主页表复制信息，不允许其他操作。下述
	代码验证了异常发生于内核空间(error_code & 4) == 0，而且异常不是保护错误
	(error_code & 9) == 0*/
	if (unlikely(address >= TASK_SIZE))
	{
		/*错误码不是0x1101（用户态下读一个已存在的页帧，但权限不足。并且检测到了保留位）
		且页表项存在*/
		if (!(error_code & 0x0000000d) && vmalloc_fault(address) >= 0)
			return;
		/**/
		if (notify_page_fault(regs))
			return;
		/*
		 * Don't take the mm semaphore here. If we fixup a prefetch
		 * fault we could otherwise deadlock.
		 */
		goto bad_area_nosemaphore;
	}
	/*缺页中断发生在用户空间中*/
	if (notify_page_fault(regs))
		return;
	/*cr2寄存取得值已经保存并且vmalloc区域的缺页中断已经处理后可以允许本地cpu中断*/
	if (regs->eflags & (X86_EFLAGS_IF|VM_MASK))
		local_irq_enable();

	/*获取当前进程的虚拟地址空间*/
	mm = tsk->mm;
	/*如果正处于中断、没有用户上下文或正处于原子区域时不能处理缺页中断*/
	if (in_atomic() || !mm)
		goto bad_area_nosemaphore;

	/* When running in the kernel we expect faults to occur only to
	 * addresses in user space.  All other faults represent errors in the
	 * kernel and should generate an OOPS.  Unfortunately, in the case of an
	 * erroneous fault occurring in a code path which already holds mmap_sem
	 * we will deadlock attempting to validate the fault against the
	 * address space.  Luckily the kernel only validly references user
	 * space from well defined areas of code, which are listed in the
	 * exceptions table.
	 *
	 * As the vast majority of faults will be valid we will only perform
	 * the source reference check when there is a possibility of a deadlock.
	 * Attempt to lock the address space, if we cannot we then validate the
	 * source.  If this is invalid we can skip the address space check,
	 * thus avoiding the deadlock.
	 */
	if (!down_read_trylock(&mm->mmap_sem))
	{
		/*用户态缺页，查询下条指令对应的异常表中可能发生错误或异常的地址，如果没有
		找到，则跳转到错误处理*/
		if ((error_code & 4) == 0 && !search_exception_tables(regs->eip))
			goto bad_area_nosemaphore;
		down_read(&mm->mmap_sem);
	}
	/*查询进程虚拟地址空间中出错的vma*/
	vma = find_vma(mm, address);
	/*出错地址没有在现存vma区域中，则跳转到错误处理*/
	if (!vma)
		goto bad_area;
	/*查找到出错的vma，则跳转到*/
	if (vma->vm_start <= address)
		goto good_area;
	/*出错地址不是栈vma以下的待扩展区域，跳转到错误处理*/
	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;
	/*出错地址位于用户空间的栈代扩展的低地址*/
	if (error_code & 4)
	{
		/*
		 * Accessing the stack below %esp is always a bug.
		 * The large cushion allows instructions like enter
		 * and pusha to work.  ("enter $65535,$31" pushes
		 * 32 pointers and then decrements %esp by 65535.)
		 */
		 /**/
		if (address + 65536 + 32 * sizeof(unsigned long) < regs->esp)
			goto bad_area;
	}
	/*扩展栈*/
	if (expand_stack(vma, address))
		goto bad_area;
/*
 * Ok, we have a good vm_area for this memory access, so
 * we can handle it..
 */
good_area:
	si_code = SEGV_ACCERR;
	write = 0;
	switch (error_code & 3)
	{
		/*3是写一个已在内存中的页帧*/
		default:
		/*写一个不在内存中的页帧（可能未建立映射。也可能建立映射后页被换出到交换区了）*/
		case 2:
			/*如果该vma没有写权限，则错误*/
			if (!(vma->vm_flags & VM_WRITE))
				goto bad_area;
			write++;
			break;
		/*读一个已在内存中的页帧*/
		case 1:
			goto bad_area;
		/*读一个不在内存中的页帧（未建立映射或内存页已被换出到交换区）。如果该vma没有
		读、写、执行权限，错误*/
		case 0:
			if (!(vma->vm_flags & (VM_READ | VM_EXEC | VM_WRITE)))
				goto bad_area;
	}

 survive:
	/*
	 * If for any reason at all we couldn't handle the fault,
	 * make sure we exit gracefully rather than endlessly redo
	 * the fault.
	 */
	fault = handle_mm_fault(mm, vma, address, write);
	if (unlikely(fault & VM_FAULT_ERROR))
	{
		if (fault & VM_FAULT_OOM)
			goto out_of_memory;
		else if (fault & VM_FAULT_SIGBUS)
			goto do_sigbus;
		BUG();
	}
	if (fault & VM_FAULT_MAJOR)
		tsk->maj_flt++;
	else
		tsk->min_flt++;

	/*
	 * Did it hit the DOS screen memory VA from vm86 mode?
	 */
	if (regs->eflags & VM_MASK) {
		unsigned long bit = (address - 0xA0000) >> PAGE_SHIFT;
		if (bit < 32)
			tsk->thread.screen_bitmap |= 1 << bit;
	}
	up_read(&mm->mmap_sem);
	return;

/*
 * Something tried to access memory that isn't in our memory map..
 * Fix it, but check if it's kernel or user first..
 */
bad_area:
	/*释放虚拟地址空间中mmap信号量*/
	up_read(&mm->mmap_sem);

bad_area_nosemaphore:
	/* User mode accesses just cause a SIGSEGV */
	if (error_code & 4)
	{
		/*
		 * It's possible to have interrupts off here.
		 */
		local_irq_enable();

		/*
		 * Valid to do another page fault here because this one came
		 * from user space.
		 */
		if (is_prefetch(regs, address, error_code))
			return;

		if (show_unhandled_signals && unhandled_signal(tsk, SIGSEGV) &&
		    printk_ratelimit())
		{
			printk("%s%s[%d]: segfault at %08lx eip %08lx "
			    "esp %08lx error %lx\n",
			    task_pid_nr(tsk) > 1 ? KERN_INFO : KERN_EMERG,
			    tsk->comm, task_pid_nr(tsk), address, regs->eip,
			    regs->esp, error_code);
		}
		tsk->thread.cr2 = address;
		/* Kernel addresses are always protection faults */
		tsk->thread.error_code = error_code | (address >= TASK_SIZE);
		tsk->thread.trap_no = 14;
		force_sig_info_fault(SIGSEGV, si_code, address, tsk);
		return;
	}

#ifdef CONFIG_X86_F00F_BUG
	/*
	 * Pentium F0 0F C7 C8 bug workaround.
	 */
	if (boot_cpu_data.f00f_bug) {
		unsigned long nr;

		nr = (address - idt_descr.address) >> 3;

		if (nr == 6) {
			do_invalid_op(regs, 0);
			return;
		}
	}
#endif

no_context:
	/* Are we prepared to handle this kernel fault?  */
	if (fixup_exception(regs))
		return;

	/*
	 * Valid to do another page fault here, because if this fault
	 * had been triggered by is_prefetch fixup_exception would have
	 * handled it.
	 */
 	if (is_prefetch(regs, address, error_code))
 		return;

/*
 * Oops. The kernel tried to access some bad page. We'll have to
 * terminate things with extreme prejudice.
 */

	bust_spinlocks(1);

	if (oops_may_print()) {
		__typeof__(pte_val(__pte(0))) page;

#ifdef CONFIG_X86_PAE
		if (error_code & 16) {
			pte_t *pte = lookup_address(address);

			if (pte && pte_present(*pte) && !pte_exec_kernel(*pte))
				printk(KERN_CRIT "kernel tried to execute "
					"NX-protected page - exploit attempt? "
					"(uid: %d)\n", current->uid);
		}
#endif
		if (address < PAGE_SIZE)
			printk(KERN_ALERT "BUG: unable to handle kernel NULL "
					"pointer dereference");
		else
			printk(KERN_ALERT "BUG: unable to handle kernel paging"
					" request");
		printk(" at virtual address %08lx\n",address);
		printk(KERN_ALERT "printing eip: %08lx ", regs->eip);

		page = read_cr3();
		page = ((__typeof__(page) *) __va(page))[address >> PGDIR_SHIFT];
#ifdef CONFIG_X86_PAE
		printk("*pdpt = %016Lx ", page);
		if ((page >> PAGE_SHIFT) < max_low_pfn
		    && page & _PAGE_PRESENT) {
			page &= PAGE_MASK;
			page = ((__typeof__(page) *) __va(page))[(address >> PMD_SHIFT)
			                                         & (PTRS_PER_PMD - 1)];
			printk(KERN_CONT "*pde = %016Lx ", page);
			page &= ~_PAGE_NX;
		}
#else
		printk("*pde = %08lx ", page);
#endif

		/*
		 * We must not directly access the pte in the highpte
		 * case if the page table is located in highmem.
		 * And let's rather not kmap-atomic the pte, just in case
		 * it's allocated already.
		 */
		if ((page >> PAGE_SHIFT) < max_low_pfn
		    && (page & _PAGE_PRESENT)
		    && !(page & _PAGE_PSE)) {
			page &= PAGE_MASK;
			page = ((__typeof__(page) *) __va(page))[(address >> PAGE_SHIFT)
			                                         & (PTRS_PER_PTE - 1)];
			printk("*pte = %0*Lx ", sizeof(page)*2, (u64)page);
		}

		printk("\n");
	}

	tsk->thread.cr2 = address;
	tsk->thread.trap_no = 14;
	tsk->thread.error_code = error_code;
	die("Oops", regs, error_code);
	bust_spinlocks(0);
	do_exit(SIGKILL);

/*
 * We ran out of memory, or some other thing happened to us that made
 * us unable to handle the page fault gracefully.
 */
out_of_memory:
	up_read(&mm->mmap_sem);
	if (is_global_init(tsk)) {
		yield();
		down_read(&mm->mmap_sem);
		goto survive;
	}
	printk("VM: killing process %s\n", tsk->comm);
	if (error_code & 4)
		do_group_exit(SIGKILL);
	goto no_context;

do_sigbus:
	up_read(&mm->mmap_sem);

	/* Kernel mode? Handle exceptions or die */
	if (!(error_code & 4))
		goto no_context;

	/* User space => ok to do another page fault */
	if (is_prefetch(regs, address, error_code))
		return;

	tsk->thread.cr2 = address;
	tsk->thread.error_code = error_code;
	tsk->thread.trap_no = 14;
	force_sig_info_fault(SIGBUS, BUS_ADRERR, address, tsk);
}

/**/
void vmalloc_sync_all(void)
{
	/*
	 * Note that races in the updates of insync and start aren't
	 * problematic: insync can only get set bits added, and updates to
	 * start are only improving performance (without affecting correctness
	 * if undone).
	 */
	static DECLARE_BITMAP(insync, PTRS_PER_PGD);
	static unsigned long start = TASK_SIZE;
	unsigned long address;

	if (SHARED_KERNEL_PMD)
		return;

	BUILD_BUG_ON(TASK_SIZE & ~PGDIR_MASK);
	for (address = start; address >= TASK_SIZE; address += PGDIR_SIZE)
	{
		if (!test_bit(pgd_index(address), insync))
		{
			unsigned long flags;
			struct page *page;

			spin_lock_irqsave(&pgd_lock, flags);
			for (page = pgd_list; page; page =	(struct page *)page->index)
				if (!vmalloc_sync_one(page_address(page),	address))
				{
					BUG_ON(page != pgd_list);
					break;
				}
			spin_unlock_irqrestore(&pgd_lock, flags);
			if (!page)
				set_bit(pgd_index(address), insync);
		}
		if (address == start && test_bit(pgd_index(address), insync))
			start = address + PGDIR_SIZE;
	}
}
