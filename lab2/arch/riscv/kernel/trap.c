#include "defs.h"

extern main(), puts(), put_num(), ticks;
extern void clock_set_next_event(void);

void handler_s(uint64_t cause)
{
	// interrupt
	if (cause & 0x8000000000000000)
	{
		// supervisor timer interrupt
		if (cause == 0x8000000000000005)
		{
			clock_set_next_event();
			puts("[S] Supervisor Mode Timer Interrupt ");
			++ticks;
			put_num(ticks);
			puts("\n");
		}
	}
}