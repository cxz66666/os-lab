#include "sched.h"
#include "buddy.h"
#include "defs.h"
#include "put.h"
#include "slub.h"
#include "syscall.h"
#include "types.h"
struct task_struct *current;
int task_num = 0;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 4, 3, 2, 1};
long COUNTER_INIT_COUNTER[NR_TASKS] = {1, 1, 2, 3, 4};

extern void init_epc(void);
extern void ret_from_fork(uint64 *stack);
extern void __switch_to(struct task_struct *current, struct task_struct *next);
extern unsigned int rand();
extern uint64 rodata_start;
extern uint64 user_paging_init();
extern uint64 USER_SIZE;

void test_section_mod()
{
	//对rodata的写入操作
	uint64 *rodata = (uint64 *)((uint64)&rodata_start + 8);
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
	// uint64 *begin = U64ADDR(_end) + 0x1000008;
	// //读取
	// putullHex(*begin);
	// putullHex(*(begin + 1));
	// //写入
	// *begin = 1;
	// *(begin + 1) = 2;
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

void forkret()
{
	return ret_from_fork((uint64 *)current->stack);
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

	//初始化task0
	current = (struct task_struct *)(kmalloc(sizeof(struct task_struct)));
	current->state = TASK_RUNNING;
	current->counter = COUNTER_INIT_COUNTER[task_num];
	current->priority = PRIORITY_INIT_COUNTER[task_num];
	current->blocked = 0;
	current->pid = task_num;
	current->stack = (struct pt_regs *)kmalloc(sizeof(struct pt_regs));
	current->sscratch = (unsigned long long)alloc_pages(1) + PGSIZE;

	printk("[PID = %d ] Process Create Successfully!\n", current->pid);
	task[task_num] = current;
	task_num++;
	//初始化task0结束

	// putullHex(U64ADDR(_end));
	current = (struct task_struct *)(kmalloc(sizeof(struct task_struct)));
	/*your code*/
	current->state = TASK_RUNNING;
	current->counter = COUNTER_INIT_COUNTER[task_num];
	current->priority = PRIORITY_INIT_COUNTER[task_num];
	current->blocked = 0;
	current->pid = task_num;
	current->thread.ra = (unsigned long long)init_epc;
	// putullHex(current);
	current->thread.sp = VM_USER_STACK_TOP;

	//初始化的时候sscratch 设置为内核栈的值，但是之后sscratch存的是用户栈的值
	current->sscratch = (unsigned long long)alloc_pages(1) + PGSIZE;
	current->mm.pgtbl = user_paging_init();
	current->mm.user_size = USER_SIZE;
	current->mm.vm_area = NULL;
	//这时候直接分配用户栈的物理内存，page fault中只需要建立映射即可
	current->mm.user_stack_begin = (uint64 *)alloc_pages(1);
	do_mmap(&current->mm, 0, USER_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
	do_mmap(&current->mm, (void *)(VM_USER_STACK_TOP - PGSIZE), PGSIZE, PROT_READ | PROT_WRITE);

	current->stack = (struct pt_regs *)kmalloc(sizeof(struct pt_regs));

	printk("[PID = %d ] Process Create Successfully!\n", current->pid);

	// printf("[PID = %d] Process Create Successfully!\n", current->pid);
	task[task_num] = current;
	task_num++;

	current = task[0];
	//初始化sscratch为task[0]的内核栈。
	asm volatile(
		"la t1,current\n"
		"ld t1,0(t1)\n"
		"ld t1,160(t1)\n"
		"csrw sscratch,t1");
}
//返回子进程的pid
int create_fork_task()
{
	puts("create fork task...\n");
	struct task_struct *new_task = (struct task_struct *)(kmalloc(sizeof(struct task_struct)));
	new_task->state = TASK_RUNNING;
	new_task->counter = COUNTER_INIT_COUNTER[task_num];
	new_task->priority = PRIORITY_INIT_COUNTER[task_num];
	new_task->blocked = 0;
	new_task->pid = task_num;

	new_task->sscratch = (unsigned long long)alloc_pages(1) + PGSIZE;
	new_task->sepc = current->sepc;

	//初始化stack
	new_task->stack = (struct pt_regs *)kmalloc(sizeof(struct pt_regs));
	uint64 *s = (uint64 *)current->stack;
	uint64 *d = (uint64 *)new_task->stack;
	for (int i = 0; i < sizeof(struct pt_regs) / sizeof(uint64); i++)
	{
		*(d + i) = *(s + i);
	}
	new_task->stack->a0 = 0;
	new_task->stack->sp = csr_read(sscratch);
	// stack初始结束

	new_task->mm.pgtbl = user_paging_init();
	new_task->mm.user_size = USER_SIZE;
	new_task->mm.vm_area = NULL;
	//这时候直接分配用户栈的物理内存，page fault中只需要建立映射即可
	new_task->mm.user_stack_begin = (uint64 *)alloc_pages(1);
	//拷贝
	for (int i = 0; i < PGSIZE / (sizeof(uint64)); i++)
	{
		*(new_task->mm.user_stack_begin + i) = *(current->mm.user_stack_begin + i);
	}

	do_mmap(&new_task->mm, 0, USER_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
	do_mmap(&new_task->mm, (void *)(VM_USER_STACK_TOP - PGSIZE), PGSIZE, PROT_READ | PROT_WRITE);

	new_task->thread.ra = (unsigned long long)forkret;

	printk("[PID = %d ] Process fork from [PID = %d]!\n", new_task->pid, current->pid);

	task[task_num] = new_task;
	task_num++;
	return new_task->pid;
}
#ifdef SJF
// simulate the cpu timeslice, which measn a short time frame that gets assigned to process for CPU execution
void do_timer(void)
{

	printk("[*PID = %d] Context Calculation: counter = %d,priority = %d\n", current->pid, current->counter, current->priority);
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
	for (int i = task_num - 1; i > 0; i--)
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
		for (int i = 1; i < task_num; i++)
		{
			task[i]->counter = rand();
			printk("[PID] %d get a new counter %d\n", task[i]->pid, task[i]->counter);
			// puts("[PID = ");
			// puti(i);
			// puts("] get a new counter: ");
			// puti(task[i]->counter);
			// puts("\n");
		}
		puts("\n-----------------------------------\n\n");
		schedule();
		return;
	}
	if (current->pid != task[next]->pid)
	{
		// puts("[ ");
		// puti(current->pid);
		// puts(" -> ");
		// puti(task[next]->pid);
		// puts(", prio: ");
		// puti(task[next]->priority);
		// puts(", counter: ");
		// puti(task[next]->counter);
		// puts("\n");
		printk("[ %d -> %d ] Switch from task %d to task %d, prio: %d, counter: %d\n",
			   current->pid, task[next]->pid,
			   current->pid,
			   task[next]->pid,
			   task[next]->priority, task[next]->counter);
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
