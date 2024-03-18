#ifndef _ASM_BOOT_H
#define _ASM_BOOT_H

/* Don't touch these, unless you really know what you're doing. */
/*默认初始化段*/
#define DEF_INITSEG		0x9000
/*默认系统段*/
#define DEF_SYSSEG		0x1000
/**/
#define DEF_SETUPSEG	0x9020
/**/
#define DEF_SYSSIZE		0x7F00

/*内部高级视频图形阵列SVGA（Super Video Graphics Array）启动常数。V——Video （视频）；
G——Graphics（图像）；A——Array（阵列）；S——Super(超级)；X——Extended(扩展)；
U——Ultra(终极)；W——Wide（加宽）；Q1——Quarter(四分之一)；Q2——Quantum(量化)
VGA（Video Graphics Array，视频图形阵列）支持640*480分辨率
SVGA（Super Video Graphics Array，高级视频图形阵列），由VESA为IBM兼容机推出的标准，
属于VGA的替代品。最大支持800×600分辨率。
XGA（Extended Graphics Array，扩展图形阵列），是IBM于1990年发明的，XGA较新的版本XGA-2
以真彩色提供800×600像素的分辨率或以65536种色彩提供1024×768像素的分辨率，这两种图像分辨
水平在当时比较常用。
SXGA（Super Extended Graphics Array，高级扩展图形阵列），一个分辨率为1280x1024的既成
事实显示标准。这种被广泛采用的显示标准的纵横比是5:4而不是常见的4:3。一般用于过去的高
端笔记本电脑。
SXGA+（Super Extended Graphics Array），作为SXGA的一种扩展，SXGA+是一种专门为笔记本
设计的屏幕。其显示分辨率为1400×1050。由于笔记本LCD屏幕的水平与垂直点距不同于普通桌面
LCD，所以其显示的精度要比普通17英寸的桌面LCD高出不少。
UVGA（Ultra Video Graphics Array，极速扩展图形阵列），支持最大1600×1200分辨率。一般用
于15英寸的笔记本电脑。由于对制造工艺要求较高，所以当时的价格也比较昂贵。
WXGA（WideExtended Graphics Array，宽屏扩展图形阵列），作为普通XGA屏幕的宽屏版本，WXGA
采用16:10的横宽比例来扩大屏幕的尺寸。其最大显示分辨率为1280×800。*/
/*80x25模式*/
#define NORMAL_VGA		0xffff
/*80x50模式*/
#define EXTENDED_VGA	0xfffe
/**/
#define ASK_VGA			0xfffd		/* ask for it at bootup */

/*内核应该加载的物理地址。CONFIG_PHYSICAL_START对齐CONFIG_PHYSICAL_ALIGN后的地址*/
#define LOAD_PHYSICAL_ADDR ((CONFIG_PHYSICAL_START + (CONFIG_PHYSICAL_ALIGN - 1)) \
				& ~(CONFIG_PHYSICAL_ALIGN - 1))

#endif /* _ASM_BOOT_H */
