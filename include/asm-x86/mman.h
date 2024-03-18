#ifndef _ASM_X86_MMAN_H
#define _ASM_X86_MMAN_H

#include <asm-generic/mman.h>

/*仅在32位地址空间中使用*/
#define MAP_32BIT			0x40
/* stack-like segment */
#define MAP_GROWSDOWN		0x0100
/*ETXTBSY：TXT文件被占用*/
#define MAP_DENYWRITE		0x0800
/*映射的区域可执行*/
#define MAP_EXECUTABLE		0x1000
/*页被锁定*/
#define MAP_LOCKED			0x2000
/*不用检查保留位。当我们使用mmap系统调用进行虚拟内存申请的时候，会受到内核overcommit
策略的影响，内核会综合物理内存的总体容量以及swap交换区的总体大小来决定是否允许本次虚
拟内存用量的申请。mmap申请过大的虚拟内存，内核会拒绝。但是当我们在mmap系统调用中设置
了MAP_NORESERVE，则内核在分配虚拟内存的时候将不会考虑物理内存的总体容量以及swap交换区
的限制因素，无论申请多大的虚拟内存，内核都会满足。但缺页的时候会容易导致OOM。
MAP_NORESERVE只会在OVERCOMMIT_GUESS和OVERCOMMIT_ALWAYS模式下才有意义，因为如果内核本身
是禁止overcommit的话，设置MAP_NORESERVE是无意义的*/
#define MAP_NORESERVE		0x4000
/*populate (prefault) pagetables*/
#define MAP_POPULATE		0x8000
/*IO操作时不阻塞*/
#define MAP_NONBLOCK		0x10000

/*锁定当前所有映射*/
#define MCL_CURRENT			1
/*锁定所有未来映射*/
#define MCL_FUTURE			2

#endif /* _ASM_X86_MMAN_H */
