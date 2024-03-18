#ifndef _LINUX_FS_STRUCT_H
#define _LINUX_FS_STRUCT_H

struct dentry;
struct vfsmount;

/*�����ļ�ϵͳ��Ϣ���ṹ��root��rootmnt��pwd��pwdmnt��altroot��altrootmnt���Ա˴˹�����
���У�root��rootmnt��ʾ���̵ĸ�Ŀ¼�͸�Ŀ��װ�ص��ļ�ϵͳ������ͨ����/��ϵͳ��root�ļ�
ϵͳ����Ȼ������ͨ��chroot������ͨ��ͬ����ϵͳ���ã�������ĳ����Ŀ¼�Ľ�����˵����ʹ��
����Ŀ¼��Ϊ��Ŀ¼��pwd��pwdmntָ���˵�ǰ���̵Ĺ���Ŀ¼�����Ѱ�װ���ļ�ϵͳ���ڽ��̸ı�
�䵱ǰĿ¼ʱ�����߶��ᶯ̬�ı䡣��ʹ��shellʱ�����Ǻ�Ƶ���ģ�cd���������ÿ��chdirϵ
ͳ���ö���ı�pwd��ֵ��Ψһ��������л���.Ŀ¼����������������һ���µ�װ�ص�ʱ��pwdmnt
�Ż�ı䡣altroot��altrootmnt��Ա����ʵ�ָ��Ի���personality����������������Ϊ�����Ƴ�
����һ�����滷����ʹ�ó�����Ϊ���ڲ�ͬ��linux��ĳ������ϵͳ�����С���������������ļ�
�Ϳⰲ����һ��Ŀ¼�У�ͨ����/usr/gnemul�����йظ�·������Ϣ������alt��Ա�У��������ļ�
ʱ��������ɨ������Ŀ¼��������Ȼ��ҵ�����Ŀ���ļ�ϵͳ��������linuxϵͳ���ļ�����Щ֮
�������������֧�ֶԲ�ͬ�Ķ����Ƹ�ʽͬʱʹ�ò�ͬ�Ŀ⣬�ü�������ʹ��*/
struct fs_struct
{
	/*���ü���*/
	atomic_t count;
	/*�����ýṹ�����Ķ�д��*/
	rwlock_t lock;
	/*��ʾ��׼�����룬���������ļ���Ȩ�ޣ���ֵ����ʹ��umask�����ȡ�����ã����ڲ���
	��ͬ����ϵͳ�������*/
	int umask;
	/*���̵ĸ�Ŀ¼��ͨ��Ϊ/��Ŀ¼��ͨ��chroot������ĳ����Ŀ¼�Ľ�����˵��
	����������Ŀ¼Ϊ��Ŀ¼*/
	struct dentry *root;
	/*���̵�ǰ�Ĺ���Ŀ¼*/
	struct dentry *pwd;
	/*ʵ�ָ��Ի���Ŀ¼*/
	struct dentry *altroot;
	/*���̸�Ŀ¼��װ�ص��ļ�ϵͳ��ͨ��Ϊϵͳ��root�ļ�ϵͳ*/
	struct vfsmount *rootmnt;
	/*���̵�ǰ����Ŀ¼��װ�ص��ļ�ϵͳ*/
	struct vfsmount *pwdmnt;
	/*���Ի���Ŀ¼��װ���ļ�ϵͳ*/
	struct vfsmount *altrootmnt;
};

/*��ʼ�����̵��ļ�ϵͳ��Ϣ*/
#define INIT_FS							\
{										\
	.count		= ATOMIC_INIT(1),		\
	.lock		= RW_LOCK_UNLOCKED,\/*��д������δ����״̬*/
	.umask		= 0022,					\/*�ļ�ϵͳ��д*/
}

extern struct kmem_cache *fs_cachep;

extern void exit_fs(struct task_struct *);
extern void set_fs_altroot(void);
extern void set_fs_root(struct fs_struct *, struct vfsmount *, struct dentry *);
extern void set_fs_pwd(struct fs_struct *, struct vfsmount *, struct dentry *);
extern struct fs_struct *copy_fs_struct(struct fs_struct *);
extern void put_fs_struct(struct fs_struct *);

#endif /* _LINUX_FS_STRUCT_H */
