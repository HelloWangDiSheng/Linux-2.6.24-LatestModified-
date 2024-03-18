#ifndef _ASM_I386_TOPOLOGY_H
#define _ASM_I386_TOPOLOGY_H

/*配置X86超线程*/
#ifdef CONFIG_X86_HT
#define topology_physical_package_id(cpu)	(cpu_data(cpu).phys_proc_id)
#define topology_core_id(cpu)			(cpu_data(cpu).cpu_core_id)
#define topology_core_siblings(cpu)		(per_cpu(cpu_core_map, cpu))
#define topology_thread_siblings(cpu)		(per_cpu(cpu_sibling_map, cpu))
#endif

#ifdef CONFIG_NUMA

#include <asm/mpspec.h>

#include <linux/cpumask.h>

/*逻辑cpu编号和结点编号之间建立映射关系*/
extern cpumask_t node_2_cpu_mask[];
extern int cpu_2_node[];

/*包含CPU'cpu'的结点编号*/
static inline int cpu_to_node(int cpu)
{ 
	return cpu_2_node[cpu];
}

/* Returns the number of the node containing Node 'node'.  This architecture is flat, 
   so it is a pretty simple function! */
#define parent_node(node) (node)

/* Returns a bitmask of CPUs on Node 'node'. */
static inline cpumask_t node_to_cpumask(int node)
{
	return node_2_cpu_mask[node];
}

/* Returns the number of the first CPU on Node 'node'. */
static inline int node_to_first_cpu(int node)
{ 
	cpumask_t mask = node_to_cpumask(node);
	return first_cpu(mask);
}

#define pcibus_to_node(bus) ((struct pci_sysdata *)((bus)->sysdata))->node
#define pcibus_to_cpumask(bus) node_to_cpumask(pcibus_to_node(bus))

/* sched_domains SD_NODE_INIT for NUMAQ machines */
#define SD_NODE_INIT (struct sched_domain)			\
{													\
	.span			= CPU_MASK_NONE,				\
	.parent			= NULL,							\
	.child			= NULL,							\
	.groups			= NULL,							\
	.min_interval		= 8,						\
	.max_interval		= 32,						\
	.busy_factor		= 32,						\
	.imbalance_pct		= 125,						\
	.cache_nice_tries	= 1,						\
	.busy_idx		= 3,							\
	.idle_idx		= 1,							\
	.newidle_idx		= 2,						\
	.wake_idx		= 1,							\
	.flags			= SD_LOAD_BALANCE | SD_BALANCE_EXEC	| SD_BALANCE_FORK	| SD_SERIALIZE		\
					| SD_WAKE_BALANCE,				\
	.last_balance		= jiffies,					\
	.balance_interval	= 1,						\
	.nr_balance_failed	= 0,						\
}

extern unsigned long node_start_pfn[];
extern unsigned long node_end_pfn[];
extern unsigned long node_remap_size[];

#define node_has_online_mem(nid) (node_start_pfn[nid] != node_end_pfn[nid])

#else /* !CONFIG_NUMA */
/*
 * Other i386 platforms should define their own version of the 
 * above macros here.
 */

#include <asm-generic/topology.h>

#endif /* CONFIG_NUMA */

extern cpumask_t cpu_coregroup_map(int cpu);

#ifdef CONFIG_SMP
#define mc_capable()	(boot_cpu_data.x86_max_cores > 1)
#define smt_capable()	(smp_num_siblings > 1)
#endif

#endif /* _ASM_I386_TOPOLOGY_H */
