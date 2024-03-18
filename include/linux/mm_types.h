#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <linux/auxvec.h>
#include <linux/types.h>
#include <linux/threads.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/prio_tree.h>
#include <linux/rbtree.h>
#include <linux/rwsem.h>
#include <linux/completion.h>
#include <asm/page.h>
#include <asm/mmu.h>

#ifndef AT_VECTOR_SIZE_ARCH
#define AT_VECTOR_SIZE_ARCH 0
#endif
#define AT_VECTOR_SIZE (2*(AT_VECTOR_SIZE_ARCH + AT_VECTOR_SIZE_BASE + 1))

struct address_space;

#if NR_CPUS >= CONFIG_SPLIT_PTLOCK_CPUS
typedef atomic_long_t mm_counter_t;
#else  /* NR_CPUS < CONFIG_SPLIT_PTLOCK_CPUS */
typedef unsigned long mm_counter_t;
#endif /* NR_CPUS < CONFIG_SPLIT_PTLOCK_CPUS */

/*
 * Each physical page in the system has a struct page associated with
 * it to keep track of whatever it is we are using the page for at the
 * moment. Note that we have no way to track which tasks are using
 * a page, though if it is a pagecache page, rmap structures can tell us
 * who is mapping it.
 */
struct page
{
	/*体系结构无关的标志，用于描述页的属性，可能异步更新*/
	unsigned long flags;
	/*内核中引用该页的次数，值为0时，内核知道struct page实例当前不使用，可以删除*/
	atomic_t _count;
	union
	{
		/*表示在页表中有多少项指向该页*/
		atomic_t _mapcount;
		/*slub中对象的数目*/
		unsigned int inuse;	/* SLUB: Nr of objects */
	};
	union
	{
	    struct
		{
			/*flags==PagePrivate时表示buffer_head，flags==PageSwapCache表示swp_entry_t，
			flags==PageBuddy	时表示分配阶*/
			unsigned long private;
			/*低位未置位时表示inode地址空间或者空，否则指向anon_vma对象*/
			/*mapping指定了页帧所在的地址空间，index时页帧在映射内部的偏移量，地址空间用
			于将文件的内容与装载数据的内存区关联起来。通过一个小技巧，mapping不仅能够保存
			一个指针，而且还能包含一些额外的信息，用于判断页是否属于未关联到地址空间的某
			个匿名内存页。如果将mapping置为1，则该指针并不指向address_space的实例，而是指
			向另一个数据结构anon_vma，该结构对匿名页的逆向映射很重要。该指针的双重使用是
			可能的，因为address_space实例总是对齐到sizeof(long)，因此，在linux支持的所有
			计算机上，指向该实例的指针最低位总是0*/
			struct address_space *mapping;
	    };
#if NR_CPUS >= CONFIG_SPLIT_PTLOCK_CPUS
	    spinlock_t ptl;
#endif
		/*slub：指向slab*/
	    struct kmem_cache *slab;
		/*复合页的尾页。内核可以将多个毗连的页合并为一个较大的复合页（compound page）。分组中
		第一个页称作首页		（head page）而其余的各页叫做尾页（tail page），所有尾页对应的page实例
		中，都将first_page		设置为指向首页*/
	    struct page *first_page;
	};
	union
	{
		/*页帧在映射内部的偏移量*/
		pgoff_t index;
		/**/
		void *freelist;		/* SLUB: freelist req. slab lock */
	};
	/*lru是一个表头，用于在各种链表上维护该页，以便将页按不同类别分组，最重要的类别时活动和
	不活动页。页为复合页时，前域表示分配阶，后域指向页面释放函数，*/
	struct list_head lru;
	/*
	 * On machines where all RAM is mapped into kernel address space,
	 * we can simply calculate the virtual address. On machines with
	 * highmem some memory is mapped into kernel virtual memory
	 * dynamically, so we need a place to store that address.
	 * Note that this field could be 16 bits on x86 ... ;)
	 *
	 * Architectures with slow multiplication can define
	 * WANT_PAGE_VIRTUAL in asm/page.h
	 */
#if defined(WANT_PAGE_VIRTUAL)
	/*virtual用于高端内存区域中的页，存储该页的虚拟地址，如果页面没有被映射则为空。m68k等几个
	体系结构使用，所有其它体系结构都采用了一种不同的方案来寻址虚拟内存页，其核心是用来查找所有
	高端内存页帧的散列表*/
	void *virtual;
#endif /* WANT_PAGE_VIRTUAL */
};

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
 /*虚拟内存域。描述的是虚拟地址空间中[vm_start, vm_end)之间左闭右开区域属性*/
struct vm_area_struct
{
	/*所属虚拟地址空间，即该区域所属的mm_struct实例*/
	struct mm_struct * vm_mm;
	/*该虚拟内存域的起始地址*/
	unsigned long vm_start;
	/*在该虚拟内存域结束地址之后的第一个字节的地址*/
	unsigned long vm_end;
	/*进程所有vm_area_struct实例的链表是通过vm_next实现的，按地址排序*/
	struct vm_area_struct *vm_next;
	/*该虚拟内存域的访问权限。VMA 由许多的虚拟页 (page) 组成，每个虚拟页需要经过页表的
	转换才能找到对应的物理页面。页表中关于内存页的访问权限就是由 vm_page_prot 决定的*/
	pgprot_t vm_page_prot;
	/*存储了定义区域性质的标志，如VM_READ/VM_WRITE/VM_EXEC等<mm.h>中声明的预处理器常数
	。偏向于定于整个虚拟内存区域的访问权限以及行为规范。描述的是虚拟内存区域中的整体信
	息，而不是虚拟内存区域中具体的某个独立页面。它是一个抽象的概念。可以通过
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags) 实现到具体页面访问权限
	vm_page_prot 的转换。
		 比如代码段这块内存区域的权限是可读，可执行，但是不可写。数据段具有可读可写的权
	限但是不可执行。堆则具有可读可写，可执行的权限（Java 中的字节码存储在堆中，所以需要
	可执行权限），栈一般是可读可写的权限，一般很少有可执行权限。而文件映射与匿名映射区
	存放了共享链接库，所以也需要可执行的权限*/
	unsigned long vm_flags;
	/*用红黑树结点将该虚拟内存域集成到红黑树中*/
	struct rb_node vm_rb;
	/*从文件到进程的虚拟地址空间中的映射，可以通过文件中的区间和内存中对应的区域唯一地
	确定，为跟踪与进程关联的所有区间，内核使用了如上所述的链表和红黑树，但还必须能够反
	向查询：给出文件中的一个区间，内核有时需要知道该区间映射到的所有进程，这种映射成为
	共享映射（shared mapping），至于这种映射的必要性，看看系统中几乎每个进程都是用的C标
	准库，读者就知道了，为提供所需的信息，所有的vm_area_struct实例都还通过一个优先树管
	理，包含在shared成员中。对于有地址空间和后备存储器的区域来说，shared链接到
	address_space—>i_mmap优先树；或连接到悬挂在优先树结点之外，类似的一组虚拟内存区域
	的链表；或连接到address_space->i_mmap_nonlinear链表中的虚拟内存区域*/
	union
	{
		struct
		{
			/**/
			struct list_head list;
			/*与prio_tree_node的prio_tree_parent成员在内存中位于同一位置*/
			void *parent;
			/**/
			struct vm_area_struct *head;
		} vm_set;
		/**/
		struct raw_prio_tree_node prio_tree_node;
	} shared;

	 /*在文件的某一页经过写时复制之后，文件的MAP_PRIVATE虚拟内存域可能同时在i_mmap树和
	 anon_vma链表中，MAP_SHARED虚拟内存域只能在i_mmap树中，匿名的MAP_PRIVATE、栈或堆虚
	 拟内存域（file指针为空）只能处于anon_vma链表中*/
	 /*对该成员的访问通过anon_vma->lock串行化*/
	 /*anon_vma_node和anon_vma用于管理原子匿名映射（anonymous mapping）的共享页，指向
	 相同	 页的映射都保存在一个双链表上，anon_vma_node充当链表元素，有若干此类链表，具体
	 的数目取	 决于共享物理内存页的映射集合的数目，anon_vma成员是一个指向与各链表关联的
	 管理结构的指针该管理结构由一个表头和相关的锁组成*/
	struct list_head anon_vma_node;
	/*对该成员的访问通过page_table_lock串行化*/
	struct anon_vma *anon_vma;
	/*处理该结构的各个函数指针，用于在该区域上执行各种标准操作*/
	struct vm_operations_struct * vm_ops;
	/*指定了vm_file内的偏移量，该值御用只映射了文件部分内容时（如果映射了整个文件，则
	偏移量为0），PAGE_SIZE对齐而非PAGE_CACHE_SIZE对齐*/
	unsigned long vm_pgoff;
	/*被映射的文件，可能为空*/
	struct file * vm_file;
	/*取决于映射类型，vm_private_data可用于存储私有数据，不用通用内存管理例程操作，内
	核只确保在创建新区域时该成员初始化为空指针。当前，只有少数声音和视频驱动程序使用了
	该选项*/
	void * vm_private_data;
	/*截断数目或者重新起始地址*/
	unsigned long vm_truncate_count;

#ifndef CONFIG_MMU
	/*没有定义MMU时共享vmas的引用计数*/
	atomic_t vm_usage;
#endif
#ifdef CONFIG_NUMA
	/*NUMA管理VMA策略*/
	struct mempolicy *vm_policy;
#endif
};

/*进程的虚拟地址空间*/
struct mm_struct
{
	/*连接该虚拟地址空间中所有的struct vm_aera_struct实例*/
	struct vm_area_struct * mmap;
	/*红黑树用于管理该虚拟地址空间中所有vm_area_struct实例*/
	struct rb_root mm_rb;
	/*保存上次find_vma的结果*/
	struct vm_area_struct * mmap_cache;
	/*在内存映射区域为新映射找到适当位置*/
	unsigned long (*get_unmapped_area) (struct file *filp,
				unsigned long addr, unsigned long len,
				unsigned long pgoff, unsigned long flags);
	/**/
	void (*unmap_area) (struct mm_struct *mm, unsigned long addr);
	/*虚拟地址空间中用于内存映射的起始地址，该地址通常设置为TASK_UNMAPPED_BASE，每个
	体系结构都需要定义，几乎所有的情况下，其值都是TASK_SIZE/3，要注意，如果使用内核
	的默认配置，则mmap区域的起始点不是随机的*/
	/*内存映射段(mmap)。     此处，内核将硬盘文件的内容直接映射到内存, 任何应用程序都可通过
	mmap系统调用请求这种映射。内存映射是一种方便高效的文件I/O方式，因而被用于装载动态
	共享库。用户也可创建匿名内存映射，该映射没有对应的文件, 可用于存放程序数据。若通
	过malloc请求一大块内存，C运行库将创建一个匿名内存映射，而不使用堆内存。”大块” 意
	味着比阈值MMAP_THRESHOLD还大，缺省为128KB，可通过mallopt调整。     该区域用于映射可执
	行文件用到的动态链接库。若可执行文件依赖共享库，则系统会为这些动态库分配相应空间，
	并在程序装载时将其载入。在Linux 2.6内核中，共享库的起始地址被往上移动至更靠近栈区
	的位置。     从进程地址空间的布局可以看到，在有共享库的情况下，留给堆的可用空间还有两
	处：一处是从.bss段到0x40000000，约不到1GB的空间；另一处是从共享库到栈之间的空间，
	约不到2GB。这两块空间大小取决于栈、共享库的大小和数量。这样来看，是否应用程序可申
	请的最大堆空间只有2GB？事实上，这与Linux内核版本有关。在上面给出的进程地址空间经
	典布局图中，共享库的装载地址为0x40000000，这实际上是Linux kernel 2.6版本之前的情
	况了，在2.6版本里，共享库的装载地址已经被挪到靠近栈的位置，即位于0xBFxxxxxx附近，
	因此，此时的堆范围就不会被共享库分割成2个“碎片”，故kernel 2.6的32位Linux系统中，
	malloc申请的最大内存理论值在2.9GB左右*/
	unsigned long mmap_base;
	/*进程虚拟内存空间的长度，TASK_SIZE*/
	unsigned long task_size;
	/*if non-zero, the largest hole below free_area_cache*/
	/*如果该值非零，最大的空洞小于free_area_cache*/
	unsigned long cached_hole_size;
	/*first hole of size cached_hole_size or larger*/
	/**/
	unsigned long free_area_cache;
	/*页目录表指针*/
	pgd_t * pgd;
	/*使用该虚拟地址空间的用户数目*/
	atomic_t mm_users;
	/*该虚拟地址空间被引用的次数，uses也算一个*/
	atomic_t mm_count;
	/*该虚拟地址空间中vm_area_struct的数目*/
	int map_count;
	/*保护该虚拟地址空间操作的读写信号量*/
	struct rw_semaphore mmap_sem;
	/*保护页表和一些计数器*/
	spinlock_t page_table_lock;
	/*连接到全局init_mm.mmlist链表，受mmlist_lock保护*/
	struct list_head mmlist;
	/*特别的计数器，在一些配置中有page_table_lock保护，其它配置为原子变量*/
	/*文件常驻内存集长度*/
	mm_counter_t _file_rss;
	/*匿名映射常驻内存集长度*/
	mm_counter_t _anon_rss;
	/*高水印时常驻内存集大小*/
	unsigned long hiwater_rss;
	/*高水印时虚拟内存使用大小*/
	unsigned long hiwater_vm;
	/*进程虚拟地址空间中总共与物理内存映射的页的总数。注意映射这个概念，它表示只是将
	虚拟内存与物理内存建立关联关系，并不代表真正的分配物理内存*/
	unsigned long total_vm;
	/*当内存吃紧的时候，有些页可以换出到硬盘上，而有些页因为比较重要，不能换出，该变
	量就是被锁定不能换出的内存页总数*/
	unsigned long locked_vm;
	/*共享的内存页数目*/
	unsigned long shared_vm,;
	/*可执行代码占用内存页的数目*/
	unsigned long exec_vm;
	/*栈中所映射的的内存页的数目*/
	unsigned long stack_vm;
	/*预留的虚拟内存长度*/
	unsigned long reserved_vm;
	/*默认的分配标识*/
	unsigned long def_flags;
	/*页表使用的页数目*/
	unsigned long nr_ptes;
	/*在ELF二进制文件映射到地址空间中之后，代码段和数据段的长度将不再改变*/
	/*可执行代码在虚拟地址空间中的开始地址，text段如何映射到虚拟地址空间中由ELF标准确定，每个
	体系结构都指定了一个特定的起始地址，IA-32系统起始于0x08048000，在text段的起始地址与最低的
	可用地址空间之间有大约128MB的间距，用于捕获NULL指针*/
	/*代码段也称正文段或文本段，通常用于存放程序执行代码(即CPU执行的机器指令)。一般
	C语言执行语句都编译成机器代码保存在代码段。通常代码段是可共享的，因此频繁执行的
	程序只需要在内存中拥有一份拷贝即可。代码段通常属于只读，以防止其他程序意外地修改
	其指令(对该段的写操作将导致段错误)。某些架构也允许代码段为可写，即允许修改程序。
	代码段指令根据程序设计流程依次执行，对于顺序指令，只会执行一次(每个进程)；若有反
	复，则需使用跳转指令；若进行递归，则需要借助栈来实现。代码段指令中包括操作码和操
	作对象(或对象地址引用)。若操作对象是立即数(具体数值)，将直接包含在代码中；若是局
	部数据，将在栈区分配空间，然后引用该数据地址；若位于BSS段和数据段，同样引用该数据
	地址。代码段最容易受优化措施影响。*/
	unsigned long start_code;
	/*可执行代码在虚拟地址空间中的结束地址*/
	unsigned long end_code;
	/*数据段通常用于存放程序中已初始化且初值不为0的全局变量和静态局部变量。数据段属于
	静态内存分配(静态存储区)，可读可写。数据段保存在目标文件中(在嵌入式系统里一般固化
	在镜像文件中)，其内容由程序初始化。例如，对于全局变量int gVar = 10，必须在目标文
	件数据段中保存10这个数据，然后在程序加载时复制到相应的内存。数据段与BSS段的区别如
	下：   1) BSS段不占用物理文件尺寸，但占用内存空间；数据段占用物理文件，也占用内存空
	间。   对于大型数组如int ar0[10000] = {1, 2, 3, ...}和int ar1[10000]，ar1放在BSS段，
	只记录共有10000*4个字节需要初始化为0，而不是像ar0那样记录每个数据1、2、3...，此时
	BSS为目标文件所节省的磁盘空间相当可观。2) 当程序读取数据段的数据时，系统会出发缺
	页故障，从而分配相应的物理内存；当程序读取BSS段的数据时，内核会将其转到一个全零页
	，不会发生缺页故障，也不会为其分配相应的物理内存。运行时数据段和BSS段的整个区段通
	常称为数据区。某些资料中“数据段”指代数据段 + BSS段 + 堆。*/
	/*已初始化数据在虚拟地址空间中的起始地址*/
	unsigned long start_data;
	/*已初始化数据在虚拟地址空间中的结束地址*/
	unsigned long end_data;
	/*BSS(Block Started by Symbol)段中通常存放程序中以下符号：未初始化的全局变量和静
	态局部变量、初始值为0的全局变量和静态局部变量(依赖于编译器实现)、未定义且初值不为
	0的符号(该初值即common block的大小)C语言中，未显式初始化的静态分配变量被初始化为0
	(算术类型)或空指针(指针类型)。由于程序加载时，BSS会被操作系统清零，所以未赋初值或
	初值为0的全局变量都在BSS中。BSS段仅为未初始化的静态分配变量预留位置，在目标文件中
	并不占据空间，这样可减少目标文件体积。但程序运行时需为变量分配内存空间，故目标文
	件必须记录所有未初始化的静态分配变量大小总和(通过start_bss和end_bss地址写入机器代
	码)。当加载器(loader)加载程序时，将为BSS段分配的内存初始化为0。在嵌入式软件中，进
	入main()函数之前BSS段被C运行时系统映射到初始化为全零的内存(效率较高)。     注意，尽管
	均放置于BSS段，但初值为0的全局变量是强符号，而未初始化的全局变量是弱符号。若其他
	地方已定义同名的强符号(初值可能非0)，则弱符号与之链接时不会引起重定义错误，但运行
	时的初值可能并非期望值(会被强符号覆盖)。因此，定义全局变量时，若只有本文件使用，
	则尽量使用static关键字修饰；否则需要为全局变量定义赋初值(哪怕0值)，保证该变量为强
	符号，以便链接时发现变量名冲突，而不是被未知值覆盖。某些编译器将未初始化的全局变
	量保存在common段，链接时再将其放入BSS段。在编译阶段可通过-fno-common选项来禁止将
	未初始化的全局变量放入common段。*/
	/*堆用于存放进程运行时动态分配的内存段，可动态扩张或缩减。堆中内容是匿名的，不能
	按名字直接访问，只能通过指针间接访问。当进程调用malloc等函数分配内存时，新分配的
	内存动态添加到堆上(扩张)；当调用free等函数释放内存时，被释放的内存从堆中剔除(缩减
	)。分配的堆内存是经过字节对齐的空间，以适合原子操作。堆管理器通过链表管理每个申请
	的内存，由于堆申请和释放是无序的，最终会产生内存碎片。堆内存一般由应用程序分配释
	放，回收的内存可供重新使用。若程序员不释放，程序结束时操作系统可能会自动回收。堆
	的末端由brk指针标识，当堆管理器需要更多内存时，可通过系统调用brk和sbrk来移动brk指
	针以扩张堆，一般由系统自动调用。     使用堆时经常出现两种问题：1)释放或改写仍在使用的
	内存(“内存破坏”)；2)未释放不再使用的内存(“内存泄漏”)。当释放次数少于申请次数时，
	可能已造成内存泄漏。泄漏的内存往往比忘记释放的数据结构更大，因为所分配的内存通常
	会圆整为下个大于申请数量的2的幂次(如申请212B，会圆整为256B)*/
	/*堆在虚拟地址空间中的起始地址，堆紧接着text段开始，向上增长*/
	unsigned long start_brk;
	/*堆结束地址*/
	unsigned long brk;
	/*栈和堆的区别：①管理方式：栈由编译器自动管理；堆由程序员控制，使用方便，但易产生
	内存泄露。②生长方向：栈向低地址扩展(即”向下生长”)，是连续的内存区域；堆向高地址扩
	展(即”向上生长”)，是不连续的内存区域。这是由于系统用链表来存储空闲内存地址，自然
	不连续，而链表从低地址向高地址遍历。③空间大小：栈顶地址和栈的最大容量由系统预先规
	定(通常默认2M或10M)；堆的大小则受限于计算机系统中有效的虚拟内存，32位Linux系统中
	堆内存可达2.9G空间。④存储内容：栈在函数调用时，首先压入主调函数中下条指令(函数调
	用语句的下条可执行语句)的地址，然后是函数实参，然后是被调函数的局部变量。本次调用
	结束后，局部变量先出栈，然后是参数，最后栈顶指针指向最开始存的指令地址，程序由该
	点继续运行下条可执行语句。堆通常在头部用一个字节存放其大小，堆用于存储生存期与函
	数调用无关的数据，具体内容由程序员安排。⑤分配方式：栈可静态分配或动态分配。静态分
	配由编译器完成，如局部变量的分配。动态分配由alloca函数在栈上申请空间，用完后自动
	释放。堆只能动态分配且手工释放。⑥分配效率：栈由计算机底层提供支持：分配专门的寄存
	器存放栈地址，压栈出栈由专门的指令执行，因此效率较高。堆由函数库提供，机制复杂，
	效率比栈低得多。⑦分配后系统响应：只要栈剩余空间大于所申请空间，系统将为程序提供内
	存，否则报告异常提示栈溢出。     操作系统为堆维护一个记录空闲内存地址的链表。当系统收
	到程序的内存分配申请时，会遍历该链表寻找第一个空间大于所申请空间的堆结点，然后将
	该结点从空闲结点链表中删除，并将该结点空间分配给程序。若无足够大小的空间(可能由于
	内存碎片太多)，有可能调用系统功能去增加程序数据段的内存空间，以便有机会分到足够大
	小的内存，然后进行返回。大多数系统会在该内存空间首地址处记录本次分配的内存大小，
	供后续的释放函数(如free/delete)正确释放本内存空间。     此外，由于找到的堆结点大小不一
	定正好等于申请的大小，系统会自动将多余的部分重新放入空闲链表中。⑧碎片问题：栈不会
	存在碎片问题，因为栈是后进先出的队列，内存块弹出栈之前，在其上面的后进的栈内容已
	弹出。而频繁申请释放操作会造成堆内存空间的不连续，从而造成大量碎片，使程序效率降
	低。   可见，堆容易造成内存碎片；由于没有专门的系统支持，效率很低；由于可能引发用户
	态和内核态切换，内存申请的代价更为昂贵。所以栈在程序中应用最广泛，函数调用也利用
	栈来完成，调用过程中的参数、返回地址、栈基指针和局部变量等都采用栈的方式存放。所
	以，建议尽量使用栈，仅在分配大量或大块内存空间时使用堆。     使用栈和堆时应避免越界发
	生，否则可能程序崩溃或破坏程序堆、栈结构，产生意想不到的后果*/
	/*栈又称堆栈，由编译器自动分配释放，行为类似数据结构中的栈(先进后出)。堆栈主要有
	三个用途：（1）为函数内部声明的非静态局部变量(C语言中称“自动变量”)提供存储空间。
	（2）记录函数调用过程相关的维护性信息，称为栈帧(Stack Frame)或过程活动记录
	(Procedure Activation Record)。它包括函数返回地址，不适合装入寄存器的函数参数及一
	些寄存器值的保存。除递归调用外，堆栈并非必需。因为编译时可获知局部变量，参数和返
	回地址所需空间，并将其分配于BSS段（3）临时存储区，用于暂存算术表达式部分计算结果
	或alloca函数分配的栈内内存。持续地重用栈空间有助于使活跃的栈内存保持在CPU缓存中，
	从而加速访问。进程中的每个线程都有属于自己的栈。向栈中不断压入数据时，若超出其容
	量就会耗尽栈对应的内存区域，从而触发一个页错误。此时若栈的大小低于堆栈最大值
	RLIMIT_STACK(通常是8M)，则栈会动态增长，程序继续运行。映射的栈区扩展到所需大小后
	，不再收缩。ulimit -s命令可查看和设置堆栈最大值，当程序使用的堆栈超过该值时, 发生
	栈溢出(Stack Overflow)，程序收到一个段错误(Segmentation Fault)。注意，调高堆栈容
	量可能会增加内存开销和启动时间。     堆栈既可向下增长(向内存低地址)也可向上增长, 这依
	赖于具体的实现。栈的大小在运行时由内核动态调整*/
	/*栈在虚拟地址空间中的起始地址，栈起始于STACK_TOP，如果设置了PF_RANDOMIZE，则起始点会减少
	一个小的随机量，每个体系结构都必须定义STACK_TOP，大多都设置为TASK_SIZE，即用户地址空间中
	最高的可用地址空间，进程的参数列表和环境变量都是栈的起始数据*/
	unsigned long start_stack;
	/*参数列表在虚拟地址空间中的起始地址*/
	unsigned long arg_start;
	/*参数列表在虚拟地址空间中的结束地址*/
	unsigned long arg_end;
	/*环境变量在虚拟地址空间中的起始地址*/
	unsigned long env_start;
	/*环境变量在虚拟地址空间中的结束地址*/
	unsigned long env_end;
	/**/
	unsigned long saved_auxv[AT_VECTOR_SIZE]; /* for /proc/PID/auxv */
	/**/
	cpumask_t cpu_vm_mask;

	/**/
	/* Architecture-specific MM context */
	mm_context_t context;

	/* Swap token stuff */
	/*
	 * Last value of global fault stamp as seen by this process.
	 * In other words, this value gives an indication of how long
	 * it has been since this task got the token.
	 * Look at mm/thrash.c
	 */
	/*内核上一次试图获取令牌时的global_faults值*/
	unsigned int faultstamp;
	/*与令牌交换相关的调度优先级，用于控制对交换令牌的访问*/
	unsigned int token_priority;
	/*该进程等待交换令牌的时间间隔的长度*/
	unsigned int last_interval;
	/*必须用原子操作来访问这些比特位*/
	unsigned long flags;

	/*内存转储支持相关*/
	int core_waiters;
	/*内核转储开始和完成的完成量*/
	struct completion *core_startup_done, core_done;

	/*异步io位*/
	rwlock_t		ioctx_list_lock;
	/**/
	struct kioctx		*ioctx_list;
};

#endif /* _LINUX_MM_TYPES_H */
