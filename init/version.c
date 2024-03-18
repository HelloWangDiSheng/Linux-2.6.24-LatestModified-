#include <linux/compile.h>
#include <linux/module.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <linux/utsrelease.h>
#include <linux/version.h>

/*版本连接符，如version(linux)则是version_linux*/
#define version(a) Version_##a
#define version_string(a) version(a)

int version_string(LINUX_VERSION_CODE);/*"LINUX_VERSION_CODE"*/

/*全局命名空间中uts命名空间初始化*/
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

/*固定字符串，不要碰*/
const char linux_banner[] =
	"Linux version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
	LINUX_COMPILE_HOST ") (" LINUX_COMPILER ") " UTS_VERSION "\n";

const char linux_proc_banner[] =
	"%s version %s"
	" (" LINUX_COMPILE_BY "@" LINUX_COMPILE_HOST ")"
	" (" LINUX_COMPILER ") %s\n";
