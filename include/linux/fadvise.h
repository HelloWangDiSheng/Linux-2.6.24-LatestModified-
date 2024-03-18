#ifndef FADVISE_H_INCLUDED
#define FADVISE_H_INCLUDED

/*�����ļ����Ԥ��ҳ��ra_pagesΪĬ��ֵ32ҳ*/
#define POSIX_FADV_NORMAL			0
/*�������������˳�����ָ�����ļ����ݡ�ra_pages ����Ϊ 0����ʾ��ֹԤ��*/
#define POSIX_FADV_RANDOM			1
/*��������˳�����ָ�����ļ����ݣ�ra_pages ֵΪĬ��ֵ������*/
#define POSIX_FADV_SEQUENTIAL		2
/*֪ͨ�ںˣ�����ָ������ļ����ݽ��ڲ���֮�󱻷���*/
#define POSIX_FADV_WILLNEED			3

/*
 * The advise values for POSIX_FADV_DONTNEED and POSIX_ADV_NOREUSE
 * for s390-64 differ from the values for the rest of the world.
 */
#if defined(__s390x__)
/*Don't need these pages*/
#define POSIX_FADV_DONTNEED			6
/*Data will be accessed once*/
#define POSIX_FADV_NOREUSE			7
#else
/*Don't need these pages*/
#define POSIX_FADV_DONTNEED			4
/*Data will be accessed once*/
#define POSIX_FADV_NOREUSE			5
#endif

#endif	/* FADVISE_H_INCLUDED */
