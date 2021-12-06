#include "put.h"
#include "sched.h"
#include "syscall.h"
#define uint64_t unsigned long long
#define size_t unsigned long long
#define uintptr_t unsigned long long
extern struct task_struct *current;

int count = 0;

struct pt_regs
{
	uint64_t sepc;
	uint64_t t6, t5, t4, t3;
	uint64_t a7, a6, a5, a4, a3, a2, a1, a0;
	uint64_t t2, t1, t0, s0, ra;
};

int handler_s(size_t scause, size_t sepc, uintptr_t *regs)
{

	struct pt_regs *pt_regs_ptr = (struct pt_regs *)regs;
	if (scause >> 63)
	{ // interrupt
		if (((scause << 1) >> 1) == 5)
		{ // supervisor timer interrupt
			asm volatile("ecall");
			do_timer();
			count++;
		}
		return 1;
	}
	else
	{

		if (scause == 15)
		{
			puts("Store Page Fault\n");
		}
		else if (scause == 13)
		{
			puts("Load Page Fault\n");
		}
		else if (scause == 12)
		{
			puts("Instruction Page Fault\n");
		}

		// call from use mode
		else if (scause == 8)
		{
			int t;
			switch (pt_regs_ptr->a7)
			{
			case SYS_WRITE:
				t = syscall_puts(pt_regs_ptr->a1, pt_regs_ptr->a2);
				pt_regs_ptr->a0 = t;
				break;

			case SYS_GETPID:
				t = current->pid;
				pt_regs_ptr->a0 = t;
				break;
			}
		}
		return 0;
	}
	return 0;
}
