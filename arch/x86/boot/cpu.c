/*
 * arch/i386/boot/cpu.c
 *
 * Check for obligatory CPU features and abort if the features are not
 * present.
 */
/*i386�ܹ�����cpuԴ�ļ������cpu������е����ԣ�������Բ�����ʱ��ֹ*/

#include "boot.h"
#include "bitops.h"
#include <asm/cpufeature.h>

/*��ȡcpu��Ϣ����i386��i486��x86-64*/
static char *cpu_name(int level)
{
	static char buf[6];

	if (level == 64)
	{
		return "x86-64";
	}
	else
	{
		sprintf(buf, "i%d86", level);
		return buf;
	}
}

int validate_cpu(void)
{
	u32 *err_flags;
	int cpu_level, req_level;

	check_cpu(&cpu_level, &req_level, &err_flags);

	if (cpu_level < req_level)
	{
		printf("This kernel requires an %s CPU, ",
		       cpu_name(req_level));
		printf("but only detected an %s CPU.\n",
		       cpu_name(cpu_level));
		return -1;
	}

	if (err_flags)
	{
		int i, j;
		puts("This kernel requires the following features "
		     "not present on the CPU:\n");

		for (i = 0; i < NCAPINTS; i++)
		{
			u32 e = err_flags[i];

			for (j = 0; j < 32; j++)
			{
				if (e & 1)
					printf("%d:%d ", i, j);

				e >>= 1;
			}
		}
		putchar('\n');
		return -1;
	} 
	else
	{
		return 0;
	}
}
