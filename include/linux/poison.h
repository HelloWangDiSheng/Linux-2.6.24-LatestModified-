#ifndef _LINUX_POISON_H
#define _LINUX_POISON_H

/*正常环境中访问这些非空指针将导致缺页异常，用来验证没有使用非初始化链表结点*/
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

/*危险区魔术字，放置在每个对象的开始之前和结束之后处一个字中，如果魔术字被修改，
程序员在分析内核内存时注意到，可能某些代码访问了不属于它们的内存区*/
/*非活动对象魔术字*/
#define	RED_INACTIVE			0x09F911029D74E35BULL
/*活动对象魔术字*/
#define	RED_ACTIVE				0xD84156C5635688C0ULL
/*非活动对象魔术字*/
#define SLUB_RED_INACTIVE		0xbb
/*活动对象魔术字*/
#define SLUB_RED_ACTIVE			0xcc

/*对象毒化（Object Poisoning）：在建立和释放slab时，将对象用预定义的魔术字填充。
如果在对象分配时注意到该内容已被修改，则发生了未授权的访问*/
/*未初始化时对象毒化*/
#define	POISON_INUSE			0x5a
/*用后释放时对象毒化*/
#define POISON_FREE				0x6b
/*对象毒化的最后一个字节*/
#define	POISON_END				0xa5

/*初始化完成后释放的初始化数据和代码所占用的物理页面，这些页被填充的对象毒化内容*/
#define POISON_FREE_INITMEM	0xcc

/********** arch/ia64/hp/common/sba_iommu.c **********/
/*
 * arch/ia64/hp/common/sba_iommu.c uses a 16-byte poison string with a
 * value of "SBAIOMMU POISON\0" for spill-over poisoning.
 */

/********** fs/jbd/journal.c **********/
#define JBD_POISON_FREE		0x5b
#define JBD2_POISON_FREE	0x5c

/********** drivers/base/dmapool.c **********/
#define	POOL_POISON_FREED	0xa7	/* !inuse */
#define	POOL_POISON_ALLOCATED	0xa9	/* !initted */

/********** drivers/atm/ **********/
#define ATM_POISON_FREE		0x12
#define ATM_POISON		0xdeadbeef

/********** net/ **********/
#define NEIGHBOR_DEAD		0xdeadbeef
#define NETFILTER_LINK_POISON	0xdead57ac

/********** kernel/mutexes **********/
#define MUTEX_DEBUG_INIT	0x11
#define MUTEX_DEBUG_FREE	0x22

/********** security/ **********/
#define KEY_DESTROY		0xbd

/********** sound/oss/ **********/
#define OSS_POISON_FREE		0xAB

#endif
