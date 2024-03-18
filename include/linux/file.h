/*�����ļ��������ṹ�е��ļ�ָ�����麯��*/
#ifndef __LINUX_FILE_H
#define __LINUX_FILE_H

#include <asm/atomic.h>
#include <linux/posix_types.h>
#include <linux/compiler.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/types.h>

/*Ĭ�ϵ�fd����������BITS_PER_LONG���������copy_fdset��֤���ص���Ŀ*/
#define NR_OPEN_DEFAULT BITS_PER_LONG


/*�Ѵ��ļ���λͼ�������fe_set�ʺ��ڴ�������ļ���Ŀ������BITS_PER_LONG�Ľ��̣�
�����Ҫ�򿪸����ļ����ں˻����һ��fd_setʵ�����滻�����embedded_fd_set*/
struct embedded_fd_set
{
	unsigned long fds_bits[1];
};

/*�ļ���������*/
struct fdtable
{
	/*��ǰ���̿��Դ�����ļ�������ļ��������������Ŀ������û�й̶����ޣ���Ϊ������ֵ
�������ڱ�Ҫʱ���ӣ�ֻҪû�г���Rlimitָ����ֵ��*/
	unsigned int max_fds;
	/*ָ�����飬ÿ��������ָ��һ��file�ṹ��ʵ��������һ�����ļ���������Ϣ���û��ռ�
	���̵��ļ��������䵱������������ĳ�����max_fds����*/
	struct file ** fd;
	/*λ��ָ�룬��λ������ŵ�ǰ���д��ļ�����������ÿ�����ܵ��ļ�����������Ӧһ��
	����λ������ñ���λ��λ�����Ӧ���ļ�����������ʹ���У����򣬸�������δʹ�ã���
	ǰ����λ�õ������Ŀ��max_fdsָ��*/
	fd_set *close_on_exec;
	/*λ��ָ�룬��λ�򱣴���������execϵͳ����ʱ��Ҫ�رյ��ļ�����������Ϣ*/
	fd_set *open_fds;
	/**/
	struct rcu_head rcu;
	/*������һ���ļ���������*/
	struct fdtable *next;
};

/*�Ѵ��ļ�����Ϣ*/
struct files_struct
{
	/*���øýṹ�ļ���*/
	atomic_t count;
	/*�ļ���������ָ��*/
	struct fdtable *fdt;
	/*�ļ���������ʵ��*/
	struct fdtable fdtab;
   	/*SMPϵͳ��д����λ��һ�������Ļ�����*/
	spinlock_t file_lock ____cacheline_aligned_in_smp;
  	/*��һ���ļ����������*/
	int next_fd;
	/*λ����ʽ������ִ��execʱ��Ҫ�رյ��ļ���Ϣ*/
	struct embedded_fd_set close_on_exec_init;
	/*λ����ʽ�������Ѵ��ļ�����Ϣ*/
	struct embedded_fd_set open_fds_init;
	/*ָ������ָ���Ѵ��ļ�struct fileʵ��*/
	struct file * fd_array[NR_OPEN_DEFAULT];
};

/*��ȡ�Ѵ��ļ����ļ�����������Ϣ*/
#define files_fdtable(files) (rcu_dereference((files)->fdt))
/*�����Ѵ��ļ�����Ϣ��slab����*/
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

/*rcu�����ͷ��ļ���������*/
static inline void free_fdtable(struct fdtable *fdt)
{
	call_rcu(&fdt->rcu, free_fdtable_rcu);
}
/*��ȡ�ļ��������ṹ���ļ���������������Ӧ���ļ�ָ��*/
static inline struct file * fcheck_files(struct files_struct *files, unsigned int fd)
{
	struct file * file = NULL;
	/*��ȡ�ļ��������ṹ��Ӧ���ļ���������*/
	struct fdtable *fdt = files_fdtable(files);
	/*���ָ���ļ����������ļ�������������Ч�����ȡ��Ӧ���ļ�ָ��*/
	if (fd < fdt->max_fds)
		file = rcu_dereference(fdt->fd[fd]);
	return file;
}

/*
 * Check whether the specified fd has an open file.
 */
/*���ָ�����ļ��������Ƿ����ļ�������������Ч*/
#define fcheck(fd) fcheck_files(current->files, fd)

extern void FASTCALL(fd_install(unsigned int fd, struct file * file));

struct task_struct;

struct files_struct *get_files_struct(struct task_struct *);
void FASTCALL(put_files_struct(struct files_struct *fs));
void reset_files_struct(struct task_struct *, struct files_struct *);

extern struct kmem_cache *files_cachep;

#endif /* __LINUX_FILE_H */
