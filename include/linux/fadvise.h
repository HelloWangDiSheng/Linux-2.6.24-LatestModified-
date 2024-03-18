#ifndef FADVISE_H_INCLUDED
#define FADVISE_H_INCLUDED

/*设置文件最大预读页数ra_pages为默认值32页*/
#define POSIX_FADV_NORMAL			0
/*进程期望以随机顺序访问指定的文件数据。ra_pages 设置为 0，表示禁止预读*/
#define POSIX_FADV_RANDOM			1
/*进程期望顺序访问指定的文件数据，ra_pages 值为默认值的两倍*/
#define POSIX_FADV_SEQUENTIAL		2
/*通知内核，进程指定这段文件数据将在不久之后被访问*/
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
