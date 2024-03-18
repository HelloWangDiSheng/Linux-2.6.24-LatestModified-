/*随机数字生成器头文件*/

#ifndef _LINUX_RANDOM_H
#define _LINUX_RANDOM_H

#include <linux/ioctl.h>

/* 随机数生成器ioctl()操作*/

/*获取熵计数*/
#define RNDGETENTCNT	_IOR( 'R', 0x00, int)

/*仅限超级用户使用的增加或减少熵计数*/
#define RNDADDTOENTCNT	_IOW( 'R', 0x01, int)

/*仅限超级用户使用的获取熵池内容*/
#define RNDGETPOOL	_IOR( 'R', 0x02, int [2])

/*仅限超级用户使用的向熵池写数据和增加熵计数*/
#define RNDADDENTROPY	_IOW( 'R', 0x03, int [2] )

/*仅限超级用户使用的清零熵计数*/
#define RNDZAPENTCNT	_IO( 'R', 0x04 )

/*仅限超级用户使用的清空熵池内容和相关计数器*/
#define RNDCLEARPOOL	_IO( 'R', 0x06 )

struct rand_pool_info
{
	int	entropy_count;
	int	buf_size;
	__u32	buf[0];
};

/*导出函数*/
#ifdef __KERNEL__

extern void rand_initialize_irq(int irq);
extern void add_input_randomness(unsigned int type, unsigned int code, unsigned int value);
extern void add_interrupt_randomness(int irq);
extern void get_random_bytes(void *buf, int nbytes);
void generate_random_uuid(unsigned char uuid_out[16]);
extern __u32 secure_ip_id(__be32 daddr);
extern u32 secure_ipv4_port_ephemeral(__be32 saddr, __be32 daddr, __be16 dport);
extern u32 secure_ipv6_port_ephemeral(const __be32 *saddr, const __be32 *daddr, __be16 dport);
extern __u32 secure_tcp_sequence_number(__be32 saddr, __be32 daddr, __be16 sport, __be16 dport);
extern __u32 secure_tcpv6_sequence_number(__be32 *saddr, __be32 *daddr, __be16 sport, __be16 dport);
extern u64 secure_dccp_sequence_number(__be32 saddr, __be32 daddr, __be16 sport, __be16 dport);

#ifndef MODULE
extern const struct file_operations random_fops, urandom_fops;
#endif

unsigned int get_random_int(void);
unsigned long randomize_range(unsigned long start, unsigned long end, unsigned long len);
u32 random32(void);
void srandom32(u32 seed);

#endif /* __KERNEL___ */

#endif /* _LINUX_RANDOM_H */
