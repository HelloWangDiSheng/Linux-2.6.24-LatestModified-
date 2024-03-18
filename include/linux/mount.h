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

/*��ֹsetuidִ��*/
#define MNT_NOSUID			0x01
/*װ�ص��ļ�ϵͳ������ģ���û���������豸*/
#define MNT_NODEV			0x02
/**/
#define MNT_NOEXEC			0x04
/**/
#define MNT_NOATIME			0x08
/**/
#define MNT_NODIRATIME		0x10
/**/
#define MNT_RELATIME		0x20
/*ר����NFS��AFS�ģ����������װ�ء������˸ñ�ǵ�װ�������Զ��Ƴ�*/
#define MNT_SHRINKABLE		0x100
/*����װ��*/
#define MNT_SHARED			0x1000
/*���ɰ�װ��*/
#define MNT_UNBINDABLE		0x2000
/*propagation flag mask*/
#define MNT_PNODE_MASK		0x3000

/*ÿ��װ�ص��ļ�ϵͳ����Ӧ��һ��vfsmount�ṹ��ʵ��*/
struct vfsmount
{
	/*ʹ����һ��ɢ�б�����mount_hashtable���Ҷ�����fs/namespace.c�С��������������
	��ʽʵ�֣�����Ԫ����mnt_hash��vfsmountʵ���ĵ�ַ����ص�dentry����ĵ�ַ��������
	ɢ�к�*/
	struct list_head mnt_hash;
	/*ָ���ļ�ϵͳ��vfsmount�ṹ*/
	struct vfsmount *mnt_parent;	/* fs we are mounted on */
	/*��ǰ�ļ�ϵͳ��װ�ص����丸Ŀ¼�е�dentry�ṹ*/
	struct dentry *mnt_mountpoint;	/* dentry of mountpoint */
	/*�ļ�ϵͳ�������Ը�Ŀ¼����Ӧ��dentry������mnt_root�С�����dentryʵ����ʾͬһ
	Ŀ¼����װ�ص㣩������ζ�ţ����ļ�ϵͳж�غ󣬲���ɾ����ǰ��װ�ص���Ϣ��mountϵͳ
	����ʱ��ʹ������dentry��ı�Ҫ�Ծ�һ�������*/
	struct dentry *mnt_root;	/* root of the mounted tree */
	/*mnt_sbָ�뽨��������صĳ�����֮��Ĺ�������ÿ��װ�ص��ļ�ϵͳ���ԣ�������ֻ��
	һ��������ʵ����*/
	struct super_block *mnt_sb;	/* pointer to superblock */
	/*�ļ�ϵͳ֮��ĸ��ӹ�ϵ�������ṹ��������Ա��ʵ�ֵ������ʾ��mnt_mounts��ͷ����
	�ļ�ϵͳ��������*/
	struct list_head mnt_mounts;	/* list of children, anchored here */
	/*mnt_child�ֶ������������������Ԫ��*/
	struct list_head mnt_child;	/* and going through their mnt_child */
	/*mnt_child�ֶ������������������Ԫ��*/
	int mnt_flags;
	/* 4 bytes hole on 64bits arches */
	/**/
	char *mnt_devname;		/* Name of device e.g. /dev/dsk/hda1 */
	/*ϵͳ��ÿ��vfsmountʵ������������ͨ����������;����ʶ��һ�������ռ������װ�ص�
	�ļ�ϵͳ��������namespace->list�����С�ʹ��vfsmount��mnt_list��Ա��Ϊ����Ԫ�ء���
	������������˽ṹ�����⣬��Ϊ���У��ļ�ϵͳ����װ�ز��������ִ�еġ�*/
	struct list_head mnt_list;
	/*mnt_expire��������Ԫ�أ����ڽ����п����Զ����ڵ�װ�ط�����һ��������*/
	struct list_head mnt_expire;	/* link in fs-specific expiry list */
	/*����װ�ظ����ױ�ʾ���ں��������ľ��ǽ����й���װ�ر�����һ��ѭ�������ϡ�
	mnt_share��Ϊ����Ԫ��*/
	struct list_head mnt_share;	/* circular list of shared mounts */
	/*mnt_slave��mnt_slave_list��mnt_master����ʵ�ִ���װ�أ�slave mount�� ����װ��
	��master mount�������д���װ�ر�����һ�������ϣ�mnt_slave_list������ͷ����
	mnt_slave��Ϊ����Ԫ�ء����д���װ�ض�ͨ��mnt_masterָ������װ��*/
	struct list_head mnt_slave_list;/* list of slave mounts */
	struct list_head mnt_slave;	/* slave list entry */
	struct vfsmount *mnt_master;	/* slave is on master->mnt_slave_list */
	/*mnt_namespace��װ�ص��ļ�ϵͳ�����������ռ�*/
	struct mnt_namespace *mnt_ns;	/* containing namespace */
	/*
	 * We put mnt_count & mnt_expiry_mark at the end of struct vfsmount
	 * to let these frequently modified fields in a separate cache line
	 * (so that reads of mnt_flags wont ping-pong on SMP machines)
	 */
	/*mnt_countʵ����һ��ʹ�ü�������ÿ��һ��vfsmountʵ��������Ҫʱ����������mntput
	����������1��mntget��mntput��ԣ��ڻ�ȡvfsmountʵ��ʹ��ʱ���������mntget*/
	atomic_t mnt_count;
	/*װ�ع�����mnt_expiry_mark�����ó�Ա������ʾװ�ص��ļ�ϵͳ�Ƿ��Ѿ�����ʹ��*/
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
