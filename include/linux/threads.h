#ifndef _LINUX_THREADS_H
#define _LINUX_THREADS_H

/*Ĭ�ϵ��߳�����ֵĿǰ��ͨ��/proc/sys/kernel/threads-max�鿴��SMPϵͳ֧�ֵ����cpu��Ŀ��
���ֵ�������ã������ĿĬ��Ϊƽ̨���ֳ�����λ��Ŀ*/
#ifdef CONFIG_SMP
#define NR_CPUS		CONFIG_NR_CPUS
#else
#define NR_CPUS		1
#endif

/*���û���������4���߳�*/
#define MIN_THREADS_LEFT_FOR_ROOT 4

/*һ������Ĭ�Ͽ��Է��������߳���Ŀ��32768��С��ϵͳ���ϣ�ͨ��ΪǶ��ʽϵͳ������
��������ѡ��BASE_SMALL����ʱ֧�ֵ������Ŀ��4096*/
#define PID_MAX_DEFAULT (CONFIG_BASE_SMALL ? 0x1000 : 0x8000)

/*����PID�����Ŀ��С��ϵͳ����32768��32λϵͳ����Ĭ�ϵ���32768���ֳ�����4�ֽ�ʱ��4M*/
/*		config						PID_MAX_DEFAULT			PID_MAX_LIIT
	CONFIG_BASE_SMALL						4K						32K
	BIT_PER_LONG == 32						32K						32K	
	BIT_PER_LONG > 32						32K						4M
*/
#define PID_MAX_LIMIT (CONFIG_BASE_SMALL ? PAGE_SIZE * 8 : \
	(sizeof(long) > 4 ? 4 * 1024 * 1024 : PID_MAX_DEFAULT))

#endif
