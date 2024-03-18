#ifndef _LINUX_HASH_H
#define _LINUX_HASH_H
/*һ�������εĿ���ɢ�к���*/

/*��һ���hash�㷨����ֱ��ȡģ���߼�һ���hash�㷨�����һ������������Դ���ݵķֲ�
 ��ò���ô������Ҳ����˵�������������һ���ֲ��ܽ��յ��������л��Ǻ�ϡ����������У�
 ���ɵĽ�����Ǻܷ�ɢ���������У�linux�ں���������hashҪ���߼�һ�㣬��������������
 ��ӽ�32λ����64λ���޻ƽ�ָ���������ֻҪ��С����λ����;������ɣ�ǰ��ȷ��������
 1��2������С��Ҳ�ᱻ������������״�ɢ������ȷ���κ�һ�������������������ʱ�򶼿���
 ͨ�����ٵ���λ����;Ϳ��Եõ��˻�������������*/
/*���ûƽ�ָ������*/
#if BITS_PER_LONG == 32
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME 0x9e370001UL
#elif BITS_PER_LONG == 64
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME 0x9e37fffffffc0001UL
#else
#error Define GOLDEN_RATIO_PRIME for your wordsize.
#endif

/*��ȡһ��������ָ��ɢ��ƫ���ڵ�����*/
static inline unsigned long hash_long(unsigned long val, unsigned int bits)
{
	unsigned long hash = val;

#if BITS_PER_LONG == 64
	/*  Sigh, gcc can't optimise this alone like it does for 32 bits. */
	unsigned long n = hash;
	n <<= 18;
	hash -= n;
	n <<= 33;
	hash -= n;
	n <<= 3;
	hash += n;
	n <<= 3;
	hash -= n;
	n <<= 4;
	hash += n;
	n <<= 2;
	hash += n;
#else
	/* On some cpus multiply is faster, on others gcc will do shifts */
	hash *= GOLDEN_RATIO_PRIME;
#endif

	/* High bits are more random, so use them. */
	return hash >> (BITS_PER_LONG - bits);
}

/*��ȡһ����ַ��ָ��ɢ��ƫ��[0, (1<<bits)-1]�ڵ�����*/
static inline unsigned long hash_ptr(void *ptr, unsigned int bits)
{
	return hash_long((unsigned long)ptr, bits);
}
#endif /* _LINUX_HASH_H */
