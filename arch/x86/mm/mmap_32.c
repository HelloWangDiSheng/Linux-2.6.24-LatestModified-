/*支持弹性mmap布局*/

#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/sched.h>

/*可以根据栈的最大长度，来计算栈最低的可能位置，用作mmap区域的起始起点。mmap区域的
顶部（进程栈下面），至少预留128M空间。另外，如果指定的栈界限非常巨大，那么内核会保
证至少有一小部分地址空间不会被栈占据。如果要求使用地址空间随机化机制，上述位置会减
去一个随机偏移量，最大为1M，另外，内核会确保该区域对齐到页帧，这是体系结构的要求*/
#define MIN_GAP (128*1024*1024)
#define MAX_GAP (TASK_SIZE/6*5)
/*获取内存映射基址*/
static inline unsigned long mmap_base(struct mm_struct *mm)
{
	/*获取当前进程栈长度*/
	unsigned long gap = current->signal->rlim[RLIMIT_STACK].rlim_cur;
	/*是否启用地址空间随机化*/
	unsigned long random_factor = 0;
	/*如果启用地址随机化，则会预留最大1M空间*/
	if (current->flags & PF_RANDOMIZE)
		random_factor = get_random_int() % (1024*1024);
	/*设置栈长度的最小值*/
	if (gap < MIN_GAP)
		gap = MIN_GAP;
	/*设置栈长度的最大值*/
	else if (gap > MAX_GAP)
		gap = MAX_GAP;
	/*设置mmap映射页对齐（体系结构要求）后的基址*/
	return PAGE_ALIGN(TASK_SIZE - gap - random_factor);
}

/*该函数早在一个新进程创建虚拟内存映像期间就被调用，创建函数使用的虚拟内存布局*/

void arch_pick_mmap_layout(struct mm_struct *mm)
{
	/*如果进程设置标准虚拟内存布局、设置个性化或者期望栈增长没有限制，则回退到标准布
	局，标准布局中mmap增长方向时由低到高，和栈相向增长*/
	if (sysctl_legacy_va_layout ||	(current->personality & ADDR_COMPAT_LAYOUT) ||
			current->signal->rlim[RLIMIT_STACK].rlim_cur == RLIM_INFINITY)
	{
		/*经典布局中mmap起始地址是PAGE_ALIGN(TASK_SIZE/3)*/
		mm->mmap_base = TASK_UNMAPPED_BASE;
		/*在mmap中为新vma查找适当的位置*/
		mm->get_unmapped_area = arch_get_unmapped_area;
		mm->unmap_area = arch_unmap_area;
	}
	else
	{
		/*新式布局。mmap由高到低增长，与堆空间的扩展方向相向而行*/
		mm->mmap_base = mmap_base(mm);

		/*使用新布局时，内存映射自顶向下增长，标准函数arch_get_unmmaped_area_topdown负责该工作*/
		mm->get_unmapped_area = arch_get_unmapped_area_topdown;
		mm->unmap_area = arch_unmap_area_topdown;
	}
}
