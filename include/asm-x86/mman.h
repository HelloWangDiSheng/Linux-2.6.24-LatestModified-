#ifndef _ASM_X86_MMAN_H
#define _ASM_X86_MMAN_H

#include <asm-generic/mman.h>

/*����32λ��ַ�ռ���ʹ��*/
#define MAP_32BIT			0x40
/* stack-like segment */
#define MAP_GROWSDOWN		0x0100
/*ETXTBSY��TXT�ļ���ռ��*/
#define MAP_DENYWRITE		0x0800
/*ӳ��������ִ��*/
#define MAP_EXECUTABLE		0x1000
/*ҳ������*/
#define MAP_LOCKED			0x2000
/*���ü�鱣��λ��������ʹ��mmapϵͳ���ý��������ڴ������ʱ�򣬻��ܵ��ں�overcommit
���Ե�Ӱ�죬�ں˻��ۺ������ڴ�����������Լ�swap�������������С�������Ƿ���������
���ڴ����������롣mmap�������������ڴ棬�ں˻�ܾ������ǵ�������mmapϵͳ����������
��MAP_NORESERVE�����ں��ڷ��������ڴ��ʱ�򽫲��ῼ�������ڴ�����������Լ�swap������
���������أ�����������������ڴ棬�ں˶������㡣��ȱҳ��ʱ������׵���OOM��
MAP_NORESERVEֻ����OVERCOMMIT_GUESS��OVERCOMMIT_ALWAYSģʽ�²������壬��Ϊ����ں˱���
�ǽ�ֹovercommit�Ļ�������MAP_NORESERVE���������*/
#define MAP_NORESERVE		0x4000
/*populate (prefault) pagetables*/
#define MAP_POPULATE		0x8000
/*IO����ʱ������*/
#define MAP_NONBLOCK		0x10000

/*������ǰ����ӳ��*/
#define MCL_CURRENT			1
/*��������δ��ӳ��*/
#define MCL_FUTURE			2

#endif /* _ASM_X86_MMAN_H */
