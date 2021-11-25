#include "put.h"
#include "sched.h"
#define uint64_t unsigned long long
int count = 0;
int handler_s(uint64_t cause)
{
	if (cause >> 63)
	{ // interrupt
		if (((cause << 1) >> 1) == 5)
		{ // supervisor timer interrupt
			asm volatile("ecall");
			do_timer();
			count++;
		}
		return 1;
	}
	else
	{

		if (cause == 15)
		{
			puts("Store Page Fault\n");
		}
		else if (cause == 13)
		{
			puts("Load Page Fault\n");
		}
		else if (cause == 12)
		{
			puts("Instruction Page Fault\n");
		}
		return 0;
	}
	return 0;
}
