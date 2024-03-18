#ifndef _LINUX_PFN_H_
#define _LINUX_PFN_H_
/*将指定地址与下一个对齐*/
#define PFN_ALIGN(x)	(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
/*获取指定地址所在页的下一页页号*/
#define PFN_UP(x)	(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
/*获取当前页的页号*/
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
/*将直接映射区页号转换为物理地址*/
#define PFN_PHYS(x)	((x) << PAGE_SHIFT)

#endif
