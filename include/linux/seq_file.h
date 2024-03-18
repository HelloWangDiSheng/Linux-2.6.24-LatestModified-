#ifndef _LINUX_SEQ_FILE_H
#define _LINUX_SEQ_FILE_H
#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/string.h>
#include <linux/mutex.h>

struct seq_operations;
struct file;
struct vfsmount;
struct dentry;
struct inode;

/*顺序文件数据结构。struct file的private成员可以指向文件私有的任意数据，通用VFS函数
不会访问该数据，在这里，seq_open使用该指针建立了与struct seq_file的一个实例之间的关
联，struct seq_file中包含了顺序文件的状态*/
struct seq_file
{
	/*指向一个缓冲区，用于构建传输给用户层的数据*/
	char *buf;
	/*缓冲区中总的字节数目*/
	size_t size;
	/*赋值操作的起始位置*/
	size_t from;
	/*指定需要传输到用户层的剩余字节数目*/
	size_t count;
	/*缓冲区的另一个索引。标记了内核向缓冲区写入下一个新纪录的起始位置。注意！index和
	from的演变过程时不同的，因为从内核向缓冲区写入数据，与将这些数据复制到用户空间，是
	两个不同的操作*/
	loff_t index;
	/**/
	u64 version;
	/**/
	struct mutex lock;
	/*从文件系统实现者的角度看，最重要的成员是该指针，它指向的实例将通用的顺序文件实现
	与提供具体文件内容的例程关联起来*/
	const struct seq_operations *op;
	/**/
	void *private;
};

struct seq_operations
{
	void * (*start) (struct seq_file *m, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	int (*show) (struct seq_file *m, void *v);
};

int seq_open(struct file *, const struct seq_operations *);
ssize_t seq_read(struct file *, char __user *, size_t, loff_t *);
loff_t seq_lseek(struct file *, loff_t, int);
int seq_release(struct inode *, struct file *);
int seq_escape(struct seq_file *, const char *, const char *);
int seq_putc(struct seq_file *m, char c);
int seq_puts(struct seq_file *m, const char *s);

int seq_printf(struct seq_file *, const char *, ...) 	__attribute__ ((format (printf,2,3)));

int seq_path(struct seq_file *, struct vfsmount *, struct dentry *, char *);

int single_open(struct file *, int (*)(struct seq_file *, void *), void *);
int single_release(struct inode *, struct file *);
void *__seq_open_private(struct file *, const struct seq_operations *, int);
int seq_open_private(struct file *, const struct seq_operations *, int);
int seq_release_private(struct inode *, struct file *);

#define SEQ_START_TOKEN ((void *)1)

/*
 * Helpers for iteration over list_head-s in seq_files
 */

extern struct list_head *seq_list_start(struct list_head *head, loff_t pos);
extern struct list_head *seq_list_start_head(struct list_head *head, loff_t pos);
extern struct list_head *seq_list_next(void *v, struct list_head *head, loff_t *ppos);

#endif
#endif
