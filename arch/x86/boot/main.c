/*i386架构启动时main文件，实模式的主模块内核代码*/

#include "boot.h"

/*启动参数，4K大小，16字节对齐*/
struct boot_params boot_params __attribute__((aligned(16)));

/*堆起始地址*/
char *HEAP = _end;
/*堆结束地址*/
char *heap_end = _end;		/* Default end of heap = no heap */

/*
 * Copy the header into the boot parameter block.  Since this
 * screws up the old-style command line protocol, adjust by
 * filling in the new-style command line pointer instead.
 */
/*复制启动扇区内容到启动参数控制块*/
static void copy_boot_params(void)
{
	struct old_cmdline
	{
		u16 cl_magic;
		u16 cl_offset;
	};
	const struct old_cmdline * const oldcmd =
		(const struct old_cmdline *)OLD_CL_ADDRESS;
	/*启动参数控制块结构4K*/
	BUILD_BUG_ON(sizeof boot_params != 4096);
	/*将启动扇区内容复制到启动控制块*/
	memcpy(&boot_params.hdr, &hdr, sizeof hdr);

	if (!boot_params.hdr.cmd_line_ptr && oldcmd->cl_magic == OLD_CL_MAGIC)
	{
		/*旧格式命令行协议*/
		u16 cmdline_seg;

		/* Figure out if the command line falls in the region
		   of memory that an old kernel would have copied up
		   to 0x90000... */
		if (oldcmd->cl_offset < boot_params.hdr.setup_move_size)
			cmdline_seg = ds();
		else
			cmdline_seg = 0x9000;

		boot_params.hdr.cmd_line_ptr =
			(cmdline_seg << 4) + oldcmd->cl_offset;
	}
}

/*
 * Set the keyboard repeat rate to maximum.  Unclear why this
 * is done here; this might be possible to kill off as stale code.
 */
static void keyboard_set_repeat(void)
{
	u16 ax = 0x0305;
	u16 bx = 0;
	asm volatile("int $0x16"
		     : "+a" (ax), "+b" (bx)
		     : : "ecx", "edx", "esi", "edi");
}

/*
 * Get Intel SpeedStep (IST) information.
 */
static void query_ist(void)
{
	asm("int $0x15"
	    : "=a" (boot_params.ist_info.signature),
	      "=b" (boot_params.ist_info.command),
	      "=c" (boot_params.ist_info.event),
	      "=d" (boot_params.ist_info.perf_level)
	    : "a" (0x0000e980),	 /* IST Support */
	      "d" (0x47534943)); /* Request value */
}

/*将cpu运行的模式告知bios。仅在x86 64位配置中设置bios模式*/
static void set_bios_mode(void)
{
#ifdef CONFIG_X86_64
	u32 eax, ebx;

	eax = 0xec00;
	ebx = 2;
	asm volatile("int $0x15"
		     : "+a" (eax), "+b" (ebx)
		     : : "ecx", "edx", "esi", "edi");
#endif
}

void main(void)
{
	/*首先，将启动扇区内容复制到第一个页帧（0页）*/
	copy_boot_params();

	/* End of heap check */
	if (boot_params.hdr.loadflags & CAN_USE_HEAP)
	{
		heap_end = (char *)(boot_params.hdr.heap_end_ptr + 0x200 - STACK_SIZE);
	}
	else
	{
		/*仅启动协议2.0使用，无堆可用*/
		puts("WARNING: Ancient bootloader, some functionality " "may be limited!\n");
	}

	/* Make sure we have all the proper CPU support */
	/*确保所有cpu*/
	if (validate_cpu())
	{
		puts("Unable to boot - please use a kernel appropriate " "for your CPU.\n");
		die();
	}

	/* Tell the BIOS what CPU mode we intend to run in. */
	/**/
	set_bios_mode();

	/* Detect memory layout */
	/*检测内存布局*/
	detect_memory();

	/* Set keyboard repeat rate (why?) */
	keyboard_set_repeat();

	/* Set the video mode */
	set_video();

	/* Query MCA information */
	query_mca();

	/* Voyager */
#ifdef CONFIG_X86_VOYAGER
	query_voyager();
#endif

	/* Query Intel SpeedStep (IST) information */
	query_ist();

	/*查询高级电源管理信息*/
#if defined(CONFIG_APM) || defined(CONFIG_APM_MODULE)
	query_apm_bios();
#endif

	/*查询EDO信息*/
#if defined(CONFIG_EDD) || defined(CONFIG_EDD_MODULE)
	query_edd();
#endif
	/*最后一件事情是调用保护模式*/
	go_to_protected_mode();
}
