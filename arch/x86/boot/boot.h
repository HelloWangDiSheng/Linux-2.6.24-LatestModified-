/*
 * arch/i386/boot/boot.h
 *
 * Header file for the real-mode kernel code
 */

#ifndef BOOT_BOOT_H
#define BOOT_BOOT_H

/*栈的最小空间（字节为单位）*/
#define STACK_SIZE	512	/* Minimum number of bytes for stack */

#ifndef __ASSEMBLY__

#include <stdarg.h>
#include <linux/types.h>
#include <linux/edd.h>
#include <asm/boot.h>
#include <asm/setup.h>

/*条件为真时强制编译终止*/
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

extern struct setup_header hdr;
extern struct boot_params boot_params;

/*基本的端口输入输出{in, out}{b, w, l}函数*/
static inline void outb(u8 v, u16 port)
{
	asm volatile("outb %0,%1" : : "a" (v), "dN" (port));
}
static inline u8 inb(u16 port)
{
	u8 v;
	asm volatile("inb %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void outw(u16 v, u16 port)
{
	asm volatile("outw %0,%1" : : "a" (v), "dN" (port));
}
static inline u16 inw(u16 port)
{
	u16 v;
	asm volatile("inw %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void outl(u32 v, u16 port)
{
	asm volatile("outl %0,%1" : : "a" (v), "dN" (port));
}
static inline u32 inl(u32 port)
{
	u32 v;
	asm volatile("inl %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void io_delay(void)
{
	const u16 DELAY_PORT = 0x80;
	asm volatile("outb %%al,%0" : : "dN" (DELAY_PORT));
}

/*经常用于引用其它段中数据的函数*/
static inline u16 ds(void)
{
	u16 seg;
	asm("movw %%ds,%0" : "=rm" (seg));
	return seg;
}

static inline void set_fs(u16 seg)
{
	asm volatile("movw %0,%%fs" : : "rm" (seg));
}
static inline u16 fs(void)
{
	u16 seg;
	asm volatile("movw %%fs,%0" : "=rm" (seg));
	return seg;
}

static inline void set_gs(u16 seg)
{
	asm volatile("movw %0,%%gs" : : "rm" (seg));
}
static inline u16 gs(void)
{
	u16 seg;
	asm volatile("movw %%gs,%0" : "=rm" (seg));
	return seg;
}

typedef unsigned int addr_t;

static inline u8 rdfs8(addr_t addr)
{
	u8 v;
	asm volatile("movb %%fs:%1,%0" : "=r" (v) : "m" (*(u8 *)addr));
	return v;
}
static inline u16 rdfs16(addr_t addr)
{
	u16 v;
	asm volatile("movw %%fs:%1,%0" : "=r" (v) : "m" (*(u16 *)addr));
	return v;
}
static inline u32 rdfs32(addr_t addr)
{
	u32 v;
	asm volatile("movl %%fs:%1,%0" : "=r" (v) : "m" (*(u32 *)addr));
	return v;
}

static inline void wrfs8(u8 v, addr_t addr)
{
	asm volatile("movb %1,%%fs:%0" : "+m" (*(u8 *)addr) : "r" (v));
}
static inline void wrfs16(u16 v, addr_t addr)
{
	asm volatile("movw %1,%%fs:%0" : "+m" (*(u16 *)addr) : "r" (v));
}
static inline void wrfs32(u32 v, addr_t addr)
{
	asm volatile("movl %1,%%fs:%0" : "+m" (*(u32 *)addr) : "r" (v));
}

static inline u8 rdgs8(addr_t addr)
{
	u8 v;
	asm volatile("movb %%gs:%1,%0" : "=r" (v) : "m" (*(u8 *)addr));
	return v;
}
static inline u16 rdgs16(addr_t addr)
{
	u16 v;
	asm volatile("movw %%gs:%1,%0" : "=r" (v) : "m" (*(u16 *)addr));
	return v;
}
static inline u32 rdgs32(addr_t addr)
{
	u32 v;
	asm volatile("movl %%gs:%1,%0" : "=r" (v) : "m" (*(u32 *)addr));
	return v;
}

static inline void wrgs8(u8 v, addr_t addr)
{
	asm volatile("movb %1,%%gs:%0" : "+m" (*(u8 *)addr) : "r" (v));
}
static inline void wrgs16(u16 v, addr_t addr)
{
	asm volatile("movw %1,%%gs:%0" : "+m" (*(u16 *)addr) : "r" (v));
}
static inline void wrgs32(u32 v, addr_t addr)
{
	asm volatile("movl %1,%%gs:%0" : "+m" (*(u32 *)addr) : "r" (v));
}

/*函数仅返回true或者false，而非有符号返回值*/
static inline int memcmp(const void *s1, const void *s2, size_t len)
{
	u8 diff;
	asm("repe; cmpsb; setnz %0"
	    : "=qm" (diff), "+D" (s1), "+S" (s2), "+c" (len));
	return diff;
}

static inline int memcmp_fs(const void *s1, addr_t s2, size_t len)
{
	u8 diff;
	asm volatile("fs; repe; cmpsb; setnz %0"
		     : "=qm" (diff), "+D" (s1), "+S" (s2), "+c" (len));
	return diff;
}
static inline int memcmp_gs(const void *s1, addr_t s2, size_t len)
{
	u8 diff;
	asm volatile("gs; repe; cmpsb; setnz %0"
		     : "=qm" (diff), "+D" (s1), "+S" (s2), "+c" (len));
	return diff;
}

static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

/*堆可用的动态列表*/
extern char _end[];
/*堆起始位置*/
extern char *HEAP;
/*对结束位置*/
extern char *heap_end;
/*重置堆为空*/
#define RESET_HEAP() ((void *)( HEAP = _end ))
/*将堆起始地址对齐到a，然后在堆内分配s*n个字节空间，并返回已分配空间的起始地址*/
static inline char *__get_heap(size_t s, size_t a, size_t n)
{
	char *tmp;
	/*将堆起始地址对齐到a*/
	HEAP = (char *)(((size_t)HEAP+(a-1)) & ~(a-1));
	/*保存分配前的堆起始位置*/
	tmp = HEAP;
	/*扩大（n*s个字节）堆空间*/
	HEAP += s*n;
	/*返回堆扩大前的起始地址*/
	return tmp;
}

/*在堆内分配n个type类型的数据空间，分配起始地址对齐type类型*/
#define GET_HEAP(type, n) ((type *)__get_heap(sizeof(type),__alignof__(type),(n)))

/**/
static inline bool heap_free(size_t n)
{
	return (int)(heap_end-HEAP) >= (int)n;
}

/* copy.S */
void copy_to_fs(addr_t dst, void *src, size_t len);
void *copy_from_fs(void *dst, addr_t src, size_t len);
void copy_to_gs(addr_t dst, void *src, size_t len);
void *copy_from_gs(void *dst, addr_t src, size_t len);
void *memcpy(void *dst, void *src, size_t len);
void *memset(void *dst, int c, size_t len);

#define memcpy(d,s,l) __builtin_memcpy(d,s,l)
#define memset(d,c,l) __builtin_memset(d,c,l)

/* a20.c */
int enable_a20(void);

/* apm.c */
int query_apm_bios(void);

/* cmdline.c */
int cmdline_find_option(const char *option, char *buffer, int bufsize);

/* cpu.c, cpucheck.c */
int check_cpu(int *cpu_level_ptr, int *req_level_ptr, u32 **err_flags_ptr);
int validate_cpu(void);

/* edd.c */
void query_edd(void);

/* header.S */
void __attribute__((noreturn)) die(void);

/* mca.c */
int query_mca(void);

/* memory.c */
int detect_memory(void);

/* pm.c */
void __attribute__((noreturn)) go_to_protected_mode(void);

/* pmjump.S */
void __attribute__((noreturn))
	protected_mode_jump(u32 entrypoint, u32 bootparams);

/* printf.c */
int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int printf(const char *fmt, ...);

/* string.c */
int strcmp(const char *str1, const char *str2);
size_t strnlen(const char *s, size_t maxlen);
unsigned int atou(const char *s);

/* tty.c */
void puts(const char *);
void putchar(int);
int getchar(void);
void kbd_flush(void);
int getchar_timeout(void);

/* video.c */
void set_video(void);

/* video-vesa.c */
void vesa_store_edid(void);

/* voyager.c */
int query_voyager(void);

#endif /* __ASSEMBLY__ */

#endif /* BOOT_BOOT_H */
