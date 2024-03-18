/*内核ELF内核转储*/ 

#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/capability.h>
#include <linux/elf.h>
#include <linux/elfcore.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define CORE_STR "CORE"

static int open_kcore(struct inode * inode, struct file * filp)
{
	return capable(CAP_SYS_RAWIO) ? 0 : -EPERM;
}

static ssize_t read_kcore(struct file *, char __user *, size_t, loff_t *);

const struct file_operations proc_kcore_operations =
{
	.read		= read_kcore,
	.open		= open_kcore,
};

#ifndef kc_vaddr_to_offset
#define	kc_vaddr_to_offset(v) ((v) - PAGE_OFFSET)
#endif
#ifndef	kc_offset_to_vaddr
#define	kc_offset_to_vaddr(o) ((o) + PAGE_OFFSET)
#endif

/* An ELF note in memory */
struct memelfnote
{
	const char *name;
	int type;
	unsigned int datasz;
	void *data;
};
/*内核转储链表头*/
static struct kcore_list *kclist;
/*内核转储读写保护锁*/
static DEFINE_RWLOCK(kclist_lock);

/*构造内核转储结点，并作为第一个结点插入kclist链表*/
void kclist_add(struct kcore_list *new, void *addr, size_t size)
{
	/*初始化内核转储结点*/
	new->addr = (unsigned long)addr;
	new->size = size;
	/*获取读写锁头插法将内核转储结点插入到kclist单链表*/
	write_lock(&kclist_lock);
	new->next = kclist;
	kclist = new;
	write_unlock(&kclist_lock);
}

static size_t get_kcore_size(int *nphdr, size_t *elf_buflen)
{
	size_t try, size;
	/*内核转储链表结点*/
	struct kcore_list *m;

	*nphdr = 1; /* PT_NOTE */
	size = 0;
	/*遍历内核转储单链表，size保存的内核转储单链表中最大偏移值，nphdr指针保存该单链表
	中结点的数目*/
	for (m=kclist; m; m=m->next)
	{
		/**/
		try = kc_vaddr_to_offset((size_t)m->addr + m->size);
		if (try > size)
			size = try;
		*nphdr = *nphdr + 1;
	}
	*elf_buflen =	sizeof(struct elfhdr) + (*nphdr + 2)*sizeof(struct elf_phdr) + 
					3 * ((sizeof(struct elf_note)) + roundup(sizeof(CORE_STR), 4)) +
					roundup(sizeof(struct elf_prstatus), 4) +
					roundup(sizeof(struct elf_prpsinfo), 4) +
					roundup(sizeof(struct task_struct), 4);
	*elf_buflen = PAGE_ALIGN(*elf_buflen);
	return size + *elf_buflen;
}


/*****************************************************************************/
/*
 * determine size of ELF note
 */
static int notesize(struct memelfnote *en)
{
	int sz;

	sz = sizeof(struct elf_note);
	sz += roundup((strlen(en->name) + 1), 4);
	sz += roundup(en->datasz, 4);

	return sz;
} /* end notesize() */

/*****************************************************************************/
/*
 * store a note in the header buffer
 */
static char *storenote(struct memelfnote *men, char *bufp)
{
	struct elf_note en;

#define DUMP_WRITE(addr,nr) do { memcpy(bufp,addr,nr); bufp += nr; } while(0)

	en.n_namesz = strlen(men->name) + 1;
	en.n_descsz = men->datasz;
	en.n_type = men->type;

	DUMP_WRITE(&en, sizeof(en));
	DUMP_WRITE(men->name, en.n_namesz);

	/* XXX - cast from long long to long to avoid need for libgcc.a */
	bufp = (char*) roundup((unsigned long)bufp,4);
	DUMP_WRITE(men->data, men->datasz);
	bufp = (char*) roundup((unsigned long)bufp,4);

#undef DUMP_WRITE

	return bufp;
} /* end storenote() */

/*
 * store an ELF coredump header in the supplied buffer
 * nphdr is the number of elf_phdr to insert
 */

/*在指定缓存区中存储一个ELF内核转储头。nphdr是插入elf_phdr的数目*/
static void elf_kcore_store_hdr(char *bufp, int nphdr, int dataoff)
{
	struct elf_prstatus prstatus;	/* NT_PRSTATUS */
	struct elf_prpsinfo prpsinfo;	/* NT_PRPSINFO */
	struct elf_phdr *nhdr, *phdr;
	struct elfhdr *elf;
	struct memelfnote notes[3];
	off_t offset = 0;
	struct kcore_list *m;

	/*建立ELF头*/
	elf = (struct elfhdr *) bufp;
	/*定位到ELF文件头结束位置*/
	bufp += sizeof(struct elfhdr);
	/*偏移量累加ELF文件头空间*/
	offset += sizeof(struct elfhdr);
	/*初始化e_ident数组前四个字节0x7F、'E'、'L'、'F'*/
	memcpy(elf->e_ident, ELFMAG, SELFMAG);
	/*32或64位ELF类*/
	elf->e_ident[EI_CLASS]	= ELF_CLASS;
	/*一般是二进制补码，小端序*/
	elf->e_ident[EI_DATA]	= ELF_DATA;
	/*当前版本*/
	elf->e_ident[EI_VERSION]= EV_CURRENT;
	/*OS ABI*/
	elf->e_ident[EI_OSABI] = ELF_OSABI;
	/*e_ident后8个字节清零*/
	memset(elf->e_ident+EI_PAD, 0, EI_NIDENT-EI_PAD);
	/*ELF type：CoreDump*/
	elf->e_type	= ET_CORE;
	/*低层平台*/
	elf->e_machine	= ELF_ARCH;
	/**/
	elf->e_version	= EV_CURRENT;
	/*程序入口点初始化，一般是0x08048000，代码段的起始地址，以下地址保留*/
	elf->e_entry	= 0;
	/*ELF文件头结束位置之后就是程序头偏移*/
	elf->e_phoff	= sizeof(struct elfhdr);
	/*节头表偏移*/
	elf->e_shoff	= 0;
#if defined(CONFIG_H8300)
	elf->e_flags	= ELF_FLAGS;
#else
	/*当前标识没有使用*/
	elf->e_flags	= 0;
#endif
	/*ELF文件头所占空间长度初始化*/
	elf->e_ehsize	= sizeof(struct elfhdr);
	/*ELF程序头表中每项所占空间长度初始化*/
	elf->e_phentsize= sizeof(struct elf_phdr);
	/*ELF程序头中包含项数目初始化*/
	elf->e_phnum	= nphdr;
	/*ELF节头表中包含项所占空间长度初始化*/
	elf->e_shentsize= 0;
	/*段头表中包含项数目初始化*/
	elf->e_shnum	= 0;
	/*节头表中包含每项名称的字符串在节头表项中的索引*/
	elf->e_shstrndx	= 0;

	/*设置ELF PT_NOTE程序头*/
	nhdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);
	nhdr->p_type	= PT_NOTE;
	nhdr->p_offset	= 0;
	nhdr->p_vaddr	= 0;
	nhdr->p_paddr	= 0;
	nhdr->p_filesz	= 0;
	nhdr->p_memsz	= 0;
	nhdr->p_flags	= 0;
	nhdr->p_align	= 0;

	/* setup ELF PT_LOAD program header for every area */
	for (m=kclist; m; m=m->next) {
		phdr = (struct elf_phdr *) bufp;
		bufp += sizeof(struct elf_phdr);
		offset += sizeof(struct elf_phdr);

		phdr->p_type	= PT_LOAD;
		phdr->p_flags	= PF_R|PF_W|PF_X;
		phdr->p_offset	= kc_vaddr_to_offset(m->addr) + dataoff;
		phdr->p_vaddr	= (size_t)m->addr;
		phdr->p_paddr	= 0;
		phdr->p_filesz	= phdr->p_memsz	= m->size;
		phdr->p_align	= PAGE_SIZE;
	}

	/*
	 * Set up the notes in similar form to SVR4 core dumps made
	 * with info from their /proc.
	 */
	nhdr->p_offset	= offset;

	/* set up the process status */
	notes[0].name = CORE_STR;
	notes[0].type = NT_PRSTATUS;
	notes[0].datasz = sizeof(struct elf_prstatus);
	notes[0].data = &prstatus;

	memset(&prstatus, 0, sizeof(struct elf_prstatus));

	nhdr->p_filesz	= notesize(&notes[0]);
	bufp = storenote(&notes[0], bufp);

	/* set up the process info */
	notes[1].name	= CORE_STR;
	notes[1].type	= NT_PRPSINFO;
	notes[1].datasz	= sizeof(struct elf_prpsinfo);
	notes[1].data	= &prpsinfo;

	memset(&prpsinfo, 0, sizeof(struct elf_prpsinfo));
	prpsinfo.pr_state	= 0;
	prpsinfo.pr_sname	= 'R';
	prpsinfo.pr_zomb	= 0;

	strcpy(prpsinfo.pr_fname, "vmlinux");
	strncpy(prpsinfo.pr_psargs, saved_command_line, ELF_PRARGSZ);

	nhdr->p_filesz	+= notesize(&notes[1]);
	bufp = storenote(&notes[1], bufp);

	/* set up the task structure */
	notes[2].name	= CORE_STR;
	notes[2].type	= NT_TASKSTRUCT;
	notes[2].datasz	= sizeof(struct task_struct);
	notes[2].data	= current;

	nhdr->p_filesz	+= notesize(&notes[2]);
	bufp = storenote(&notes[2], bufp);

} /* end elf_kcore_store_hdr() */

/*****************************************************************************/
/*
 * read from the ELF header and then kernel memory
 */
static ssize_t
read_kcore(struct file *file, char __user *buffer, size_t buflen, loff_t *fpos)
{
	ssize_t acc = 0;
	size_t size, tsz;
	size_t elf_buflen;
	int nphdr;
	unsigned long start;

	read_lock(&kclist_lock);
	proc_root_kcore->size = size = get_kcore_size(&nphdr, &elf_buflen);
	if (buflen == 0 || *fpos >= size) {
		read_unlock(&kclist_lock);
		return 0;
	}

	/* trim buflen to not go beyond EOF */
	if (buflen > size - *fpos)
		buflen = size - *fpos;

	/* construct an ELF core header if we'll need some of it */
	if (*fpos < elf_buflen) {
		char * elf_buf;

		tsz = elf_buflen - *fpos;
		if (buflen < tsz)
			tsz = buflen;
		elf_buf = kzalloc(elf_buflen, GFP_ATOMIC);
		if (!elf_buf) {
			read_unlock(&kclist_lock);
			return -ENOMEM;
		}
		elf_kcore_store_hdr(elf_buf, nphdr, elf_buflen);
		read_unlock(&kclist_lock);
		if (copy_to_user(buffer, elf_buf + *fpos, tsz)) {
			kfree(elf_buf);
			return -EFAULT;
		}
		kfree(elf_buf);
		buflen -= tsz;
		*fpos += tsz;
		buffer += tsz;
		acc += tsz;

		/* leave now if filled buffer already */
		if (buflen == 0)
			return acc;
	} else
		read_unlock(&kclist_lock);

	/*
	 * Check to see if our file offset matches with any of
	 * the addresses in the elf_phdr on our list.
	 */
	start = kc_offset_to_vaddr(*fpos - elf_buflen);
	if ((tsz = (PAGE_SIZE - (start & ~PAGE_MASK))) > buflen)
		tsz = buflen;
		
	while (buflen) {
		struct kcore_list *m;

		read_lock(&kclist_lock);
		for (m=kclist; m; m=m->next) {
			if (start >= m->addr && start < (m->addr+m->size))
				break;
		}
		read_unlock(&kclist_lock);

		if (m == NULL) {
			if (clear_user(buffer, tsz))
				return -EFAULT;
		} else if ((start >= VMALLOC_START) && (start < VMALLOC_END)) {
			char * elf_buf;
			struct vm_struct *m;
			unsigned long curstart = start;
			unsigned long cursize = tsz;

			elf_buf = kzalloc(tsz, GFP_KERNEL);
			if (!elf_buf)
				return -ENOMEM;

			read_lock(&vmlist_lock);
			for (m=vmlist; m && cursize; m=m->next) {
				unsigned long vmstart;
				unsigned long vmsize;
				unsigned long msize = m->size - PAGE_SIZE;

				if (((unsigned long)m->addr + msize) < 
								curstart)
					continue;
				if ((unsigned long)m->addr > (curstart + 
								cursize))
					break;
				vmstart = (curstart < (unsigned long)m->addr ? 
					(unsigned long)m->addr : curstart);
				if (((unsigned long)m->addr + msize) > 
							(curstart + cursize))
					vmsize = curstart + cursize - vmstart;
				else
					vmsize = (unsigned long)m->addr + 
							msize - vmstart;
				curstart = vmstart + vmsize;
				cursize -= vmsize;
				/* don't dump ioremap'd stuff! (TA) */
				if (m->flags & VM_IOREMAP)
					continue;
				memcpy(elf_buf + (vmstart - start),
					(char *)vmstart, vmsize);
			}
			read_unlock(&vmlist_lock);
			if (copy_to_user(buffer, elf_buf, tsz)) {
				kfree(elf_buf);
				return -EFAULT;
			}
			kfree(elf_buf);
		} else {
			if (kern_addr_valid(start)) {
				unsigned long n;

				n = copy_to_user(buffer, (char *)start, tsz);
				/*
				 * We cannot distingush between fault on source
				 * and fault on destination. When this happens
				 * we clear too and hope it will trigger the
				 * EFAULT again.
				 */
				if (n) { 
					if (clear_user(buffer + tsz - n,
								n))
						return -EFAULT;
				}
			} else {
				if (clear_user(buffer, tsz))
					return -EFAULT;
			}
		}
		buflen -= tsz;
		*fpos += tsz;
		buffer += tsz;
		acc += tsz;
		start += tsz;
		tsz = (buflen > PAGE_SIZE ? PAGE_SIZE : buflen);
	}

	return acc;
}
