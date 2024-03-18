#ifndef __LINUX_DCACHE_H
#define __LINUX_DCACHE_H

#ifdef __KERNEL__

#include <asm/atomic.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/cache.h>
#include <linux/rcupdate.h>

struct nameidata;
struct vfsmount;

/*测试指定的struct dentry实例是否是实例根结点*/
#define IS_ROOT(x) ((x) == (x)->d_parent)

/*
 * "quick string" -- eases parameter passing, but more importantly
 * saves "metadata" about the string (ie length and the hash).
 *
 * hash comes first so it snuggles against d_parent in the
 * dentry.
 */
struct qstr
{
	/*字符串散列值*/
	unsigned int hash;
	/*字符串长度*/
	unsigned int len;
	/*字符串名称*/
	const unsigned char *name;
};

/**/
struct dentry_stat_t
{
	/**/
	int nr_dentry;
	/**/
	int nr_unused;
	/**/
	int age_limit;          /* age in seconds */
	/**/
	int want_pages;         /* pages requested by system */
	/**/
	int dummy[2];
};
extern struct dentry_stat_t dentry_stat;

/* Name hashing routines. Initial hash value */
/* Hash courtesy of the R5 hash in reiserfs modulo sign bits */
#define init_name_hash()		0

/* partial hash update function. Assume roughly 4 bits per character */
static inline unsigned long partial_name_hash(unsigned long c, unsigned long prevhash)
{
	return (prevhash + (c << 4) + (c >> 4)) * 11;
}

/*
 * Finally: cut down the number of bits to a int value (and try to avoid
 * losing bits)
 */
static inline unsigned long end_name_hash(unsigned long hash)
{
	return (unsigned int) hash;
}

/* Compute the hash for a name string. */
static inline unsigned int full_name_hash(const unsigned char *name, unsigned int len)
{
	unsigned long hash = init_name_hash();
	while (len--)
		hash = partial_name_hash(*name++, hash);
	return end_name_hash(hash);
}

struct dcookie_struct;

#define DNAME_INLINE_LEN_MIN 36

/*由于块设备速度较慢，可能需要很长时间才能找到与一个文件名关联的inode。即使设备数据
已经在页缓存中，仍然每次都会重复整个查找操作（简直荒谬）。Linux使用目录项缓存（简称
dentry缓存）来快速访问此前的查找操作的结果。该缓存围绕着struct dentry建立，在VFS连
同文件系统实现读取的一个目录项（目录或文件） 的数据之后，则创建一个dentry实例，以缓
存找到的数据。在内核需要获取有关文件的信息时，使用dentry对象很方便，但它不是表示文件
及其内容的主要对象，这一职责分配给了inode。例如，根据dentry对象无法确认文件是否已经
修改。必须考察对应的inode实例，才能确认这一点，而使用dentry对象很容易找到inode实例*/
struct dentry
{
	/*计数器指定访问该dentry结构的进程数目*/
	atomic_t d_count;
	/*受d_lock保护的d_flags可以包含几个标志：DCACHE_DISCONNECTED指定一个dentry当前没
	有连接到超级块的dentry树。DCACHE_UNHASHED表明该dentry实例没有包含在任何inode的散
	列表中。要注意，这两个标志是彼此完全独立的。*/
	unsigned int d_flags;
	/*每一个struct dentry实例的保护锁*/
	spinlock_t d_lock;
	/*指向相关的inode实例的指针。如果dentry对象是为一个不存在的文件名建立的，则
	d_inode为NULL指针。这有助于加速查找不存在的文件名，通常情况下，这与查找实际存
	在的文件名同样耗时*/
	struct inode *d_inode;
	/*查找散列表。接下来的三个变量被__d_lookup访问，将它们放置于此，确保位于同一个缓存行*/
	struct hlist_node d_hash;
	/*d_parent是一个指针，指向当前结点父目录的dentry实例，当前的dentry实例即位于父目
	录的d_subdirs链表中。对于根目录（没有父目录），d_parent指向其自身的dentry实例*/
	struct dentry *d_parent;
	/* d_name指定了文件的名称。qstr是一个内核字符串的包装器。它存储了实际的char  *字符
	串以及字符串长度和散列值，这使得更容易处理查找工作。这里并不存储绝对路径，只有路
	径的最后一个分量，例如对/usr/bin/emacs只存储emacs，因为上述链表结构已经映射了目录
	结构*/
	struct qstr d_name;
	/*lru链表*/
	struct list_head d_lru;
	/*d_child和d_rcu能共享内存	 */
	union
	{
		/*父dentry实例的子链表*/
		struct list_head d_child;
	 	/**/
	 	struct rcu_head d_rcu;
	} d_u;
	/*各个dentry实例组成了一个网络，与文件系统的结构形成一定的映射关系。与给定目录下
	的所有文件和子目录相关联的dentry实例，都归入到d_subdirs链表（在目录对应的dentry
	实例中）。子结点的d_child成员充当链表元素（与表头共享一个联合的RCU成员，在从父结
	点的链表删除当前结点时，将发挥作用）。但其中并非完全映射文件系统的拓扑结构，因为
	dentry缓存只包含文件系统结构的一小部分。  最常用文件和目录对应的目录项才保存在内存
	中。原则上，可以为所有文件系统对象都生成dentry项，但物理内存空间和性能原因都限制
	了这样做*/
	struct list_head d_subdirs;	/* our children */
	/*d_alias用作链表元素，以连接表示相同文件的各个dentry对象。在利用硬链接用两个不
	同名称表示同一文件时，会发生这种情况。对应于文件的inode的i_dentry成员用作该链表
	的表头。各个dentry对象通过d_alias连接到该链表中*/
	struct list_head d_alias;	/* inode alias list */
	/**/
	unsigned long d_time;		/* used by d_revalidate */
	/*d_op指向一个结构，其中包含了各种函数指针，提供对dentry对象的各种操作。这些操作
	必须由底层文件系统实现*/
	struct dentry_operations *d_op;
	/*d_sb是一个指针，指向dentry对象所属文件系统超级块的实例。该指针使得各个dentry实
	例散布到可用的（已装载的）文件系统。由于每个超级块结构都包含了一个指针，指向该文
	件系统装载点对应目录的dentry实例，因此dentry组成的树可以划分为几个子树*/
	struct super_block *d_sb;
	/*fs-specific data*/
	void *d_fsdata;
#ifdef CONFIG_PROFILING
	/*cookie, if any*/
	struct dcookie_struct *d_cookie;
#endif
	/*当前dentry对象表示一个装载点，那么d_mounted设置为1；否则其值为0*/
	int d_mounted;
	/*如果文件名只由少量字符组成，则保存在d_iname中，而不是dname中，以加速访问。短文
	件名的长度上限由DNAME_INLINE_NAME_LEN指定，最多不超过16个字符。但内核有时能够容
	纳更长的文件名，因为该成员位于结构的末尾，而容纳该数据的缓存行可能仍然有可用空间
	（这取决于体系结构和处理器类型）*/
	unsigned char d_iname[DNAME_INLINE_LEN_MIN];
};

/*
 * dentry->d_lock spinlock nesting subclasses:
 *
 * 0: normal
 * 1: nested
 */
enum dentry_d_lock_class
{
	DENTRY_D_LOCK_NORMAL, /* implicitly used by plain spin_lock() APIs. */
	DENTRY_D_LOCK_NESTED
};

/*dentry_operations结构保存了一些指向各种特定于文件系统可以对dentry对象执行的操作的
函数指针。由于大多数文件系统都没有实现前述的这些函数，内核的惯例是这样：如果文件系统
对每个函数提供的实现为NULL指针，则将其替换为VFS的默认实现*/
struct dentry_operations
{
	/*d_revalidate对网络文件系统特别重要。它检查内存中的各个dentry对象构成的结构是否
	仍然能够反映当前文件系统中的情况。因为网络文件系统并不直接关联到内核/VFS，所有信
	息都必须通过网络连接收集，可能由于文件系统在存储端的改变，致使某些dentry不再有效
	。该函数用于确保一致性。本地文件系统通常不会发生此类不一致情况，VFS对d_revalidate
	的默认实现什么都不做*/
	int (*d_revalidate)(struct dentry *, struct nameidata *);
	/*d_hash计算散列值，该值用于将对象放置到dentry散列表中*/
	int (*d_hash) (struct dentry *, struct qstr *);
	/*d_compare比较两个dentry对象的文件名。尽管VFS只执行简单的字符串比较，但文件系统
	可以替换默认实现，以适合自身的需求。例如，FAT实现中的文件名是不区分大小写的。因
	为不区分大写字母和小写字母，所以简单的字符串匹配将返回错误的结果。在这种情况下必
	须提供一个特定于FAT的函数*/
	int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
	/*在最后一个引用已经移除（d_count到达0时）后，将调用d_delete*/
	int (*d_delete)(struct dentry *);
	/*在最后删除一个dentry对象之前，将调用d_release。d_release和d_delete的两个默认实
	现什么都不做*/
	void (*d_release)(struct dentry *);
	/*d_iput从一个不再使用的dentry对象中释放inode（在默认的情况下，将inode的使用计数
	器减1，计数器到达0后，将inode从各种链表中移除）*/
	void (*d_iput)(struct dentry *, struct inode *);
	/**/
	char *(*d_dname)(struct dentry *, char *, int);
};

/* the dentry parameter passed to d_hash and d_compare is the parent
 * directory of the entries to be compared. It is used in case these
 * functions need any directory specific information for determining
 * equivalency classes.  Using the dentry itself might not work, as it
 * might be a negative dentry which has no information associated with
 * it */

/*
locking rules:
		big lock	dcache_lock	d_lock   may block
d_revalidate:	no		no		no       yes
d_hash		no		no		no       yes
d_compare:	no		yes		yes      no
d_delete:	no		yes		no       no
d_release:	no		no		no       yes
d_iput:		no		no		no       yes
 */

/* d_flags entries */
#define DCACHE_AUTOFS_PENDING 0x0001    /* autofs: "under construction" */
#define DCACHE_NFSFS_RENAMED  0x0002    /* this dentry has been "silly
					 * renamed" and has to be
					 * deleted on the last dput()*/
/*指定一个dentry当前没有连接到超级块的dentry树*/
#define	DCACHE_DISCONNECTED 0x0004
     /* This dentry is possibly not currently connected to the dcache tree,
      * in which case its parent will either be itself, or will have this
      * flag as well.  nfsd will not use a dentry with this bit set, but will
      * first endeavour to clear the bit either by discovering that it is
      * connected, or by performing lookup operations.   Any filesystem which
      * supports nfsd_operations MUST have a lookup function which, if it finds
      * a directory inode with a DCACHE_DISCONNECTED dentry, will d_move
      * that dentry into place and return that dentry rather than the passed one,
      * typically using d_splice_alias.
      */

#define DCACHE_REFERENCED	0x0008  /* Recently used, don't discard. */
/*DCACHE_UNHASHED表明该dentry实例没有包含在任何inode的散列表中*/
#define DCACHE_UNHASHED		0x0010

#define DCACHE_INOTIFY_PARENT_WATCHED	0x0020 /* Parent inode is watched */

extern spinlock_t dcache_lock;
extern seqlock_t rename_lock;

/**
 * d_drop - drop a dentry
 * @dentry: dentry to drop
 *
 * d_drop() unhashes the entry from the parent dentry hashes, so that it won't
 * be found through a VFS lookup any more. Note that this is different from
 * deleting the dentry - d_delete will try to mark the dentry negative if
 * possible, giving a successful _negative_ lookup, while d_drop will
 * just make the cache lookup fail.
 *
 * d_drop() is used mainly for stuff that wants to invalidate a dentry for some
 * reason (NFS timeouts or autofs deletes).
 *
 * __d_drop requires dentry->d_lock.
 */
/*将指定的struct dentry实例从散列表中删除*/
static inline void __d_drop(struct dentry *dentry)
{
	/*如果struct dentry实例还在散列表中，则将其从散列表中删除*/
	if (!(dentry->d_flags & DCACHE_UNHASHED))
	{
		dentry->d_flags |= DCACHE_UNHASHED;
		hlist_del_rcu(&dentry->d_hash);
	}
}

/*d_drop将一个dentry实例从全局dentry散列表移除。调用dput时，如果使用计数下降到0则
自动调用该函数，另外如果需要使一个缓存的dentry对象失效，也可以手工调用。__d_drop
是d_drop的一个变体，并不自动处理锁定*/
static inline void d_drop(struct dentry *dentry)
{
	spin_lock(&dcache_lock);
	spin_lock(&dentry->d_lock);
 	__d_drop(dentry);
	spin_unlock(&dentry->d_lock);
	spin_unlock(&dcache_lock);
}

/*测试dentry对象中长短文件名是否不同*/
static inline int dname_external(struct dentry *dentry)
{
	return dentry->d_name.name != dentry->d_iname;
}

/*dcache底层文件系统接口*/
extern void d_instantiate(struct dentry *, struct inode *);
extern struct dentry * d_instantiate_unique(struct dentry *, struct inode *);
extern struct dentry * d_materialise_unique(struct dentry *, struct inode *);
extern void d_delete(struct dentry *);

/*分配释放函数*/
extern struct dentry * d_alloc(struct dentry *, const struct qstr *);
extern struct dentry * d_alloc_anon(struct inode *);
extern struct dentry * d_splice_alias(struct inode *, struct dentry *);
extern void shrink_dcache_sb(struct super_block *);
extern void shrink_dcache_parent(struct dentry *);
extern void shrink_dcache_for_umount(struct super_block *);
extern int d_invalidate(struct dentry *);

/*仅在装载时使用*/
extern struct dentry * d_alloc_root(struct inode *);

/* <clickety>-<click> the ramfs-type tree */
extern void d_genocide(struct dentry *);

extern struct dentry *d_find_alias(struct inode *);
extern void d_prune_aliases(struct inode *);

/* test whether we have any submounts in a subdir tree */
extern int have_submounts(struct dentry *);

/*
 * This adds the entry to the hash queues.
 */
extern void d_rehash(struct dentry *);

/**
 * d_add - add dentry to hash queues
 * @entry: dentry to add
 * @inode: The inode to attach to this dentry
 *
 * This adds the entry to the hash queues and initializes @inode.
 * The entry was actually filled in earlier during d_alloc().
 */

/*d_add调用了d_instantiate。该对象还添加到全局dentry散列表dentry_hashtable中*/
static inline void d_add(struct dentry *entry, struct inode *inode)
{
	d_instantiate(entry, inode);
	d_rehash(entry);
}

/**
 * d_add_unique - add dentry to hash queues without aliasing
 * @entry: dentry to add
 * @inode: The inode to attach to this dentry
 *
 * This adds the entry to the hash queues and initializes @inode.
 * The entry was actually filled in earlier during d_alloc().
 */
static inline struct dentry *d_add_unique(struct dentry *entry, struct inode *inode)
{
	struct dentry *res;

	res = d_instantiate_unique(entry, inode);
	d_rehash(res != NULL ? res : entry);
	return res;
}

/* used for rename() and baskets */
extern void d_move(struct dentry *, struct dentry *);

/* appendix may either be NULL or be used for transname suffixes */
extern struct dentry * d_lookup(struct dentry *, struct qstr *);
extern struct dentry * __d_lookup(struct dentry *, struct qstr *);
extern struct dentry * d_hash_and_lookup(struct dentry *, struct qstr *);

/* validate "insecure" dentry pointer */
extern int d_validate(struct dentry *, struct dentry *);

/*
 * helper function for dentry_operations.d_dname() members
 */
extern char *dynamic_dname(struct dentry *, char *, int, const char *, ...);

extern char * d_path(struct dentry *, struct vfsmount *, char *, int);

/* Allocation counts.. */

/**
 *	dget, dget_locked	-	get a reference to a dentry
 *	@dentry: dentry to get a reference to
 *
 *	Given a dentry or %NULL pointer increment the reference count
 *	if appropriate and return the dentry. A dentry will not be
 *	destroyed when it has references. dget() should never be
 *	called for dentries with zero reference counter. For these cases
 *	(preferably none, functions in dcache.c are sufficient for normal
 *	needs and they take necessary precautions) you should hold dcache_lock
 *	and call dget_locked() instead of dget().
 */

static inline struct dentry *dget(struct dentry *dentry)
{
	if (dentry) {
		BUG_ON(!atomic_read(&dentry->d_count));
		atomic_inc(&dentry->d_count);
	}
	return dentry;
}

extern struct dentry * dget_locked(struct dentry *);

/**
 *	d_unhashed -	is dentry hashed
 *	@dentry: entry to check
 *
 *	Returns true if the dentry passed is not currently hashed.
 */

static inline int d_unhashed(struct dentry *dentry)
{
	return (dentry->d_flags & DCACHE_UNHASHED);
}

static inline struct dentry *dget_parent(struct dentry *dentry)
{
	struct dentry *ret;

	spin_lock(&dentry->d_lock);
	ret = dget(dentry->d_parent);
	spin_unlock(&dentry->d_lock);
	return ret;
}

extern void dput(struct dentry *);

static inline int d_mountpoint(struct dentry *dentry)
{
	return dentry->d_mounted;
}

extern struct vfsmount *lookup_mnt(struct vfsmount *, struct dentry *);
extern struct vfsmount *__lookup_mnt(struct vfsmount *, struct dentry *, int);
extern struct dentry *lookup_create(struct nameidata *nd, int is_dir);

extern int sysctl_vfs_cache_pressure;

#endif /* __KERNEL__ */

#endif	/* __LINUX_DCACHE_H */
