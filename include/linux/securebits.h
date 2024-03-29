#ifndef _LINUX_SECUREBITS_H
#define _LINUX_SECUREBITS_H 1

/*安全管理标志（securebits）是一种仅作用于任务tasks的概念，它们用于控制上述凭证在特定操作
（如execve）中的操作和继承方式，与capability类似，安全管理标志并不直接用作客观或主观凭证*/

#define SECUREBITS_DEFAULT 0x00000000

extern unsigned securebits;

/* When set UID 0 has no special privileges. When unset, we support
   inheritance of root-permissions and suid-root executable under
   compatibility mode. We raise the effective and inheritable bitmasks
   *of the executable file* if the effective uid of the new process is
   0. If the real uid is 0, we raise the inheritable bitmask of the
   executable file. */
#define SECURE_NOROOT            	0

/* When set, setuid to/from uid 0 does not trigger capability-"fixes"
   to be compatible with old programs relying on set*uid to loose
   privileges. When unset, setuid doesn't change privileges. */
#define SECURE_NO_SETUID_FIXUP	2

/* Each securesetting is implemented using two bits. One bit specify
   whether the setting is on or off. The other bit specify whether the
   setting is fixed or not. A setting which is fixed cannot be changed
   from user-level. */

#define issecure(X) ( (1 << (X+1)) & SECUREBITS_DEFAULT ? 	\
		      (1 << (X)) & SECUREBITS_DEFAULT :		\
		      (1 << (X)) & securebits )

#endif /* !_LINUX_SECUREBITS_H */
