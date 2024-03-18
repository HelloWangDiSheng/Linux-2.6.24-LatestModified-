/*i386�ܹ�����ʱmain�ļ���ʵģʽ����ģ���ں˴���*/

#include "boot.h"

/*����������4K��С��16�ֽڶ���*/
struct boot_params boot_params __attribute__((aligned(16)));

/*����ʼ��ַ*/
char *HEAP = _end;
/*�ѽ�����ַ*/
char *heap_end = _end;		/* Default end of heap = no heap */

/*
 * Copy the header into the boot parameter block.  Since this
 * screws up the old-style command line protocol, adjust by
 * filling in the new-style command line pointer instead.
 */
/*���������������ݵ������������ƿ�*/
static void copy_boot_params(void)
{
	struct old_cmdline
	{
		u16 cl_magic;
		u16 cl_offset;
	};
	const struct old_cmdline * const oldcmd =
		(const struct old_cmdline *)OLD_CL_ADDRESS;
	/*�����������ƿ�ṹ4K*/
	BUILD_BUG_ON(sizeof boot_params != 4096);
	/*�������������ݸ��Ƶ��������ƿ�*/
	memcpy(&boot_params.hdr, &hdr, sizeof hdr);

	if (!boot_params.hdr.cmd_line_ptr && oldcmd->cl_magic == OLD_CL_MAGIC)
	{
		/*�ɸ�ʽ������Э��*/
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

/*��cpu���е�ģʽ��֪bios������x86 64λ����������biosģʽ*/
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
	/*���ȣ��������������ݸ��Ƶ���һ��ҳ֡��0ҳ��*/
	copy_boot_params();

	/* End of heap check */
	if (boot_params.hdr.loadflags & CAN_USE_HEAP)
	{
		heap_end = (char *)(boot_params.hdr.heap_end_ptr + 0x200 - STACK_SIZE);
	}
	else
	{
		/*������Э��2.0ʹ�ã��޶ѿ���*/
		puts("WARNING: Ancient bootloader, some functionality " "may be limited!\n");
	}

	/* Make sure we have all the proper CPU support */
	/*ȷ������cpu*/
	if (validate_cpu())
	{
		puts("Unable to boot - please use a kernel appropriate " "for your CPU.\n");
		die();
	}

	/* Tell the BIOS what CPU mode we intend to run in. */
	/**/
	set_bios_mode();

	/* Detect memory layout */
	/*����ڴ沼��*/
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

	/*��ѯ�߼���Դ������Ϣ*/
#if defined(CONFIG_APM) || defined(CONFIG_APM_MODULE)
	query_apm_bios();
#endif

	/*��ѯEDO��Ϣ*/
#if defined(CONFIG_EDD) || defined(CONFIG_EDD_MODULE)
	query_edd();
#endif
	/*���һ�������ǵ��ñ���ģʽ*/
	go_to_protected_mode();
}
