#ifndef _LINUX_KDEV_T_H
#define _LINUX_KDEV_T_H
#ifdef __KERNEL__
/*�豸�ŵľɱ�ʶ��ʽ��u16����8λ���豸�ţ���8λ���豸�ţ����±�ʶ��ʽ��u32���ں��ڲ�
��ʾ��ʽ����12λ���豸�ţ���20λ���豸�ţ����û��ռ��ʾ��ʽ����12λ���豸�߲��֣�
�м�12λ���豸�ţ���8λ���豸�ŵͲ��֣�*/

/*�µĴ��豸����ռ����λ��Ŀ*/
#define MINORBITS	20
/*���豸������*/
#define MINORMASK	((1U << MINORBITS) - 1)
/*���豸��*/
#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
/*���豸��*/
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
/*�������豸�źϲ�Ϊ�豸�ŵ��ں��ڲ���ʾ��ʽ����12������20�ӣ�*/
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))
/*��ӡ�����豸��*/
#define print_dev_t(buffer, dev)	sprintf((buffer), "%u:%u\n", MAJOR(dev), MINOR(dev))
/*��ʽ�������豸��*/
#define format_dev_t(buffer, dev)									\
({																	\
		sprintf(buffer, "%u:%u", MAJOR(dev), MINOR(dev));			\
		buffer;														\
})

/*���ļ�ϵͳ�ɽ��ܵ��豸�ţ�u16�����豸����8��8ģʽ��*/
static inline int old_valid_dev(dev_t dev)
{
	return MAJOR(dev) < 256 && MINOR(dev) < 256;
}
/*����8��8ģʽ�ľ��豸��ת��Ϊu16��ʽ*/
static inline u16 old_encode_dev(dev_t dev)
{
	return (MAJOR(dev) << 8) | MINOR(dev);
}
/*��u16��ʽ���豸��ת������8��8��ʽ���豸��*/
static inline dev_t old_decode_dev(u16 val)
{
	return MKDEV((val >> 8) & 255, val & 255);
}

static inline int new_valid_dev(dev_t dev)
{
	return 1;
}

/*���µ��ں��ڲ��豸�ű�ʾ��ʽת��Ϊ�û��ռ��б�ʾ��ʽ����8���豸�ŵ�λ����12���豸
�ţ���12���豸�߲��֣�*/
static inline u32 new_encode_dev(dev_t dev)
{
	/*��ȡ���豸��*/
	unsigned major = MAJOR(dev);
	/*��ȡ���豸��*/
	unsigned minor = MINOR(dev);
	/*�������豸��ת��Ϊ�û��ռ��еı�ʾ��ʽ*/
	return (minor & 0xff) | (major << 8) | ((minor & ~0xff) << 12);
}

/*�������豸�ŵ��µ��û��ռ��ʾ��ʽת��Ϊ�µ��ں��ڲ���ʾ��ʽ*/
static inline dev_t new_decode_dev(u32 dev)
{
	/*��ȡ�м�12λ���豸��*/
	unsigned major = (dev & 0xfff00) >> 8;
	/*��ʣ��20λ���ϳɴ��豸��*/
	unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
	/*�������豸��ת�����ں��ڲ���ʾ��ʽ*/
	return MKDEV(major, minor);
}

static inline int huge_valid_dev(dev_t dev)
{
	return 1;
}
/*�������豸���µ��ں��ڲ���ʾ��ʽת����u64���µ��û��ռ��ʾ��ʽΪ32λ����λ���0
����ʽ*/
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
/*�����豸�ŵľɵı�ʾ����������8�ӵ�8��*/
#define MAJOR(dev)	((dev)>>8)
#define MINOR(dev)	((dev) & 0xff)
#define MKDEV(ma,mi)	((ma)<<8 | (mi))
#endif /* __KERNEL__ */
#endif
