#ifndef _LINUX_UTS_H
#define _LINUX_UTS_H

#ifndef UTS_SYSNAME
/*定义系统名，修改为其它没意义*/
#define UTS_SYSNAME "Linux"
#endif

#ifndef UTS_NODENAME
/*nodename可以通过sethostname设置*/
#define UTS_NODENAME "(none)"	
#endif

#ifndef UTS_DOMAINNAME
/*domainname可以通过setdomainname设置*/
#define UTS_DOMAINNAME "(none)"
#endif

#endif
