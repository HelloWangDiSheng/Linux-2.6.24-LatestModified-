/*如果页长度是4KB，则定义最小32B的通用缓存*/
#if (PAGE_SIZE == 4096)
	CACHE(32)
#endif
	/*定义64B长度通用缓存*/
	CACHE(64)
/*如果L1缓存行长度小于64B，则定义长度为96B的通用缓存*/
#if L1_CACHE_BYTES < 64
	CACHE(96)
#endif
	/*定义长度为128B的通用缓存*/
	CACHE(128)
/*如果L1缓存行长度小于128B，则定义长度为192B的通用缓存*/
#if L1_CACHE_BYTES < 128
	CACHE(192)
#endif
	/*定义长度为256B的通用缓存*/
	CACHE(256)
	/*定义长度为512B的通用缓存*/
	CACHE(512)
	/*定义长度为1KB的通用缓存*/
	CACHE(1024)
	/*定义长度为2KBB的通用缓存*/
	CACHE(2048)
	/*定义长度为4KB的通用缓存*/
	CACHE(4096)
	/*定义长度为8KB的通用缓存*/
	CACHE(8192)
	/*定义长度为16KB的通用缓存*/
	CACHE(16384)
	/*定义长度为32KB的通用缓存*/
	CACHE(32768)
	/*定义长度为64KB的通用缓存*/
	CACHE(65536)
	/*定义长度为128KB的通用缓存*/
	CACHE(131072)
/*如果KMALLOC_MAX_SIZE大于2^18，则定义长度为256KB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 262144
	CACHE(262144)
#endif
/*如果KMALLOC_MAX_SIZE大于2^19，则定义长度为512KB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 524288
	CACHE(524288)
#endif
/*如果KMALLOC_MAX_SIZE大于2^20，则定义长度为1MB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 1048576
	CACHE(1048576)
#endif
/*如果KMALLOC_MAX_SIZE大于2^21，则定义长度为2MB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 2097152
	CACHE(2097152)
#endif
/*如果KMALLOC_MAX_SIZE大于2^22，则定义长度为4MB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 4194304
	CACHE(4194304)
#endif
/*如果KMALLOC_MAX_SIZE大于2^23，则定义长度为8MB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 8388608
	CACHE(8388608)
#endif
/*如果KMALLOC_MAX_SIZE大于2^24，则定义长度为16MB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 16777216
	CACHE(16777216)
#endif
/*如果KMALLOC_MAX_SIZE大于2^25，则定义长度为32MB的通用缓存*/
#if KMALLOC_MAX_SIZE >= 33554432
	CACHE(33554432)
#endif
