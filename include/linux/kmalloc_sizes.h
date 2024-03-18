/*���ҳ������4KB��������С32B��ͨ�û���*/
#if (PAGE_SIZE == 4096)
	CACHE(32)
#endif
	/*����64B����ͨ�û���*/
	CACHE(64)
/*���L1�����г���С��64B�����峤��Ϊ96B��ͨ�û���*/
#if L1_CACHE_BYTES < 64
	CACHE(96)
#endif
	/*���峤��Ϊ128B��ͨ�û���*/
	CACHE(128)
/*���L1�����г���С��128B�����峤��Ϊ192B��ͨ�û���*/
#if L1_CACHE_BYTES < 128
	CACHE(192)
#endif
	/*���峤��Ϊ256B��ͨ�û���*/
	CACHE(256)
	/*���峤��Ϊ512B��ͨ�û���*/
	CACHE(512)
	/*���峤��Ϊ1KB��ͨ�û���*/
	CACHE(1024)
	/*���峤��Ϊ2KBB��ͨ�û���*/
	CACHE(2048)
	/*���峤��Ϊ4KB��ͨ�û���*/
	CACHE(4096)
	/*���峤��Ϊ8KB��ͨ�û���*/
	CACHE(8192)
	/*���峤��Ϊ16KB��ͨ�û���*/
	CACHE(16384)
	/*���峤��Ϊ32KB��ͨ�û���*/
	CACHE(32768)
	/*���峤��Ϊ64KB��ͨ�û���*/
	CACHE(65536)
	/*���峤��Ϊ128KB��ͨ�û���*/
	CACHE(131072)
/*���KMALLOC_MAX_SIZE����2^18�����峤��Ϊ256KB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 262144
	CACHE(262144)
#endif
/*���KMALLOC_MAX_SIZE����2^19�����峤��Ϊ512KB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 524288
	CACHE(524288)
#endif
/*���KMALLOC_MAX_SIZE����2^20�����峤��Ϊ1MB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 1048576
	CACHE(1048576)
#endif
/*���KMALLOC_MAX_SIZE����2^21�����峤��Ϊ2MB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 2097152
	CACHE(2097152)
#endif
/*���KMALLOC_MAX_SIZE����2^22�����峤��Ϊ4MB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 4194304
	CACHE(4194304)
#endif
/*���KMALLOC_MAX_SIZE����2^23�����峤��Ϊ8MB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 8388608
	CACHE(8388608)
#endif
/*���KMALLOC_MAX_SIZE����2^24�����峤��Ϊ16MB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 16777216
	CACHE(16777216)
#endif
/*���KMALLOC_MAX_SIZE����2^25�����峤��Ϊ32MB��ͨ�û���*/
#if KMALLOC_MAX_SIZE >= 33554432
	CACHE(33554432)
#endif
