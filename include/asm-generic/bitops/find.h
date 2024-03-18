#ifndef _ASM_GENERIC_BITOPS_FIND_H_
#define _ASM_GENERIC_BITOPS_FIND_H_

extern unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
extern unsigned long find_next_zero_bit(const unsigned long *addr, unsigned	long size, unsigned long offset);

/*获取内存域[addr, addr+size]中第一个已置位的比特位编号*/
#define find_first_bit(addr, size) find_next_bit((addr), (size), 0)
/*获取内存域[addr, addr+szie]中第一个未置位的比特位编号*/
#define find_first_zero_bit(addr, size) find_next_zero_bit((addr), (size), 0)

#endif /*_ASM_GENERIC_BITOPS_FIND_H_ */
