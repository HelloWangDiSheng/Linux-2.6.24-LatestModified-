#ifndef _LINUX_FS_H
#define _LINUX_FS_H

/*定义一些如文件描述符表等重要数据结构的文件*/

#include <linux/limits.h>
#include <linux/ioctl.h>

/*
 * It's silly to have NR_OPEN bigger than NR_FILE, but you can change
 * the file limit at runtime and only root can increase the per-process
 * nr_file rlimit, so it's safe to set up a ridiculously high absolute
 * upper limit on files-per-process.
 *
 * Some programs (notably those using select()) may have to be
 * recompiled to take full advantage of the new limits..
 */

/* Fixed constants first: */
#undef NR_OPEN
/*可打开文件数目的绝对上限*/
#define NR_OPEN 			(1024*1024)
/*初始化时设置的可打开文件数目限制*/
#define INR_OPEN 			1024

/*块大小偏移位*/
#define BLOCK_SIZE_BITS 10
/*块大小*/
#define BLOCK_SIZE 			(1<<BLOCK_SIZE_BITS)

/*查找相关的文件起始位*/
#define SEEK_SET			0
/*文件当前位置*/
#define SEEK_CUR			1
/*查找相关的文件末尾*/
#define SEEK_END			2
#define SEEK_MAX			SEEK_END

/* And dynamically-tunable limits and defaults: */
struct files_stat_struct
{
	/*只读*/
	int nr_files;
	/*只读*/
	int nr_free_files;
	/*tunable*/
	int max_files;
};
extern struct files_stat_struct files_stat;
extern int get_max_files(void);

struct inodes_stat_t
{
	int nr_inodes;
	int nr_unused;
	/*为sysctl ABI兼容性填充字节*/
	int dummy[5];
};

extern struct inodes_stat_t inodes_stat;
extern int leases_enable, lease_break_time;

#ifdef CONFIG_DNOTIFY
extern int dir_notify_enable;
#endif
/*大系统上可以打开的文件上限上溢*/
#define NR_FILE			8192

#define MAY_EXEC 		1
#define MAY_WRITE 		2
#define MAY_READ 		4
#define MAY_APPEND 		8

#define FMODE_READ 		1
#define FMODE_WRITE 	2

/*内核内部扩展*/
#define FMODE_LSEEK		4
#define FMODE_PREAD		8
#define FMODE_PWRITE	FMODE_PREAD	/* These go hand in hand */

/* File is being opened for execution. Primary users of this flag are
   distributed filesystems that can use it to achieve correct ETXTBUSY
   behavior for cross-node execution/opening_for_writing of files */
/*文件为执行才打开*/
#define FMODE_EXEC		16

#define RW_MASK			1
#define RWA_MASK		2
#define READ 			0
#define WRITE 			1
/*预读，如果没有资源时不要阻塞*/
#define READA 			2
/*为ll_rw_block()等待缓冲区锁*/
#define SWRITE 			3
/*BIO同步读写*/
#define READ_SYNC			(READ | (1 << BIO_RW_SYNC))
#define READ_META			(READ | (1 << BIO_RW_META))
#define WRITE_SYNC			(WRITE | (1 << BIO_RW_SYNC))
#define WRITE_BARRIER		((1 << BIO_RW) | (1 << BIO_RW_BARRIER))

#define SEL_IN		1
#define SEL_OUT		2
#define SEL_EX		4

/*文件系统类型公共标识*/
#define FS_REQUIRES_DEV 			1
#define FS_BINARY_MOUNTDATA 		2
#define FS_HAS_SUBTYPE 				4
/* Check the paths ".", ".." for staleness */
#define FS_REVAL_DOT				(1 << 14)
/*rename()期间，内部文件系统处理d_move()*/
#define FS_RENAME_DOES_D_MOVE		(1 << 15)

/*文件系统独立的安装标识，最多支持32个标识*/
/*只读安装*/
#define MS_RDONLY	 		1
/*忽略suid和sgid标识*/
#define MS_NOSUID	 		(1 << 1)
/*不能访问设备特殊文件*/
#define MS_NODEV	 		(1 << 2)
/*不允许程序运行*/
#define MS_NOEXEC	 		(1 << 3)
/*立即写同步*/
#define MS_SYNCHRONOUS		(1 << 4)
/*安装文件系统的*/
#define MS_REMOUNT			(1 << 5)	/* Alter flags of a mounted FS */
/**/
#define MS_MANDLOCK			(1 << 6)	/* Allow mandatory locks on an FS */
/*同步修改目录项*/
#define MS_DIRSYNC			(1 << 7)
/*不更新访问时间*/
#define MS_NOATIME			(1 << 10)
/*不更新目录的访问时间*/
#define MS_NODIRATIME		(1 << 11)
/**/
#define MS_BIND				(1 << 12)
/**/
#define MS_MOVE				(1 << 13)
/**/
#define MS_REC				(1 << 14)
/*过期标识*/
#define MS_VERBOSE			(1 << 15)	/* War is peace. Verbosity is silence. MS_VERBOSE is deprecated. */
#define MS_SILENT			(1 << 15)
/*虚拟文件系统不使用umask标识*/
#define MS_POSIXACL			(1<<16)
/*不可绑定安装*/
#define MS_UNBINDABLE		(1<<17)
/*私有安装*/
#define MS_PRIVATE			(1<<18)
/*从属安装*/
#define MS_SLAVE			(1<<19)
/*更改为共享安装*/
#define MS_SHARED			(1<<20)
/*Update atime relative to mtime/ctime*/
#define MS_RELATIME			(1<<21)
/*kern_mount调用*/
#define MS_KERNMOUNT		(1<<22)
/**/
#define MS_ACTIVE			(1<<30)
/**/
#define MS_NOUSER			(1<<31)

/*可被MS_REMOUNT修改的超级块标识*/
#define MS_RMT_MASK	(MS_RDONLY|MS_SYNCHRONOUS|MS_MANDLOCK)

/*Old magic mount flag and mask*/
#define MS_MGC_VAL 0xC0ED0000
#define MS_MGC_MSK 0xffff0000

/* Inode flags - they have nothing to superblock flags now */
/*立即写同步*/
#define S_SYNC			1
/*不更新文件访问时间*/
#define S_NOATIME		(1 << 1)
/*仅追加文件*/
#define S_APPEND		(1 << 2)
/*Immutable file*/
#define S_IMMUTABLE		(1 << 3)
/*已删除，但其目录仍然处于打开状态*/
#define S_DEAD			(1 << 4)
/*Inode is not counted to quota*/
#define S_NOQUOTA		(1 << 5)
/*同步修改目录项*/
#define S_DIRSYNC		(1 << 6)
/*不更新文件的ctime和mtime*/
#define S_NOCMTIME		(1 << 7)
/*Do not truncate: swapon got its bmaps*/
#define S_SWAPFILE		(1 << 8)
/*文件系统内部Inode*/
#define S_PRIVATE		(1 << 9)

/*
 * Note that nosuid etc flags are inode-specific: setting some file-system
 * flags just means all the inodes inherit those flags by default. It might be
 * possible to override it selectively if you really wanted to with some
 * ioctl() that is not currently implemented.
 *
 * Exception: MS_RDONLY is always applied to the entire file system.
 *
 * Unfortunately, it is possible to change a filesystems flags with it mounted
 * with files in use.  This means that all of the inodes will not have their
 * i_flags updated.  Hence, i_flags no longer inherit the superblock mount
 * flags, so these have to be checked separately. -- rmk@arm.uk.linux.org
 */
#define __IS_FLG(inode, flg) ((inode)->i_sb->s_flags & (flg))
#define IS_RDONLY(inode) ((inode)->i_sb->s_flags & MS_RDONLY)
#define IS_SYNC(inode)		(__IS_FLG(inode, MS_SYNCHRONOUS) || ((inode)->i_flags & S_SYNC))
#define IS_DIRSYNC(inode)	(__IS_FLG(inode, MS_SYNCHRONOUS|MS_DIRSYNC) || \
					((inode)->i_flags & (S_SYNC|S_DIRSYNC)))
#define IS_MANDLOCK(inode)	__IS_FLG(inode, MS_MANDLOCK)
#define IS_NOATIME(inode)   __IS_FLG(inode, MS_RDONLY|MS_NOATIME)
#define IS_NOQUOTA(inode)	((inode)->i_flags & S_NOQUOTA)
#define IS_APPEND(inode)	((inode)->i_flags & S_APPEND)
#define IS_IMMUTABLE(inode)	((inode)->i_flags & S_IMMUTABLE)
#define IS_POSIXACL(inode)	__IS_FLG(inode, MS_POSIXACL)
#define IS_DEADDIR(inode)	((inode)->i_flags & S_DEAD)
#define IS_NOCMTIME(inode)	((inode)->i_flags & S_NOCMTIME)
#define IS_SWAPFILE(inode)	((inode)->i_flags & S_SWAPFILE)
#define IS_PRIVATE(inode)	((inode)->i_flags & S_PRIVATE)

/* the read-only stuff doesn't really belong here, but any other place is
   probably as bad and I don't want to create yet another include file. */

#define BLKROSET   _IO(0x12,93)	/* set device read-only (0 = read-write) */
#define BLKROGET   _IO(0x12,94)	/* get read-only status (0 = read_write) */
#define BLKRRPART  _IO(0x12,95)	/* re-read partition table */
#define BLKGETSIZE _IO(0x12,96)	/* return device size /512 (long *arg) */
#define BLKFLSBUF  _IO(0x12,97)	/* flush buffer cache */
#define BLKRASET   _IO(0x12,98)	/* set read ahead for block device */
#define BLKRAGET   _IO(0x12,99)	/* get current read ahead setting */
#define BLKFRASET  _IO(0x12,100)/* set filesystem (mm/filemap.c) read-ahead */
#define BLKFRAGET  _IO(0x12,101)/* get filesystem (mm/filemap.c) read-ahead */
#define BLKSECTSET _IO(0x12,102)/* set max sectors per request (ll_rw_blk.c) */
#define BLKSECTGET _IO(0x12,103)/* get max sectors per request (ll_rw_blk.c) */
#define BLKSSZGET  _IO(0x12,104)/* get block device sector size */
#if 0
#define BLKPG      _IO(0x12,105)/* See blkpg.h */

/* Some people are morons.  Do not use sizeof! */

#define BLKELVGET  _IOR(0x12,106,size_t)/* elevator get */
#define BLKELVSET  _IOW(0x12,107,size_t)/* elevator set */
/* This was here just to show that the number is taken -
   probably all these _IO(0x12,*) ioctls should be moved to blkpg.h. */
#endif
/* A jump here: 108-111 have been used for various private purposes. */
#define BLKBSZGET  _IOR(0x12,112,size_t)
#define BLKBSZSET  _IOW(0x12,113,size_t)
#define BLKGETSIZE64 _IOR(0x12,114,size_t)	/* return device size in bytes (u64 *arg) */
#define BLKTRACESETUP _IOWR(0x12,115,struct blk_user_trace_setup)
#define BLKTRACESTART _IO(0x12,116)
#define BLKTRACESTOP _IO(0x12,117)
#define BLKTRACETEARDOWN _IO(0x12,118)

#define BMAP_IOCTL 1		/* obsolete - kept for compatibility */
#define FIBMAP	   _IO(0x00,1)	/* bmap access */
#define FIGETBSZ   _IO(0x00,2)	/* get the block size used for bmap */

#define	FS_IOC_GETFLAGS			_IOR('f', 1, long)
#define	FS_IOC_SETFLAGS			_IOW('f', 2, long)
#define	FS_IOC_GETVERSION		_IOR('v', 1, long)
#define	FS_IOC_SETVERSION		_IOW('v', 2, long)
#define FS_IOC32_GETFLAGS		_IOR('f', 1, int)
#define FS_IOC32_SETFLAGS		_IOW('f', 2, int)
#define FS_IOC32_GETVERSION		_IOR('v', 1, int)
#define FS_IOC32_SETVERSION		_IOW('v', 2, int)

/*
 * Inode flags (FS_IOC_GETFLAGS / FS_IOC_SETFLAGS)
 */
#define	FS_SECRM_FL			0x00000001 /* Secure deletion */
#define	FS_UNRM_FL			0x00000002 /* Undelete */
#define	FS_COMPR_FL			0x00000004 /* Compress file */
#define FS_SYNC_FL			0x00000008 /* Synchronous updates */
#define FS_IMMUTABLE_FL			0x00000010 /* Immutable file */
#define FS_APPEND_FL			0x00000020 /* writes to file may only append */
#define FS_NODUMP_FL			0x00000040 /* do not dump file */
#define FS_NOATIME_FL			0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define FS_DIRTY_FL			0x00000100
#define FS_COMPRBLK_FL			0x00000200 /* One or more compressed clusters */
#define FS_NOCOMP_FL			0x00000400 /* Don't compress */
#define FS_ECOMPR_FL			0x00000800 /* Compression error */
/* End compression flags --- maybe not all used */
#define FS_BTREE_FL			0x00001000 /* btree format dir */
#define FS_INDEX_FL			0x00001000 /* hash-indexed directory */
#define FS_IMAGIC_FL			0x00002000 /* AFS directory */
#define FS_JOURNAL_DATA_FL		0x00004000 /* Reserved for ext3 */
#define FS_NOTAIL_FL			0x00008000 /* file tail should not be merged */
#define FS_DIRSYNC_FL			0x00010000 /* dirsync behaviour (directories only) */
#define FS_TOPDIR_FL			0x00020000 /* Top of directory hierarchies*/
#define FS_EXTENT_FL			0x00080000 /* Extents */
#define FS_DIRECTIO_FL			0x00100000 /* Use direct i/o */
#define FS_RESERVED_FL			0x80000000 /* reserved for ext2 lib */

#define FS_FL_USER_VISIBLE		0x0003DFFF /* User visible flags */
#define FS_FL_USER_MODIFIABLE		0x000380FF /* User modifiable flags */


#define SYNC_FILE_RANGE_WAIT_BEFORE	1
#define SYNC_FILE_RANGE_WRITE		2
#define SYNC_FILE_RANGE_WAIT_AFTER	4

#ifdef __KERNEL__

#include <linux/linkage.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/stat.h>
#include <linux/cache.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/radix-tree.h>
#include <linux/prio_tree.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/mutex.h>
#include <linux/capability.h>

#include <asm/atomic.h>
#include <asm/semaphore.h>
#include <asm/byteorder.h>

struct export_operations;
struct hd_geometry;
struct iovec;
struct nameidata;
struct kiocb;
struct pipe_inode_info;
struct poll_table_struct;
struct kstatfs;
struct vm_area_struct;
struct vfsmount;

extern void __init inode_init(void);
extern void __init inode_init_early(void);
extern void __init mnt_init(void);
extern void __init files_init(unsigned long);

struct buffer_head;
typedef int (get_block_t)(struct inode *inode, sector_t iblock,
			struct buffer_head *bh_result, int create);
typedef void (dio_iodone_t)(struct kiocb *iocb, loff_t offset,	ssize_t bytes, void *private);

/*
 * Attribute flags.  These should be or-ed together to figure out what
 * has been changed!
 */
#define ATTR_MODE	1
#define ATTR_UID	2
#define ATTR_GID	4
#define ATTR_SIZE	8
#define ATTR_ATIME	16
#define ATTR_MTIME	32
#define ATTR_CTIME	64
#define ATTR_ATIME_SET	128
#define ATTR_MTIME_SET	256
#define ATTR_FORCE	512	/* Not a change, but a change it */
#define ATTR_ATTR_FLAG	1024
#define ATTR_KILL_SUID	2048
#define ATTR_KILL_SGID	4096
#define ATTR_FILE	8192
#define ATTR_KILL_PRIV	16384
#define ATTR_OPEN	32768	/* Truncating from open(O_TRUNC) */

/*
 * This is the Inode Attributes structure, used for notify_change().  It
 * uses the above definitions as flags, to know which values have changed.
 * Also, in this manner, a Filesystem can look at only the values it cares
 * about.  Basically, these are the attributes that the VFS layer can
 * request to change from the FS layer.
 *
 * Derek Atkins <warlord@MIT.EDU> 94-10-20
 */
struct iattr
{
	unsigned int	ia_valid;
	umode_t		ia_mode;
	uid_t		ia_uid;
	gid_t		ia_gid;
	loff_t		ia_size;
	struct timespec	ia_atime;
	struct timespec	ia_mtime;
	struct timespec	ia_ctime;

	/*
	 * Not an attribute, but an auxilary info for filesystems wanting to
	 * implement an ftruncate() like method.  NOTE: filesystem should
	 * check for (ia_valid & ATTR_FILE), and not for (ia_file != NULL).
	 */
	struct file	*ia_file;
};

/*磁盘配额头文件
 * Includes for diskquotas.
 */
#include <linux/quota.h>

/**
 * enum positive_aop_returns - aop return codes with specific semantics
 *
 * @AOP_WRITEPAGE_ACTIVATE: Informs the caller that page writeback has
 * 			    completed, that the page is still locked, and
 * 			    should be considered active.  The VM uses this hint
 * 			    to return the page to the active list -- it won't
 * 			    be a candidate for writeback again in the near
 * 			    future.  Other callers must be careful to unlock
 * 			    the page if they get this return.  Returned by
 * 			    writepage();
 *
 * @AOP_TRUNCATED_PAGE: The AOP method that was handed a locked page has
 *  			unlocked it and the page might have been truncated.
 *  			The caller should back up to acquiring a new page and
 *  			trying again.  The aop will be taking reasonable
 *  			precautions not to livelock.  If the caller held a page
 *  			reference, it should drop it before retrying.  Returned
 *  			by readpage().
 *
 * address_space_operation functions return these large constants to indicate
 * special semantics to the caller.  These are much larger than the bytes in a
 * page to allow for functions that return the number of bytes operated on in a
 * given page.
 */

enum positive_aop_returns
{
	AOP_WRITEPAGE_ACTIVATE	= 0x80000,
	AOP_TRUNCATED_PAGE	= 0x80001,
};

#define AOP_FLAG_UNINTERRUPTIBLE	0x0001 /* will not do a short write */
#define AOP_FLAG_CONT_EXPAND		0x0002 /* called from cont_expand */

/*
 * oh the beauties of C type declarations.
 */
struct page;
struct address_space;
struct writeback_control;

struct iov_iter
{
	const struct iovec *iov;
	unsigned long nr_segs;
	size_t iov_offset;
	size_t count;
};

size_t iov_iter_copy_from_user_atomic(struct page *page,	struct iov_iter *i,
												unsigned long offset, size_t bytes);
size_t iov_iter_copy_from_user(struct page *page,	struct iov_iter *i,
									unsigned long offset, size_t bytes);
void iov_iter_advance(struct iov_iter *i, size_t bytes);
int iov_iter_fault_in_readable(struct iov_iter *i, size_t bytes);
size_t iov_iter_single_seg_count(struct iov_iter *i);
static inline void iov_iter_init(struct iov_iter *i,	const struct iovec *iov,
									unsigned long nr_segs, size_t count, size_t written)
{
	i->iov = iov;
	i->nr_segs = nr_segs;
	i->iov_offset = 0;
	i->count = count + written;

	iov_iter_advance(i, written);
}

static inline size_t iov_iter_count(struct iov_iter *i)
{
	return i->count;
}


struct address_space_operations
{
	int (*writepage)(struct page *page, struct writeback_control *wbc);
	int (*readpage)(struct file *, struct page *);
	void (*sync_page)(struct page *);
	/* Write back some dirty pages from this mapping. */
	int (*writepages)(struct address_space *, struct writeback_control *);
	/* Set a page dirty.  Return true if this dirtied it */
	int (*set_page_dirty)(struct page *page);
	int (*readpages)(struct file *filp, struct address_space *mapping,
						struct list_head *pages, unsigned nr_pages);

	/*
	 * ext3 requires that a successful prepare_write() call be followed
	 * by a commit_write() call - they must be balanced
	 */
	int (*prepare_write)(struct file *, struct page *, unsigned, unsigned);
	int (*commit_write)(struct file *, struct page *, unsigned, unsigned);
	int (*write_begin)(struct file *, struct address_space *mapping, loff_t pos,
						unsigned len, unsigned flags, struct page **pagep, void **fsdata);
	int (*write_end)(struct file *, struct address_space *mapping,	loff_t pos,
						unsigned len, unsigned copied, struct page *page, void *fsdata);

	/* Unfortunately this kludge is needed for FIBMAP. Don't use it */
	sector_t (*bmap)(struct address_space *, sector_t);
	void (*invalidatepage) (struct page *, unsigned long);
	int (*releasepage) (struct page *, gfp_t);
	ssize_t (*direct_IO)(int, struct kiocb *, const struct iovec *iov,	loff_t offset,
							unsigned long nr_segs);
	struct page* (*get_xip_page)(struct address_space *, sector_t,	int);
	/* migrate the contents of a page to the specified target */
	int (*migratepage) (struct address_space *, struct page *, struct page *);
	int (*launder_page) (struct page *);
};

/*
 * pagecache_write_begin/pagecache_write_end must be used by general code
 * to write into the pagecache.
 */
int pagecache_write_begin(struct file *, struct address_space *mapping,	loff_t pos,
							unsigned len, unsigned flags, struct page **pagep, void **fsdata);
int pagecache_write_end(struct file *, struct address_space *mapping, loff_t pos,
							unsigned len, unsigned copied,	struct page *page, void *fsdata);

struct backing_dev_info;
/**/
struct address_space
{
	/**/
	struct inode		*host;		/* owner: inode, block_device */
	/**/
	struct radix_tree_root	page_tree;	/* radix tree of all pages */
	/**/
	rwlock_t		tree_lock;	/* and rwlock protecting it */
	/**/
	unsigned int		i_mmap_writable;/* count VM_SHARED mappings */
	/**/
	struct prio_tree_root	i_mmap;		/* tree of private and shared mappings */
	/**/
	struct list_head	i_mmap_nonlinear;/*list VM_NONLINEAR mappings */
	/**/
	spinlock_t		i_mmap_lock;	/* protect tree, count, list */
	/**/
	unsigned int		truncate_count;	/* Cover race condition with truncate */
	/**/
	unsigned long		nrpages;	/* number of total pages */
	/**/
	pgoff_t			writeback_index;/* writeback starts here */
	/**/
	const struct address_space_operations *a_ops;	/* methods */
	/**/
	unsigned long		flags;		/* error bits/gfp mask */
	/**/
	struct backing_dev_info *backing_dev_info; /* device readahead, etc */
	/**/
	spinlock_t		private_lock;	/* for use by the address_space */
	/**/
	struct list_head	private_list;	/* ditto */
	/**/
	struct address_space	*assoc_mapping;	/* ditto */
} __attribute__((aligned(sizeof(long))));
	/*
	 * On most architectures that alignment is already the case; but
	 * must be enforced here for CRIS, to let the least signficant bit
	 * of struct page's "mapping" pointer be used for PAGE_MAPPING_ANON.
	 */

/**/
struct block_device
{
	/**/
	dev_t			bd_dev;  /* not a kdev_t - it's a search key */
	/**/
	struct inode *		bd_inode;	/* will die */
	/**/
	int			bd_openers;
	/**/
	struct mutex		bd_mutex;	/* open/close mutex */
	/**/
	struct semaphore	bd_mount_sem;
	/**/
	struct list_head	bd_inodes;
	/**/
	void *			bd_holder;
	/**/
	int			bd_holders;
#ifdef CONFIG_SYSFS
	/**/
	struct list_head	bd_holder_list;
#endif
	/**/
	struct block_device *	bd_contains;
	/**/
	unsigned		bd_block_size;
	/**/
	struct hd_struct *	bd_part;
	/* number of times partitions within this device have been opened. */
	/**/
	unsigned		bd_part_count;
	/**/
	int			bd_invalidated;
	/**/
	struct gendisk *	bd_disk;
	/**/
	struct list_head	bd_list;
	/**/
	struct backing_dev_info *bd_inode_backing_dev_info;
	/*
	 * Private data.  You must have bd_claim'ed the block_device
	 * to use this.  NOTE:  bd_claim allows an owner to claim
	 * the same device multiple times, the owner must take special
	 * care to not mess up bd_private for that case.
	 */
	/**/
	unsigned long		bd_private;
};

/*
 * Radix-tree tags, for tagging dirty and writeback pages within the pagecache
 * radix trees
 */
#define PAGECACHE_TAG_DIRTY	0
#define PAGECACHE_TAG_WRITEBACK	1

int mapping_tagged(struct address_space *mapping, int tag);

/*
 * Might pages of this file be mapped into userspace?
 */
static inline int mapping_mapped(struct address_space *mapping)
{
	return	!prio_tree_empty(&mapping->i_mmap) ||
		!list_empty(&mapping->i_mmap_nonlinear);
}

/*
 * Might pages of this file have been modified in userspace?
 * Note that i_mmap_writable counts all VM_SHARED vmas: do_mmap_pgoff
 * marks vma as VM_SHARED if it is shared, and the file was opened for
 * writing i.e. vma may be mprotected writable even if now readonly.
 */
static inline int mapping_writably_mapped(struct address_space *mapping)
{
	return mapping->i_mmap_writable != 0;
}

/*
 * Use sequence counter to get consistent i_size on 32-bit processors.
 */
#if BITS_PER_LONG==32 && defined(CONFIG_SMP)
#include <linux/seqlock.h>
#define __NEED_I_SIZE_ORDERED
#define i_size_ordered_init(inode) seqcount_init(&inode->i_size_seqcount)
#else
#define i_size_ordered_init(inode) do { } while (0)
#endif

/*通用文件模型中inode的内存表示形式。inode的成员可能分为下面两类：(1) 描述文件状态
的元数据。例如，访问权限或上次修改的日期(2) 保存实际文件内容的数据段（或指向数据的
指针）。就文本文件来说，用于保存文本。这里考察的inode结构是用于在内存中进行处理，因
而包含了一些实际介质上存储的inode所没有的成员。这些是由内核自身在从底层文件系统读入
信息时生成或动态建立。还有一些文件系统，如FAT和Reiserfs没有使用经典意义上的inode，因
此必须从其包含的数据中提取信息并生成这里给出的形式。*/
struct inode
{
	/*每个inode不仅出现在特定于状态的链表中，还在一个散列表中出现，以支持根据inode编
	号和超级块快速访问inode，这两项的组合在系统范围内是唯一的。该散列表是一个数组，可
	以借助于全局变量inode_hashtable（也定义在fs/inode.c中）来访问。该表启动期间在fs/
	inode.c中的inode_init函数中初始化。消息输出表明数组的长度基于可用的物理内存计算。
	wolfgang@meitner> dmesg
	...
	Inode-cache hash table entries: 262144 (order: 9, 2097152 bytes)
	...
	fs/inode.c中的hash函数用于计算散列和。它将inode编号和超级块对象的地址合并为一个唯
	一的编号，保证位于散列表已经分配的下标范围内。①碰撞照例通过溢出链表解决。inode的
	成员i_hash用于管理溢出链表*/
	struct hlist_node	i_hash;
	/*每个inode都有一个i_list成员，可以将inode存储在一个链表中。根据inode的状态，它可
	能有3种主要的情况：(1) inode存在于内存中，未关联到任何文件，也不处于活动使用状态。
	(2) inode结构在内存中，正在由一个或多个进程使用，通常表示一个文件。两个计数器（
	i_count和i_nlink）的值都必须大于0。文件内容和inode元数据都与底层块设备上的信息相
	同。也就是说，从上一次与存储介质同步以来，该inode没有改变过。(3) inode处于活动使
	用状态。其数据内容已经改变，与存储介质上的内容不同。这种状态的inode被称作脏的。在
	fs/inode.c中内核定义了两个全局变量用作表头，inode_unused用于有效但非活动的inode（
	第1类），inode_in_use用于所有使用但未改变的inode（第2类） 。脏的inode（第3类）保
	存在一个特定于超级块的链表中。第4种可能性出现得不那么频繁，一般是与一个超级块相关
	的所有inode都无效时。在检测到可移动设备的介质改变时，此前使用的inode就都没有意义了
	，另外文件系统重新装载时也会发生这种情况。在所有情况下，代码都结束于
	invalidate_inodes函数中，无效inode保存在一个本地链表中，与VFS代码再没有关系了。
	如果一个inode是脏的，即其内容已经被修改，则列入脏链表，表头为super_block->s_dirty
	，链表元素是i_list。这样做有下列好处：在写回数据时（数据回写通常也称之为同步）不
	需要扫描系统所有的inode，考虑脏链表上所有的inode就足够了。另外两个链表（表头为
	super_block->s_io和super_block->s_more_io）使用同样的链表元素i_list。这两个链表
	包含的是已经选中向磁盘回写的inode，但正在等待回写进行。 */
	struct list_head	i_list;
	/*inode还通过一个特定于超级块的链表维护，表头是super_block->s_inodes。i_sb_list用
	作链表元素。但超级块管理了更多的inode链表，与i_sb_list所在的链表是独立的*/
	struct list_head	i_sb_list;
	/**/
	struct list_head	i_dentry;
	/*每个VFS inode（对给定的文件系统）都由一个唯一的编号标识，保存在i_ino中*/
	unsigned long i_ino;
	/*计数器指定访问该inode结构的进程数目*/
	atomic_t i_count;
	/*计数器记录使用该inode的硬链接总数*/
	unsigned int i_nlink;
	/*用户id*/
	uid_t i_uid;
	/*组id*/
	gid_t i_gid;
	/*在inode表示设备文件时，需要i_rdev表示与哪个设备进行通信。注意！i_rdev只是一
	个数字！不是数据结构！但这个数字包含的信息，足以找到有关目标设备、对块设备，最
	终会找到struct block_device的一个实例. 已知i_rdev中的设备标识符，我们就可使用
	辅助函数bdget构建一个block_device的实例*/
	dev_t i_rdev;
	/*版本信息*/
	unsigned long i_version;
	/*文件大小*/
	loff_t			i_size;
#ifdef __NEED_I_SIZE_ORDERED
	/**/
	seqcount_t		i_size_seqcount;
#endif
	/*最后访问inode的时间*/
	struct timespec i_atime;
	/*最后修改inode数据段的时间*/
	struct timespec	i_mtime;
	/*最后修改inode属性的时间*/
	struct timespec	i_ctime;
	/*按字节位表示的块大小，1024对应10*/
	unsigned int i_blkbits;
	/*文件占用的块数目*/
	blkcnt_t i_blocks;
	/*文件长度*/
	unsigned short i_bytes;
	/*inode类型及权限*/
	umode_t			i_mode;
	/*文件保护锁*/
	spinlock_t		i_lock;	/* i_blocks, i_bytes, maybe i_size */
	/**/
	struct mutex		i_mutex;
	/*文件读写信号量*/
	struct rw_semaphore	i_alloc_sem;
	/*内核提供了大量函数，对inode进行操作。为此定义了一个函数指针的集合，以抽象这些操
	作，因为实际数据是通过具体文件系统的实现操作的。调用接口总是保持不变，但实际工作
	是由特定于实现的函数完成的。inode结构有两个指针（i_op和i_fop），指向实现了上述抽
	象的数组。一个数组与特定于inode的操作有关，另一个数组则提供了文件操作*/
	/*inode_operations负责管理结构性的操作（如删除文件）和文件相关的元数据（如属性）*/
	const struct inode_operations	*i_op;
	/*用于操作文件中包含的数据*/
	const struct file_operations	*i_fop;
	/*inode所属超级块指针*/
	struct super_block	*i_sb;
	/**/
	struct file_lock	*i_flock;
	/*地址空间指针*/
	struct address_space	*i_mapping;
	/**/
	struct address_space	i_data;
#ifdef CONFIG_QUOTA
	/**/
	struct dquot		*i_dquot[MAXQUOTAS];
#endif
	/*i_devices也与设备文件的处理有关联：利用该成员作为链表元素，使得块设备或字符设备
	可以维护一个inode的链表，每个inode表示一个设备文件，通过设备文件可以访问对应的设
	备。尽管在很多情况下每个设备一个设备文件就足够了，但还有很多种可能性。如chroot造
	成的环境，其中一个给定的块设备或字符设备可以通过多个设备文件，因而需要多个inode*/
	struct list_head	i_devices;
	/*如果inode表示设备特殊文件，该匿名联合就包含了指向设备专用数据结构的指针。i_bdev
	用于块设备，i_pipe包含了用于实现管道的inode的相关信息，而i_cdev用于字符设备。由于
	一个inode一次只能表示一种类型的设备，所以将i_pipe、i_bdev和i_cdev放置在联合中是安
	全的*/
	union
	{
		/*用于实现管道的inode的相关信息*/
		struct pipe_inode_info	*i_pipe;
		/*块设备*/
		struct block_device	*i_bdev;
		/*字符设备*/
		struct cdev		*i_cdev;
	};
	/**/
	int			i_cindex;
	/**/
	__u32			i_generation;

#ifdef CONFIG_DNOTIFY
	/**/
	unsigned long		i_dnotify_mask; /* Directory notify events */
	/**/
	struct dnotify_struct	*i_dnotify; /* for directory notifications */
#endif

#ifdef CONFIG_INOTIFY
	/**/
	struct list_head	inotify_watches; /* watches on this inode */
	/**/
	struct mutex		inotify_mutex;	/* protects the watches list */
#endif
	/*inode状态*/
	unsigned long		i_state;
	/*第一次变脏时的jiffies值*/
	unsigned long		dirtied_when;	/* jiffies of first dirtying */
	/**/
	unsigned int		i_flags;
	/**/
	atomic_t		i_writecount;
#ifdef CONFIG_SECURITY
	/*LSM相关*/
	void			*i_security;
#endif
	/*文件系统或设备私有数据指针*/
	void			*i_private; /* fs or device private pointer */
};

/*
 * inode->i_mutex nesting subclasses for the lock validator:
 *
 * 0: the object of the current VFS operation
 * 1: parent
 * 2: child/target
 * 3: quota file
 *
 * The locking order between these classes is
 * parent -> child -> normal -> xattr -> quota
 */
enum inode_i_mutex_lock_class
{
	I_MUTEX_NORMAL,
	I_MUTEX_PARENT,
	I_MUTEX_CHILD,
	I_MUTEX_XATTR,
	I_MUTEX_QUOTA
};

extern void inode_double_lock(struct inode *inode1, struct inode *inode2);
extern void inode_double_unlock(struct inode *inode1, struct inode *inode2);

/*
 * NOTE: in a 32bit arch with a preemptable kernel and
 * an UP compile the i_size_read/write must be atomic
 * with respect to the local cpu (unlike with preempt disabled),
 * but they don't need to be atomic with respect to other cpus like in
 * true SMP (so they need either to either locally disable irq around
 * the read or for example on x86 they can be still implemented as a
 * cmpxchg8b without the need of the lock prefix). For SMP compiles
 * and 64bit archs it makes no difference if preempt is enabled or not.
 */
static inline loff_t i_size_read(const struct inode *inode)
{
#if BITS_PER_LONG==32 && defined(CONFIG_SMP)
	loff_t i_size;
	unsigned int seq;

	do {
		seq = read_seqcount_begin(&inode->i_size_seqcount);
		i_size = inode->i_size;
	} while (read_seqcount_retry(&inode->i_size_seqcount, seq));
	return i_size;
#elif BITS_PER_LONG==32 && defined(CONFIG_PREEMPT)
	loff_t i_size;

	preempt_disable();
	i_size = inode->i_size;
	preempt_enable();
	return i_size;
#else
	return inode->i_size;
#endif
}

/*
 * NOTE: unlike i_size_read(), i_size_write() does need locking around it
 * (normally i_mutex), otherwise on 32bit/SMP an update of i_size_seqcount
 * can be lost, resulting in subsequent i_size_read() calls spinning forever.
 */
static inline void i_size_write(struct inode *inode, loff_t i_size)
{
#if BITS_PER_LONG==32 && defined(CONFIG_SMP)
	write_seqcount_begin(&inode->i_size_seqcount);
	inode->i_size = i_size;
	write_seqcount_end(&inode->i_size_seqcount);
#elif BITS_PER_LONG==32 && defined(CONFIG_PREEMPT)
	preempt_disable();
	inode->i_size = i_size;
	preempt_enable();
#else
	inode->i_size = i_size;
#endif
}

static inline unsigned iminor(const struct inode *inode)
{
	return MINOR(inode->i_rdev);
}

static inline unsigned imajor(const struct inode *inode)
{
	return MAJOR(inode->i_rdev);
}

extern struct block_device *I_BDEV(struct inode *inode);

struct fown_struct {
	rwlock_t lock;          /* protects pid, uid, euid fields */
	struct pid *pid;	/* pid or -pgrp where SIGIO should be sent */
	enum pid_type pid_type;	/* Kind of process group SIGIO should be sent to */
	uid_t uid, euid;	/* uid/euid of process setting the owner */
	int signum;		/* posix.1b rt signal to be delivered on IO */
};

/*描述文件预读信息的结构体
 * Track a single file's readahead state
 */

struct file_ra_state
{
	/*预读的起始位置*/
	pgoff_t start;
	/*预读的页数，-1表示临时禁止预读*/
	unsigned int size;
	/*阈值，在读取方向上剩余页数为该值时，启动异步预读*/
	unsigned int async_size;
	/*文件允许的最大预读页数*/
	unsigned int ra_pages;		/* Maximum readahead window */
	/*Cache miss stat for mmap accesses*/
	int mmap_miss;
	/*缓存上次read()的位置*/
	loff_t prev_pos;
};

/*
 * Check if @index falls in the readahead windows.
 */
static inline int ra_has_index(struct file_ra_state *ra, pgoff_t index)
{
	return (index >= ra->start &&
		index <  ra->start + ra->size);
}

struct file
{
	/*
	 * fu_list becomes invalid after file_free is called and queued via
	 * fu_rcuhead for RCU freeing
	 */
	union
	{
		/*每个超级块都提供了一个s_list成员用作表头，以建立file对象的链表，链表元素
		是file->fu_list。该链表包含该超级块表示的文件爱你系统所打开的文件。例如，在
		以读/写模式装载的文件系统以只读模式重新装载时，会扫描该链表。当然，如果仍然
		有按写模式打开的文件，时无法重新装载的。因而，内核需要检查该链表来确认*/
		struct list_head	fu_list;
		/**/
		struct rcu_head 	fu_rcuhead;
	} f_u;
	/**/
	struct path		f_path;
/*文件名与inode之间的关联*/
#define f_dentry	f_path.dentry
/*文件所在文件系统的有关信息*/
#define f_vfsmnt	f_path.mnt
	/*操作文件内容的函数集合*/
	const struct file_operations	*f_op;
	/*文件的引用计数*/
	atomic_t		f_count;
	/*指定了在open系统调用是传递的额外标识*/
	unsigned int 		f_flags;
	/*打开文件时传递的模式参数（通常指定读、写或读写访问模式）*/
	mode_t			f_mode;
	/*文件位置指针的当前值（对于顺序读取操作或读取文件特定部分的操作，都很重要），
	表示与文件起始处的字节偏移*/
	loff_t			f_pos;
	/*包含处理该文件的进程有关信息（因而页确定了SIGIO信号发送的目标PID，以实现
	异步输入输出）*/
	struct fown_struct	f_owner;
	/*文件的UID和GID*/
	unsigned int f_uid, f_gid;
	/*预读特征信息。这些值指定了实际请求文件数据之前，是否预读文件数据，如何预读
	以提高系统性能*/
	struct file_ra_state	f_ra;
	/*文件系统使用，以检查一个file实例是否仍然与相关的inode内容兼容，这对于确保
	已缓存对象的一致性很重要*/
	u64	 f_version;
#ifdef CONFIG_SECURITY
	void *f_security;
#endif
	/*终端设备驱动程序或可能其它需要的数据*/
	void *private_data;

#ifdef CONFIG_EPOLL
	/* Used by fs/eventpoll.c to link all the hooks to this file */
	struct list_head	f_ep_links;
	spinlock_t		f_ep_lock;
#endif /* #ifdef CONFIG_EPOLL */
	/*指向属于文件相关的inode实例的地址空间映射。通常它设置为inode->i_mapping，
	但文件系统或其它内核子系统可能会修改它*/
	struct address_space	*f_mapping;
};

extern spinlock_t files_lock;
#define file_list_lock() spin_lock(&files_lock);
#define file_list_unlock() spin_unlock(&files_lock);

#define get_file(x)	atomic_inc(&(x)->f_count)
#define file_count(x)	atomic_read(&(x)->f_count)

#define	MAX_NON_LFS	((1UL<<31) - 1)

/* Page cache limit. The filesystems should put that into their s_maxbytes
   limits, otherwise bad things can happen in VM. */
#if BITS_PER_LONG==32
#define MAX_LFS_FILESIZE	(((u64)PAGE_CACHE_SIZE << (BITS_PER_LONG-1))-1)
#elif BITS_PER_LONG==64
#define MAX_LFS_FILESIZE 	0x7fffffffffffffffUL
#endif

#define FL_POSIX	1
#define FL_FLOCK	2
#define FL_ACCESS	8	/* not trying to lock, just looking */
#define FL_EXISTS	16	/* when unlocking, test for existence */
#define FL_LEASE	32	/* lease held on this file */
#define FL_CLOSE	64	/* unlock on close */
#define FL_SLEEP	128	/* A blocking lock */

/*
 * The POSIX file lock owner is determined by
 * the "struct files_struct" in the thread group
 * (or NULL for no owner - BSD locks).
 *
 * Lockd stuffs a "host" pointer into this.
 */
typedef struct files_struct *fl_owner_t;

struct file_lock_operations
{
	void (*fl_insert)(struct file_lock *);	/* lock insertion callback */
	void (*fl_remove)(struct file_lock *);	/* lock removal callback */
	void (*fl_copy_lock)(struct file_lock *, struct file_lock *);
	void (*fl_release_private)(struct file_lock *);
};

/**/
struct lock_manager_operations
{
	int (*fl_compare_owner)(struct file_lock *, struct file_lock *);
	void (*fl_notify)(struct file_lock *);	/* unblock callback */
	int (*fl_grant)(struct file_lock *, struct file_lock *, int);
	void (*fl_copy_lock)(struct file_lock *, struct file_lock *);
	void (*fl_release_private)(struct file_lock *);
	void (*fl_break)(struct file_lock *);
	int (*fl_mylease)(struct file_lock *, struct file_lock *);
	int (*fl_change)(struct file_lock **, int);
};

/* that will die - we need it for nfs_lock_info */
#include <linux/nfs_fs_i.h>

/**/
struct file_lock
{
	struct file_lock *fl_next;	/* singly linked list for this inode  */
	struct list_head fl_link;	/* doubly linked list of all locks */
	struct list_head fl_block;	/* circular list of blocked processes */
	fl_owner_t fl_owner;
	unsigned int fl_pid;
	wait_queue_head_t fl_wait;
	struct file *fl_file;
	unsigned char fl_flags;
	unsigned char fl_type;
	loff_t fl_start;
	loff_t fl_end;

	struct fasync_struct *	fl_fasync; /* for lease break notifications */
	unsigned long fl_break_time;	/* for nonblocking lease breaks */

	struct file_lock_operations *fl_ops;	/* Callbacks for filesystems */
	struct lock_manager_operations *fl_lmops;	/* Callbacks for lockmanagers */
	union {
		struct nfs_lock_info	nfs_fl;
		struct nfs4_lock_info	nfs4_fl;
		struct {
			struct list_head link;	/* link in AFS vnode's pending_locks list */
			int state;		/* state of grant or error if -ve */
		} afs;
	} fl_u;
};

/* The following constant reflects the upper bound of the file/locking space */
#ifndef OFFSET_MAX
#define INT_LIMIT(x)	(~((x)1 << (sizeof(x)*8 - 1)))
#define OFFSET_MAX	INT_LIMIT(loff_t)
#define OFFT_OFFSET_MAX	INT_LIMIT(off_t)
#endif

#include <linux/fcntl.h>

extern int fcntl_getlk(struct file *, struct flock __user *);
extern int fcntl_setlk(unsigned int, struct file *, unsigned int,
			struct flock __user *);

#if BITS_PER_LONG == 32
extern int fcntl_getlk64(struct file *, struct flock64 __user *);
extern int fcntl_setlk64(unsigned int, struct file *, unsigned int,
			struct flock64 __user *);
#endif

extern void send_sigio(struct fown_struct *fown, int fd, int band);
extern int fcntl_setlease(unsigned int fd, struct file *filp, long arg);
extern int fcntl_getlease(struct file *filp);

/* fs/sync.c */
extern int do_sync_mapping_range(struct address_space *mapping, loff_t offset,
			loff_t endbyte, unsigned int flags);

/* fs/locks.c */
extern void locks_init_lock(struct file_lock *);
extern void locks_copy_lock(struct file_lock *, struct file_lock *);
extern void locks_remove_posix(struct file *, fl_owner_t);
extern void locks_remove_flock(struct file *);
extern void posix_test_lock(struct file *, struct file_lock *);
extern int posix_lock_file(struct file *, struct file_lock *, struct file_lock *);
extern int posix_lock_file_wait(struct file *, struct file_lock *);
extern int posix_unblock_lock(struct file *, struct file_lock *);
extern int vfs_test_lock(struct file *, struct file_lock *);
extern int vfs_lock_file(struct file *, unsigned int, struct file_lock *, struct file_lock *);
extern int vfs_cancel_lock(struct file *filp, struct file_lock *fl);
extern int flock_lock_file_wait(struct file *filp, struct file_lock *fl);
extern int __break_lease(struct inode *inode, unsigned int flags);
extern void lease_get_mtime(struct inode *, struct timespec *time);
extern int generic_setlease(struct file *, long, struct file_lock **);
extern int vfs_setlease(struct file *, long, struct file_lock **);
extern int lease_modify(struct file_lock **, int);
extern int lock_may_read(struct inode *, loff_t start, unsigned long count);
extern int lock_may_write(struct inode *, loff_t start, unsigned long count);
extern struct seq_operations locks_seq_operations;

struct fasync_struct {
	int	magic;
	int	fa_fd;
	struct	fasync_struct	*fa_next; /* singly linked list */
	struct	file 		*fa_file;
};

#define FASYNC_MAGIC 0x4601

/* SMP safe fasync helpers: */
extern int fasync_helper(int, struct file *, int, struct fasync_struct **);
/* can be called from interrupts */
extern void kill_fasync(struct fasync_struct **, int, int);
/* only for net: no internal synchronization */
extern void __kill_fasync(struct fasync_struct *, int, int);

extern int __f_setown(struct file *filp, struct pid *, enum pid_type, int force);
extern int f_setown(struct file *filp, unsigned long arg, int force);
extern void f_delown(struct file *filp);
extern pid_t f_getown(struct file *filp);
extern int send_sigurg(struct fown_struct *fown);

/*
 *	Umount options
 */

#define MNT_FORCE	0x00000001	/* Attempt to forcibily umount */
#define MNT_DETACH	0x00000002	/* Just detach from the tree */
#define MNT_EXPIRE	0x00000004	/* Mark for expiry */

extern struct list_head super_blocks;
extern spinlock_t sb_lock;

#define sb_entry(list)	list_entry((list), struct super_block, s_list)
#define S_BIAS (1<<30)

/*在装载新的文件系统时，vfsmount并不是唯一需要在内存中创建的结构。装载操作开始于超
级块的读取。file_system_type对象中保存的read_super函数指针返回一个类型为super_block
的对象，用于在内存中表示一个超级块。它是借助于底层实现产生的。*/
struct super_block
{
	/*结构的第一个成员s_list是一个链表元素，用于将系统中所有的超级块聚集到一个链表中
	。该链表的表头是全程变量super_blocks，定义在fs/super.c中*/
	struct list_head	s_list;
	/*查找索引，而非kdev_t。s_dev项总是一个数字（即使对于不需要块设备的虚拟文件系统
	，也是如此）*/
	dev_t	s_dev;
	/*s_blocksize和s_blocksize_bits指定了文件系统的块长度（这对于硬盘上的数据组织特
	别有用）。本质上，这两个变量以不同的方式表示了相同信息。s_blocksize的单位是字节，
	而s_blocksize_bits则是对前一个值取以2为底的对数。标准的Ext2文件系统使用的块长度
	为1024字节，因此s_blocksize保存的值是1024，而s_blocksize_bits为10（2^10=1024）*/
	unsigned long		s_blocksize;
	unsigned char		s_blocksize_bits;
	/*一个简单的整型变量。如果以任何方式改变了超级块，需要向磁盘回写，都会将s_dirt设
	置为1。否则，其值为0*/
	unsigned char		s_dirt;
	/*s_maxbytes保存了文件系统可以处理的最大文件长度，因实现而异*/
	unsigned long long	s_maxbytes;
	/*s_type指向file_system_type实例，其中保存了与文件系统有关的一般类型的信息*/
	struct file_system_type	*s_type;
	/*s_op指向一个包含了函数指针的结构，该结构按熟悉的VFS方式，提供了一个一般性的接
	口，用于处理超级块相关操作。操作的实现必须由底层文件系统的代码提供*/
	const struct super_operations	*s_op;
	/**/
	struct dquot_operations	*dq_op;
 	/**/
	struct quotactl_ops	*s_qcop;
	/**/
	const struct export_operations *s_export_op;
	/**/
	unsigned long		s_flags;
	/**/
	unsigned long		s_magic;
	/*s_root将超级块与全局根目录的dentry项关联起来。只有通常可见的文件系统的超级块，
	才指向/（根）目录的dentry实例。具有特殊功能、不出现在通常的目录层次结构中的文件
	系统（例如，管道或套接字文件系统），指向专门的项，不能通过普通的文件命令访问。处
	理文件系统对象的代码经常需要检查文件系统是否已经装载，而s_root可用于该目的。如果
	它为NULL，则该文件系统是一个伪文件系统，只在内核内部可见。否则，该文件系统在用户
	空间中是可见的*/
	struct dentry		*s_root;
	/**/
	struct rw_semaphore	s_umount;
	/**/
	struct mutex		s_lock;
	/**/
	int			s_count;
	/**/
	int			s_syncing;
	/**/
	int			s_need_sync_fs;
	/**/
	atomic_t		s_active;
#ifdef CONFIG_SECURITY
	/**/
	void                    *s_security;
#endif
	/*一个指向结构的指针，该结构包含了一些用于处理扩展属性的函数指针*/
	struct xattr_handler	**s_xattr;
	/**/
	struct list_head	s_inodes;	/* all inodes */
	/*s_dirty是一个表头，用于脏inode的链表，在同步内存内容与底层存储介质上的数据时，
	使用该链表会更加高效。该链表只包含已经修改的inode，因此回写数据时并不需要扫描全
	部inode。该字段不能与s_dirt混淆，后者不是表头，而是一个简单的整型变量。如果以任
	何方式改变了超级块，需要向磁盘回写，都会将s_dirt设置为1。否则，其值为0。 */
	struct list_head	s_dirty;	/* dirty inodes */
	/*两个链表（表头为s_io和s_more_io）使用同样的链表元素inode->i_list包含的是已经选
	中向磁盘回写的inode，但正在等待回写进行*/
	struct list_head	s_io;
	struct list_head	s_more_io;
	/*anonymous dentries for (nfs) exporting*/
	struct hlist_head	s_anon;
	/*s_files链表包含了一系列file结构，列出了该超级块表示的文件系统上所有打开的文件。
	内核在卸载文件系统时将参考该链表。如果其中仍然包含为写入而打开的文件，则文件系统
	仍然处于使用中，卸载操作失败，并将返回适当的错误信息*/
	struct list_head	s_files;
	/*s_dev和s_bdev指定了底层文件系统的数据所在的块设备。前者使用了内核内部的编号，
	而后者是一个指向内存中的block_device结构的指针，该结构用于更详细地定义设备操作
	和功能。s_dev项总是一个数字（即使对于不需要块设备的虚拟文件系统，也是如此）。与
	此相反， s_bdev可以为NULL指针*/
	struct block_device	*s_bdev;
	/**/
	struct mtd_info		*s_mtd;
	/*各个超级块都连接到另一个链表中，表示同一类型文件系统的所有超级块实例，这里不考
	虑底层的块设备，但链表中的超级块的文件系统类型都是相同的。表头是file_system_type
	结构的fs_supers成员，s_instances用作链表元素*/
	struct list_head	s_instances;
	/**/
	struct quota_info	s_dquot;	/* Diskquota specific options */
	/**/
	int s_frozen;
	/**/
	wait_queue_head_t	s_wait_unfrozen;
	/**/
	char s_id[32];				/* Informational name */
	/*s_fs_info是一个指向文件系统实现的私有数据的指针，VFS不操作该数据*/
	void *s_fs_info;

	/*
	 * The next field is for VFS *only*. No filesystems have any business
	 * even looking at it. You had been warned.
	 */
	/**/
	struct mutex s_vfs_rename_mutex;	/* Kludge */
	/*指定了文件系统支持的各种时间戳的最大可能的粒度。该值对所有时间戳都是相同的，
	单位为ns，即1秒的10^9分之一*/
	u32		   s_time_gran;

	/*
	 * Filesystem subtype.  If non-empty the filesystem type field
	 * in /proc/mounts will be "type.subtype"
	 */
	/**/
	char *s_subtype;
};

extern struct timespec current_fs_time(struct super_block *sb);

/*
 * Snapshotting support.
 */
enum {
	SB_UNFROZEN = 0,
	SB_FREEZE_WRITE	= 1,
	SB_FREEZE_TRANS = 2,
};

#define vfs_check_frozen(sb, level) \
	wait_event((sb)->s_wait_unfrozen, ((sb)->s_frozen < (level)))

#define get_fs_excl() atomic_inc(&current->fs_excl)
#define put_fs_excl() atomic_dec(&current->fs_excl)
#define has_fs_excl() atomic_read(&current->fs_excl)

#define is_owner_or_cap(inode)	\
	((current->fsuid == (inode)->i_uid) || capable(CAP_FOWNER))

/* not quite ready to be deprecated, but... */
extern void lock_super(struct super_block *);
extern void unlock_super(struct super_block *);

/*
 * VFS helper functions..
 */
extern int vfs_permission(struct nameidata *, int);
extern int vfs_create(struct inode *, struct dentry *, int, struct nameidata *);
extern int vfs_mkdir(struct inode *, struct dentry *, int);
extern int vfs_mknod(struct inode *, struct dentry *, int, dev_t);
extern int vfs_symlink(struct inode *, struct dentry *, const char *, int);
extern int vfs_link(struct dentry *, struct inode *, struct dentry *);
extern int vfs_rmdir(struct inode *, struct dentry *);
extern int vfs_unlink(struct inode *, struct dentry *);
extern int vfs_rename(struct inode *, struct dentry *, struct inode *, struct dentry *);

/*
 * VFS dentry helper functions.
 */
extern void dentry_unhash(struct dentry *dentry);

/*
 * VFS file helper functions.
 */
extern int file_permission(struct file *, int);

/*
 * File types
 *
 * NOTE! These match bits 12..15 of stat.st_mode
 * (ie "(i_mode >> 12) & 15").
 */
#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14

#define OSYNC_METADATA	(1<<0)
#define OSYNC_DATA	(1<<1)
#define OSYNC_INODE	(1<<2)
int generic_osync_inode(struct inode *, struct address_space *, int);

/*
 * This is the "filldir" function type, used by readdir() to let
 * the kernel specify what kind of dirent layout it wants to have.
 * This allows the kernel to read directories into kernel space or
 * to have different dirent layouts depending on the binary type.
 */
typedef int (*filldir_t)(void *, const char *, int, loff_t, u64, unsigned);

struct block_device_operations {
	int (*open) (struct inode *, struct file *);
	int (*release) (struct inode *, struct file *);
	int (*ioctl) (struct inode *, struct file *, unsigned, unsigned long);
	long (*unlocked_ioctl) (struct file *, unsigned, unsigned long);
	long (*compat_ioctl) (struct file *, unsigned, unsigned long);
	int (*direct_access) (struct block_device *, sector_t, unsigned long *);
	int (*media_changed) (struct gendisk *);
	int (*revalidate_disk) (struct gendisk *);
	int (*getgeo)(struct block_device *, struct hd_geometry *);
	struct module *owner;
};

/*
 * "descriptor" for what we're up to with a read.
 * This allows us to use the same read code yet
 * have multiple different users of the data that
 * we read from a file.
 *
 * The simplest case just copies the data to user
 * mode.
 */
typedef struct {
	size_t written;
	size_t count;
	union {
		char __user * buf;
		void *data;
	} arg;
	int error;
} read_descriptor_t;

typedef int (*read_actor_t)(read_descriptor_t *, struct page *, unsigned long, unsigned long);

/* These macros are for out of kernel modules to test that
 * the kernel supports the unlocked_ioctl and compat_ioctl
 * fields in struct file_operations. */
#define HAVE_COMPAT_IOCTL 1
#define HAVE_UNLOCKED_IOCTL 1

/*
 * NOTE:
 * read, write, poll, fsync, readv, writev, unlocked_ioctl and compat_ioctl
 * can be called without the big kernel lock held in all filesystems.
 */
/**/
struct file_operations
{
	/**/
	struct module *owner;
	/**/
	loff_t (*llseek) (struct file *, loff_t, int);
	/**/
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	/**/
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	/**/
	ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
	/**/
	ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
	/**/
	int (*readdir) (struct file *, void *, filldir_t);
	/**/
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	/**/
	int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
	/**/
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	/**/
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	/**/
	int (*mmap) (struct file *, struct vm_area_struct *);
	/**/
	int (*open) (struct inode *, struct file *);
	/**/
	int (*flush) (struct file *, fl_owner_t id);
	/**/
	int (*release) (struct inode *, struct file *);
	/**/
	int (*fsync) (struct file *, struct dentry *, int datasync);
	/**/
	int (*aio_fsync) (struct kiocb *, int datasync);
	/**/
	int (*fasync) (int, struct file *, int);
	/**/
	int (*lock) (struct file *, int, struct file_lock *);
	/**/
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	/**/
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long,
			unsigned long, unsigned long);
	/**/
	int (*check_flags)(int);
	/**/
	int (*dir_notify)(struct file *filp, unsigned long arg);
	/**/
	int (*flock) (struct file *, int, struct file_lock *);
	/**/
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *,
			size_t, unsigned int);
	/**/
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *,
			size_t, unsigned int);
	/**/
	int (*setlease)(struct file *, long, struct file_lock **);
};

/**/
struct inode_operations
{
	/**/
	int (*create) (struct inode *,struct dentry *,int, struct nameidata *);
	/*p根据文件系统对象的名称（表示为字符串）查找其inode实例*/
	struct dentry * (*lookup) (struct inode *,struct dentry *, struct nameidata *);
	/**/
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	/*用于删除文件。但根据上文的描述，如果硬链接的引用计数器表明该inode仍然被多
	个文件使用，则不会执行删除操作*/
	int (*unlink) (struct inode *,struct dentry *);
	/**/
	int (*symlink) (struct inode *,struct dentry *,const char *);
	/**/
	int (*mkdir) (struct inode *,struct dentry *,int);
	/**/
	int (*rmdir) (struct inode *,struct dentry *);
	/**/
	int (*mknod) (struct inode *,struct dentry *,int,dev_t);
	/**/
	int (*rename) (struct inode *, struct dentry *,	struct inode *, struct dentry *);
	/**/
	int (*readlink) (struct dentry *, char __user *,int);
	/*根据符号链接查找目标文件的inode。因为符号链接可能是跨文件系统边界的，该例程的
	实现通常非常短，实际工作很快委托给一般的VFS例程完成。*/
	void * (*follow_link) (struct dentry *, struct nameidata *);
	/**/
	void (*put_link) (struct dentry *, struct nameidata *, void *);
	/*修改指定inode的长度。该函数只接受一个参数，即所处理的inode的数据结构。在调用
	该函数之前，必须将新的文件长度手工设置到inode结构的i_size成员。*/
	void (*truncate) (struct inode *);
	/**/
	int (*permission) (struct inode *, int, struct nameidata *);
	/**/
	int (*setattr) (struct dentry *, struct iattr *);
	/**/
	int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
	/*建立经典的UNIX模型不支持这些属性。例如，可使用这些属性实现访问控制表（access
	control list，简称ACL）*/
	int (*setxattr) (struct dentry *, const char *,const void *,size_t,int);
	/*读取文件的扩展属性*/
	ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
	/*显示文件的扩展属性*/
	ssize_t (*listxattr) (struct dentry *, char *, size_t);
	/*删除文件的扩展属性*/
	int (*removexattr) (struct dentry *, const char *);
	/*truncate_range用于截断一个范围内的块（即，在文件中穿孔），但该操作当前只有共
	享内存文件系统支持*/
	void (*truncate_range)(struct inode *, loff_t, loff_t);
	/*用于对文件预先分配空间，在一些情况下可以提高性能。但只有很新的文件系统 （如
	Reiserfs或Ext4）才支持该操作。*/
	long (*fallocate)(struct inode *inode, int mode, loff_t offset, loff_t len);
};

struct seq_file;

ssize_t rw_copy_check_uvector(int type, const struct iovec __user * uvector,
				unsigned long nr_segs, unsigned long fast_segs,
				struct iovec *fast_pointer,
				struct iovec **ret_pointer);

extern ssize_t vfs_read(struct file *, char __user *, size_t, loff_t *);
extern ssize_t vfs_write(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t vfs_readv(struct file *, const struct iovec __user *,
		unsigned long, loff_t *);
extern ssize_t vfs_writev(struct file *, const struct iovec __user *,
		unsigned long, loff_t *);

/*
 * NOTE: write_inode, delete_inode, clear_inode, put_inode can be called
 * without the big kernel lock held in all filesystems.
 */
/**/
struct super_operations
{
	/**/
   	struct inode *(*alloc_inode)(struct super_block *sb);
	/**/
	void (*destroy_inode)(struct inode *);
	/**/
	void (*read_inode) (struct inode *);
	/**/
   	void (*dirty_inode) (struct inode *);
	/**/
	int (*write_inode) (struct inode *, int);
	/**/
	void (*put_inode) (struct inode *);
	/**/
	void (*drop_inode) (struct inode *);
	/**/
	void (*delete_inode) (struct inode *);
	/**/
	void (*put_super) (struct super_block *);
	/**/
	void (*write_super) (struct super_block *);
	/**/
	int (*sync_fs)(struct super_block *sb, int wait);
	/**/
	void (*write_super_lockfs) (struct super_block *);
	/**/
	void (*unlockfs) (struct super_block *);
	/**/
	int (*statfs) (struct dentry *, struct kstatfs *);
	/**/
	int (*remount_fs) (struct super_block *, int *, char *);
	/**/
	void (*clear_inode) (struct inode *);
	/**/
	void (*umount_begin) (struct vfsmount *, int);
	/**/
	int (*show_options)(struct seq_file *, struct vfsmount *);
	/**/
	int (*show_stats)(struct seq_file *, struct vfsmount *);
#ifdef CONFIG_QUOTA
	/**/
	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	/**/
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
#endif
};

/*
 * Inode state bits.  Protected by inode_lock.
 *
 * Three bits determine the dirty state of the inode, I_DIRTY_SYNC,
 * I_DIRTY_DATASYNC and I_DIRTY_PAGES.
 *
 * Four bits define the lifetime of an inode.  Initially, inodes are I_NEW,
 * until that flag is cleared.  I_WILL_FREE, I_FREEING and I_CLEAR are set at
 * various stages of removing an inode.
 *
 * Two bits are used for locking and completion notification, I_LOCK and I_SYNC.
 *
 * I_DIRTY_SYNC		Inode itself is dirty.
 * I_DIRTY_DATASYNC	Data-related inode changes pending
 * I_DIRTY_PAGES	Inode has dirty pages.  Inode itself may be clean.
 * I_NEW		get_new_inode() sets i_state to I_LOCK|I_NEW.  Both
 *			are cleared by unlock_new_inode(), called from iget().
 * I_WILL_FREE		Must be set when calling write_inode_now() if i_count
 *			is zero.  I_FREEING must be set when I_WILL_FREE is
 *			cleared.
 * I_FREEING		Set when inode is about to be freed but still has dirty
 *			pages or buffers attached or the inode itself is still
 *			dirty.
 * I_CLEAR		Set by clear_inode().  In this state the inode is clean
 *			and can be destroyed.
 *
 *			Inodes that are I_WILL_FREE, I_FREEING or I_CLEAR are
 *			prohibited for many purposes.  iget() must wait for
 *			the inode to be completely released, then create it
 *			anew.  Other functions will just ignore such inodes,
 *			if appropriate.  I_LOCK is used for waiting.
 *
 * I_LOCK		Serves as both a mutex and completion notification.
 *			New inodes set I_LOCK.  If two processes both create
 *			the same inode, one of them will release its inode and
 *			wait for I_LOCK to be released before returning.
 *			Inodes in I_WILL_FREE, I_FREEING or I_CLEAR state can
 *			also cause waiting on I_LOCK, without I_LOCK actually
 *			being set.  find_inode() uses this to prevent returning
 *			nearly-dead inodes.
 * I_SYNC		Similar to I_LOCK, but limited in scope to writeback
 *			of inode dirty data.  Having a seperate lock for this
 *			purpose reduces latency and prevents some filesystem-
 *			specific deadlocks.
 *
 * Q: Why does I_DIRTY_DATASYNC exist?  It appears as if it could be replaced
 *    by (I_DIRTY_SYNC|I_DIRTY_PAGES).
 * Q: What is the difference between I_WILL_FREE and I_FREEING?
 * Q: igrab() only checks on (I_FREEING|I_WILL_FREE).  Should it also check on
 *    I_CLEAR?  If not, why?
 */
#define I_DIRTY_SYNC		1
#define I_DIRTY_DATASYNC	2
#define I_DIRTY_PAGES		4
#define I_NEW			8
#define I_WILL_FREE		16
#define I_FREEING		32
#define I_CLEAR			64
#define __I_LOCK		7
#define I_LOCK			(1 << __I_LOCK)
#define __I_SYNC		8
#define I_SYNC			(1 << __I_SYNC)

#define I_DIRTY (I_DIRTY_SYNC | I_DIRTY_DATASYNC | I_DIRTY_PAGES)

extern void __mark_inode_dirty(struct inode *, int);
static inline void mark_inode_dirty(struct inode *inode)
{
	__mark_inode_dirty(inode, I_DIRTY);
}

static inline void mark_inode_dirty_sync(struct inode *inode)
{
	__mark_inode_dirty(inode, I_DIRTY_SYNC);
}

/**
 * inc_nlink - directly increment an inode's link count
 * @inode: inode
 *
 * This is a low-level filesystem helper to replace any
 * direct filesystem manipulation of i_nlink.  Currently,
 * it is only here for parity with dec_nlink().
 */
static inline void inc_nlink(struct inode *inode)
{
	inode->i_nlink++;
}

static inline void inode_inc_link_count(struct inode *inode)
{
	inc_nlink(inode);
	mark_inode_dirty(inode);
}

/**
 * drop_nlink - directly drop an inode's link count
 * @inode: inode
 *
 * This is a low-level filesystem helper to replace any
 * direct filesystem manipulation of i_nlink.  In cases
 * where we are attempting to track writes to the
 * filesystem, a decrement to zero means an imminent
 * write when the file is truncated and actually unlinked
 * on the filesystem.
 */
static inline void drop_nlink(struct inode *inode)
{
	inode->i_nlink--;
}

/**
 * clear_nlink - directly zero an inode's link count
 * @inode: inode
 *
 * This is a low-level filesystem helper to replace any
 * direct filesystem manipulation of i_nlink.  See
 * drop_nlink() for why we care about i_nlink hitting zero.
 */
static inline void clear_nlink(struct inode *inode)
{
	inode->i_nlink = 0;
}

static inline void inode_dec_link_count(struct inode *inode)
{
	drop_nlink(inode);
	mark_inode_dirty(inode);
}

extern void touch_atime(struct vfsmount *mnt, struct dentry *dentry);
static inline void file_accessed(struct file *file)
{
	if (!(file->f_flags & O_NOATIME))
		touch_atime(file->f_path.mnt, file->f_path.dentry);
}

int sync_inode(struct inode *inode, struct writeback_control *wbc);

/**/
struct file_system_type
{
	/*name保存了文件系统的名称，是一个字符串（包含了例如reiserfs、ext3等类似的值）*/
	const char *name;
	/*fs_flags是使用的标志，例如标明只读装载、禁止setuid/setgid操作或进行其他的微调*/
	int fs_flags;
	/*用于从底层存储介质读取超级块的函数（其地址保存在get_sb）对装载过程也很重要。逻
	辑上，该函数依赖具体的文件系统，不能实现为抽象。而且该函数也不能保存在上述的
	super_operations结构中，因为超级块对象和指向该结构的指针都是在调用get_sb之后创建的*/
	int (*get_sb) (struct file_system_type *, int,	const char *, void *, struct vfsmount *);
	/*kill_super在不再需要某个文件系统类型时执行清理工作*/
	void (*kill_sb) (struct super_block *);
	/*owner是一个指向module结构的指针，仅当文件系统以模块形式加载时，owner才包含有意
	义的值（NULL指针表示文件系统已经持久编译到内核中）*/
	struct module *owner;
	/*各个可用的文件系统通过next成员连接起来，这里无法利用标准的链表功能，因为这是一
	个单链表*/
	struct file_system_type * next;
	/*对于每个已经装载的文件系统，在内存中都创建了一个超级块结构。该结构保存了文件系
	统它本身和装载点的有关信息。由于可以装载几个同一类型的文件系统（最好的例子是home
	和root分区，二者的文件系统类型通常相同），同一文件系统类型可能对应了多个超级块结
	构，这些超级块聚集在一个链表中。fs_supers是对应的表头*/
	struct list_head fs_supers;
	/**/
	struct lock_class_key s_lock_key;
	/**/
	struct lock_class_key s_umount_key;
	/**/
	struct lock_class_key i_lock_key;
	/**/
	struct lock_class_key i_mutex_key;
	/**/
	struct lock_class_key i_mutex_dir_key;
	/**/
	struct lock_class_key i_alloc_sem_key;
};

extern int get_sb_bdev(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data,
	int (*fill_super)(struct super_block *, void *, int),
	struct vfsmount *mnt);
extern int get_sb_single(struct file_system_type *fs_type,
	int flags, void *data,
	int (*fill_super)(struct super_block *, void *, int),
	struct vfsmount *mnt);
extern int get_sb_nodev(struct file_system_type *fs_type,
	int flags, void *data,
	int (*fill_super)(struct super_block *, void *, int),
	struct vfsmount *mnt);
void generic_shutdown_super(struct super_block *sb);
void kill_block_super(struct super_block *sb);
void kill_anon_super(struct super_block *sb);
void kill_litter_super(struct super_block *sb);
void deactivate_super(struct super_block *sb);
int set_anon_super(struct super_block *s, void *data);
struct super_block *sget(struct file_system_type *type,
			int (*test)(struct super_block *,void *),
			int (*set)(struct super_block *,void *),
			void *data);
extern int get_sb_pseudo(struct file_system_type *, char *,
	const struct super_operations *ops, unsigned long,
	struct vfsmount *mnt);
extern int simple_set_mnt(struct vfsmount *mnt, struct super_block *sb);
int __put_super(struct super_block *sb);
int __put_super_and_need_restart(struct super_block *sb);
void unnamed_dev_init(void);

/* Alas, no aliases. Too much hassle with bringing module.h everywhere */
#define fops_get(fops) \
	(((fops) && try_module_get((fops)->owner) ? (fops) : NULL))
#define fops_put(fops) \
	do { if (fops) module_put((fops)->owner); } while(0)

extern int register_filesystem(struct file_system_type *);
extern int unregister_filesystem(struct file_system_type *);
extern struct vfsmount *kern_mount_data(struct file_system_type *, void *data);
#define kern_mount(type) kern_mount_data(type, NULL)
extern int may_umount_tree(struct vfsmount *);
extern int may_umount(struct vfsmount *);
extern void umount_tree(struct vfsmount *, int, struct list_head *);
extern void release_mounts(struct list_head *);
extern long do_mount(char *, char *, char *, unsigned long, void *);
extern struct vfsmount *copy_tree(struct vfsmount *, struct dentry *, int);
extern void mnt_set_mountpoint(struct vfsmount *, struct dentry *,
				  struct vfsmount *);
extern struct vfsmount *collect_mounts(struct vfsmount *, struct dentry *);
extern void drop_collected_mounts(struct vfsmount *);

extern int vfs_statfs(struct dentry *, struct kstatfs *);

/* /sys/fs */
extern struct kset fs_subsys;

#define FLOCK_VERIFY_READ  1
#define FLOCK_VERIFY_WRITE 2

extern int locks_mandatory_locked(struct inode *);
extern int locks_mandatory_area(int, struct inode *, struct file *, loff_t, size_t);

/*
 * Candidates for mandatory locking have the setgid bit set
 * but no group execute bit -  an otherwise meaningless combination.
 */

static inline int __mandatory_lock(struct inode *ino)
{
	return (ino->i_mode & (S_ISGID | S_IXGRP)) == S_ISGID;
}

/*
 * ... and these candidates should be on MS_MANDLOCK mounted fs,
 * otherwise these will be advisory locks
 */

static inline int mandatory_lock(struct inode *ino)
{
	return IS_MANDLOCK(ino) && __mandatory_lock(ino);
}

static inline int locks_verify_locked(struct inode *inode)
{
	if (mandatory_lock(inode))
		return locks_mandatory_locked(inode);
	return 0;
}

extern int rw_verify_area(int, struct file *, loff_t *, size_t);

static inline int locks_verify_truncate(struct inode *inode,
				    struct file *filp,
				    loff_t size)
{
	if (inode->i_flock && mandatory_lock(inode))
		return locks_mandatory_area(
			FLOCK_VERIFY_WRITE, inode, filp,
			size < inode->i_size ? size : inode->i_size,
			(size < inode->i_size ? inode->i_size - size
			 : size - inode->i_size)
		);
	return 0;
}

static inline int break_lease(struct inode *inode, unsigned int mode)
{
	if (inode->i_flock)
		return __break_lease(inode, mode);
	return 0;
}

/* fs/open.c */

extern int do_truncate(struct dentry *, loff_t start, unsigned int time_attrs,
		       struct file *filp);
extern long do_sys_open(int dfd, const char __user *filename, int flags,
			int mode);
extern struct file *filp_open(const char *, int, int);
extern struct file * dentry_open(struct dentry *, struct vfsmount *, int);
extern int filp_close(struct file *, fl_owner_t id);
extern char * getname(const char __user *);

/* fs/dcache.c */
extern void __init vfs_caches_init_early(void);
extern void __init vfs_caches_init(unsigned long);

extern struct kmem_cache *names_cachep;

#define __getname()	kmem_cache_alloc(names_cachep, GFP_KERNEL)
#define __putname(name) kmem_cache_free(names_cachep, (void *)(name))
#ifndef CONFIG_AUDITSYSCALL
#define putname(name)   __putname(name)
#else
extern void putname(const char *name);
#endif

#ifdef CONFIG_BLOCK
extern int register_blkdev(unsigned int, const char *);
extern void unregister_blkdev(unsigned int, const char *);
extern struct block_device *bdget(dev_t);
extern void bd_set_size(struct block_device *, loff_t size);
extern void bd_forget(struct inode *inode);
extern void bdput(struct block_device *);
extern struct block_device *open_by_devnum(dev_t, unsigned);
extern const struct address_space_operations def_blk_aops;
#else
static inline void bd_forget(struct inode *inode) {}
#endif
extern const struct file_operations def_blk_fops;
extern const struct file_operations def_chr_fops;
extern const struct file_operations bad_sock_fops;
extern const struct file_operations def_fifo_fops;
#ifdef CONFIG_BLOCK
extern int ioctl_by_bdev(struct block_device *, unsigned, unsigned long);
extern int blkdev_ioctl(struct inode *, struct file *, unsigned, unsigned long);
extern int blkdev_driver_ioctl(struct inode *inode, struct file *file,
			       struct gendisk *disk, unsigned cmd,
			       unsigned long arg);
extern long compat_blkdev_ioctl(struct file *, unsigned, unsigned long);
extern int blkdev_get(struct block_device *, mode_t, unsigned);
extern int blkdev_put(struct block_device *);
extern int bd_claim(struct block_device *, void *);
extern void bd_release(struct block_device *);
#ifdef CONFIG_SYSFS
extern int bd_claim_by_disk(struct block_device *, void *, struct gendisk *);
extern void bd_release_from_disk(struct block_device *, struct gendisk *);
#else
#define bd_claim_by_disk(bdev, holder, disk)	bd_claim(bdev, holder)
#define bd_release_from_disk(bdev, disk)	bd_release(bdev)
#endif
#endif

/* fs/char_dev.c */
#define CHRDEV_MAJOR_HASH_SIZE	255
extern int alloc_chrdev_region(dev_t *, unsigned, unsigned, const char *);
extern int register_chrdev_region(dev_t, unsigned, const char *);
extern int register_chrdev(unsigned int, const char *,
			   const struct file_operations *);
extern void unregister_chrdev(unsigned int, const char *);
extern void unregister_chrdev_region(dev_t, unsigned);
extern int chrdev_open(struct inode *, struct file *);
extern void chrdev_show(struct seq_file *,off_t);

/* fs/block_dev.c */
#define BDEVNAME_SIZE	32	/* Largest string for a blockdev identifier */

#ifdef CONFIG_BLOCK
#define BLKDEV_MAJOR_HASH_SIZE	255
extern const char *__bdevname(dev_t, char *buffer);
extern const char *bdevname(struct block_device *bdev, char *buffer);
extern struct block_device *lookup_bdev(const char *);
extern struct block_device *open_bdev_excl(const char *, int, void *);
extern void close_bdev_excl(struct block_device *);
extern void blkdev_show(struct seq_file *,off_t);
#else
#define BLKDEV_MAJOR_HASH_SIZE	0
#endif

extern void init_special_inode(struct inode *, umode_t, dev_t);

/* Invalid inode operations -- fs/bad_inode.c */
extern void make_bad_inode(struct inode *);
extern int is_bad_inode(struct inode *);

extern const struct file_operations read_fifo_fops;
extern const struct file_operations write_fifo_fops;
extern const struct file_operations rdwr_fifo_fops;

extern int fs_may_remount_ro(struct super_block *);

#ifdef CONFIG_BLOCK
/*
 * return READ, READA, or WRITE
 */
#define bio_rw(bio)		((bio)->bi_rw & (RW_MASK | RWA_MASK))

/*
 * return data direction, READ or WRITE
 */
#define bio_data_dir(bio)	((bio)->bi_rw & 1)

extern int check_disk_change(struct block_device *);
extern int __invalidate_device(struct block_device *);
extern int invalidate_partition(struct gendisk *, int);
#endif
extern int invalidate_inodes(struct super_block *);
unsigned long __invalidate_mapping_pages(struct address_space *mapping,
					pgoff_t start, pgoff_t end,
					bool be_atomic);
unsigned long invalidate_mapping_pages(struct address_space *mapping,
					pgoff_t start, pgoff_t end);

static inline unsigned long __deprecated
invalidate_inode_pages(struct address_space *mapping)
{
	return invalidate_mapping_pages(mapping, 0, ~0UL);
}

static inline void invalidate_remote_inode(struct inode *inode)
{
	if (S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
	    S_ISLNK(inode->i_mode))
		invalidate_mapping_pages(inode->i_mapping, 0, -1);
}
extern int invalidate_inode_pages2(struct address_space *mapping);
extern int invalidate_inode_pages2_range(struct address_space *mapping,
					 pgoff_t start, pgoff_t end);
extern int write_inode_now(struct inode *, int);
extern int filemap_fdatawrite(struct address_space *);
extern int filemap_flush(struct address_space *);
extern int filemap_fdatawait(struct address_space *);
extern int filemap_write_and_wait(struct address_space *mapping);
extern int filemap_write_and_wait_range(struct address_space *mapping,
				        loff_t lstart, loff_t lend);
extern int wait_on_page_writeback_range(struct address_space *mapping,
				pgoff_t start, pgoff_t end);
extern int __filemap_fdatawrite_range(struct address_space *mapping,
				loff_t start, loff_t end, int sync_mode);

extern long do_fsync(struct file *file, int datasync);
extern void sync_supers(void);
extern void sync_filesystems(int wait);
extern void __fsync_super(struct super_block *sb);
extern void emergency_sync(void);
extern void emergency_remount(void);
extern int do_remount_sb(struct super_block *sb, int flags,
			 void *data, int force);
#ifdef CONFIG_BLOCK
extern sector_t bmap(struct inode *, sector_t);
#endif
extern int notify_change(struct dentry *, struct iattr *);
extern int permission(struct inode *, int, struct nameidata *);
extern int generic_permission(struct inode *, int,
		int (*check_acl)(struct inode *, int));

extern int get_write_access(struct inode *);
extern int deny_write_access(struct file *);
static inline void put_write_access(struct inode * inode)
{
	atomic_dec(&inode->i_writecount);
}
static inline void allow_write_access(struct file *file)
{
	if (file)
		atomic_inc(&file->f_path.dentry->d_inode->i_writecount);
}
extern int do_pipe(int *);
extern struct file *create_read_pipe(struct file *f);
extern struct file *create_write_pipe(void);
extern void free_write_pipe(struct file *);

extern int open_namei(int dfd, const char *, int, int, struct nameidata *);
extern int may_open(struct nameidata *, int, int);

extern int kernel_read(struct file *, unsigned long, char *, unsigned long);
extern struct file * open_exec(const char *);

/* fs/dcache.c -- generic fs support functions */
extern int is_subdir(struct dentry *, struct dentry *);
extern ino_t find_inode_number(struct dentry *, struct qstr *);

#include <linux/err.h>

/* needed for stackable file system support */
extern loff_t default_llseek(struct file *file, loff_t offset, int origin);

extern loff_t vfs_llseek(struct file *file, loff_t offset, int origin);

extern void inode_init_once(struct inode *);
extern void iput(struct inode *);
extern struct inode * igrab(struct inode *);
extern ino_t iunique(struct super_block *, ino_t);
extern int inode_needs_sync(struct inode *inode);
extern void generic_delete_inode(struct inode *inode);
extern void generic_drop_inode(struct inode *inode);

extern struct inode *ilookup5_nowait(struct super_block *sb,
		unsigned long hashval, int (*test)(struct inode *, void *),
		void *data);
extern struct inode *ilookup5(struct super_block *sb, unsigned long hashval,
		int (*test)(struct inode *, void *), void *data);
extern struct inode *ilookup(struct super_block *sb, unsigned long ino);

extern struct inode * iget5_locked(struct super_block *, unsigned long, int (*test)(struct inode *, void *), int (*set)(struct inode *, void *), void *);
extern struct inode * iget_locked(struct super_block *, unsigned long);
extern void unlock_new_inode(struct inode *);

static inline struct inode *iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode = iget_locked(sb, ino);

	if (inode && (inode->i_state & I_NEW)) {
		sb->s_op->read_inode(inode);
		unlock_new_inode(inode);
	}

	return inode;
}

extern void __iget(struct inode * inode);
extern void clear_inode(struct inode *);
extern void destroy_inode(struct inode *);
extern struct inode *new_inode(struct super_block *);
extern int __remove_suid(struct dentry *, int);
extern int should_remove_suid(struct dentry *);
extern int remove_suid(struct dentry *);

extern void __insert_inode_hash(struct inode *, unsigned long hashval);
extern void remove_inode_hash(struct inode *);
static inline void insert_inode_hash(struct inode *inode) {
	__insert_inode_hash(inode, inode->i_ino);
}

extern struct file * get_empty_filp(void);
extern void file_move(struct file *f, struct list_head *list);
extern void file_kill(struct file *f);
#ifdef CONFIG_BLOCK
struct bio;
extern void submit_bio(int, struct bio *);
extern int bdev_read_only(struct block_device *);
#endif
extern int set_blocksize(struct block_device *, int);
extern int sb_set_blocksize(struct super_block *, int);
extern int sb_min_blocksize(struct super_block *, int);
extern int sb_has_dirty_inodes(struct super_block *);

extern int generic_file_mmap(struct file *, struct vm_area_struct *);
extern int generic_file_readonly_mmap(struct file *, struct vm_area_struct *);
extern int file_read_actor(read_descriptor_t * desc, struct page *page, unsigned long offset, unsigned long size);
int generic_write_checks(struct file *file, loff_t *pos, size_t *count, int isblk);
extern ssize_t generic_file_aio_read(struct kiocb *, const struct iovec *, unsigned long, loff_t);
extern ssize_t generic_file_aio_write(struct kiocb *, const struct iovec *, unsigned long, loff_t);
extern ssize_t generic_file_aio_write_nolock(struct kiocb *, const struct iovec *,
		unsigned long, loff_t);
extern ssize_t generic_file_direct_write(struct kiocb *, const struct iovec *,
		unsigned long *, loff_t, loff_t *, size_t, size_t);
extern ssize_t generic_file_buffered_write(struct kiocb *, const struct iovec *,
		unsigned long, loff_t, loff_t *, size_t, ssize_t);
extern ssize_t do_sync_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos);
extern ssize_t do_sync_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos);
extern void do_generic_mapping_read(struct address_space *mapping,
				    struct file_ra_state *, struct file *,
				    loff_t *, read_descriptor_t *, read_actor_t);
extern int generic_segment_checks(const struct iovec *iov,
		unsigned long *nr_segs, size_t *count, int access_flags);

/* fs/splice.c */
extern ssize_t generic_file_splice_read(struct file *, loff_t *,
		struct pipe_inode_info *, size_t, unsigned int);
extern ssize_t generic_file_splice_write(struct pipe_inode_info *,
		struct file *, loff_t *, size_t, unsigned int);
extern ssize_t generic_file_splice_write_nolock(struct pipe_inode_info *,
		struct file *, loff_t *, size_t, unsigned int);
extern ssize_t generic_splice_sendpage(struct pipe_inode_info *pipe,
		struct file *out, loff_t *, size_t len, unsigned int flags);
extern long do_splice_direct(struct file *in, loff_t *ppos, struct file *out,
		size_t len, unsigned int flags);

extern void
file_ra_state_init(struct file_ra_state *ra, struct address_space *mapping);
extern loff_t no_llseek(struct file *file, loff_t offset, int origin);
extern loff_t generic_file_llseek(struct file *file, loff_t offset, int origin);
extern loff_t remote_llseek(struct file *file, loff_t offset, int origin);
extern int generic_file_open(struct inode * inode, struct file * filp);
extern int nonseekable_open(struct inode * inode, struct file * filp);

#ifdef CONFIG_FS_XIP
extern ssize_t xip_file_read(struct file *filp, char __user *buf, size_t len,
			     loff_t *ppos);
extern int xip_file_mmap(struct file * file, struct vm_area_struct * vma);
extern ssize_t xip_file_write(struct file *filp, const char __user *buf,
			      size_t len, loff_t *ppos);
extern int xip_truncate_page(struct address_space *mapping, loff_t from);
#else
static inline int xip_truncate_page(struct address_space *mapping, loff_t from)
{
	return 0;
}
#endif

static inline void do_generic_file_read(struct file * filp, loff_t *ppos,
					read_descriptor_t * desc,
					read_actor_t actor)
{
	do_generic_mapping_read(filp->f_mapping,
				&filp->f_ra,
				filp,
				ppos,
				desc,
				actor);
}

#ifdef CONFIG_BLOCK
ssize_t __blockdev_direct_IO(int rw, struct kiocb *iocb, struct inode *inode,
	struct block_device *bdev, const struct iovec *iov, loff_t offset,
	unsigned long nr_segs, get_block_t get_block, dio_iodone_t end_io,
	int lock_type);

enum {
	DIO_LOCKING = 1, /* need locking between buffered and direct access */
	DIO_NO_LOCKING,  /* bdev; no locking at all between buffered/direct */
	DIO_OWN_LOCKING, /* filesystem locks buffered and direct internally */
};

static inline ssize_t blockdev_direct_IO(int rw, struct kiocb *iocb,
	struct inode *inode, struct block_device *bdev, const struct iovec *iov,
	loff_t offset, unsigned long nr_segs, get_block_t get_block,
	dio_iodone_t end_io)
{
	return __blockdev_direct_IO(rw, iocb, inode, bdev, iov, offset,
				nr_segs, get_block, end_io, DIO_LOCKING);
}

static inline ssize_t blockdev_direct_IO_no_locking(int rw, struct kiocb *iocb,
	struct inode *inode, struct block_device *bdev, const struct iovec *iov,
	loff_t offset, unsigned long nr_segs, get_block_t get_block,
	dio_iodone_t end_io)
{
	return __blockdev_direct_IO(rw, iocb, inode, bdev, iov, offset,
				nr_segs, get_block, end_io, DIO_NO_LOCKING);
}

static inline ssize_t blockdev_direct_IO_own_locking(int rw, struct kiocb *iocb,
	struct inode *inode, struct block_device *bdev, const struct iovec *iov,
	loff_t offset, unsigned long nr_segs, get_block_t get_block,
	dio_iodone_t end_io)
{
	return __blockdev_direct_IO(rw, iocb, inode, bdev, iov, offset,
				nr_segs, get_block, end_io, DIO_OWN_LOCKING);
}
#endif

extern const struct file_operations generic_ro_fops;

#define special_file(m) (S_ISCHR(m)||S_ISBLK(m)||S_ISFIFO(m)||S_ISSOCK(m))

extern int vfs_readlink(struct dentry *, char __user *, int, const char *);
extern int vfs_follow_link(struct nameidata *, const char *);
extern int page_readlink(struct dentry *, char __user *, int);
extern void *page_follow_link_light(struct dentry *, struct nameidata *);
extern void page_put_link(struct dentry *, struct nameidata *, void *);
extern int __page_symlink(struct inode *inode, const char *symname, int len,
		gfp_t gfp_mask);
extern int page_symlink(struct inode *inode, const char *symname, int len);
extern const struct inode_operations page_symlink_inode_operations;
extern int generic_readlink(struct dentry *, char __user *, int);
extern void generic_fillattr(struct inode *, struct kstat *);
extern int vfs_getattr(struct vfsmount *, struct dentry *, struct kstat *);
void inode_add_bytes(struct inode *inode, loff_t bytes);
void inode_sub_bytes(struct inode *inode, loff_t bytes);
loff_t inode_get_bytes(struct inode *inode);
void inode_set_bytes(struct inode *inode, loff_t bytes);

extern int vfs_readdir(struct file *, filldir_t, void *);

extern int vfs_stat(char __user *, struct kstat *);
extern int vfs_lstat(char __user *, struct kstat *);
extern int vfs_stat_fd(int dfd, char __user *, struct kstat *);
extern int vfs_lstat_fd(int dfd, char __user *, struct kstat *);
extern int vfs_fstat(unsigned int, struct kstat *);

extern int vfs_ioctl(struct file *, unsigned int, unsigned int, unsigned long);

extern void get_filesystem(struct file_system_type *fs);
extern void put_filesystem(struct file_system_type *fs);
extern struct file_system_type *get_fs_type(const char *name);
extern struct super_block *get_super(struct block_device *);
extern struct super_block *user_get_super(dev_t);
extern void drop_super(struct super_block *sb);

extern int dcache_dir_open(struct inode *, struct file *);
extern int dcache_dir_close(struct inode *, struct file *);
extern loff_t dcache_dir_lseek(struct file *, loff_t, int);
extern int dcache_readdir(struct file *, void *, filldir_t);
extern int simple_getattr(struct vfsmount *, struct dentry *, struct kstat *);
extern int simple_statfs(struct dentry *, struct kstatfs *);
extern int simple_link(struct dentry *, struct inode *, struct dentry *);
extern int simple_unlink(struct inode *, struct dentry *);
extern int simple_rmdir(struct inode *, struct dentry *);
extern int simple_rename(struct inode *, struct dentry *, struct inode *, struct dentry *);
extern int simple_sync_file(struct file *, struct dentry *, int);
extern int simple_empty(struct dentry *);
extern int simple_readpage(struct file *file, struct page *page);
extern int simple_prepare_write(struct file *file, struct page *page,
			unsigned offset, unsigned to);
extern int simple_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata);
extern int simple_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata);

extern struct dentry *simple_lookup(struct inode *, struct dentry *, struct nameidata *);
extern ssize_t generic_read_dir(struct file *, char __user *, size_t, loff_t *);
extern const struct file_operations simple_dir_operations;
extern const struct inode_operations simple_dir_inode_operations;
struct tree_descr { char *name; const struct file_operations *ops; int mode; };
struct dentry *d_alloc_name(struct dentry *, const char *);
extern int simple_fill_super(struct super_block *, int, struct tree_descr *);
extern int simple_pin_fs(struct file_system_type *, struct vfsmount **mount, int *count);
extern void simple_release_fs(struct vfsmount **mount, int *count);

extern ssize_t simple_read_from_buffer(void __user *, size_t, loff_t *, const void *, size_t);

#ifdef CONFIG_MIGRATION
extern int buffer_migrate_page(struct address_space *,
				struct page *, struct page *);
#else
#define buffer_migrate_page NULL
#endif

extern int inode_change_ok(struct inode *, struct iattr *);
extern int __must_check inode_setattr(struct inode *, struct iattr *);

extern void file_update_time(struct file *file);

static inline ino_t parent_ino(struct dentry *dentry)
{
	ino_t res;

	spin_lock(&dentry->d_lock);
	res = dentry->d_parent->d_inode->i_ino;
	spin_unlock(&dentry->d_lock);
	return res;
}

/* kernel/fork.c */
extern int unshare_files(void);

/* Transaction based IO helpers */

/*
 * An argresp is stored in an allocated page and holds the
 * size of the argument or response, along with its content
 */
struct simple_transaction_argresp {
	ssize_t size;
	char data[0];
};

#define SIMPLE_TRANSACTION_LIMIT (PAGE_SIZE - sizeof(struct simple_transaction_argresp))

char *simple_transaction_get(struct file *file, const char __user *buf,
				size_t size);
ssize_t simple_transaction_read(struct file *file, char __user *buf,
				size_t size, loff_t *pos);
int simple_transaction_release(struct inode *inode, struct file *file);

static inline void simple_transaction_set(struct file *file, size_t n)
{
	struct simple_transaction_argresp *ar = file->private_data;

	BUG_ON(n > SIMPLE_TRANSACTION_LIMIT);

	/*
	 * The barrier ensures that ar->size will really remain zero until
	 * ar->data is ready for reading.
	 */
	smp_mb();
	ar->size = n;
}

/*
 * simple attribute files
 *
 * These attributes behave similar to those in sysfs:
 *
 * Writing to an attribute immediately sets a value, an open file can be
 * written to multiple times.
 *
 * Reading from an attribute creates a buffer from the value that might get
 * read with multiple read calls. When the attribute has been read
 * completely, no further read calls are possible until the file is opened
 * again.
 *
 * All attributes contain a text representation of a numeric value
 * that are accessed with the get() and set() functions.
 */
#define DEFINE_SIMPLE_ATTRIBUTE(__fops, __get, __set, __fmt)		\
static int __fops ## _open(struct inode *inode, struct file *file)	\
{									\
	__simple_attr_check_format(__fmt, 0ull);			\
	return simple_attr_open(inode, file, __get, __set, __fmt);	\
}									\
static struct file_operations __fops = {				\
	.owner	 = THIS_MODULE,						\
	.open	 = __fops ## _open,					\
	.release = simple_attr_close,					\
	.read	 = simple_attr_read,					\
	.write	 = simple_attr_write,					\
};

static inline void __attribute__((format(printf, 1, 2)))
__simple_attr_check_format(const char *fmt, ...)
{
	/* don't do anything, just let the compiler check the arguments; */
}

int simple_attr_open(struct inode *inode, struct file *file,
		     u64 (*get)(void *), void (*set)(void *, u64),
		     const char *fmt);
int simple_attr_close(struct inode *inode, struct file *file);
ssize_t simple_attr_read(struct file *file, char __user *buf,
			 size_t len, loff_t *ppos);
ssize_t simple_attr_write(struct file *file, const char __user *buf,
			  size_t len, loff_t *ppos);


#ifdef CONFIG_SECURITY
static inline char *alloc_secdata(void)
{
	return (char *)get_zeroed_page(GFP_KERNEL);
}

static inline void free_secdata(void *secdata)
{
	free_page((unsigned long)secdata);
}
#else
static inline char *alloc_secdata(void)
{
	return (char *)1;
}

static inline void free_secdata(void *secdata)
{ }
#endif	/* CONFIG_SECURITY */

struct ctl_table;
int proc_nr_files(struct ctl_table *table, int write, struct file *filp,
		  void __user *buffer, size_t *lenp, loff_t *ppos);


#endif /* __KERNEL__ */
#endif /* _LINUX_FS_H */
