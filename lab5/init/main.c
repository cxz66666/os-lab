#include "put.h"
#include "sched.h"

int start_kernel()
{
	const char *msg = "ZJU OS LAB 5     Student1:3190104611 陈旭征 \n";
	puts(msg);
	// test_page_fault();
	// test_section_mod();
	task_init();
	dead_loop();

	return 0;
}
