/*处理对象的通用引用计数的库例程*/

#include <linux/kref.h>
#include <linux/module.h>

/*初始化对象的引用计数*/
void kref_init(struct kref *kref)
{
	/*将对象的引用计数设置为1*/
	atomic_set(&kref->refcount,1);
	/*使用内存屏障，确保对象的引用计数值能正确的更新*/
	smp_mb();
}

/*将对象的引用计数值自增1*/
void kref_get(struct kref *kref)
{
	/*未初始化对象引用计数而直接使用时警告*/
	WARN_ON(!atomic_read(&kref->refcount));
	/*原子操作将对象的引用计数自增1*/
	atomic_inc(&kref->refcount);
	/*对象引用计数自增后启用内存优化屏障，确保引用计数值正确更新*/
	smp_mb__after_atomic_inc();
}

/*将对象的引用计数器自减1，对象没有被引用时，则被释放，释放函数的函数指针是必须存在的，
传递给该函数kfree指针是不可接受的。对象被删除成功时返回1，否则返回0，要注意，如果函数
返回0，剩余内存操作仍然不能使用该引用*/
int kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
	/*释放函数release为空时警告*/
	WARN_ON(release == NULL);
	/*释放函数release等于kfree时警告*/
	WARN_ON(release == (void (*)(struct kref *))kfree);

	if (atomic_dec_and_test(&kref->refcount))
	{
		/*引用计数为零时，调用release函数释放该引用*/
		release(kref);
		return 1;
	}
	return 0;
}

EXPORT_SYMBOL(kref_init);
EXPORT_SYMBOL(kref_get);
EXPORT_SYMBOL(kref_put);
