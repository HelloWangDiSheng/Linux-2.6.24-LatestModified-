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

/*����ָ����struct dentryʵ���Ƿ���ʵ�������*/
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
	/*�ַ���ɢ��ֵ*/
	unsigned int hash;
	/*�ַ�������*/
	unsigned int len;
	/*�ַ�������*/
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

/*���ڿ��豸�ٶȽ�����������Ҫ�ܳ�ʱ������ҵ���һ���ļ���������inode����ʹ�豸����
�Ѿ���ҳ�����У���Ȼÿ�ζ����ظ��������Ҳ�������ֱ��������Linuxʹ��Ŀ¼��棨���
dentry���棩�����ٷ��ʴ�ǰ�Ĳ��Ҳ����Ľ�����û���Χ����struct dentry��������VFS��
ͬ�ļ�ϵͳʵ�ֶ�ȡ��һ��Ŀ¼�Ŀ¼���ļ��� ������֮���򴴽�һ��dentryʵ�����Ի�
���ҵ������ݡ����ں���Ҫ��ȡ�й��ļ�����Ϣʱ��ʹ��dentry����ܷ��㣬�������Ǳ�ʾ�ļ�
�������ݵ���Ҫ������һְ��������inode�����磬����dentry�����޷�ȷ���ļ��Ƿ��Ѿ�
�޸ġ����뿼���Ӧ��inodeʵ��������ȷ����һ�㣬��ʹ��dentry����������ҵ�inodeʵ��*/
struct dentry
{
	/*������ָ�����ʸ�dentry�ṹ�Ľ�����Ŀ*/
	atomic_t d_count;
	/*��d_lock������d_flags���԰���������־��DCACHE_DISCONNECTEDָ��һ��dentry��ǰû
	�����ӵ��������dentry����DCACHE_UNHASHED������dentryʵ��û�а������κ�inode��ɢ
	�б��С�Ҫע�⣬��������־�Ǳ˴���ȫ�����ġ�*/
	unsigned int d_flags;
	/*ÿһ��struct dentryʵ���ı�����*/
	spinlock_t d_lock;
	/*ָ����ص�inodeʵ����ָ�롣���dentry������Ϊһ�������ڵ��ļ��������ģ���
	d_inodeΪNULLָ�롣�������ڼ��ٲ��Ҳ����ڵ��ļ�����ͨ������£��������ʵ�ʴ�
	�ڵ��ļ���ͬ����ʱ*/
	struct inode *d_inode;
	/*����ɢ�б�������������������__d_lookup���ʣ������Ƿ����ڴˣ�ȷ��λ��ͬһ��������*/
	struct hlist_node d_hash;
	/*d_parent��һ��ָ�룬ָ��ǰ��㸸Ŀ¼��dentryʵ������ǰ��dentryʵ����λ�ڸ�Ŀ
	¼��d_subdirs�����С����ڸ�Ŀ¼��û�и�Ŀ¼����d_parentָ���������dentryʵ��*/
	struct dentry *d_parent;
	/* d_nameָ�����ļ������ơ�qstr��һ���ں��ַ����İ�װ�������洢��ʵ�ʵ�char  *�ַ�
	���Լ��ַ������Ⱥ�ɢ��ֵ����ʹ�ø����״�����ҹ��������ﲢ���洢����·����ֻ��·
	�������һ�������������/usr/bin/emacsֻ�洢emacs����Ϊ��������ṹ�Ѿ�ӳ����Ŀ¼
	�ṹ*/
	struct qstr d_name;
	/*lru����*/
	struct list_head d_lru;
	/*d_child��d_rcu�ܹ����ڴ�	 */
	union
	{
		/*��dentryʵ����������*/
		struct list_head d_child;
	 	/**/
	 	struct rcu_head d_rcu;
	} d_u;
	/*����dentryʵ�������һ�����磬���ļ�ϵͳ�Ľṹ�γ�һ����ӳ���ϵ�������Ŀ¼��
	�������ļ�����Ŀ¼�������dentryʵ���������뵽d_subdirs������Ŀ¼��Ӧ��dentry
	ʵ���У����ӽ���d_child��Ա�䵱����Ԫ�أ����ͷ����һ�����ϵ�RCU��Ա���ڴӸ���
	�������ɾ����ǰ���ʱ�����������ã��������в�����ȫӳ���ļ�ϵͳ�����˽ṹ����Ϊ
	dentry����ֻ�����ļ�ϵͳ�ṹ��һС���֡�  ����ļ���Ŀ¼��Ӧ��Ŀ¼��ű������ڴ�
	�С�ԭ���ϣ�����Ϊ�����ļ�ϵͳ��������dentry��������ڴ�ռ������ԭ������
	��������*/
	struct list_head d_subdirs;	/* our children */
	/*d_alias��������Ԫ�أ������ӱ�ʾ��ͬ�ļ��ĸ���dentry����������Ӳ������������
	ͬ���Ʊ�ʾͬһ�ļ�ʱ���ᷢ�������������Ӧ���ļ���inode��i_dentry��Ա����������
	�ı�ͷ������dentry����ͨ��d_alias���ӵ���������*/
	struct list_head d_alias;	/* inode alias list */
	/**/
	unsigned long d_time;		/* used by d_revalidate */
	/*d_opָ��һ���ṹ�����а����˸��ֺ���ָ�룬�ṩ��dentry����ĸ��ֲ�������Щ����
	�����ɵײ��ļ�ϵͳʵ��*/
	struct dentry_operations *d_op;
	/*d_sb��һ��ָ�룬ָ��dentry���������ļ�ϵͳ�������ʵ������ָ��ʹ�ø���dentryʵ
	��ɢ�������õģ���װ�صģ��ļ�ϵͳ������ÿ��������ṹ��������һ��ָ�룬ָ�����
	��ϵͳװ�ص��ӦĿ¼��dentryʵ�������dentry��ɵ������Ի���Ϊ��������*/
	struct super_block *d_sb;
	/*fs-specific data*/
	void *d_fsdata;
#ifdef CONFIG_PROFILING
	/*cookie, if any*/
	struct dcookie_struct *d_cookie;
#endif
	/*��ǰdentry�����ʾһ��װ�ص㣬��ôd_mounted����Ϊ1��������ֵΪ0*/
	int d_mounted;
	/*����ļ���ֻ�������ַ���ɣ��򱣴���d_iname�У�������dname�У��Լ��ٷ��ʡ�����
	�����ĳ���������DNAME_INLINE_NAME_LENָ������಻����16���ַ������ں���ʱ�ܹ���
	�ɸ������ļ�������Ϊ�ó�Աλ�ڽṹ��ĩβ�������ɸ����ݵĻ����п�����Ȼ�п��ÿռ�
	����ȡ������ϵ�ṹ�ʹ��������ͣ�*/
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

/*dentry_operations�ṹ������һЩָ������ض����ļ�ϵͳ���Զ�dentry����ִ�еĲ�����
����ָ�롣���ڴ�����ļ�ϵͳ��û��ʵ��ǰ������Щ�������ں˵Ĺ���������������ļ�ϵͳ
��ÿ�������ṩ��ʵ��ΪNULLָ�룬�����滻ΪVFS��Ĭ��ʵ��*/
struct dentry_operations
{
	/*d_revalidate�������ļ�ϵͳ�ر���Ҫ��������ڴ��еĸ���dentry���󹹳ɵĽṹ�Ƿ�
	��Ȼ�ܹ���ӳ��ǰ�ļ�ϵͳ�е��������Ϊ�����ļ�ϵͳ����ֱ�ӹ������ں�/VFS��������
	Ϣ������ͨ�����������ռ������������ļ�ϵͳ�ڴ洢�˵ĸı䣬��ʹĳЩdentry������Ч
	���ú�������ȷ��һ���ԡ������ļ�ϵͳͨ�����ᷢ�����಻һ�������VFS��d_revalidate
	��Ĭ��ʵ��ʲô������*/
	int (*d_revalidate)(struct dentry *, struct nameidata *);
	/*d_hash����ɢ��ֵ����ֵ���ڽ�������õ�dentryɢ�б���*/
	int (*d_hash) (struct dentry *, struct qstr *);
	/*d_compare�Ƚ�����dentry������ļ���������VFSִֻ�м򵥵��ַ����Ƚϣ����ļ�ϵͳ
	�����滻Ĭ��ʵ�֣����ʺ�������������磬FATʵ���е��ļ����ǲ����ִ�Сд�ġ���
	Ϊ�����ִ�д��ĸ��Сд��ĸ�����Լ򵥵��ַ���ƥ�佫���ش���Ľ��������������±�
	���ṩһ���ض���FAT�ĺ���*/
	int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
	/*�����һ�������Ѿ��Ƴ���d_count����0ʱ���󣬽�����d_delete*/
	int (*d_delete)(struct dentry *);
	/*�����ɾ��һ��dentry����֮ǰ��������d_release��d_release��d_delete������Ĭ��ʵ
	��ʲô������*/
	void (*d_release)(struct dentry *);
	/*d_iput��һ������ʹ�õ�dentry�������ͷ�inode����Ĭ�ϵ�����£���inode��ʹ�ü���
	����1������������0�󣬽�inode�Ӹ����������Ƴ���*/
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
/*ָ��һ��dentry��ǰû�����ӵ��������dentry��*/
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
/*DCACHE_UNHASHED������dentryʵ��û�а������κ�inode��ɢ�б���*/
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
/*��ָ����struct dentryʵ����ɢ�б���ɾ��*/
static inline void __d_drop(struct dentry *dentry)
{
	/*���struct dentryʵ������ɢ�б��У������ɢ�б���ɾ��*/
	if (!(dentry->d_flags & DCACHE_UNHASHED))
	{
		dentry->d_flags |= DCACHE_UNHASHED;
		hlist_del_rcu(&dentry->d_hash);
	}
}

/*d_drop��һ��dentryʵ����ȫ��dentryɢ�б��Ƴ�������dputʱ�����ʹ�ü����½���0��
�Զ����øú��������������Ҫʹһ�������dentry����ʧЧ��Ҳ�����ֹ����á�__d_drop
��d_drop��һ�����壬�����Զ���������*/
static inline void d_drop(struct dentry *dentry)
{
	spin_lock(&dcache_lock);
	spin_lock(&dentry->d_lock);
 	__d_drop(dentry);
	spin_unlock(&dentry->d_lock);
	spin_unlock(&dcache_lock);
}

/*����dentry�����г����ļ����Ƿ�ͬ*/
static inline int dname_external(struct dentry *dentry)
{
	return dentry->d_name.name != dentry->d_iname;
}

/*dcache�ײ��ļ�ϵͳ�ӿ�*/
extern void d_instantiate(struct dentry *, struct inode *);
extern struct dentry * d_instantiate_unique(struct dentry *, struct inode *);
extern struct dentry * d_materialise_unique(struct dentry *, struct inode *);
extern void d_delete(struct dentry *);

/*�����ͷź���*/
extern struct dentry * d_alloc(struct dentry *, const struct qstr *);
extern struct dentry * d_alloc_anon(struct inode *);
extern struct dentry * d_splice_alias(struct inode *, struct dentry *);
extern void shrink_dcache_sb(struct super_block *);
extern void shrink_dcache_parent(struct dentry *);
extern void shrink_dcache_for_umount(struct super_block *);
extern int d_invalidate(struct dentry *);

/*����װ��ʱʹ��*/
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

/*d_add������d_instantiate���ö�����ӵ�ȫ��dentryɢ�б�dentry_hashtable��*/
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
