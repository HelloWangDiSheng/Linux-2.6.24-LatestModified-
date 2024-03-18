#ifndef _ASM_X86_KMAP_TYPES_H
#define _ASM_X86_KMAP_TYPES_H

/*ӳ������*/
#if defined(CONFIG_X86_32) && defined(CONFIG_DEBUG_HIGHMEM)
#define D(n) __KM_FENCE_##n ,
#else
#define D(n)
#endif

/*�̶�ӳ����ƣ�ʹ�ÿ������ں˵�ַ�ռ��з������ڽ���ԭ��ӳ����ڴ档��FIX_KMAP_BEGIN
��FIX_KMAP_END֮�佨��һ������ӳ��߶��ڴ�ҳ�����򣬸�����λ��fixed_addresses�����С�
׼ȷ��λ����Ҫ���ݵ�ǰ���CPU������ӳ�����ͼ��㡣
idx = type + KM_TYPE_NR*smp_processor_id();
vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
�ڹ̶�ӳ�������У�ϵͳ�е�ÿ������������һ����Ӧ�ġ����ڡ���ÿ�������У�ÿ��ӳ�����Ͷ�
��Ӧ��һ�KM_TYPE_NR����һ�����������ͣ�ֻ���ڱ�ʾkm_type���ж�������������ֲ���
�����Ǻ����������ʹ��kmap_atomicʱ�������������������������ô��һ�����̿��ܽ���ͬ��
���͵�ӳ�䣬�����ִ�����ʹ��������ʽ������ʵ����������ҵ���صĹ̶�ӳ���ַ֮��
�ں�ֻ����Ӧ���޸�ҳ����ˢ��TLBʹҳ���޸���Ч*/
enum km_type
{
	D(0)	KM_BOUNCE_READ,
	D(1)	KM_SKB_SUNRPC_DATA,
	D(2)	KM_SKB_DATA_SOFTIRQ,
	D(3)	KM_USER0,
	D(4)	KM_USER1,
	D(5)	KM_BIO_SRC_IRQ,
	D(6)	KM_BIO_DST_IRQ,
	D(7)	KM_PTE0,
	D(8)	KM_PTE1,
	D(9)	KM_IRQ0,
	D(10)	KM_IRQ1,
	D(11)	KM_SOFTIRQ0,
	D(12)	KM_SOFTIRQ1,
	D(13)	KM_TYPE_NR
};

#undef D

#endif
