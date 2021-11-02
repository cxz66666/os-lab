#include "sched.h"
#include "stdio.h"

#define Kernel_Page 0x80210000
#define LOW_MEMORY 0x80211000
#define PAGE_SIZE 4096UL

struct task_struct *current;
struct task_struct *task[NR_TASKS];

extern volatile int task_test_done;
extern void __init_sepc(void);

// If next==current,do nothing; else update current and call __switch_to.
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

int task_init_done = 0;
// initialize tasks, set member variables
void task_init(void)
{
    puts("task init...\n");

    // initialize task[0]
    current = (struct task_struct *)Kernel_Page;
    current->state = TASK_RUNNING;
    current->counter = 1;
    current->priority = 5;
    current->blocked = 0;
    current->pid = 0;
    task[0] = current;
    task[0]->thread.ra = (unsigned long long)__init_sepc;
    task[0]->thread.sp = (unsigned long long)task[0] + TASK_SIZE;

    // set other 4 tasks
    for (int i = 1; i <= LAB_TEST_NUM; ++i)
    {
        current = (struct task_struct *)((unsigned long long)current + TASK_SIZE);
        /*your code*/
        current->state = TASK_RUNNING;
        current->counter = 0;
        current->priority = 5;
        current->blocked = 0;
        current->pid = i;
        task[i] = current;
        task[i]->thread.ra = (unsigned long long)__init_sepc;
        task[i]->thread.sp = (unsigned long long)current + TASK_SIZE;
        printf("[PID = %d] Process Create Successfully!\n", task[i]->pid);
    }
    current = task[0];
    task_init_done = 1;
}

#ifdef SJF
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
        next = 0;
        task[0]->counter++;
    }
    if (current->pid != task[next]->pid)
    {
        printf("[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, counter: %d\n",
               current->pid, task[next]->pid,
               current->pid, (unsigned long)current->thread.sp,
               task[next]->pid, (unsigned long)task[next],
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
               current->pid, (unsigned long)current->thread.sp,
               task[next]->pid, (unsigned long)task[next],
               task[next]->priority, task[next]->counter);
    }
    switch_to(task[next]);
}

#endif
