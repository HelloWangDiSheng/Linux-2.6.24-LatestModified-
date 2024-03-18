/*֧�ֵ���mmap����*/

#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/sched.h>

/*���Ը���ջ����󳤶ȣ�������ջ��͵Ŀ���λ�ã�����mmap�������ʼ��㡣mmap�����
����������ջ���棩������Ԥ��128M�ռ䡣���⣬���ָ����ջ���޷ǳ��޴���ô�ں˻ᱣ
֤������һС���ֵ�ַ�ռ䲻�ᱻջռ�ݡ����Ҫ��ʹ�õ�ַ�ռ���������ƣ�����λ�û��
ȥһ�����ƫ���������Ϊ1M�����⣬�ں˻�ȷ����������뵽ҳ֡��������ϵ�ṹ��Ҫ��*/
#define MIN_GAP (128*1024*1024)
#define MAX_GAP (TASK_SIZE/6*5)
/*��ȡ�ڴ�ӳ���ַ*/
static inline unsigned long mmap_base(struct mm_struct *mm)
{
	/*��ȡ��ǰ����ջ����*/
	unsigned long gap = current->signal->rlim[RLIMIT_STACK].rlim_cur;
	/*�Ƿ����õ�ַ�ռ������*/
	unsigned long random_factor = 0;
	/*������õ�ַ����������Ԥ�����1M�ռ�*/
	if (current->flags & PF_RANDOMIZE)
		random_factor = get_random_int() % (1024*1024);
	/*����ջ���ȵ���Сֵ*/
	if (gap < MIN_GAP)
		gap = MIN_GAP;
	/*����ջ���ȵ����ֵ*/
	else if (gap > MAX_GAP)
		gap = MAX_GAP;
	/*����mmapӳ��ҳ���루��ϵ�ṹҪ�󣩺�Ļ�ַ*/
	return PAGE_ALIGN(TASK_SIZE - gap - random_factor);
}

/*�ú�������һ���½��̴��������ڴ�ӳ���ڼ�ͱ����ã���������ʹ�õ������ڴ沼��*/

void arch_pick_mmap_layout(struct mm_struct *mm)
{
	/*����������ñ�׼�����ڴ沼�֡����ø��Ի���������ջ����û�����ƣ�����˵���׼��
	�֣���׼������mmap��������ʱ�ɵ͵��ߣ���ջ��������*/
	if (sysctl_legacy_va_layout ||	(current->personality & ADDR_COMPAT_LAYOUT) ||
			current->signal->rlim[RLIMIT_STACK].rlim_cur == RLIM_INFINITY)
	{
		/*���䲼����mmap��ʼ��ַ��PAGE_ALIGN(TASK_SIZE/3)*/
		mm->mmap_base = TASK_UNMAPPED_BASE;
		/*��mmap��Ϊ��vma�����ʵ���λ��*/
		mm->get_unmapped_area = arch_get_unmapped_area;
		mm->unmap_area = arch_unmap_area;
	}
	else
	{
		/*��ʽ���֡�mmap�ɸߵ�����������ѿռ����չ�����������*/
		mm->mmap_base = mmap_base(mm);

		/*ʹ���²���ʱ���ڴ�ӳ���Զ�������������׼����arch_get_unmmaped_area_topdown����ù���*/
		mm->get_unmapped_area = arch_get_unmapped_area_topdown;
		mm->unmap_area = arch_unmap_area_topdown;
	}
}
