#ifndef _SCHED_H
#define _SCHED_H

#define TASK_SIZE (4096)
#define THREAD_OFFSET (5 * 0x08)

#ifndef __ASSEMBLER__

/* task的最大数量 */
#define NR_TASKS 64

#define FIRST_TASK (task[0])
#define LAST_TASK (task[NR_TASKS - 1])

/* 定义task的状态，Lab3中task只需要一种状态。*/
#define TASK_RUNNING 0
// #define TASK_INTERRUPTIBLE       1
// #define TASK_UNINTERRUPTIBLE     2
// #define TASK_ZOMBIE              3
// #define TASK_STOPPED             4

#define PREEMPT_ENABLE 0
#define PREEMPT_DISABLE 1

/* Lab3中进程的数量以及每个进程初始的时间片 */
#define LAB_TEST_NUM 4
#define LAB_TEST_COUNTER 5

#include "types.h"
/* 当前进程 */
extern struct task_struct *current;

/* 进程指针数组 */
extern struct task_struct *task[NR_TASKS];

typedef struct
{
    unsigned long pgprot;
} pgprot_t;

struct pt_regs
{
    uint64 ra, sp, gp, tp;
    uint64 t0, t1, t2, s0, s1;
    uint64 a0, a1, a2, a3, a4, a5, a6, a7;
    uint64 s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64 t3, t4, t5, t6;
    uint64 sepc;
};
/* 进程状态段数据结构 */
struct thread_struct
{
    unsigned long long ra;
    unsigned long long sp;
    unsigned long long s0;
    unsigned long long s1;
    unsigned long long s2;
    unsigned long long s3;
    unsigned long long s4;
    unsigned long long s5;
    unsigned long long s6;
    unsigned long long s7;
    unsigned long long s8;
    unsigned long long s9;
    unsigned long long s10;
    unsigned long long s11;
};

struct vm_area_struct
{
    /* Our start address within vm_area. */
    unsigned long vm_start;
    /* The first byte after our end address within vm_area. */
    unsigned long vm_end;
    /* linked list of VM areas per task, sorted by address. */
    struct vm_area_struct *vm_next, *vm_prev;
    /* The address space we belong to. */
    struct mm_struct *vm_mm;
    /* Access permissions of this VMA. */
    pgprot_t vm_page_prot;
    /* Flags*/
    unsigned long vm_flags;
};
struct mm_struct
{
    /* data */
    unsigned long long pgtbl;
    unsigned long long user_size; // user所分配的物理空间多大
    struct vm_area_struct *vm_area;
    uint64 *user_stack_begin;
};

/* 进程数据结构 */
struct task_struct
{
    long state;    // 进程状态 Lab3中进程初始化时置为TASK_RUNNING
    long counter;  // 运行剩余时间
    long priority; // 运行优先级 1最高 5最低
    long blocked;
    long pid; // 进程标识符
    // Above Size Cost: 40 bytes

    struct thread_struct thread; // 该进程状态段

    unsigned long long sepc;
    unsigned long long sscratch;
    struct mm_struct mm;
    struct pt_regs *stack; // fork使用
};

/* 进程初始化 创建四个dead_loop进程 */
void task_init(void);

/* 在时钟中断处理中被调用 */
void do_timer(void);

/* 调度程序 */
void schedule(void);

/* 切换当前任务current到下一个任务next */
void switch_to(struct task_struct *next);

/* 死循环 */
void dead_loop(void);

//测试page fault
void test_page_fault();

//测试代码段权限
void test_section_mod();

//创建子进程 返回pid
int create_fork_task();
#endif

#endif
