#include <linux/bitops.h>
#include <linux/module.h>
#include <asm/types.h>
#include <asm/byteorder.h>

/*nr�����ֳ���Ŀ*/
#define BITOP_WORD(nr)		((nr) / BITS_PER_LONG)

/*��ѯ�ڴ�����[addr,addr+size]�У�������[addr+offset, addr+size]�е�һ����λ�ı���λ����*/
unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset)
{
	/*������ʼλoffset���ֳ���ַ������IA-32�ϣ�����offsetΪ100����ǰ�����ֳ�96λֱ��������
	��λ�����ĸ��ֳ�����ʼλ�ã�(unsigned long *)addr+3*/
	const unsigned long *p = addr + BITOP_WORD(offset);
	/*��ƫ��λ���ֳ����룬����������100�ĵ�5λȫ�����㣬0x64����1100100����������0x60*/
	unsigned long result = offset & ~(BITS_PER_LONG-1);
	unsigned long tmp;
	/*��ѯ��ʼλ�ò��ڷ�Χ�ڣ�ֱ���˳�*/
	if (offset >= size)
		return size;
	size -= result;
	offset %= BITS_PER_LONG;
	if (offset) 
	{
		tmp = *(p++);
		tmp &= (~0UL << offset);
		if (size < BITS_PER_LONG)
			goto found_first;
		if (tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG-1)) {
		if ((tmp = *(p++)))
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp &= (~0UL >> (BITS_PER_LONG - size));
	if (tmp == 0UL)		/* Are any bits set? */
		return result + size;	/* Nope. */
found_middle:
	return result + __ffs(tmp);
}

EXPORT_SYMBOL(find_next_bit);

/*���ҵ�ַ�ռ���[addr,addr+size]��[offset,size]�����ڵ�һ��δ��λ�ı���λ���*/
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size, unsigned long offset)
{
	const unsigned long *p = addr + BITOP_WORD(offset);
	unsigned long result = offset & ~(BITS_PER_LONG-1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset %= BITS_PER_LONG;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (BITS_PER_LONG - offset);
		if (size < BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}
	while (size & ~(BITS_PER_LONG-1)) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
	if (tmp == ~0UL)	/* Are any bits zero? */
		return result + size;	/* Nope. */
found_middle:
	return result + ffz(tmp);
}

EXPORT_SYMBOL(find_next_zero_bit);

#ifdef __BIG_ENDIAN

/* include/linux/byteorder does not support "unsigned long" type */
static inline unsigned long ext2_swabp(const unsigned long * x)
{
#if BITS_PER_LONG == 64
	return (unsigned long) __swab64p((u64 *) x);
#elif BITS_PER_LONG == 32
	return (unsigned long) __swab32p((u32 *) x);
#else
#error BITS_PER_LONG not defined
#endif
}

/* include/linux/byteorder doesn't support "unsigned long" type */
static inline unsigned long ext2_swab(const unsigned long y)
{
#if BITS_PER_LONG == 64
	return (unsigned long) __swab64((u64) y);
#elif BITS_PER_LONG == 32
	return (unsigned long) __swab32((u32) y);
#else
#error BITS_PER_LONG not defined
#endif
}

unsigned long generic_find_next_zero_le_bit(const unsigned long *addr, unsigned
		long size, unsigned long offset)
{
	const unsigned long *p = addr + BITOP_WORD(offset);
	unsigned long result = offset & ~(BITS_PER_LONG - 1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= (BITS_PER_LONG - 1UL);
	if (offset) {
		tmp = ext2_swabp(p++);
		tmp |= (~0UL >> (BITS_PER_LONG - offset));
		if (size < BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}

	while (size & ~(BITS_PER_LONG - 1)) {
		if (~(tmp = *(p++)))
			goto found_middle_swap;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = ext2_swabp(p);
found_first:
	tmp |= ~0UL << size;
	if (tmp == ~0UL)	/* Are any bits zero? */
		return result + size; /* Nope. Skip ffz */
found_middle:
	return result + ffz(tmp);

found_middle_swap:
	return result + ffz(ext2_swab(tmp));
}

EXPORT_SYMBOL(generic_find_next_zero_le_bit);

#endif /* __BIG_ENDIAN */
