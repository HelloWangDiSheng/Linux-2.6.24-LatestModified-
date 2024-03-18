/*Definitions for mount interface. This describes the in the kernel build linkedlist
with mounted filesystems*/
#ifndef _LINUX_MOUNT_H
#define _LINUX_MOUNT_H
#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/atomic.h>

struct super_block;
struct vfsmount;
struct dentry;
struct mnt_namespace;

/*禁止setuid执行*/
#define MNT_NOSUID			0x01
/*装载的文件系统是虚拟的，即没有物理后端设备*/
#define MNT_NODEV			0x02
/**/
#define MNT_NOEXEC			0x04
/**/
#define MNT_NOATIME			0x08
/**/
#define MNT_NODIRATIME		0x10
/**/
#define MNT_RELATIME		0x20
/*专用于NFS和AFS的，用来标记子装载。设置了该标记的装载允许自动移除*/
#define MNT_SHRINKABLE		0x100
/*共享装载*/
#define MNT_SHARED			0x1000
/*不可绑定装载*/
#define MNT_UNBINDABLE		0x2000
/*propagation flag mask*/
#define MNT_PNODE_MASK		0x3000

/*每个装载的文件系统都对应于一个vfsmount结构的实例*/
struct vfsmount
{
	/*使用了一个散列表，称作mount_hashtable，且定义在fs/namespace.c中。溢出链表以链表
	形式实现，链表元素是mnt_hash。vfsmount实例的地址和相关的dentry对象的地址用来计算
	散列和*/
	struct list_head mnt_hash;
	/*指向父文件系统的vfsmount结构*/
	struct vfsmount *mnt_parent;	/* fs we are mounted on */
	/*当前文件系统的装载点在其父目录中的dentry结构*/
	struct dentry *mnt_mountpoint;	/* dentry of mountpoint */
	/*文件系统本身的相对根目录所对应的dentry保存在mnt_root中。两个dentry实例表示同一
	目录（即装载点）。这意味着，在文件系统卸载后，不必删除此前的装载点信息。mount系统
	调用时，使用两个dentry项的必要性就一清二楚了*/
	struct dentry *mnt_root;	/* root of the mounted tree */
	/*mnt_sb指针建立了与相关的超级块之间的关联（对每个装载的文件系统而言，都有且只有
	一个超级块实例）*/
	struct super_block *mnt_sb;	/* pointer to superblock */
	/*文件系统之间的父子关系由上述结构的两个成员所实现的链表表示。mnt_mounts表头是子
	文件系统链表的起点*/
	struct list_head mnt_mounts;	/* list of children, anchored here */
	/*mnt_child字段则用作该链表的链表元素*/
	struct list_head mnt_child;	/* and going through their mnt_child */
	/*mnt_child字段则用作该链表的链表元素*/
	int mnt_flags;
	/* 4 bytes hole on 64bits arches */
	/**/
	char *mnt_devname;		/* Name of device e.g. /dev/dsk/hda1 */
	/*系统的每个vfsmount实例，都还可以通过另外两种途径标识。一个命名空间的所有装载的
	文件系统都保存在namespace->list链表中。使用vfsmount的mnt_list成员作为链表元素。我
	在这里忽视拓扑结构的问题，因为所有（文件系统）的装载操作是相继执行的。*/
	struct list_head mnt_list;
	/*mnt_expire用作链表元素，用于将所有可能自动过期的装载放置在一个链表上*/
	struct list_head mnt_expire;	/* link in fs-specific expiry list */
	/*共享装载更容易表示。内核所需做的就是将所有共享装载保存在一个循环链表上。
	mnt_share作为链表元素*/
	struct list_head mnt_share;	/* circular list of shared mounts */
	/*mnt_slave、mnt_slave_list和mnt_master用来实现从属装载（slave mount） 。主装载
	（master mount）将所有从属装载保存在一个链表上，mnt_slave_list用作表头，而
	mnt_slave作为链表元素。所有从属装载都通过mnt_master指向其主装载*/
	struct list_head mnt_slave_list;/* list of slave mounts */
	struct list_head mnt_slave;	/* slave list entry */
	struct vfsmount *mnt_master;	/* slave is on master->mnt_slave_list */
	/*mnt_namespace是装载的文件系统所属的命名空间*/
	struct mnt_namespace *mnt_ns;	/* containing namespace */
	/*
	 * We put mnt_count & mnt_expiry_mark at the end of struct vfsmount
	 * to let these frequently modified fields in a separate cache line
	 * (so that reads of mnt_flags wont ping-pong on SMP machines)
	 */
	/*mnt_count实现了一个使用计数器。每当一个vfsmount实例不再需要时，都必须用mntput
	将计数器减1。mntget与mntput相对，在获取vfsmount实例使用时，必须调用mntget*/
	atomic_t mnt_count;
	/*装载过期用mnt_expiry_mark处理。该成员用来表示装载的文件系统是否已经不再使用*/
	int mnt_expiry_mark;		/* true if marked for expiry */
	/**/
	int mnt_pinned;
};

/**/
static inline struct vfsmount *mntget(struct vfsmount *mnt)
{
	if (mnt)
		atomic_inc(&mnt->mnt_count);
	return mnt;
}

extern void mntput_no_expire(struct vfsmount *mnt);
extern void mnt_pin(struct vfsmount *mnt);
extern void mnt_unpin(struct vfsmount *mnt);

/**/
static inline void mntput(struct vfsmount *mnt)
{
	if (mnt) {
		mnt->mnt_expiry_mark = 0;
		mntput_no_expire(mnt);
	}
}

extern void free_vfsmnt(struct vfsmount *mnt);
extern struct vfsmount *alloc_vfsmnt(const char *name);
extern struct vfsmount *do_kern_mount(const char *fstype, int flags,
				      const char *name, void *data);

struct file_system_type;
extern struct vfsmount *vfs_kern_mount(struct file_system_type *type,
				      int flags, const char *name,
				      void *data);

struct nameidata;

extern int do_add_mount(struct vfsmount *newmnt, struct nameidata *nd,
			int mnt_flags, struct list_head *fslist);

extern void mark_mounts_for_expiry(struct list_head *mounts);
extern void shrink_submounts(struct vfsmount *mountpoint, struct list_head *mounts);

extern spinlock_t vfsmount_lock;
extern dev_t name_to_dev_t(char *name);

#endif
#endif /* _LINUX_MOUNT_H */
