/*i386�ܹ������ڼ���ڴ������*/

#include "boot.h"

/*ASCII "SMAP"*/
#define SMAP	0x534d4150

/*�ڴ��ܴ�С����Ϣ��Ϊ�豸�Ĺؼ���Ϣ��Ӧ����Ӳ���������ھ���CPU��ò��洢������ϵͳ
ֻ��Ҫͨ��CPU�����Э����ȡ���ɣ����Э������BIOS�жϡ���x86оƬ�У�̽�������ڴ沼��
�õ�BIOS�ж�������0x15������ax�Ĵ���ֵ�Ĳ�ͬ�������ֳ����ķ�ʽ��0xe820��0x801��0x88*/

/*
E820�ж�
Input:
AX = E820h
EAX = 0000E820h
EDX = 534D4150h ('SMAP')
EBX = continuation value or 00000000h to start at beginning of map
ECX = size of buffer for result, in bytes (should be >= 20 bytes)
ES:DI -> buffer for result (see #00581)
int 0x15
������E820�жϵ����룬����AX���������ţ�ECX�洢���ؽ���ĳ��ȣ���ֵ������ڵ���20��ԭ
��ο����˵����EDXĬ��Ϊ0x534D4150���ֱ����'S'��'M'��'A'��'P'���ĸ��ַ���ASCIIֵ��DI
��ʾ�洢���ؽ���ĵ�ַ������Ҫ���ES�Ĵ������ڴ棬ʵ�ʴ洢�ĵ�ַΪES:DI(ES << 4 + DI)��

Ouput:
CF clear if successful
EAX = 534D4150h ('SMAP')
ES:DI buffer filled
EBX = next offset from which to copy or 00000000h if all done
ECX = actual length returned in bytes
CF set on error
AH = error code (86h) (see #00496 at INT 15/AH=80h)
��e820�жϵ�����У�EAXĬ����0x534D4150h������ο����룬������ħ����У�飻ECX��ʾ����
����ĳ��ȣ�Ĭ����20���ֽڣ�EBX������ʶ�Ƿ������һ�0��ʾ�Ѿ�������DI��ָ�򷵻ؽ��
���������DIֵ��ͬ���������eflags��CF�ֶλᱻ��1����Ӧ��������AH�Ĵ�����
���������DIָ��ķ��ؽ����ÿһ�˵���Ĭ�Ϸ��ص���20�ֽڣ��ֱ���8�ֽڵĻ���ַ + 8�ֽ�
�Ĵ�С + 4�ֽڵ��ڴ����ͣ������ڴ����ͳ����������¼��֣�
1 -- ϵͳ�����ڴ�
2 -- ϵͳ�����ڴ棬����ROM
3 -- ACPI Reclaim�ڴ�
4 -- ACPI NVS�ڴ�
other
*/

/*ͨ��E820�ж�̽���ڴ�
������Linux 5.10.68�汾�ķ���ʵ�����������ڴ���128M����E820̽����Ϊ��

[    0.000000] BIOS-provided physical RAM map:
[    0.000000] BIOS-e820: [mem 0x0000000000000000-0x000000000009fbff] usable
[    0.000000] BIOS-e820: [mem 0x000000000009fc00-0x000000000009ffff] reserved
[    0.000000] BIOS-e820: [mem 0x00000000000f0000-0x00000000000fffff] reserved
[    0.000000] BIOS-e820: [mem 0x0000000000100000-0x0000000007fdffff] usable
[    0.000000] BIOS-e820: [mem 0x0000000007fe0000-0x0000000007ffffff] reserved
[    0.000000] BIOS-e820: [mem 0x00000000fffc0000-0x0000000

����ϵͳ������ȡ�豸���ڴ��С��
��ͨ��BIOS 0x15�жϣ�������E820��E801��E88���жϺš�

�豸�������ڴ棬����ϵͳ������ʹ����
�𣺲��ǵģ�ֻ���ڴ�����Ϊusable�Ĳ��ܱ�����ϵͳ��ʹ�á�
*/
static int detect_memory_e820(void)
{
	/*E820̽�⵽���ڴ����Ŀ*/
	int count = 0;
	u32 next = 0;
	u32 size, id;
	u8 err;
	/*����E820̽����*/
	struct e820entry *desc = boot_params.e820_map;

	do
	{
		size = sizeof(struct e820entry);

		/* Important: %edx is clobbered by some BIOSes,
		   so it must be either used for the error output
		   or explicitly marked clobbered. */
		asm("int $0x15; setc %0"
		    : "=d" (err), "+b" (next), "=a" (id), "+c" (size),
		      "=m" (*desc)
		    : "D" (desc), "d" (SMAP), "a" (0xe820));

		/* Some BIOSes stop returning SMAP in the middle of
		   the search loop.  We don't know exactly how the BIOS
		   screwed up the map at that point, we might have a
		   partial map, the full map, or complete garbage, so
		   just return failure. */
		/*У��ħ����*/
		if (id != SMAP)
		{
			count = 0;
			break;
		}

		if (err)
			break;

		count++;
		desc++;
	} while (next && count < E820MAX);

	return boot_params.e820_entries = count;
}

/*
E801�жϣ����E820�жϣ�E801�����������ͼ��˲��١�
Input:
AX = E801h
int 0x15
ֻ��һ����������������������š�
Ouput:
CF clear if successful
AX = extended memory between 1M and 16M, in K (max 3C00h = 15MB)
BX = extended memory above 16M, in 64K blocks
CX = configured memory 1M to 16M, in K
DX = configured memory above 16M, in 64K blocks
CF set on error
E801�ж�̽�⵽���ڴ��Ϊ���Σ�С��16MB�������ڴ沼�ִ洢��AX�У�������16MB��С��4GB��
�ڴ�洢��BX�С�����AX�ĵ�λΪK���������ֵΪ0x3C00�����Ե�һ�η�Χ��0-15M��BX�ĵ�λ��
64K����2^16 * 64K = 4G���ʵڶ��η�Χ��16MB-4GB��Ҳ����˵BX�ķ�Χ��0��256-65536��������
������ΪʲôAX�����ֵ��0x3C00������������ʷԤ�����µģ���80286ʱ����24λ�ĵ�ַ�����
Ѱַ�ռ�λ16M������ʱһЩISA�豸Ҫ��15M���ϵ��ڴ���Ϊ������������15MB-16MB�Ŀռ���Ϊϵ
ͳԤ������͵������������ڴ�ն���*/
static int detect_memory_e801(void)
{
	u16 ax, bx, cx, dx;
	u8 err;

	bx = cx = dx = 0;
	ax = 0xe801;
	asm("stc; int $0x15; setc %0"
	    : "=m" (err), "+a" (ax), "+b" (bx), "+c" (cx), "+d" (dx));

	if (err)
		return -1;

	/* Do we really need to do this? */
	if (cx || dx)
	{
		ax = cx;
		bx = dx;
	}

	if (ax > 15*1024)
		return -1;	/* Bogus! */

	/* This ignores memory above 16MB if we have a memory hole
	   there.  If someone actually finds a machine with a memory
	   hole at 16MB and no support for 0E820h they should probably
	   generate a fake e820 map. */
	boot_params.alt_mem_k = (ax == 15*1024) ? (dx << 6)+ax : ax;

	return 0;
}

/*
���E820��E88�жϿ���˵�ǳ����ˡ�
Input:
AH = 88h
int 0x15
ͬ����ֻ��һ�����������
Output:
CF clear if successful
AX = number of contiguous KB starting at absolute address 100000h
CF set on error
AH = status
80h invalid command (PC,PCjr)
86h unsupported function (XT,PS30)
ֻ��̽��1MB���ϣ�64MB���µ��ڴ沼�֣������ŵ�AX�С�
*/
static int detect_memory_88(void)
{
	u16 ax;
	u8 err;

	ax = 0x8800;
	asm("stc; int $0x15; setc %0" : "=bcdm" (err), "+a" (ax));
	boot_params.screen_info.ext_mem_k = ax;

	return -err;
}

/*�����ڴ沼��̽�������*/
int detect_memory(void)
{
	int err = -1;
	/*ʹ��e820 BIOS�жϻ�ȡ�����ڴ沼��*/
	if (detect_memory_e820() > 0)
		err = 0;
	/*ʹ��e801 BIOS�жϻ�ȡ�����ڴ沼��*/
	if (!detect_memory_e801())
		err = 0;
	/*ʹ��e88 BIOS�жϻ�ȡ�����ڴ沼��*/
	if (!detect_memory_88())
		err = 0;

	return err;
}
