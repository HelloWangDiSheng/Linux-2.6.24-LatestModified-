#ifndef _LINUX_UTS_H
#define _LINUX_UTS_H

#ifndef UTS_SYSNAME
/*����ϵͳ�����޸�Ϊ����û����*/
#define UTS_SYSNAME "Linux"
#endif

#ifndef UTS_NODENAME
/*nodename����ͨ��sethostname����*/
#define UTS_NODENAME "(none)"	
#endif

#ifndef UTS_DOMAINNAME
/*domainname����ͨ��setdomainname����*/
#define UTS_DOMAINNAME "(none)"
#endif

#endif
