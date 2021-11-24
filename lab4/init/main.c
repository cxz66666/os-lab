#include "put.h"
#include "sched.h"

int start_kernel()
{
	const char *msg = "ZJU OS LAB 4     Student1:3190104611 陈旭征 \n";
	puts(msg);
	task_init();
	dead_loop();

	return 0;
}
