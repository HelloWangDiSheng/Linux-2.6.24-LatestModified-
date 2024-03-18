#include <linux/compile.h>
#include <linux/module.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <linux/utsrelease.h>
#include <linux/version.h>

/*�汾���ӷ�����version(linux)����version_linux*/
#define version(a) Version_##a
#define version_string(a) version(a)

int version_string(LINUX_VERSION_CODE);/*"LINUX_VERSION_CODE"*/

/*ȫ�������ռ���uts�����ռ��ʼ��*/
struct uts_namespace init_uts_ns =
{
	.kref =
	{
		.refcount	= ATOMIC_INIT(2),
	},
	.name =
	{
		.sysname	= UTS_SYSNAME,
		.nodename	= UTS_NODENAME,
		.release	= UTS_RELEASE,
		.version	= UTS_VERSION,
		.machine	= UTS_MACHINE,
		.domainname	= UTS_DOMAINNAME,
	},
};
EXPORT_SYMBOL_GPL(init_uts_ns);

/*�̶��ַ�������Ҫ��*/
const char linux_banner[] =
	"Linux version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
	LINUX_COMPILE_HOST ") (" LINUX_COMPILER ") " UTS_VERSION "\n";

const char linux_proc_banner[] =
	"%s version %s"
	" (" LINUX_COMPILE_BY "@" LINUX_COMPILE_HOST ")"
	" (" LINUX_COMPILER ") %s\n";
