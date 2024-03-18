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
	/*��ϵ�ṹ�޹صı�־����������ҳ�����ԣ������첽����*/
	unsigned long flags;
	/*�ں������ø�ҳ�Ĵ�����ֵΪ0ʱ���ں�֪��struct pageʵ����ǰ��ʹ�ã�����ɾ��*/
	atomic_t _count;
	union
	{
		/*��ʾ��ҳ�����ж�����ָ���ҳ*/
		atomic_t _mapcount;
		/*slub�ж������Ŀ*/
		unsigned int inuse;	/* SLUB: Nr of objects */
	};
	union
	{
	    struct
		{
			/*flags==PagePrivateʱ��ʾbuffer_head��flags==PageSwapCache��ʾswp_entry_t��
			flags==PageBuddy	ʱ��ʾ�����*/
			unsigned long private;
			/*��λδ��λʱ��ʾinode��ַ�ռ���߿գ�����ָ��anon_vma����*/
			/*mappingָ����ҳ֡���ڵĵ�ַ�ռ䣬indexʱҳ֡��ӳ���ڲ���ƫ��������ַ�ռ���
			�ڽ��ļ���������װ�����ݵ��ڴ�������������ͨ��һ��С���ɣ�mapping�����ܹ�����
			һ��ָ�룬���һ��ܰ���һЩ�������Ϣ�������ж�ҳ�Ƿ�����δ��������ַ�ռ��ĳ
			�������ڴ�ҳ�������mapping��Ϊ1�����ָ�벢��ָ��address_space��ʵ��������ָ
			����һ�����ݽṹanon_vma���ýṹ������ҳ������ӳ�����Ҫ����ָ���˫��ʹ����
			���ܵģ���Ϊaddress_spaceʵ�����Ƕ��뵽sizeof(long)����ˣ���linux֧�ֵ�����
			������ϣ�ָ���ʵ����ָ�����λ����0*/
			struct address_space *mapping;
	    };
#if NR_CPUS >= CONFIG_SPLIT_PTLOCK_CPUS
	    spinlock_t ptl;
#endif
		/*slub��ָ��slab*/
	    struct kmem_cache *slab;
		/*����ҳ��βҳ���ں˿��Խ����������ҳ�ϲ�Ϊһ���ϴ�ĸ���ҳ��compound page����������
		��һ��ҳ������ҳ		��head page��������ĸ�ҳ����βҳ��tail page��������βҳ��Ӧ��pageʵ��
		�У�����first_page		����Ϊָ����ҳ*/
	    struct page *first_page;
	};
	union
	{
		/*ҳ֡��ӳ���ڲ���ƫ����*/
		pgoff_t index;
		/**/
		void *freelist;		/* SLUB: freelist req. slab lock */
	};
	/*lru��һ����ͷ�������ڸ���������ά����ҳ���Ա㽫ҳ����ͬ�����飬����Ҫ�����ʱ���
	���ҳ��ҳΪ����ҳʱ��ǰ���ʾ����ף�����ָ��ҳ���ͷź�����*/
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
	/*virtual���ڸ߶��ڴ������е�ҳ���洢��ҳ�������ַ�����ҳ��û�б�ӳ����Ϊ�ա�m68k�ȼ���
	��ϵ�ṹʹ�ã�����������ϵ�ṹ��������һ�ֲ�ͬ�ķ�����Ѱַ�����ڴ�ҳ���������������������
	�߶��ڴ�ҳ֡��ɢ�б�*/
	void *virtual;
#endif /* WANT_PAGE_VIRTUAL */
};

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
 /*�����ڴ����������������ַ�ռ���[vm_start, vm_end)֮������ҿ���������*/
struct vm_area_struct
{
	/*���������ַ�ռ䣬��������������mm_structʵ��*/
	struct mm_struct * vm_mm;
	/*�������ڴ������ʼ��ַ*/
	unsigned long vm_start;
	/*�ڸ������ڴ��������ַ֮��ĵ�һ���ֽڵĵ�ַ*/
	unsigned long vm_end;
	/*��������vm_area_structʵ����������ͨ��vm_nextʵ�ֵģ�����ַ����*/
	struct vm_area_struct *vm_next;
	/*�������ڴ���ķ���Ȩ�ޡ�VMA �����������ҳ (page) ��ɣ�ÿ������ҳ��Ҫ����ҳ����
	ת�������ҵ���Ӧ������ҳ�档ҳ���й����ڴ�ҳ�ķ���Ȩ�޾����� vm_page_prot ������*/
	pgprot_t vm_page_prot;
	/*�洢�˶����������ʵı�־����VM_READ/VM_WRITE/VM_EXEC��<mm.h>��������Ԥ����������
	��ƫ���ڶ������������ڴ�����ķ���Ȩ���Լ���Ϊ�淶���������������ڴ������е�������
	Ϣ�������������ڴ������о����ĳ������ҳ�档����һ������ĸ������ͨ��
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags) ʵ�ֵ�����ҳ�����Ȩ��
	vm_page_prot ��ת����
		 ������������ڴ������Ȩ���ǿɶ�����ִ�У����ǲ���д�����ݶξ��пɶ���д��Ȩ
	�޵��ǲ���ִ�С�������пɶ���д����ִ�е�Ȩ�ޣ�Java �е��ֽ���洢�ڶ��У�������Ҫ
	��ִ��Ȩ�ޣ���ջһ���ǿɶ���д��Ȩ�ޣ�һ������п�ִ��Ȩ�ޡ����ļ�ӳ��������ӳ����
	����˹������ӿ⣬����Ҳ��Ҫ��ִ�е�Ȩ��*/
	unsigned long vm_flags;
	/*�ú������㽫�������ڴ��򼯳ɵ��������*/
	struct rb_node vm_rb;
	/*���ļ������̵������ַ�ռ��е�ӳ�䣬����ͨ���ļ��е�������ڴ��ж�Ӧ������Ψһ��
	ȷ����Ϊ��������̹������������䣬�ں�ʹ�������������������ͺ���������������ܹ���
	���ѯ�������ļ��е�һ�����䣬�ں���ʱ��Ҫ֪��������ӳ�䵽�����н��̣�����ӳ���Ϊ
	����ӳ�䣨shared mapping������������ӳ��ı�Ҫ�ԣ�����ϵͳ�м���ÿ�����̶����õ�C��
	׼�⣬���߾�֪���ˣ�Ϊ�ṩ�������Ϣ�����е�vm_area_structʵ������ͨ��һ����������
	����������shared��Ա�С������е�ַ�ռ�ͺ󱸴洢����������˵��shared���ӵ�
	address_space��>i_mmap�������������ӵ����������������֮�⣬���Ƶ�һ�������ڴ�����
	�������������ӵ�address_space->i_mmap_nonlinear�����е������ڴ�����*/
	union
	{
		struct
		{
			/**/
			struct list_head list;
			/*��prio_tree_node��prio_tree_parent��Ա���ڴ���λ��ͬһλ��*/
			void *parent;
			/**/
			struct vm_area_struct *head;
		} vm_set;
		/**/
		struct raw_prio_tree_node prio_tree_node;
	} shared;

	 /*���ļ���ĳһҳ����дʱ����֮���ļ���MAP_PRIVATE�����ڴ������ͬʱ��i_mmap����
	 anon_vma�����У�MAP_SHARED�����ڴ���ֻ����i_mmap���У�������MAP_PRIVATE��ջ�����
	 ���ڴ���fileָ��Ϊ�գ�ֻ�ܴ���anon_vma������*/
	 /*�Ըó�Ա�ķ���ͨ��anon_vma->lock���л�*/
	 /*anon_vma_node��anon_vma���ڹ���ԭ������ӳ�䣨anonymous mapping���Ĺ���ҳ��ָ��
	 ��ͬ	 ҳ��ӳ�䶼������һ��˫�����ϣ�anon_vma_node�䵱����Ԫ�أ������ɴ�������������
	 ����Ŀȡ	 ���ڹ��������ڴ�ҳ��ӳ�伯�ϵ���Ŀ��anon_vma��Ա��һ��ָ���������������
	 �����ṹ��ָ��ù����ṹ��һ����ͷ����ص������*/
	struct list_head anon_vma_node;
	/*�Ըó�Ա�ķ���ͨ��page_table_lock���л�*/
	struct anon_vma *anon_vma;
	/*�����ýṹ�ĸ�������ָ�룬�����ڸ�������ִ�и��ֱ�׼����*/
	struct vm_operations_struct * vm_ops;
	/*ָ����vm_file�ڵ�ƫ��������ֵ����ֻӳ�����ļ���������ʱ�����ӳ���������ļ�����
	ƫ����Ϊ0����PAGE_SIZE�������PAGE_CACHE_SIZE����*/
	unsigned long vm_pgoff;
	/*��ӳ����ļ�������Ϊ��*/
	struct file * vm_file;
	/*ȡ����ӳ�����ͣ�vm_private_data�����ڴ洢˽�����ݣ�����ͨ���ڴ�������̲�������
	��ֻȷ���ڴ���������ʱ�ó�Ա��ʼ��Ϊ��ָ�롣��ǰ��ֻ��������������Ƶ��������ʹ����
	��ѡ��*/
	void * vm_private_data;
	/*�ض���Ŀ����������ʼ��ַ*/
	unsigned long vm_truncate_count;

#ifndef CONFIG_MMU
	/*û�ж���MMUʱ����vmas�����ü���*/
	atomic_t vm_usage;
#endif
#ifdef CONFIG_NUMA
	/*NUMA����VMA����*/
	struct mempolicy *vm_policy;
#endif
};

/*���̵������ַ�ռ�*/
struct mm_struct
{
	/*���Ӹ������ַ�ռ������е�struct vm_aera_structʵ��*/
	struct vm_area_struct * mmap;
	/*��������ڹ����������ַ�ռ�������vm_area_structʵ��*/
	struct rb_root mm_rb;
	/*�����ϴ�find_vma�Ľ��*/
	struct vm_area_struct * mmap_cache;
	/*���ڴ�ӳ������Ϊ��ӳ���ҵ��ʵ�λ��*/
	unsigned long (*get_unmapped_area) (struct file *filp,
				unsigned long addr, unsigned long len,
				unsigned long pgoff, unsigned long flags);
	/**/
	void (*unmap_area) (struct mm_struct *mm, unsigned long addr);
	/*�����ַ�ռ��������ڴ�ӳ�����ʼ��ַ���õ�ַͨ������ΪTASK_UNMAPPED_BASE��ÿ��
	��ϵ�ṹ����Ҫ���壬�������е�����£���ֵ����TASK_SIZE/3��Ҫע�⣬���ʹ���ں�
	��Ĭ�����ã���mmap�������ʼ�㲻�������*/
	/*�ڴ�ӳ���(mmap)��     �˴����ں˽�Ӳ���ļ�������ֱ��ӳ�䵽�ڴ�, �κ�Ӧ�ó��򶼿�ͨ��
	mmapϵͳ������������ӳ�䡣�ڴ�ӳ����һ�ַ����Ч���ļ�I/O��ʽ�����������װ�ض�̬
	�����⡣�û�Ҳ�ɴ��������ڴ�ӳ�䣬��ӳ��û�ж�Ӧ���ļ�, �����ڴ�ų������ݡ���ͨ
	��malloc����һ����ڴ棬C���п⽫����һ�������ڴ�ӳ�䣬����ʹ�ö��ڴ档����顱 ��
	ζ�ű���ֵMMAP_THRESHOLD����ȱʡΪ128KB����ͨ��mallopt������     ����������ӳ���ִ
	���ļ��õ��Ķ�̬���ӿ⡣����ִ���ļ����������⣬��ϵͳ��Ϊ��Щ��̬�������Ӧ�ռ䣬
	���ڳ���װ��ʱ�������롣��Linux 2.6�ں��У����������ʼ��ַ�������ƶ���������ջ��
	��λ�á�     �ӽ��̵�ַ�ռ�Ĳ��ֿ��Կ��������й����������£������ѵĿ��ÿռ仹����
	����һ���Ǵ�.bss�ε�0x40000000��Լ����1GB�Ŀռ䣻��һ���Ǵӹ����⵽ջ֮��Ŀռ䣬
	Լ����2GB��������ռ��Сȡ����ջ��������Ĵ�С�������������������Ƿ�Ӧ�ó������
	������ѿռ�ֻ��2GB����ʵ�ϣ�����Linux�ں˰汾�йء�����������Ľ��̵�ַ�ռ侭
	�䲼��ͼ�У��������װ�ص�ַΪ0x40000000����ʵ������Linux kernel 2.6�汾֮ǰ����
	���ˣ���2.6�汾��������װ�ص�ַ�Ѿ���Ų������ջ��λ�ã���λ��0xBFxxxxxx������
	��ˣ���ʱ�Ķѷ�Χ�Ͳ��ᱻ������ָ��2������Ƭ������kernel 2.6��32λLinuxϵͳ�У�
	malloc���������ڴ�����ֵ��2.9GB����*/
	unsigned long mmap_base;
	/*���������ڴ�ռ�ĳ��ȣ�TASK_SIZE*/
	unsigned long task_size;
	/*if non-zero, the largest hole below free_area_cache*/
	/*�����ֵ���㣬���Ŀն�С��free_area_cache*/
	unsigned long cached_hole_size;
	/*first hole of size cached_hole_size or larger*/
	/**/
	unsigned long free_area_cache;
	/*ҳĿ¼��ָ��*/
	pgd_t * pgd;
	/*ʹ�ø������ַ�ռ���û���Ŀ*/
	atomic_t mm_users;
	/*�������ַ�ռ䱻���õĴ�����usesҲ��һ��*/
	atomic_t mm_count;
	/*�������ַ�ռ���vm_area_struct����Ŀ*/
	int map_count;
	/*�����������ַ�ռ�����Ķ�д�ź���*/
	struct rw_semaphore mmap_sem;
	/*����ҳ����һЩ������*/
	spinlock_t page_table_lock;
	/*���ӵ�ȫ��init_mm.mmlist��������mmlist_lock����*/
	struct list_head mmlist;
	/*�ر�ļ���������һЩ��������page_table_lock��������������Ϊԭ�ӱ���*/
	/*�ļ���פ�ڴ漯����*/
	mm_counter_t _file_rss;
	/*����ӳ�䳣פ�ڴ漯����*/
	mm_counter_t _anon_rss;
	/*��ˮӡʱ��פ�ڴ漯��С*/
	unsigned long hiwater_rss;
	/*��ˮӡʱ�����ڴ�ʹ�ô�С*/
	unsigned long hiwater_vm;
	/*���������ַ�ռ����ܹ��������ڴ�ӳ���ҳ��������ע��ӳ������������ʾֻ�ǽ�
	�����ڴ��������ڴ潨��������ϵ���������������ķ��������ڴ�*/
	unsigned long total_vm;
	/*���ڴ�Խ���ʱ����Щҳ���Ի�����Ӳ���ϣ�����Щҳ��Ϊ�Ƚ���Ҫ�����ܻ������ñ�
	�����Ǳ��������ܻ������ڴ�ҳ����*/
	unsigned long locked_vm;
	/*�������ڴ�ҳ��Ŀ*/
	unsigned long shared_vm,;
	/*��ִ�д���ռ���ڴ�ҳ����Ŀ*/
	unsigned long exec_vm;
	/*ջ����ӳ��ĵ��ڴ�ҳ����Ŀ*/
	unsigned long stack_vm;
	/*Ԥ���������ڴ泤��*/
	unsigned long reserved_vm;
	/*Ĭ�ϵķ����ʶ*/
	unsigned long def_flags;
	/*ҳ��ʹ�õ�ҳ��Ŀ*/
	unsigned long nr_ptes;
	/*��ELF�������ļ�ӳ�䵽��ַ�ռ���֮�󣬴���κ����ݶεĳ��Ƚ����ٸı�*/
	/*��ִ�д����������ַ�ռ��еĿ�ʼ��ַ��text�����ӳ�䵽�����ַ�ռ�����ELF��׼ȷ����ÿ��
	��ϵ�ṹ��ָ����һ���ض�����ʼ��ַ��IA-32ϵͳ��ʼ��0x08048000����text�ε���ʼ��ַ����͵�
	���õ�ַ�ռ�֮���д�Լ128MB�ļ�࣬���ڲ���NULLָ��*/
	/*�����Ҳ�����Ķλ��ı��Σ�ͨ�����ڴ�ų���ִ�д���(��CPUִ�еĻ���ָ��)��һ��
	C����ִ����䶼����ɻ������뱣���ڴ���Ρ�ͨ��������ǿɹ����ģ����Ƶ��ִ�е�
	����ֻ��Ҫ���ڴ���ӵ��һ�ݿ������ɡ������ͨ������ֻ�����Է�ֹ��������������޸�
	��ָ��(�Ըöε�д���������¶δ���)��ĳЩ�ܹ�Ҳ���������Ϊ��д���������޸ĳ���
	�����ָ����ݳ��������������ִ�У�����˳��ָ�ֻ��ִ��һ��(ÿ������)�����з�
	��������ʹ����תָ������еݹ飬����Ҫ����ջ��ʵ�֡������ָ���а���������Ͳ�
	������(������ַ����)��������������������(������ֵ)����ֱ�Ӱ����ڴ����У����Ǿ�
	�����ݣ�����ջ������ռ䣬Ȼ�����ø����ݵ�ַ����λ��BSS�κ����ݶΣ�ͬ�����ø�����
	��ַ����������������Ż���ʩӰ�졣*/
	unsigned long start_code;
	/*��ִ�д����������ַ�ռ��еĽ�����ַ*/
	unsigned long end_code;
	/*���ݶ�ͨ�����ڴ�ų������ѳ�ʼ���ҳ�ֵ��Ϊ0��ȫ�ֱ����;�̬�ֲ����������ݶ�����
	��̬�ڴ����(��̬�洢��)���ɶ���д�����ݶα�����Ŀ���ļ���(��Ƕ��ʽϵͳ��һ��̻�
	�ھ����ļ���)���������ɳ����ʼ�������磬����ȫ�ֱ���int gVar = 10��������Ŀ����
	�����ݶ��б���10������ݣ�Ȼ���ڳ������ʱ���Ƶ���Ӧ���ڴ档���ݶ���BSS�ε�������
	�£�   1) BSS�β�ռ�������ļ��ߴ磬��ռ���ڴ�ռ䣻���ݶ�ռ�������ļ���Ҳռ���ڴ��
	�䡣   ���ڴ���������int ar0[10000] = {1, 2, 3, ...}��int ar1[10000]��ar1����BSS�Σ�
	ֻ��¼����10000*4���ֽ���Ҫ��ʼ��Ϊ0����������ar0������¼ÿ������1��2��3...����ʱ
	BSSΪĿ���ļ�����ʡ�Ĵ��̿ռ��൱�ɹۡ�2) �������ȡ���ݶε�����ʱ��ϵͳ�����ȱ
	ҳ���ϣ��Ӷ�������Ӧ�������ڴ棻�������ȡBSS�ε�����ʱ���ں˻Ὣ��ת��һ��ȫ��ҳ
	�����ᷢ��ȱҳ���ϣ�Ҳ����Ϊ�������Ӧ�������ڴ档����ʱ���ݶκ�BSS�ε���������ͨ
	����Ϊ��������ĳЩ�����С����ݶΡ�ָ�����ݶ� + BSS�� + �ѡ�*/
	/*�ѳ�ʼ�������������ַ�ռ��е���ʼ��ַ*/
	unsigned long start_data;
	/*�ѳ�ʼ�������������ַ�ռ��еĽ�����ַ*/
	unsigned long end_data;
	/*BSS(Block Started by Symbol)����ͨ����ų��������·��ţ�δ��ʼ����ȫ�ֱ����;�
	̬�ֲ���������ʼֵΪ0��ȫ�ֱ����;�̬�ֲ�����(�����ڱ�����ʵ��)��δ�����ҳ�ֵ��Ϊ
	0�ķ���(�ó�ֵ��common block�Ĵ�С)C�����У�δ��ʽ��ʼ���ľ�̬�����������ʼ��Ϊ0
	(��������)���ָ��(ָ������)�����ڳ������ʱ��BSS�ᱻ����ϵͳ���㣬����δ����ֵ��
	��ֵΪ0��ȫ�ֱ�������BSS�С�BSS�ν�Ϊδ��ʼ���ľ�̬�������Ԥ��λ�ã���Ŀ���ļ���
	����ռ�ݿռ䣬�����ɼ���Ŀ���ļ����������������ʱ��Ϊ���������ڴ�ռ䣬��Ŀ����
	�������¼����δ��ʼ���ľ�̬���������С�ܺ�(ͨ��start_bss��end_bss��ַд�������
	��)����������(loader)���س���ʱ����ΪBSS�η�����ڴ��ʼ��Ϊ0����Ƕ��ʽ�����У���
	��main()����֮ǰBSS�α�C����ʱϵͳӳ�䵽��ʼ��Ϊȫ����ڴ�(Ч�ʽϸ�)��     ע�⣬����
	��������BSS�Σ�����ֵΪ0��ȫ�ֱ�����ǿ���ţ���δ��ʼ����ȫ�ֱ����������š�������
	�ط��Ѷ���ͬ����ǿ����(��ֵ���ܷ�0)������������֮����ʱ���������ض�����󣬵�����
	ʱ�ĳ�ֵ���ܲ�������ֵ(�ᱻǿ���Ÿ���)����ˣ�����ȫ�ֱ���ʱ����ֻ�б��ļ�ʹ�ã�
	����ʹ��static�ؼ������Σ�������ҪΪȫ�ֱ������帳��ֵ(����0ֵ)����֤�ñ���Ϊǿ
	���ţ��Ա�����ʱ���ֱ�������ͻ�������Ǳ�δֵ֪���ǡ�ĳЩ��������δ��ʼ����ȫ�ֱ�
	��������common�Σ�����ʱ�ٽ������BSS�Ρ��ڱ���׶ο�ͨ��-fno-commonѡ������ֹ��
	δ��ʼ����ȫ�ֱ�������common�Ρ�*/
	/*�����ڴ�Ž�������ʱ��̬������ڴ�Σ��ɶ�̬���Ż����������������������ģ�����
	������ֱ�ӷ��ʣ�ֻ��ͨ��ָ���ӷ��ʡ������̵���malloc�Ⱥ��������ڴ�ʱ���·����
	�ڴ涯̬���ӵ�����(����)��������free�Ⱥ����ͷ��ڴ�ʱ�����ͷŵ��ڴ�Ӷ����޳�(����
	)������Ķ��ڴ��Ǿ����ֽڶ���Ŀռ䣬���ʺ�ԭ�Ӳ������ѹ�����ͨ����������ÿ������
	���ڴ棬���ڶ�������ͷ�������ģ����ջ�����ڴ���Ƭ�����ڴ�һ����Ӧ�ó��������
	�ţ����յ��ڴ�ɹ�����ʹ�á�������Ա���ͷţ��������ʱ����ϵͳ���ܻ��Զ����ա���
	��ĩ����brkָ���ʶ�����ѹ�������Ҫ�����ڴ�ʱ����ͨ��ϵͳ����brk��sbrk���ƶ�brkָ
	�������Ŷѣ�һ����ϵͳ�Զ����á�     ʹ�ö�ʱ���������������⣺1)�ͷŻ��д����ʹ�õ�
	�ڴ�(���ڴ��ƻ���)��2)δ�ͷŲ���ʹ�õ��ڴ�(���ڴ�й©��)�����ͷŴ��������������ʱ��
	����������ڴ�й©��й©���ڴ������������ͷŵ����ݽṹ������Ϊ��������ڴ�ͨ��
	��Բ��Ϊ�¸���������������2���ݴ�(������212B����Բ��Ϊ256B)*/
	/*���������ַ�ռ��е���ʼ��ַ���ѽ�����text�ο�ʼ����������*/
	unsigned long start_brk;
	/*�ѽ�����ַ*/
	unsigned long brk;
	/*ջ�Ͷѵ����𣺢ٹ�����ʽ��ջ�ɱ������Զ����������ɳ���Ա���ƣ�ʹ�÷��㣬���ײ���
	�ڴ�й¶������������ջ��͵�ַ��չ(��������������)�����������ڴ����򣻶���ߵ�ַ��
	չ(��������������)���ǲ��������ڴ�������������ϵͳ���������洢�����ڴ��ַ����Ȼ
	���������������ӵ͵�ַ��ߵ�ַ�������ۿռ��С��ջ����ַ��ջ�����������ϵͳԤ�ȹ�
	��(ͨ��Ĭ��2M��10M)���ѵĴ�С�������ڼ����ϵͳ����Ч�������ڴ棬32λLinuxϵͳ��
	���ڴ�ɴ�2.9G�ռ䡣�ܴ洢���ݣ�ջ�ں�������ʱ������ѹ����������������ָ��(������
	������������ִ�����)�ĵ�ַ��Ȼ���Ǻ���ʵ�Σ�Ȼ���Ǳ��������ľֲ����������ε���
	�����󣬾ֲ������ȳ�ջ��Ȼ���ǲ��������ջ��ָ��ָ���ʼ���ָ���ַ�������ɸ�
	���������������ִ����䡣��ͨ����ͷ����һ���ֽڴ�����С�������ڴ洢�������뺯
	�������޹ص����ݣ����������ɳ���Ա���š��ݷ��䷽ʽ��ջ�ɾ�̬�����̬���䡣��̬��
	���ɱ�������ɣ���ֲ������ķ��䡣��̬������alloca������ջ������ռ䣬������Զ�
	�ͷš���ֻ�ܶ�̬�������ֹ��ͷš��޷���Ч�ʣ�ջ�ɼ�����ײ��ṩ֧�֣�����ר�ŵļĴ�
	�����ջ��ַ��ѹջ��ջ��ר�ŵ�ָ��ִ�У����Ч�ʽϸߡ����ɺ������ṩ�����Ƹ��ӣ�
	Ч�ʱ�ջ�͵öࡣ�߷����ϵͳ��Ӧ��ֻҪջʣ��ռ����������ռ䣬ϵͳ��Ϊ�����ṩ��
	�棬���򱨸��쳣��ʾջ�����     ����ϵͳΪ��ά��һ����¼�����ڴ��ַ����������ϵͳ��
	��������ڴ��������ʱ�������������Ѱ�ҵ�һ���ռ����������ռ�Ķѽ�㣬Ȼ��
	�ý��ӿ��н��������ɾ���������ý��ռ��������������㹻��С�Ŀռ�(��������
	�ڴ���Ƭ̫��)���п��ܵ���ϵͳ����ȥ���ӳ������ݶε��ڴ�ռ䣬�Ա��л���ֵ��㹻��
	С���ڴ棬Ȼ����з��ء������ϵͳ���ڸ��ڴ�ռ��׵�ַ����¼���η�����ڴ��С��
	���������ͷź���(��free/delete)��ȷ�ͷű��ڴ�ռ䡣     ���⣬�����ҵ��Ķѽ���С��һ
	�����õ�������Ĵ�С��ϵͳ���Զ�������Ĳ������·�����������С�����Ƭ���⣺ջ����
	������Ƭ���⣬��Ϊջ�Ǻ���ȳ��Ķ��У��ڴ�鵯��ջ֮ǰ����������ĺ����ջ������
	��������Ƶ�������ͷŲ�������ɶ��ڴ�ռ�Ĳ��������Ӷ���ɴ�����Ƭ��ʹ����Ч�ʽ�
	�͡�   �ɼ�������������ڴ���Ƭ������û��ר�ŵ�ϵͳ֧�֣�Ч�ʺܵͣ����ڿ��������û�
	̬���ں�̬�л����ڴ�����Ĵ��۸�Ϊ��������ջ�ڳ�����Ӧ����㷺����������Ҳ����
	ջ����ɣ����ù����еĲ��������ص�ַ��ջ��ָ��;ֲ������ȶ�����ջ�ķ�ʽ��š���
	�ԣ����龡��ʹ��ջ�����ڷ�����������ڴ�ռ�ʱʹ�öѡ�     ʹ��ջ�Ͷ�ʱӦ����Խ�緢
	����������ܳ���������ƻ�����ѡ�ջ�ṹ���������벻���ĺ��*/
	/*ջ�ֳƶ�ջ���ɱ������Զ������ͷţ���Ϊ�������ݽṹ�е�ջ(�Ƚ����)����ջ��Ҫ��
	������;����1��Ϊ�����ڲ������ķǾ�̬�ֲ�����(C�����гơ��Զ�������)�ṩ�洢�ռ䡣
	��2����¼�������ù�����ص�ά������Ϣ����Ϊջ֡(Stack Frame)����̻��¼
	(Procedure Activation Record)���������������ص�ַ�����ʺ�װ��Ĵ����ĺ���������һ
	Щ�Ĵ���ֵ�ı��档���ݹ�����⣬��ջ���Ǳ��衣��Ϊ����ʱ�ɻ�֪�ֲ������������ͷ�
	�ص�ַ����ռ䣬�����������BSS�Σ�3����ʱ�洢���������ݴ���������ʽ���ּ�����
	��alloca���������ջ���ڴ档����������ջ�ռ�������ʹ��Ծ��ջ�ڴ汣����CPU�����У�
	�Ӷ����ٷ��ʡ������е�ÿ���̶߳��������Լ���ջ����ջ�в���ѹ������ʱ������������
	���ͻ�ľ�ջ��Ӧ���ڴ����򣬴Ӷ�����һ��ҳ���󡣴�ʱ��ջ�Ĵ�С���ڶ�ջ���ֵ
	RLIMIT_STACK(ͨ����8M)����ջ�ᶯ̬����������������С�ӳ���ջ����չ�������С��
	������������ulimit -s����ɲ鿴�����ö�ջ���ֵ��������ʹ�õĶ�ջ������ֵʱ, ����
	ջ���(Stack Overflow)�������յ�һ���δ���(Segmentation Fault)��ע�⣬���߶�ջ��
	�����ܻ������ڴ濪��������ʱ�䡣     ��ջ�ȿ���������(���ڴ�͵�ַ)Ҳ����������, ����
	���ھ����ʵ�֡�ջ�Ĵ�С������ʱ���ں˶�̬����*/
	/*ջ�������ַ�ռ��е���ʼ��ַ��ջ��ʼ��STACK_TOP�����������PF_RANDOMIZE������ʼ������
	һ��С���������ÿ����ϵ�ṹ�����붨��STACK_TOP����඼����ΪTASK_SIZE�����û���ַ�ռ���
	��ߵĿ��õ�ַ�ռ䣬���̵Ĳ����б��ͻ�����������ջ����ʼ����*/
	unsigned long start_stack;
	/*�����б��������ַ�ռ��е���ʼ��ַ*/
	unsigned long arg_start;
	/*�����б��������ַ�ռ��еĽ�����ַ*/
	unsigned long arg_end;
	/*���������������ַ�ռ��е���ʼ��ַ*/
	unsigned long env_start;
	/*���������������ַ�ռ��еĽ�����ַ*/
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
	/*�ں���һ����ͼ��ȡ����ʱ��global_faultsֵ*/
	unsigned int faultstamp;
	/*�����ƽ�����صĵ������ȼ������ڿ��ƶԽ������Ƶķ���*/
	unsigned int token_priority;
	/*�ý��̵ȴ��������Ƶ�ʱ�����ĳ���*/
	unsigned int last_interval;
	/*������ԭ�Ӳ�����������Щ����λ*/
	unsigned long flags;

	/*�ڴ�ת��֧�����*/
	int core_waiters;
	/*�ں�ת����ʼ����ɵ������*/
	struct completion *core_startup_done, core_done;

	/*�첽ioλ*/
	rwlock_t		ioctx_list_lock;
	/**/
	struct kioctx		*ioctx_list;
};

#endif /* _LINUX_MM_TYPES_H */