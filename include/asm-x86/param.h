#ifndef _ASM_X86_PARAM_H
#define _ASM_X86_PARAM_H

#ifdef __KERNEL__
/*�ں��ڲ�ʱ��Ƶ��*/
#define HZ		CONFIG_HZ
/*һЩ�û��ӿ�ʹ�õĵδ�*/
#define USER_HZ				100
/*ÿ���ӵδ����*/
#define CLOCKS_PER_SEC		(USER_HZ)
#endif

/*�޶���HZ��Ĭ��ֵ*/
#ifndef HZ
#define HZ 100
#endif

#define EXEC_PAGESIZE		4096

#ifndef NOGROUP
#define NOGROUP				(-1)
#endif

/*��������󳤶�*/
#define MAXHOSTNAMELEN		64

#endif /* _ASM_X86_PARAM_H */
