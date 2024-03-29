#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/fs.h>
#include <linux/mqueue.h>

#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/desc.h>

/*初始化进程已装载的文件系统*/
static struct fs_struct init_fs = INIT_FS;
/*初始化进程已打开的文件信息*/
static struct files_struct init_files = INIT_FILES;
/*初始化进程的信号描述符*/
static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
/*初始化进程的信号处理函数相关信息*/
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);
/*初始化进程虚拟内存地址空间信息*/
struct mm_struct init_mm = INIT_MM(init_mm);
EXPORT_SYMBOL(init_mm);

/*
 * Initial thread structure.
 *
 * We need to make sure that this is THREAD_SIZE aligned due to the
 * way process stacks are handled. This is done by having a special
 * "init_task" linker map entry..
 */
 /*初始化进程的线程低层信息，存储在“.data.init_task”数据段中*/
union thread_union init_thread_union
	__attribute__((__section__(".data.init_task"))) = { INIT_THREAD_INFO(init_task) };

/*
 * Initial task structure.
 *
 * All other task structs will be allocated on slabs in fork.c
 */
 /*初始化进程的初始化*/
struct task_struct init_task = INIT_TASK(init_task);
EXPORT_SYMBOL(init_task);

/*
 * per-CPU TSS segments. Threads are completely 'soft' on Linux,
 * no more per-task TSS's. The TSS size is kept cacheline-aligned
 * so they are allowed to end up in the .data.cacheline_aligned
 * section. Since TSS's are completely CPU-local, we want them
 * on exact cacheline boundaries, to eliminate cacheline ping-pong.
 */
DEFINE_PER_CPU_SHARED_ALIGNED(struct tss_struct, init_tss) = INIT_TSS;

