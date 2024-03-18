#ifndef _ASM_BOOT_H
#define _ASM_BOOT_H

/* Don't touch these, unless you really know what you're doing. */
/*Ĭ�ϳ�ʼ����*/
#define DEF_INITSEG		0x9000
/*Ĭ��ϵͳ��*/
#define DEF_SYSSEG		0x1000
/**/
#define DEF_SETUPSEG	0x9020
/**/
#define DEF_SYSSIZE		0x7F00

/*�ڲ��߼���Ƶͼ������SVGA��Super Video Graphics Array������������V����Video ����Ƶ����
G����Graphics��ͼ�񣩣�A����Array�����У���S����Super(����)��X����Extended(��չ)��
U����Ultra(�ռ�)��W����Wide���ӿ���Q1����Quarter(�ķ�֮һ)��Q2����Quantum(����)
VGA��Video Graphics Array����Ƶͼ�����У�֧��640*480�ֱ���
SVGA��Super Video Graphics Array���߼���Ƶͼ�����У�����VESAΪIBM���ݻ��Ƴ��ı�׼��
����VGA�����Ʒ�����֧��800��600�ֱ��ʡ�
XGA��Extended Graphics Array����չͼ�����У�����IBM��1990�귢���ģ�XGA���µİ汾XGA-2
�����ɫ�ṩ800��600���صķֱ��ʻ���65536��ɫ���ṩ1024��768���صķֱ��ʣ�������ͼ��ֱ�
ˮƽ�ڵ�ʱ�Ƚϳ��á�
SXGA��Super Extended Graphics Array���߼���չͼ�����У���һ���ֱ���Ϊ1280x1024�ļȳ�
��ʵ��ʾ��׼�����ֱ��㷺���õ���ʾ��׼���ݺ����5:4�����ǳ�����4:3��һ�����ڹ�ȥ�ĸ�
�˱ʼǱ����ԡ�
SXGA+��Super Extended Graphics Array������ΪSXGA��һ����չ��SXGA+��һ��ר��Ϊ�ʼǱ�
��Ƶ���Ļ������ʾ�ֱ���Ϊ1400��1050�����ڱʼǱ�LCD��Ļ��ˮƽ�봹ֱ��಻ͬ����ͨ����
LCD����������ʾ�ľ���Ҫ����ͨ17Ӣ�������LCD�߳����١�
UVGA��Ultra Video Graphics Array��������չͼ�����У���֧�����1600��1200�ֱ��ʡ�һ����
��15Ӣ��ıʼǱ����ԡ����ڶ����칤��Ҫ��ϸߣ����Ե�ʱ�ļ۸�Ҳ�Ƚϰ���
WXGA��WideExtended Graphics Array��������չͼ�����У�����Ϊ��ͨXGA��Ļ�Ŀ����汾��WXGA
����16:10�ĺ�������������Ļ�ĳߴ硣�������ʾ�ֱ���Ϊ1280��800��*/
/*80x25ģʽ*/
#define NORMAL_VGA		0xffff
/*80x50ģʽ*/
#define EXTENDED_VGA	0xfffe
/**/
#define ASK_VGA			0xfffd		/* ask for it at bootup */

/*�ں�Ӧ�ü��ص������ַ��CONFIG_PHYSICAL_START����CONFIG_PHYSICAL_ALIGN��ĵ�ַ*/
#define LOAD_PHYSICAL_ADDR ((CONFIG_PHYSICAL_START + (CONFIG_PHYSICAL_ALIGN - 1)) \
				& ~(CONFIG_PHYSICAL_ALIGN - 1))

#endif /* _ASM_BOOT_H */
