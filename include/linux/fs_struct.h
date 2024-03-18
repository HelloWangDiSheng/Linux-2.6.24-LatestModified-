#ifndef _LINUX_FS_STRUCT_H
#define _LINUX_FS_STRUCT_H

struct dentry;
struct vfsmount;

/*进程文件系统信息，结构内root与rootmnt、pwd与pwdmnt和altroot与altrootmnt三对彼此关联，
其中，root和rootmnt表示进程的根目录和根目已装载的文件系统，二者通常是/和系统的root文件
系统，当然，对于通过chroot（暗中通过同名的系统调用）锁定到某个子目录的进程来说，其使用
该子目录作为根目录，pwd和pwdmnt指定了当前进程的工作目录和其已安装的文件系统，在进程改变
其当前目录时，二者都会动态改变。在使用shell时，这是很频繁的（cd命令），尽管每次chdir系
统调用都会改变pwd的值（唯一例外的是切换到.目录），但仅当进入了一个新的装载点时，pwdmnt
才会改变。altroot和altrootmnt成员用于实现个性化（personality）。这种特性允许为二进制程
序建立一个仿真环境，使得程序认为是在不同于linux的某个操作系统下运行。仿真所需的特殊文件
和库安置在一个目录中（通常是/usr/gnemul），有关该路径的信息保存在alt成员中，在搜索文件
时总是优先扫描上述目录，因此首先会找到仿真的库或文件系统，而不是linux系统的文件（这些之
后才搜索），这支持对不同的二进制格式同时使用不同的库，该技术很少使用*/
struct fs_struct
{
	/*引用计数*/
	atomic_t count;
	/*保护该结构操作的读写锁*/
	rwlock_t lock;
	/*表示标准的掩码，用于设置文件新权限，其值可以使用umask命令读取或设置，在内部，
	由同名的系统调用完成*/
	int umask;
	/*进程的根目录，通常为/根目录，通过chroot锁定到某个子目录的进程来说，
	被锁定的子目录为根目录*/
	struct dentry *root;
	/*进程当前的工作目录*/
	struct dentry *pwd;
	/*实现个性化根目录*/
	struct dentry *altroot;
	/*进程根目录已装载的文件系统，通常为系统的root文件系统*/
	struct vfsmount *rootmnt;
	/*进程当前工作目录已装载的文件系统*/
	struct vfsmount *pwdmnt;
	/*个性化根目录已装载文件系统*/
	struct vfsmount *altrootmnt;
};

/*初始化进程的文件系统信息*/
#define INIT_FS							\
{										\
	.count		= ATOMIC_INIT(1),		\
	.lock		= RW_LOCK_UNLOCKED,\/*读写锁处于未锁定状态*/
	.umask		= 0022,					\/*文件系统可写*/
}

extern struct kmem_cache *fs_cachep;

extern void exit_fs(struct task_struct *);
extern void set_fs_altroot(void);
extern void set_fs_root(struct fs_struct *, struct vfsmount *, struct dentry *);
extern void set_fs_pwd(struct fs_struct *, struct vfsmount *, struct dentry *);
extern struct fs_struct *copy_fs_struct(struct fs_struct *);
extern void put_fs_struct(struct fs_struct *);

#endif /* _LINUX_FS_STRUCT_H */
