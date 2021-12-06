#include "put.h"
#include "sched.h"
#include "slub.h"
int start_kernel()
{
	const char *msg = "ZJU OS LAB 6     Student1:3190104611 陈旭征 \n";
	puts(msg);
	// test_page_fault();
	// test_section_mod();
	slub_init();
	task_init();
	dead_loop();

	return 0;
}
