/*��������ͨ�����ü����Ŀ�����*/

#include <linux/kref.h>
#include <linux/module.h>

/*��ʼ����������ü���*/
void kref_init(struct kref *kref)
{
	/*����������ü�������Ϊ1*/
	atomic_set(&kref->refcount,1);
	/*ʹ���ڴ����ϣ�ȷ����������ü���ֵ����ȷ�ĸ���*/
	smp_mb();
}

/*����������ü���ֵ����1*/
void kref_get(struct kref *kref)
{
	/*δ��ʼ���������ü�����ֱ��ʹ��ʱ����*/
	WARN_ON(!atomic_read(&kref->refcount));
	/*ԭ�Ӳ�������������ü�������1*/
	atomic_inc(&kref->refcount);
	/*�������ü��������������ڴ��Ż����ϣ�ȷ�����ü���ֵ��ȷ����*/
	smp_mb__after_atomic_inc();
}

/*����������ü������Լ�1������û�б�����ʱ�����ͷţ��ͷź����ĺ���ָ���Ǳ�����ڵģ�
���ݸ��ú���kfreeָ���ǲ��ɽ��ܵġ�����ɾ���ɹ�ʱ����1�����򷵻�0��Ҫע�⣬�������
����0��ʣ���ڴ������Ȼ����ʹ�ø�����*/
int kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
	/*�ͷź���releaseΪ��ʱ����*/
	WARN_ON(release == NULL);
	/*�ͷź���release����kfreeʱ����*/
	WARN_ON(release == (void (*)(struct kref *))kfree);

	if (atomic_dec_and_test(&kref->refcount))
	{
		/*���ü���Ϊ��ʱ������release�����ͷŸ�����*/
		release(kref);
		return 1;
	}
	return 0;
}

EXPORT_SYMBOL(kref_init);
EXPORT_SYMBOL(kref_get);
EXPORT_SYMBOL(kref_put);
