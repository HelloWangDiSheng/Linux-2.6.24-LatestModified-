#ifndef _LINUX_KDEV_T_H
#define _LINUX_KDEV_T_H
#ifdef __KERNEL__
/*设备号的旧标识方式是u16（高8位主设备号，低8位从设备号）。新标识方式是u32，内核内部
表示方式（高12位主设备号，低20位从设备号），用户空间表示方式（高12位从设备高部分，
中间12位主设备号，低8位从设备号低部分）*/

/*新的从设备号所占比特位数目*/
#define MINORBITS	20
/*从设备号掩码*/
#define MINORMASK	((1U << MINORBITS) - 1)
/*主设备号*/
#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
/*从设备号*/
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
/*将主从设备号合并为设备号的内核内部表示方式（高12主，低20从）*/
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))
/*打印主从设备号*/
#define print_dev_t(buffer, dev)	sprintf((buffer), "%u:%u\n", MAJOR(dev), MINOR(dev))
/*格式化主从设备号*/
#define format_dev_t(buffer, dev)									\
({																	\
		sprintf(buffer, "%u:%u", MAJOR(dev), MINOR(dev));			\
		buffer;														\
})

/*旧文件系统可接受的设备号（u16类型设备号主8从8模式）*/
static inline int old_valid_dev(dev_t dev)
{
	return MAJOR(dev) < 256 && MINOR(dev) < 256;
}
/*将主8从8模式的旧设备号转换为u16形式*/
static inline u16 old_encode_dev(dev_t dev)
{
	return (MAJOR(dev) << 8) | MINOR(dev);
}
/*将u16格式的设备号转换成主8从8格式的设备号*/
static inline dev_t old_decode_dev(u16 val)
{
	return MKDEV((val >> 8) & 255, val & 255);
}

static inline int new_valid_dev(dev_t dev)
{
	return 1;
}

/*将新的内核内部设备号表示形式转换为用户空间中表示形式（低8从设备号低位，中12主设备
号，高12从设备高部分）*/
static inline u32 new_encode_dev(dev_t dev)
{
	/*获取主设备号*/
	unsigned major = MAJOR(dev);
	/*获取从设备号*/
	unsigned minor = MINOR(dev);
	/*将主从设备号转换为用户空间中的表示形式*/
	return (minor & 0xff) | (major << 8) | ((minor & ~0xff) << 12);
}

/*将主从设备号的新的用户空间表示形式转换为新的内核内部表示形式*/
static inline dev_t new_decode_dev(u32 dev)
{
	/*获取中间12位主设备号*/
	unsigned major = (dev & 0xfff00) >> 8;
	/*将剩余20位整合成从设备号*/
	unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
	/*将主从设备号转换成内核内部表示形式*/
	return MKDEV(major, minor);
}

static inline int huge_valid_dev(dev_t dev)
{
	return 1;
}
/*将主从设备号新的内核内部表示形式转换成u64（新的用户空间表示形式为32位，高位填充0
）形式*/
static inline u64 huge_encode_dev(dev_t dev)
{
	return new_encode_dev(dev);
}

static inline dev_t huge_decode_dev(u64 dev)
{
	return new_decode_dev(dev);
}

static inline int sysv_valid_dev(dev_t dev)
{
	return MAJOR(dev) < (1<<14) && MINOR(dev) < (1<<18);
}

static inline u32 sysv_encode_dev(dev_t dev)
{
	return MINOR(dev) | (MAJOR(dev) << 18);
}

static inline unsigned sysv_major(u32 dev)
{
	return (dev >> 18) & 0x3fff;
}

static inline unsigned sysv_minor(u32 dev)
{
	return dev & 0x3ffff;
}

#else /* __KERNEL__ */

/*
Some programs want their definitions of MAJOR and MINOR and MKDEV
from the kernel sources. These must be the externally visible ones.
*/
/*主从设备号的旧的表示方法（主高8从低8）*/
#define MAJOR(dev)	((dev)>>8)
#define MINOR(dev)	((dev) & 0xff)
#define MKDEV(ma,mi)	((ma)<<8 | (mi))
#endif /* __KERNEL__ */
#endif
