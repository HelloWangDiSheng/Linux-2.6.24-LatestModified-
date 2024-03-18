#include <linux/module.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <linux/version.h>
#include <linux/err.h>

/*申请一个新的uts命名空间并赋值为输入的uts命名空间*/
static struct uts_namespace *clone_uts_ns(struct uts_namespace *old_ns)
{
	struct uts_namespace *ns;
	/*申请uts命名空间相关的slab缓存*/
	ns = kmalloc(sizeof(struct uts_namespace), GFP_KERNEL);
	/*内存不足，失败返回*/	
	if (!ns)
		return ERR_PTR(-ENOMEM);
	/*申请uts命名空间的读信号量*/
	down_read(&uts_sem);
	/*新uts命名空间赋值为输入的命名空间*/
	memcpy(&ns->name, &old_ns->name, sizeof(ns->name));
	/*释放uts命名空间读信号量*/
	up_read(&uts_sem);
	/*初始化新的uts命名空间引用计数*/
	kref_init(&ns->kref);
	return ns;
}

/*复制uts命名空间*/
struct uts_namespace *copy_utsname(unsigned long flags, struct uts_namespace *old_ns)
{
	struct uts_namespace *new_ns;
	/*输入uts命名空间不能为空*/
	BUG_ON(!old_ns);
	/*引用输入uts命名空间*/
	get_uts_ns(old_ns);
	/*共享命名空间*/
	if (!(flags & CLONE_NEWUTS))
		return old_ns;
	/*用输入uts命名空间赋值给新创建的uts命名空间*/
	new_ns = clone_uts_ns(old_ns);
	/*取消对输入uts命名空间的引用*/
	put_uts_ns(old_ns);
	return new_ns;
}

/*释放uts命名空间（利用container_of机制）*/
void free_uts_ns(struct kref *kref)
{
	struct uts_namespace *ns;
	/*利用container_of机制获取对应的uts命名空间*/
	ns = container_of(kref, struct uts_namespace, kref);
	/*释放uts占用的内存空间*/
	kfree(ns);
}
