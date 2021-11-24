#include "sched.h"
#include "put.h"
#define offset (0xffffffe000000000 - 0x80000000)

struct task_struct *current;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};

extern void init_epc(void);
extern void __switch_to(struct task_struct *current, struct task_struct *next);
extern unsigned int rand();
extern uint64_t cur;

void task_init(void)
{
}

void do_timer(void)
{
}

void schedule(void)
{
}

void switch_to(struct task_struct *next)
{
}

void dead_loop(void)
{
	while (1)
	{
		continue;
	}
}
