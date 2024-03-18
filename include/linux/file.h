/*访问文件描述符结构中的文件指针数组函数*/
#ifndef __LINUX_FILE_H
#define __LINUX_FILE_H

#include <asm/atomic.h>
#include <linux/posix_types.h>
#include <linux/compiler.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/types.h>

/*默认的fd数组至少有BITS_PER_LONG个数组项，有copy_fdset保证返回的数目*/
#define NR_OPEN_DEFAULT BITS_PER_LONG


/*已打开文件的位图，袖珍版fe_set适合于大多数打开文件数目不超过BITS_PER_LONG的进程，
如果需要打开更多文件，内核会分配一个fd_set实例，替换最初的embedded_fd_set*/
struct embedded_fd_set
{
	unsigned long fds_bits[1];
};

/*文件描述符表*/
struct fdtable
{
	/*当前进程可以处理的文件对象和文件描述符的最大数目，这里没有固定上限，因为这两个值
都可以在必要时增加（只要没有超出Rlimit指定的值）*/
	unsigned int max_fds;
	/*指针数组，每个数组项指向一个file结构的实例，管理一个打开文件的所有信息。用户空间
	进程的文件描述符充当索引，该数组的长度有max_fds定义*/
	struct file ** fd;
	/*位域指针，该位域管理着当前所有打开文件的描述符，每个可能的文件描述符都对应一个
	比特位，如果该比特位置位，则对应的文件描述符处于使用中，否则，该描述符未使用，当
	前比特位置的最大数目有max_fds指定*/
	fd_set *close_on_exec;
	/*位域指针，该位域保存了所有在exec系统调用时将要关闭的文件描述符的信息*/
	fd_set *open_fds;
	/**/
	struct rcu_head rcu;
	/*连接下一个文件描述符表*/
	struct fdtable *next;
};

/*已打开文件表信息*/
struct files_struct
{
	/*引用该结构的计数*/
	atomic_t count;
	/*文件描述符表指针*/
	struct fdtable *fdt;
	/*文件描述符表实例*/
	struct fdtable fdtab;
   	/*SMP系统中写部分位于一个单独的缓存行*/
	spinlock_t file_lock ____cacheline_aligned_in_smp;
  	/*下一个文件描述符编号*/
	int next_fd;
	/*位域形式描述的执行exec时需要关闭的文件信息*/
	struct embedded_fd_set close_on_exec_init;
	/*位域形式描述的已打开文件的信息*/
	struct embedded_fd_set open_fds_init;
	/*指针属性指向已打开文件struct file实例*/
	struct file * fd_array[NR_OPEN_DEFAULT];
};

/*获取已打开文件的文件描述符表信息*/
#define files_fdtable(files) (rcu_dereference((files)->fdt))
/*保存已打开文件的信息的slab缓存*/
extern struct kmem_cache *filp_cachep;

extern void FASTCALL(__fput(struct file *));
extern void FASTCALL(fput(struct file *));

struct file_operations;
struct vfsmount;
struct dentry;
extern int init_file(struct file *, struct vfsmount *mnt, struct dentry *dentry,
						mode_t mode, const struct file_operations *fop);
extern struct file *alloc_file(struct vfsmount *, struct dentry *dentry, mode_t mode,
										const struct file_operations *fop);

static inline void fput_light(struct file *file, int fput_needed)
{
	if (unlikely(fput_needed))
		fput(file);
}

extern struct file * FASTCALL(fget(unsigned int fd));
extern struct file * FASTCALL(fget_light(unsigned int fd, int *fput_needed));
extern void FASTCALL(set_close_on_exec(unsigned int fd, int flag));
extern void put_filp(struct file *);
extern int get_unused_fd(void);
extern int get_unused_fd_flags(int flags);
extern void FASTCALL(put_unused_fd(unsigned int fd));
struct kmem_cache;

extern int expand_files(struct files_struct *, int nr);
extern void free_fdtable_rcu(struct rcu_head *rcu);
extern void __init files_defer_init(void);

/*rcu保护释放文件描述符表*/
static inline void free_fdtable(struct fdtable *fdt)
{
	call_rcu(&fdt->rcu, free_fdtable_rcu);
}
/*获取文件描述符结构中文件描述符表索引对应的文件指针*/
static inline struct file * fcheck_files(struct files_struct *files, unsigned int fd)
{
	struct file * file = NULL;
	/*获取文件描述符结构对应的文件描述符表*/
	struct fdtable *fdt = files_fdtable(files);
	/*如果指定文件描述符在文件描述符表中有效，则获取对应的文件指针*/
	if (fd < fdt->max_fds)
		file = rcu_dereference(fdt->fd[fd]);
	return file;
}

/*
 * Check whether the specified fd has an open file.
 */
/*检查指定的文件描述符是否在文件描述符表中有效*/
#define fcheck(fd) fcheck_files(current->files, fd)

extern void FASTCALL(fd_install(unsigned int fd, struct file * file));

struct task_struct;

struct files_struct *get_files_struct(struct task_struct *);
void FASTCALL(put_files_struct(struct files_struct *fs));
void reset_files_struct(struct task_struct *, struct files_struct *);

extern struct kmem_cache *files_cachep;

#endif /* __LINUX_FILE_H */
