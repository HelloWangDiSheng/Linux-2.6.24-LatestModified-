#ifndef _LINUX_PFN_H_
#define _LINUX_PFN_H_
/*��ָ����ַ����һ������*/
#define PFN_ALIGN(x)	(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)
/*��ȡָ����ַ����ҳ����һҳҳ��*/
#define PFN_UP(x)	(((x) + PAGE_SIZE-1) >> PAGE_SHIFT)
/*��ȡ��ǰҳ��ҳ��*/
#define PFN_DOWN(x)	((x) >> PAGE_SHIFT)
/*��ֱ��ӳ����ҳ��ת��Ϊ�����ַ*/
#define PFN_PHYS(x)	((x) << PAGE_SHIFT)

#endif
