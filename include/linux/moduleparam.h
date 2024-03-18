#ifndef _LINUX_MODULE_PARAMS_H
#define _LINUX_MODULE_PARAMS_H

#include <linux/init.h>
#include <linux/stringify.h>
#include <linux/kernel.h>

/*可手动修改，但通常应该和模块名相同*/
#ifdef MODULE
#define MODULE_PARAM_PREFIX /*空*/
#else
#define MODULE_PARAM_PREFIX KBUILD_MODNAME "."
#endif

#ifdef MODULE
#define ___module_cat(a,b) __mod_##a##b
#define __module_cat(a,b) ___module_cat(a, b)
#define __MODULE_INFO(tag, name, info)					\
static const char __module_cat(name,__LINE__)[]			\
  __attribute_used__							  		\
  __attribute__((section(".modinfo"),unused)) = __stringify(tag) "=" info
#else  /* !MODULE */
#define __MODULE_INFO(tag, name, info)
#endif
#define __MODULE_PARM_TYPE(name, _type) __MODULE_INFO(parmtype, name##type, #name ":" _type)

struct kernel_param;

/*成功返回0，失败返回-errno，参数在kp->arg中*/
typedef int (*param_set_fn)(const char *val, struct kernel_param *kp);
/*成功返回写入长度，失败返回-errno，缓冲区时4K*/
typedef int (*param_get_fn)(char *buffer, struct kernel_param *kp);

/*二进制文件对每个内核参数都包含了一个对应的kernel_param实例，这一点对动态加载的模块
和静态的内核二进制映像文件都是成立的*/
struct kernel_param
{
	/*参数名称*/
	const char *name;
	/**/
	unsigned int perm;
	/*设置参数值的函数*/
	param_set_fn set;
	/*获取参数值的函数*/
	param_get_fn get;
	union
	{
		/*一个可选的参数，也会传递到这两个函数，它使得同一个函数可以对不同的参数使用，
		该指针也可以具体解释为字符串或数组*/
		void *arg;
		/*参数时字符串形式*/
		const struct kparam_string *str;
		/*参数时数组的形式*/
		const struct kparam_array *arr;
	};
};

/* Special one for strings we want to copy into */
/*一个预复制的字符串*/
struct kparam_string
{
	unsigned int maxlen;
	char *string;
};

/* Special one for arrays */
/*数组参数*/
struct kparam_array
{
	/**/
	unsigned int max;
	/**/
	unsigned int *num;
	/**/
	param_set_fn set;
	/**/
	param_get_fn get;
	/**/
	unsigned int elemsize;
	/**/
	void *elem;
};

/* This is the fundamental function for registering boot/module
   parameters.  perm sets the visibility in sysfs: 000 means it's
   not there, read bits mean it's readable, write bits mean it's
   writable. */
/*该宏是注册启动参数和模块参数的基础函数。perm设置sysfs中权限：读位意味着可读，写位意味着可写*/
#define __module_param_call(prefix, name, set, get, arg, perm)						\
	/*默认值代替权限？*/																	\
	static int __param_perm_check_##name __attribute__((unused)) =					\
	BUILD_BUG_ON_ZERO((perm) < 0 || (perm) > 0777 || ((perm) & 2));			\
	static const char __param_str_##name[] = prefix #name;							\
	static struct kernel_param const __param_##name									\
	__attribute_used__																\
    __attribute__ ((unused,__section__ ("__param"),aligned(sizeof(void *)))) 		\
	= { __param_str_##name, perm, set, get, { arg } }

#define module_param_call(name, set, get, arg, perm)			      \
	__module_param_call(MODULE_PARAM_PREFIX, name, set, get, arg, perm)

/* Helper functions: type is byte, short, ushort, int, uint, long,
   ulong, charp, bool or invbool, or XXX if you define param_get_XXX,
   param_set_XXX and param_check_XXX. */
#define module_param_named(name, value, type, perm) param_check_##type(name, &(value));\
	module_param_call(name, param_set_##type, param_get_##type, &value, perm); 						\
	__MODULE_PARM_TYPE(name, #type)

/*该宏将参数注册到内核。用适当的值填充一个kernel_param的实例，并将其写入到二进制文件
的__param段*/
#define module_param(name, type, perm)	module_param_named(name, name, type, perm)

/*实际复制字符串：len参数通常是sizeof(string)*/
#define module_param_string(name, string, len, perm)			\
	static const struct kparam_string __param_string_##name	= { len, string };					\
	module_param_call(name, param_set_copystring, param_get_string,	\
			  .str = &__param_string_##name, perm);		\
	__MODULE_PARM_TYPE(name, "string")

/*内核启动或模块插入时调用*/
extern int parse_args(const char *name, char *args, struct kernel_param *params,
		      			unsigned num, int (*unknown)(char *param, char *val));

/* All the helper functions */
/* The macros to do compile-time type checking stolen from Jakub
   Jelinek, who IIRC came up with this idea for the 2.4 module init code. */
#define __param_check(name, p, type)	static inline type *__check_##name(void) { return(p); }

extern int param_set_byte(const char *val, struct kernel_param *kp);
extern int param_get_byte(char *buffer, struct kernel_param *kp);
#define param_check_byte(name, p) __param_check(name, p, unsigned char)

extern int param_set_short(const char *val, struct kernel_param *kp);
extern int param_get_short(char *buffer, struct kernel_param *kp);
#define param_check_short(name, p) __param_check(name, p, short)

extern int param_set_ushort(const char *val, struct kernel_param *kp);
extern int param_get_ushort(char *buffer, struct kernel_param *kp);
#define param_check_ushort(name, p) __param_check(name, p, unsigned short)

extern int param_set_int(const char *val, struct kernel_param *kp);
extern int param_get_int(char *buffer, struct kernel_param *kp);
#define param_check_int(name, p) __param_check(name, p, int)

extern int param_set_uint(const char *val, struct kernel_param *kp);
extern int param_get_uint(char *buffer, struct kernel_param *kp);
#define param_check_uint(name, p) __param_check(name, p, unsigned int)

extern int param_set_long(const char *val, struct kernel_param *kp);
extern int param_get_long(char *buffer, struct kernel_param *kp);
#define param_check_long(name, p) __param_check(name, p, long)

extern int param_set_ulong(const char *val, struct kernel_param *kp);
extern int param_get_ulong(char *buffer, struct kernel_param *kp);
#define param_check_ulong(name, p) __param_check(name, p, unsigned long)

extern int param_set_charp(const char *val, struct kernel_param *kp);
extern int param_get_charp(char *buffer, struct kernel_param *kp);
#define param_check_charp(name, p) __param_check(name, p, char *)

extern int param_set_bool(const char *val, struct kernel_param *kp);
extern int param_get_bool(char *buffer, struct kernel_param *kp);
#define param_check_bool(name, p) __param_check(name, p, int)

extern int param_set_invbool(const char *val, struct kernel_param *kp);
extern int param_get_invbool(char *buffer, struct kernel_param *kp);
#define param_check_invbool(name, p) __param_check(name, p, int)

/* Comma-separated array: *nump is set to number they actually specified. */
#define module_param_array_named(name, array, type, nump, perm)		\
	static const struct kparam_array __param_arr_##name		\
	= { ARRAY_SIZE(array), nump, param_set_##type, param_get_##type, sizeof(array[0]), array };\
	module_param_call(name, param_array_set, param_array_get,.arr = &__param_arr_##name, perm);\
	__MODULE_PARM_TYPE(name, "array of " #type)

#define module_param_array(name, type, nump, perm)	module_param_array_named(name, name, type, nump, perm)

extern int param_array_set(const char *val, struct kernel_param *kp);
extern int param_array_get(char *buffer, struct kernel_param *kp);

extern int param_set_copystring(const char *val, struct kernel_param *kp);
extern int param_get_string(char *buffer, struct kernel_param *kp);

/* for exporting parameters in /sys/parameters */

struct module;

#if defined(CONFIG_SYSFS) && defined(CONFIG_MODULES)
extern int module_param_sysfs_setup(struct module *mod, struct kernel_param *kparam,
				    						unsigned int num_params);

extern void module_param_sysfs_remove(struct module *mod);
#else
static inline int module_param_sysfs_setup(struct module *mod, struct kernel_param *kparam,
			     								unsigned int num_params)
{
	return 0;
}

static inline void module_param_sysfs_remove(struct module *mod)
{ }
#endif

#endif /* _LINUX_MODULE_PARAMS_H */
