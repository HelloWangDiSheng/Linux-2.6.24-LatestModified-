#ifndef __ASM_LINKAGE_H
#define __ASM_LINKAGE_H

/*CPP_ASMLINKAGE�ؼ���ͨ����չΪһ���մ�������ʹ��C++�����������ں�ʱ�Ų���extern C
�ؼ��֣�����֪ͨ������ʹ��C����Լ������һ�����������ջ����������C++����Լ������һ��
����������ջ��*/
/*��ʶΪasmlinkage�����ӻ������ڱ����á���Ϊ����������²������ݱ�������ֹ�����
������������޷����ʣ�����ͨ��ջ�ܹ����ݵĲ�����Ŀ��ȣ�ͨ���Ĵ������ݵĲ�����Ŀ��Ȼ
��������Ǵ������ȣ���Ҳ��ͨ���Ĵ������ݲ�����ѡ�������ʽ���õ�ԭ��*/
#define asmlinkage CPP_ASMLINKAGE __attribute__((regparm(0)))
#define FASTCALL(x)	x __attribute__((regparm(3)))
#define fastcall	__attribute__((regparm(3)))
/*��������IA-32�������ϵ�ṹ�϶�����Ϊ�մ�*/

#define prevent_tail_call(ret) __asm__ ("" : "=r" (ret) : "0" (ret))

#ifdef CONFIG_X86_ALIGNMENT_16
#define __ALIGN .align 16,0x90
#define __ALIGN_STR ".align 16,0x90"
#endif

#endif
