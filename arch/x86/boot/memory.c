/*i386架构启动期间的内存检测代码*/

#include "boot.h"

/*ASCII "SMAP"*/
#define SMAP	0x534d4150

/*内存总大小等信息作为设备的关键信息，应该在硬件启动初期就由CPU获得并存储，操作系统
只需要通过CPU的相关协定读取即可，这个协定就是BIOS中断。在x86芯片中，探测物理内存布局
用的BIOS中断向量是0x15，根据ax寄存器值的不同，有三种常见的方式：0xe820，0x801和0x88*/

/*
E820中断
Input:
AX = E820h
EAX = 0000E820h
EDX = 534D4150h ('SMAP')
EBX = continuation value or 00000000h to start at beginning of map
ECX = size of buffer for result, in bytes (should be >= 20 bytes)
ES:DI -> buffer for result (see #00581)
int 0x15
以上是E820中断的输入，其中AX是子向量号；ECX存储返回结果的长度，其值必须大于等于20，原
因参考输出说明；EDX默认为0x534D4150，分别代表'S'，'M'，'A'，'P'这四个字符的ASCII值；DI
表示存储返回结果的地址，这里要结合ES寄存器的内存，实际存储的地址为ES:DI(ES << 4 + DI)。

Ouput:
CF clear if successful
EAX = 534D4150h ('SMAP')
ES:DI buffer filled
EBX = next offset from which to copy or 00000000h if all done
ECX = actual length returned in bytes
CF set on error
AH = error code (86h) (see #00496 at INT 15/AH=80h)
在e820中断的输出中，EAX默认是0x534D4150h，意义参考输入，用来做魔术字校验；ECX表示返回
结果的长度，默认是20个字节；EBX用来标识是否是最后一项，0表示已经结束；DI是指向返回结果
，和输入的DI值相同；如果出错，eflags的CF字段会被置1，相应错误码在AH寄存器。
这里分析下DI指向的返回结果。每一趟调用默认返回的是20字节，分别是8字节的基地址 + 8字节
的大小 + 4字节的内存类型，其中内存类型常见的有如下几种：
1 -- 系统可用内存
2 -- 系统保留内存，例如ROM
3 -- ACPI Reclaim内存
4 -- ACPI NVS内存
other
*/

/*通过E820中断探测内存
以下是Linux 5.10.68版本的仿真实例，总物理内存是128M，其E820探测结果为：

[    0.000000] BIOS-provided physical RAM map:
[    0.000000] BIOS-e820: [mem 0x0000000000000000-0x000000000009fbff] usable
[    0.000000] BIOS-e820: [mem 0x000000000009fc00-0x000000000009ffff] reserved
[    0.000000] BIOS-e820: [mem 0x00000000000f0000-0x00000000000fffff] reserved
[    0.000000] BIOS-e820: [mem 0x0000000000100000-0x0000000007fdffff] usable
[    0.000000] BIOS-e820: [mem 0x0000000007fe0000-0x0000000007ffffff] reserved
[    0.000000] BIOS-e820: [mem 0x00000000fffc0000-0x0000000

操作系统怎样获取设备总内存大小？
答：通过BIOS 0x15中断，常见有E820、E801和E88子中断号。

设备的所有内存，操作系统都可以使用吗？
答：不是的，只有内存类型为usable的才能被操作系统所使用。
*/
static int detect_memory_e820(void)
{
	/*E820探测到的内存块数目*/
	int count = 0;
	u32 next = 0;
	u32 size, id;
	u8 err;
	/*保存E820探测结果*/
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
		/*校验魔术字*/
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
E801中断：相比E820中断，E801的输出和输出就简单了不少。
Input:
AX = E801h
int 0x15
只有一个输入参数，就是子向量号。
Ouput:
CF clear if successful
AX = extended memory between 1M and 16M, in K (max 3C00h = 15MB)
BX = extended memory above 16M, in 64K blocks
CX = configured memory 1M to 16M, in K
DX = configured memory above 16M, in 64K blocks
CF set on error
E801中断探测到的内存分为两段：小于16MB的物理内存布局存储到AX中，而大于16MB且小于4GB的
内存存储到BX中。由于AX的单位为K，并且最大值为0x3C00，所以第一段范围是0-15M；BX的单位是
64K，而2^16 * 64K = 4G，故第二段范围是16MB-4GB（也就是说BX的范围是0或256-65536）。这里
介绍下为什么AX的最大值是0x3C00。这是由于历史预留导致的，在80286时代，24位的地址线最大
寻址空间位16M，而当时一些ISA设备要将15M以上的内存作为缓冲区，所以15MB-16MB的空间作为系
统预留，这就导致了著名的内存空洞。*/
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
相比E820，E88中断可以说非常简单了。
Input:
AH = 88h
int 0x15
同样，只有一个输入参数。
Output:
CF clear if successful
AX = number of contiguous KB starting at absolute address 100000h
CF set on error
AH = status
80h invalid command (PC,PCjr)
86h unsupported function (XT,PS30)
只能探测1MB以上，64MB以下的内存布局，结果存放到AX中。
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

/*物理内存布局探测总入口*/
int detect_memory(void)
{
	int err = -1;
	/*使用e820 BIOS中断获取物理内存布局*/
	if (detect_memory_e820() > 0)
		err = 0;
	/*使用e801 BIOS中断获取物理内存布局*/
	if (!detect_memory_e801())
		err = 0;
	/*使用e88 BIOS中断获取物理内存布局*/
	if (!detect_memory_88())
		err = 0;

	return err;
}
