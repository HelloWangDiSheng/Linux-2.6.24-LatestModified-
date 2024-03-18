#ifndef _LINUX_PROC_FS_H
#define _LINUX_PROC_FS_H

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/magic.h>
#include <asm/atomic.h>

struct net;
struct completion;

/*第一个进程在/proc根目录中的偏移*/
#define FIRST_PROCESS_ENTRY 256

enum
{
	PROC_ROOT_INO = 1,
};

/*
 * This is not completely implemented yet. The idea is to
 * create an in-memory tree (like the actual /proc filesystem
 * tree) of these proc_dir_entries, so that we can dynamically
 * add new files to /proc.
 *
 * The "next" pointer creates a linked list of one /proc directory,
 * while parent/subdir create the directory structure (every
 * /proc file has a parent, but "subdir" is NULL for all
 * non-directory entries).
 *
 * "get_info" is called at "read", while "owner" is used to protect module
 * from unloading while proc_dir_entry is in use
 */
/*虽然数据是以内存页为基准读取（当然，也可以指定要读取数据的偏移量和长度），但数据的
写入则基于file实例。这两个例程都有一个额外的data参数，该参数在注册新的proc数据项时定
义，每次调用这两个例程时，都作为参数传递进来（data参数平时保存在prc_dir_entry的data中
）。这意味着，可以将一个函数注册为多个proc数据项的读写例程。函数的代码可以根据data参
数区分各种不同的情况（因为get_info没有data参数，所以不可能这样做）*/
typedef	int (read_proc_t)(char *page, char **start, off_t off, int count, int *eof, void *data);
typedef	int (write_proc_t)(struct file *file, const char __user *buffer, unsigned long count,
								void *data);
typedef int (get_info_t)(char *, char **, off_t, int);
typedef struct proc_dir_entry *(shadow_proc_t)(struct task_struct *task,
															struct proc_dir_entry *pde);

/*proc文件系统中每个数据项都由proc_dir_entryd的一个实例描述*/
struct proc_dir_entry
{
	/*inode编号*/
	unsigned int low_ino;
	/*文件名长度*/
	unsigned short namelen;
	/*指向存储文件名的字符串的指针*/
	const char *name;
	/*文件类型及权限*/
	mode_t mode;
	/*指定了目录中子目录和符号链接的数目，其它类型文件的数目时不支持的*/
	nlink_t nlink;
	/*文件所有者的用户ID*/
	uid_t uid;
	/*文件爱你所有者的组ID*/
	gid_t gid;
	/*按字节计算的文件长度，由于proc数据项是动态生成的，所以文件的长度通常五法预先
	知道，在这种情况下，该值为0*/
	loff_t size;
	/*inode的操作指针*/
	const struct inode_operations *proc_iops;
	/*
	 * NULL ->proc_fops means "PDE is going away RSN" or
	 * "PDE is just created". In either case, e.g. ->read_proc won't be
	 * called because it's too late or too early, respectively.
	 *
	 * If you're allocating ->proc_fops dynamically, save a pointer
	 * somewhere.
	 */
	/*文件操作指针，这些操作充当与虚拟文件系统之间的接口，所使用的操作依赖具体的文件类型*/
	const struct file_operations *proc_fops;
	/*函数指针指向相关子系统中返回所需数据的例程。如同普通的文件访问，我们也可以指定
	所需范围的偏移量和长度，这样就不必读取整个数据集。该接口很有用，例如，可以用于proc
	数据项的自动分析*/
	get_info_t *get_info;
	/*如果一个proc数据项由动态加载的模块产生，那么owner指向相关联模块在内存中的数据
	结构（如果该项由持久编译到内核中的代码产生，那么该项为NULL）*/
	struct module *owner;
	/*父目录指针，父目录中包含了一个文件（或子目录），对应于当前的proc_dir_entry实例*/
	struct proc_dir_entry *parent;
	/*指向一个目录中的第一个子数据项（虽然该成员的名称是subdir，但可能是文件，也可能
	是目录）。subdir和next支持文件和目录的层次化布置*/
	struct proc_dir_entry *subdir;
	/*将目录下的所有常见数据项都集中在一个链表中*/
	struct proc_dir_entry *next;
	/**/
	void *data;
	/*支持从内核中读取数据*/
	read_proc_t *read_proc;
	/*支持向内核中写入数据*/
	write_proc_t *write_proc;
	/*该实例被使用的计数*/
	atomic_t count;		/* use count */
	/**/
	int pde_users;	/* number of callers into module in progress */
	/**/
	spinlock_t pde_unload_lock; /* proc_fops checks and pde_users bumps */
	/**/
	struct completion *pde_unload_completion;
	/**/
	shadow_proc_t *shadow_proc;
};

/*内核转储单链表*/
struct kcore_list
{
	/*连接下一个结点*/
	struct kcore_list *next;
	/*起始地址*/
	unsigned long addr;
	/*长度*/
	size_t size;
};

struct vmcore
{
	struct list_head list;
	unsigned long long paddr;
	unsigned long long size;
	loff_t offset;
};

#ifdef CONFIG_PROC_FS

extern struct proc_dir_entry proc_root;
extern struct proc_dir_entry *proc_root_fs;
extern struct proc_dir_entry *proc_bus;
extern struct proc_dir_entry *proc_root_driver;
extern struct proc_dir_entry *proc_root_kcore;

extern spinlock_t proc_subdir_lock;

extern void proc_root_init(void);
extern void proc_misc_init(void);

struct mm_struct;

void proc_flush_task(struct task_struct *task);
struct dentry *proc_pid_lookup(struct inode *dir, struct dentry * dentry, 
										struct nameidata *);
int proc_pid_readdir(struct file * filp, void * dirent, filldir_t filldir);
unsigned long task_vsize(struct mm_struct *);
int task_statm(struct mm_struct *, int *, int *, int *, int *);
char *task_mem(struct mm_struct *, char *);
void clear_refs_smap(struct mm_struct *mm);

struct proc_dir_entry *de_get(struct proc_dir_entry *de);
void de_put(struct proc_dir_entry *de);

extern struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
														struct proc_dir_entry *parent);
extern void remove_proc_entry(const char *name, struct proc_dir_entry *parent);

extern struct vfsmount *proc_mnt;
struct pid_namespace;
extern int proc_fill_super(struct super_block *);
extern struct inode *proc_get_inode(struct super_block *, unsigned int, struct proc_dir_entry *);

/*
 * These are generic /proc routines that use the internal
 * "struct proc_dir_entry" tree to traverse the filesystem.
 *
 * The /proc root directory has extended versions to take care
 * of the /proc/<pid> subdirectories.
 */
extern int proc_readdir(struct file *, void *, filldir_t);
extern struct dentry *proc_lookup(struct inode *, struct dentry *,
											struct nameidata *);
extern const struct file_operations proc_kcore_operations;
extern const struct file_operations proc_kmsg_operations;
extern const struct file_operations ppc_htab_operations;

extern int pid_ns_prepare_proc(struct pid_namespace *ns);
extern void pid_ns_release_proc(struct pid_namespace *ns);

/*
 * proc_tty.c
 */
struct tty_driver;
extern void proc_tty_init(void);
extern void proc_tty_register_driver(struct tty_driver *driver);
extern void proc_tty_unregister_driver(struct tty_driver *driver);

/*
 * proc_devtree.c
 */
#ifdef CONFIG_PROC_DEVICETREE
struct device_node;
struct property;
extern void proc_device_tree_init(void);
extern void proc_device_tree_add_node(struct device_node *, struct proc_dir_entry *);
extern void proc_device_tree_add_prop(struct proc_dir_entry *pde, struct property *prop);
extern void proc_device_tree_remove_prop(struct proc_dir_entry *pde,
					 struct property *prop);
extern void proc_device_tree_update_prop(struct proc_dir_entry *pde,
					 struct property *newprop,	struct property *oldprop);
#endif /* CONFIG_PROC_DEVICETREE */

extern struct proc_dir_entry *proc_symlink(const char *,	struct proc_dir_entry *,
													const char *);
extern struct proc_dir_entry *proc_mkdir(const char *,struct proc_dir_entry *);
extern struct proc_dir_entry *proc_mkdir_mode(const char *name, mode_t mode,
			struct proc_dir_entry *parent);

static inline struct proc_dir_entry *create_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, 
	read_proc_t *read_proc, void * data)
{
	struct proc_dir_entry *res=create_proc_entry(name,mode,base);
	if (res) {
		res->read_proc=read_proc;
		res->data=data;
	}
	return res;
}
 
static inline struct proc_dir_entry *create_proc_info_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, get_info_t *get_info)
{
	struct proc_dir_entry *res=create_proc_entry(name,mode,base);
	if (res) res->get_info=get_info;
	return res;
}

extern struct proc_dir_entry *proc_net_fops_create(struct net *net,
	const char *name, mode_t mode, const struct file_operations *fops);
extern void proc_net_remove(struct net *net, const char *name);

#else

#define proc_root_driver NULL
#define proc_bus NULL

#define proc_net_fops_create(net, name, mode, fops)  ({ (void)(mode), NULL; })
static inline void proc_net_remove(struct net *net, const char *name) {}

static inline void proc_flush_task(struct task_struct *task)
{
}

static inline struct proc_dir_entry *create_proc_entry(const char *name,
	mode_t mode, struct proc_dir_entry *parent) { return NULL; }

#define remove_proc_entry(name, parent) do {} while (0)

static inline struct proc_dir_entry *proc_symlink(const char *name,
		struct proc_dir_entry *parent,const char *dest) {return NULL;}
static inline struct proc_dir_entry *proc_mkdir(const char *name,
	struct proc_dir_entry *parent) {return NULL;}

static inline struct proc_dir_entry *create_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, 
	read_proc_t *read_proc, void * data) { return NULL; }
static inline struct proc_dir_entry *create_proc_info_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base, get_info_t *get_info)
	{ return NULL; }

struct tty_driver;
static inline void proc_tty_register_driver(struct tty_driver *driver) {};
static inline void proc_tty_unregister_driver(struct tty_driver *driver) {};

extern struct proc_dir_entry proc_root;

static inline int pid_ns_prepare_proc(struct pid_namespace *ns)
{
	return 0;
}

static inline void pid_ns_release_proc(struct pid_namespace *ns)
{
}

#endif /* CONFIG_PROC_FS */

#if !defined(CONFIG_PROC_KCORE)
static inline void kclist_add(struct kcore_list *new, void *addr, size_t size)
{
}
#else
extern void kclist_add(struct kcore_list *, void *, size_t);
#endif

/*proc_get_link和proc_read（两者位于一个联合中，因为每次只有其中一个有意义）*/
union proc_op
{
	/*获得特定于进程的信息*/
	int (*proc_get_link)(struct inode *, struct dentry **, struct vfsmount **);
	/*用于在虚拟文件系统中建立谅解，指向特定于进程的数据*/
	int (*proc_read)(struct task_struct *task, char *page);
};

/*支持以面向inode的方式来查看proc文件系统的数据项。该机构用来将特定于proc的数据项与
VFS层的inode数据关联起来*/
struct proc_inode
{
	/*指向进程的pid实例。由于可能会以这种方式访问大量特定于进程的信息，因而特定于进程
	的inode与该数据建立关联的原因很明显*/
	struct pid *pid;
	/*fd记录了文件描述符，它对应于/proc/<pid>/fd/中的某个文件。借助fd，该目录下的所有
	文件都可以使用同一个file_operations*/
	int fd;
	/**/
	union proc_op op;
	/*指针指向关联到proc数据项的proc_dir_entry实例*/
	struct proc_dir_entry *pde;
	/*关联的inode实例，实际数据，而非指向该结构实例的指针。与VFS层用于inode管理的
	数据组织方式一样*/
	struct inode vfs_inode;
};

/*根据inode信息，可以使用container_of机制获取proc_inode实例地址。仅当该inode表示一个
特定于进程的数据项时，才会使用（这些数据项位于proc/pid目录下）*/
static inline struct proc_inode *PROC_I(const struct inode *inode)
{
	return container_of(inode, struct proc_inode, vfs_inode);
}

/*根据inode信息获取该proc_dir_entry实例*/
static inline struct proc_dir_entry *PDE(const struct inode *inode)
{
	return PROC_I(inode)->pde;
}

static inline struct net *PDE_NET(struct proc_dir_entry *pde)
{
	return pde->parent->data;
}

struct net *get_proc_net(const struct inode *inode);

struct proc_maps_private
{
	struct pid *pid;
	struct task_struct *task;
#ifdef CONFIG_MMU
	struct vm_area_struct *tail_vma;
#endif
};

#endif /* _LINUX_PROC_FS_H */
