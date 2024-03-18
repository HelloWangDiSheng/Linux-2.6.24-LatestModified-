#ifndef _LINUX_NUMA_H
#define _LINUX_NUMA_H

/*配置系统中NUMA结点偏移位数目，以此计算NUMA结点的数目*/
#ifdef CONFIG_NODES_SHIFT
#define NODES_SHIFT     CONFIG_NODES_SHIFT
#else
#define NODES_SHIFT     0
#endif
#define MAX_NUMNODES    (1 << NODES_SHIFT)

#endif /* _LINUX_NUMA_H */
