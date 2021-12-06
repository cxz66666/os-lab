#include "sched.h"
#include "put.h"

struct task_struct *current;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 4, 3, 2, 1};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};
#define U64ADDR(num) ((uint64_t)&num)

extern void init_epc(void);
extern void __switch_to(struct task_struct *current, struct task_struct *next);
extern unsigned int rand();
extern uint64_t cur;
extern uint64_t _end;
extern uint64_t offset;
extern uint64_t rodata_start;
extern uint64_t *user_paging_init();
extern uint64_t USER_SIZE;
extern uint64_t USER_STACK_TOP;
void test_section_mod()
{
	//对rodata的写入操作
	uint64_t *rodata = U64ADDR(rodata_start) + 8;
	*rodata = 1;
	rodata++;
	*rodata = 2;

	//对data段执行操作
	asm volatile(
		"la t1,next\n"
		"la t2,data_end\n"
		"sd t1,0(t2)\n"
		"mv s1,ra\n"
		"call data_start\n"
		"mv ra,s1\n"
		"next: add x0,x1,x2\n");
}

void test_page_fault()
{
	//恰好不在pt里面
	uint64_t *begin = U64ADDR(_end) + 0x1000008;
	//读取
	putullHex(*begin);
	putullHex(*(begin + 1));
	//写入
	*begin = 1;
	*(begin + 1) = 2;
}

void switch_to(struct task_struct *next)
{
	/*your code*/
	if (current->pid == next->pid)
	{
		return;
	}
	struct task_struct *prev = current;
	current = next;
	__switch_to(prev, current);
}

void dead_loop(void)
{
	while (1)
	{
		continue;
	}
}

void task_init(void)
{
	puts("task init...\n");

	for (int i = 0; i <= LAB_TEST_NUM; ++i)
	{
		++cur;
		// putullHex(U64ADDR(_end));
		current = (struct task_struct *)(U64ADDR(_end) + (cur << 12));
		/*your code*/
		current->state = TASK_RUNNING;
		current->counter = COUNTER_INIT_COUNTER[i];
		current->priority = PRIORITY_INIT_COUNTER[i];
		current->blocked = 0;
		current->pid = i;
		current->thread.ra = (unsigned long long)init_epc;
		// putullHex(current);
		current->thread.sp = USER_STACK_TOP;

		//初始化的时候sscratch 设置为内核栈的值，但是之后sscratch存的是用户栈的值
		current->sscratch = (unsigned long long)current + TASK_SIZE;
		current->mm.pgtbl = user_paging_init();
		current->mm.user_size = USER_SIZE;

		puts("[PID = ");
		puti(current->pid);
		puts(" Process Create Successfully!\n");
		// printf("[PID = %d] Process Create Successfully!\n", current->pid);
		task[i] = current;
	}
	current = task[0];

	//初始化sscratch为task[0]的内核栈。
	asm volatile(
		"la t1,current\n"
		"ld t1,0(t1)\n"
		"ld t1,160(t1)\n"
		"csrw sscratch,t1");
}

#ifdef SJF
// simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void)
{
	puts("[*PID = ");
	puti(current->pid);
	puts("] Context Calculation: counter = ");
	puti(current->counter);
	puts(",priority = ");
	puti(current->priority);
	puts("\n");
	// printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter, current->priority);
	// current process's counter -1, judge whether to schedule or go on.
	/*your code*/
	if (!current->counter)
	{
		schedule();
	}
	current->counter--;
	if (!current->counter)
	{
		schedule();
	}
	return;
}

// Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would
// assign new test case.
void schedule(void)
{
	unsigned char next;
	/*your code*/
	next = 0;
	long minCounter = 0xfffffff;
	for (int i = LAB_TEST_NUM; i > 0; i--)
	{
		if (task[i]->counter && task[i]->counter < minCounter)
		{
			minCounter = task[i]->counter;
			next = i;
		}
	}
	if (!next)
	{
		puts("\n---------------New loop---------------\n");
		for (int i = 1; i <= LAB_TEST_NUM; i++)
		{
			task[i]->counter = rand();
			puts("[PID = ");
			puti(i);
			puts("] get a new counter: ");
			puti(task[i]->counter);
			puts("\n");
		}
		puts("\n-----------------------------------\n\n");
		schedule();
		return;
	}
	if (current->pid != task[next]->pid)
	{
		puts("[ ");
		puti(current->pid);
		puts(" -> ");
		puti(task[next]->pid);
		puts(", prio: ");
		puti(task[next]->priority);
		puts(", counter: ");
		puti(task[next]->counter);
		puts("\n");
		// printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n",
		// 	   current->pid, task[next]->pid,
		// 	   current->pid, (unsigned long)current->thread.sp,
		// 	   task[next]->pid, (unsigned long)task[next],
		// 	   task[next]->priority, task[next]->counter);
	}
	switch_to(task[next]);
}

#endif

#ifdef PRIORITY

// simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void)
{
	if (!task_init_done)
		return;
	if (task_test_done)
		return;
	printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter, current->priority);
	// current process's counter -1, judge whether to schedule or go on.
	/*your code*/

	current->counter--;
	schedule();
	return;
}

// Select the next task to run. If all tasks are done(counter=0), set task0's counter to 1 and it would
// assign new test case.
void schedule(void)
{
	unsigned char next;
	/*your code*/
	next = 0;
	long MaxPriority = -1;
	for (int i = LAB_TEST_NUM; i > 0; i--)
	{
		if (task[i]->counter && task[i]->priority > MaxPriority)
		{
			MaxPriority = task[i]->priority;
		}
	}

	long minCounter = 0xfffffff;

	for (int i = LAB_TEST_NUM; i > 0; i--)
	{
		if (task[i]->priority == MaxPriority && task[i]->counter && task[i]->counter < minCounter)
		{
			minCounter = task[i]->counter;
			next = i;
		}
	}
	if (!next)
	{
		task[0]->counter++;
	}
	if (current->pid != task[next]->pid)
	{
		printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n",
			   current->pid, task[next]->pid,
			   current->pid, (unsigned long long)current->thread.sp,
			   task[next]->pid, (unsigned long long)task[next],
			   task[next]->priority, task[next]->counter);
	}
	switch_to(task[next]);
}

#endif
