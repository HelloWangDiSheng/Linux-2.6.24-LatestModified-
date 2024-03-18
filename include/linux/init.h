#ifndef _LINUX_INIT_H
#define _LINUX_INIT_H

#include <linux/compiler.h>

/*__init相关宏经常被用来标识一些函数或已初始化的变量（警告！不能用于未初始化的变量），
 被标记为__init的函数只能在内核初始化阶段使用，其所使用的内存资源之后会被释放，标记
 可以在函数定义时函数名之前，也可以放在函数声明处的分号之前的末尾。对标记为__initdata
 的已初始化数据，该标记应该被放置在变量名和赋值等号之间。勿忘初始化非文件作用域的数据，
 例如，在函数内部，否则gcc将数据放置到堆段而非初始化段中，还要提示，该数据不能为const
 类型，*/

/* These are for everybody (although not all archs will actually
   discard it in modules) */
/*section属性允许编译器将变量和函数置于二进制文件的不同于通常设置的其他段中。在属性中
，必须以字符串参数形式定义相关变量或函数的目标段名*/
/**/
#define __init		__attribute__ ((__section__ (".init.text"))) __cold
/**/
#define __initdata	__attribute__ ((__section__ (".init.data")))
/**/
#define __exitdata	__attribute__ ((__section__(".exit.data")))
/**/
#define __exit_call	__attribute_used__ __attribute__ ((__section__ (".exitcall.exit")))

/* modpost check for section mismatches during the kernel build.
 * A section mismatch happens when there are references from a
 * code or data section to an init section (both code or data).
 * The init sections are (for most archs) discarded by the kernel
 * when early init has completed so all such references are potential bugs.
 * For exit sections the same issue exists.
 * The following markers are used for the cases where the reference to
 * the init/exit section (code or data) is valid and will teach modpost
 * not to issue a warning.
 * The markers follow same syntax rules as __init / __initdata. */
#define __init_refok     noinline __attribute__ ((__section__ (".text.init.refok")))
#define __initdata_refok          __attribute__ ((__section__ (".data.init.refok")))
#define __exit_refok     noinline __attribute__ ((__section__ (".exit.text.refok")))

#ifdef MODULE
#define __exit		__attribute__ ((__section__(".exit.text"))) __cold
#else
#define __exit		__attribute_used__ __attribute__ ((__section__(".exit.text"))) __cold
#endif

/*汇编函数*/
#define __INIT					.section	".init.text","ax"
#define __INIT_REFOK			.section	".text.init.refok","ax"
#define __FINIT					.previous
#define __INITDATA				.section	".init.data","aw"
#define __INITDATA_REFOK		.section	".data.init.refok","aw"

#ifndef __ASSEMBLY__

/*初始化期间调用的初始化及退出函数指针定义*/
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);

extern initcall_t __con_initcall_start[], __con_initcall_end[];
extern initcall_t __security_initcall_start[], __security_initcall_end[];


/*init/main.c中定义*/
extern char __initdata boot_command_line[];
extern char *saved_command_line;
extern unsigned int reset_devices;

/*init/main.c中使用*/
void setup_arch(char **);
void prepare_namespace(void);

#endif
  
#ifndef MODULE

#ifndef __ASSEMBLY__
/*initcalls目前按函数功能分组到单独的子段中，在子段内的排序是根据连接顺序决定的。
为向后兼容，initcall()将该函数放置到设备初始化子段中。对__define_initcall()来说，
id参数时需要的，因此多个initcalls能指向同一个句柄而不会引起复制符号创建错误*/

/*为定义init调用，编译器使用了下列宏将函数置于末尾.inticall0.init的段中。该宏用来
检测初始化例程并定义其顺序或优先级。函数名称作为参数传递给宏，如
core_initcall(init_elf_binfmt)，则该宏扩展后如下所示：
static initcall_t _initcall_init_elf_binfmt_1 __attribute_used__
__attribute__((__section__(".initcall1.init"))) = init_elf_binfmt;
定义__initcall_init_elf_binfmt1为一个初始化函数指针，并将该变量赋值为init_elf_binfmt，
保存在".initcall1.init"代码段中*/
#define __define_initcall(level,fn,id) 								\
	static initcall_t __initcall_##fn##id __attribute_used__ \
	__attribute__((__section__(".initcall" level ".init"))) = fn

/*
 * A "pure" initcall has no dependencies on anything else, and purely
 * initializes variables that couldn't be statically initialized.
 *
 * This only exists for built-in code, not for modules.
 */
#define pure_initcall(fn)					__define_initcall("0",fn,0)

#define core_initcall(fn)					__define_initcall("1",fn,1)
#define core_initcall_sync(fn)				__define_initcall("1s",fn,1s)
#define postcore_initcall(fn)				__define_initcall("2",fn,2)
#define postcore_initcall_sync(fn)			__define_initcall("2s",fn,2s)
#define arch_initcall(fn)					__define_initcall("3",fn,3)
#define arch_initcall_sync(fn)				__define_initcall("3s",fn,3s)
#define subsys_initcall(fn)					__define_initcall("4",fn,4)
#define subsys_initcall_sync(fn)			__define_initcall("4s",fn,4s)
#define fs_initcall(fn)						__define_initcall("5",fn,5)
#define fs_initcall_sync(fn)				__define_initcall("5s",fn,5s)
#define rootfs_initcall(fn)					__define_initcall("rootfs",fn,rootfs)
#define device_initcall(fn)					__define_initcall("6",fn,6)
#define device_initcall_sync(fn)			__define_initcall("6s",fn,6s)
#define late_initcall(fn)					__define_initcall("7",fn,7)
#define late_initcall_sync(fn)				__define_initcall("7s",fn,7s)

#define __initcall(fn) device_initcall(fn)
#define __exitcall(fn) static exitcall_t __exitcall_##fn __exit_call = fn

#define console_initcall(fn) \
	static initcall_t __initcall_##fn \
	__attribute_used__ __attribute__((__section__(".con_initcall.init")))=fn

#define security_initcall(fn) \
	static initcall_t __initcall_##fn \
	__attribute_used__ __attribute__((__section__(".security_initcall.init"))) = fn

struct obs_kernel_param
{
	const char *str;
	int (*setup_func)(char *);
	int early;
};

/*
 * Only for really core code.  See moduleparam.h for the normal way.
 *
 * Force the alignment so the compiler doesn't space elements of the
 * obs_kernel_param "array" too far apart in .init.setup.
 */
#define __setup_param(str, unique_id, fn, early)									\
	static char __setup_str_##unique_id[] __initdata __aligned(1) = str;			\
	static struct obs_kernel_param __setup_##unique_id								\
		__attribute_used__															\
		__attribute__((__section__(".init.setup")))									\
		__attribute__((aligned((sizeof(long)))))									\
		= { __setup_str_##unique_id, fn, early }

#define __setup_null_param(str, unique_id)	__setup_param(str, unique_id, NULL, 0)

#define __setup(str, fn)	__setup_param(str, fn, fn, 0)

/* NOTE: fn is as per module_param, not __setup!  Emits warning if fn
 * returns non-zero. */
#define early_param(str, fn) 	__setup_param(str, fn, fn, 1)

/* Relies on boot_command_line being set */
void __init parse_early_param(void);
#endif /* __ASSEMBLY__ */

/**
 * module_init() - driver initialization entry point
 * @x: function to be run at kernel boot time or module insertion
 * 
 * module_init() will either be called during do_initcalls() (if
 * builtin) or at module insertion time (if a module).  There can only
 * be one per module.
 */
#define module_init(x)	__initcall(x);

/**
 * module_exit() - driver exit entry point
 * @x: function to be run when driver is removed
 * 
 * module_exit() will wrap the driver clean-up code
 * with cleanup_module() when used with rmmod when
 * the driver is a module.  If the driver is statically
 * compiled into the kernel, module_exit() has no effect.
 * There can only be one per module.
 */
#define module_exit(x)	__exitcall(x);

#else /* MODULE */

/*不能在模块中使用*/
#define core_initcall(fn)		module_init(fn)
#define postcore_initcall(fn)		module_init(fn)
#define arch_initcall(fn)		module_init(fn)
#define subsys_initcall(fn)		module_init(fn)
#define fs_initcall(fn)			module_init(fn)
#define device_initcall(fn)		module_init(fn)
#define late_initcall(fn)		module_init(fn)

#define security_initcall(fn)		module_init(fn)

/* These macros create a dummy inline: gcc 2.9x does not count alias
 as usage, hence the `unused function' warning when __init functions
 are declared static. We use the dummy __*_module_inline functions
 both to kill the warning and check the type of the init/cleanup
 function. */

/* Each module must use one module_init(), or one no_module_init */
#define module_init(initfn)					\
	static inline initcall_t __inittest(void)		\
	{ return initfn; }					\
	int init_module(void) __attribute__((alias(#initfn)));

/* This is only required if you want to be unloadable. */
#define module_exit(exitfn)					\
	static inline exitcall_t __exittest(void)		\
	{ return exitfn; }					\
	void cleanup_module(void) __attribute__((alias(#exitfn)));

#define __setup_param(str, unique_id, fn)	/* nothing */
#define __setup_null_param(str, unique_id) 	/* nothing */
#define __setup(str, func) 			/* nothing */
#endif

/* Data marked not to be saved by software suspend */
#define __nosavedata __attribute__ ((__section__ (".data.nosave")))

/* This means "can be init if no module support, otherwise module load
   may call it." */
#ifdef CONFIG_MODULES
#define __init_or_module
#define __initdata_or_module
#else
#define __init_or_module __init
#define __initdata_or_module __initdata
#endif /*CONFIG_MODULES*/

#ifdef CONFIG_HOTPLUG
#define __devinit
#define __devinitdata
#define __devexit
#define __devexitdata
#else
#define __devinit __init
#define __devinitdata __initdata
#define __devexit __exit
#define __devexitdata __exitdata
#endif

#ifdef CONFIG_HOTPLUG_CPU
#define __cpuinit
#define __cpuinitdata
#define __cpuexit
#define __cpuexitdata
#else
#define __cpuinit	__init
#define __cpuinitdata __initdata
#define __cpuexit __exit
#define __cpuexitdata	__exitdata
#endif

#if defined(CONFIG_MEMORY_HOTPLUG) || defined(CONFIG_ACPI_HOTPLUG_MEMORY) \
	|| defined(CONFIG_ACPI_HOTPLUG_MEMORY_MODULE)
#define __meminit
#define __meminitdata
#define __memexit
#define __memexitdata
#else
#define __meminit	__init
#define __meminitdata __initdata
#define __memexit __exit
#define __memexitdata	__exitdata
#endif

/* Functions marked as __devexit may be discarded at kernel link time, depending
   on config options.  Newer versions of binutils detect references from
   retained sections to discarded sections and flag an error.  Pointers to
   __devexit functions must use __devexit_p(function_name), the wrapper will
   insert either the function_name or NULL, depending on the config options.
 */
#if defined(MODULE) || defined(CONFIG_HOTPLUG)
#define __devexit_p(x) x
#else
#define __devexit_p(x) NULL
#endif

#ifdef MODULE
#define __exit_p(x) x
#else
#define __exit_p(x) NULL
#endif

#endif /* _LINUX_INIT_H */
