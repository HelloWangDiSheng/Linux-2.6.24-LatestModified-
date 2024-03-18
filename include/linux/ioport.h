/*ioport.h������һЩ��⡢���䡢�ͷ�ϵͳ��Դ�ĺ���*/
#ifndef _LINUX_IOPORT_H
#define _LINUX_IOPORT_H

#include <linux/compiler.h>
#include <linux/types.h>

/*linux�ṩ��һ��ͨ�üܹ����������ڴ��й������ݽṹ����Щ�ṹ������ϵͳ�п��õ���Դ
��ʹ���ں˴����ܹ�����ͷ�����Դ�����йؼ������ݽṹ��resource�����ں�������״��ʽ
��ʾ����������parent��child��sibling��Ա�Ĺ���ܼ򵥣���1��ÿ���ӽڵ㶼��һ�����ڵ�
��2��һ�����ڵ������������Ŀ���ӽڵ㣨3��ͬһ�����ڵ�������ӽڵ㣬�����ӵ��ֵܽ��
�����ϡ����ڴ��б�ʾ���ݽṹʱ������ע���������⣺��1������ÿ���ӽڵ㶼��һ��ָ��ָ��
���ڵ㣬�����ڵ�ֻ��һ��ָ��ָ���һ���ӽڵ㣬���������ӽڵ㶼ͨ���ֵܽ��������ʣ�2��
ָ�򸸽ڵ��ָ��ͬ������Ϊ�գ�����������£�˵���Ѿ�û�и��߲�εĽ����*/
struct resource
{
	/*��Դ������ַ�ռ����ʼ��ַ*/
	resource_size_t start;
	/*��Դ������ַ�ռ����ֹ��ַ*/
	resource_size_t end;
	/*�洢��һ���ַ������Ա����Դ����һ������������֣���Դ����ʵ�����ں��޹أ���
	���Կɶ���ʽ�����Դ�б���proc�ļ�ϵͳ�У�ʱ�Ƚ�����*/
	const char *name;
	/*���ڸ�׼ȷ��������Դ����ǰ״̬*/
	unsigned long flags;
	/*���ڵ�ָ�롣�����ǵ�������˱�����ʱ����Ҫһ��ָ��ָ��ǰ��㣬��һ��ָ��
	��ǰ���ĵ�ַ����ɱ��������е�����ɾ����������Ȼ�ˣ��ڲ����ڼ�Ҫ���б�����Դ
	�Ķ�д������*/
	struct resource *parent;
	/*�ֵܽ��ָ������ͬһ���ڵ�������ӽڵ�*/
	struct resource *sibling;
	/*�ӽڵ�ָ��ָֻ���һ���ӽڵ�*/
	struct resource *child;
};

/*��Դ�б�*/
struct resource_list
{
	/*��Դ�б��ѵ��������ʽ����*/
	struct resource_list *next;
	/*ָ����Դ���*/
	struct resource *res;
	/*��Դ��Ӧ��pci�豸*/
	struct pci_dev *dev;
};

/*
 * IO resources have these defined flags.
 */
/*IO��Դӵ���Ѷ���ı�ʶ*/
/*�������еı���λ��ϵͳ�����ɶ���256��1<<8����PCI����*/
#define IORESOURCE_BITS		0x000000ff	/* Bus-specific bits */
/*��Դ����*/
/*io�˿�*/
#define IORESOURCE_IO					0x00000100
/*io�ڴ�*/
#define IORESOURCE_MEM					0x00000200
/*�ж�����*/
#define IORESOURCE_IRQ					0x00000400
/*DMA*/
#define IORESOURCE_DMA					0x00000800

#define IORESOURCE_PREFETCH				0x00001000	/* No side effects */
#define IORESOURCE_READONLY				0x00002000
#define IORESOURCE_CACHEABLE			0x00004000
#define IORESOURCE_RANGELENGTH			0x00008000
#define IORESOURCE_SHADOWABLE			0x00010000
#define IORESOURCE_BUS_HAS_VGA			0x00080000

#define IORESOURCE_DISABLED				0x10000000
#define IORESOURCE_UNSET				0x20000000
#define IORESOURCE_AUTO					0x40000000
/*�������Ա�ʶ��ǰ��Դ��æ������*/
#define IORESOURCE_BUSY					0x80000000

/* ISA PnP IRQ specific bits (IORESOURCE_BITS) */
#define IORESOURCE_IRQ_HIGHEDGE				(1<<0)
#define IORESOURCE_IRQ_LOWEDGE				(1<<1)
#define IORESOURCE_IRQ_HIGHLEVEL			(1<<2)
#define IORESOURCE_IRQ_LOWLEVEL				(1<<3)
#define IORESOURCE_IRQ_SHAREABLE			(1<<4)

/* ISA PnP DMA specific bits (IORESOURCE_BITS) */
#define IORESOURCE_DMA_TYPE_MASK			(3<<0)
#define IORESOURCE_DMA_8BIT					(0<<0)
#define IORESOURCE_DMA_8AND16BIT			(1<<0)
#define IORESOURCE_DMA_16BIT				(2<<0)

#define IORESOURCE_DMA_MASTER				(1<<2)
#define IORESOURCE_DMA_BYTE					(1<<3)
#define IORESOURCE_DMA_WORD					(1<<4)

#define IORESOURCE_DMA_SPEED_MASK			(3<<6)
#define IORESOURCE_DMA_COMPATIBLE			(0<<6)
#define IORESOURCE_DMA_TYPEA				(1<<6)
#define IORESOURCE_DMA_TYPEB				(2<<6)
#define IORESOURCE_DMA_TYPEF				(3<<6)

/* ISA PnP memory I/O specific bits (IORESOURCE_BITS) */
#define IORESOURCE_MEM_WRITEABLE			(1<<0)	/* dup: IORESOURCE_READONLY */
#define IORESOURCE_MEM_CACHEABLE			(1<<1)	/* dup: IORESOURCE_CACHEABLE */
#define IORESOURCE_MEM_RANGELENGTH			(1<<2)	/* dup: IORESOURCE_RANGELENGTH */
#define IORESOURCE_MEM_TYPE_MASK			(3<<3)
#define IORESOURCE_MEM_8BIT					(0<<3)
#define IORESOURCE_MEM_16BIT				(1<<3)
#define IORESOURCE_MEM_8AND16BIT			(2<<3)
#define IORESOURCE_MEM_32BIT				(3<<3)
#define IORESOURCE_MEM_SHADOWABLE			(1<<5)	/* dup: IORESOURCE_SHADOWABLE */
#define IORESOURCE_MEM_EXPANSIONROM			(1<<6)

/* PCI ROM control bits (IORESOURCE_BITS) */
#define IORESOURCE_ROM_ENABLE				(1<<0)	/* ROM is enabled, same as PCI_ROM_ADDRESS_ENABLE */
#define IORESOURCE_ROM_SHADOW				(1<<1)	/* ROM is copy at C000:0 */
#define IORESOURCE_ROM_COPY					(1<<2)	/* ROM is alloc'd copy, resource field overlaid */
#define IORESOURCE_ROM_BIOS_COPY			(1<<3)	/* ROM is BIOS copy, resource field overlaid */

/* PCI control bits.  Shares IORESOURCE_BITS with above PCI ROM.  */
#define IORESOURCE_PCI_FIXED				(1<<4)	/* Do not move resource */

/* PC/ISA/whatever - the normal PC address spaces: IO and memory */
/*PCI/ISA���κ���ͨPC��ַ�ռ䣺IO���ڴ�*/
/*����һ��io�˿���Դ����ioport_resource��ȫ��io�˿ڸ���Դ����*/
extern struct resource ioport_resource;
/*����һ��io�ڴ���Դ����iomen_resource��ȫ��io�ڴ����Դ����*/
extern struct resource iomem_resource;

extern int request_resource(struct resource *root, struct resource *new);
extern int release_resource(struct resource *new);
extern int insert_resource(struct resource *parent, struct resource *new);
extern int allocate_resource(struct resource *root, struct resource *new,
			     		resource_size_t size, resource_size_t min, resource_size_t max,
			     		resource_size_t align, void (*alignf)(void *, struct resource *,
						resource_size_t, resource_size_t), void *alignf_data);
int adjust_resource(struct resource *res, resource_size_t start, resource_size_t size);

/* Convenience shorthand with allocation */
#define request_region(start,n,name)	__request_region(&ioport_resource, (start), (n), (name))
#define request_mem_region(start,n,name) __request_region(&iomem_resource, (start), (n), (name))
#define rename_region(region, newname) do { (region)->name = (newname); } while (0)

extern struct resource * __request_region(struct resource *, resource_size_t start,
													resource_size_t n, const char *name);

/* Compatibility cruft */
#define release_region(start,n)	__release_region(&ioport_resource, (start), (n))
#define check_mem_region(start,n)	__check_region(&iomem_resource, (start), (n))
#define release_mem_region(start,n)	__release_region(&iomem_resource, (start), (n))

extern int __check_region(struct resource *, resource_size_t, resource_size_t);
extern void __release_region(struct resource *, resource_size_t, resource_size_t);

static inline int __deprecated check_region(resource_size_t s, resource_size_t n)
{
	return __check_region(&ioport_resource, s, n);
}

/* Wrappers for managed devices */
struct device;
#define devm_request_region(dev,start,n,name) \
	__devm_request_region(dev, &ioport_resource, (start), (n), (name))
#define devm_request_mem_region(dev,start,n,name) \
	__devm_request_region(dev, &iomem_resource, (start), (n), (name))

extern struct resource * __devm_request_region(struct device *dev,
				struct resource *parent, resource_size_t start,
				resource_size_t n, const char *name);

#define devm_release_region(start,n) \
	__devm_release_region(dev, &ioport_resource, (start), (n))
#define devm_release_mem_region(start,n) \
	__devm_release_region(dev, &iomem_resource, (start), (n))

extern void __devm_release_region(struct device *dev, struct resource *parent,
				  resource_size_t start, resource_size_t n);

#endif	/* _LINUX_IOPORT_H */
