#include "fault.h"
#include "put.h"
#include "sched.h"
#include "syscall.h"
#include "types.h"
extern struct task_struct *current;

int count = 0;

void copy_stack(struct pt_regs *src, struct pt_regs *dst)
{
	uint64 *s = (uint64 *)src;
	uint64 *d = (uint64 *)dst;
	for (int i = 0; i < sizeof(struct pt_regs) / sizeof(uint64); i++)
	{
		*(d + i) = *(s + i);
	}
}
void handler_s(uint64 scause, uint64 sepc, uintptr_t *regs)
{
	struct pt_regs *pt_regs_ptr = (struct pt_regs *)regs;

	copy_stack(pt_regs_ptr, current->stack); // fork will use
	if (scause >> 63)
	{ // interrupt
		if (((scause << 1) >> 1) == 5)
		{ // supervisor timer interrupt
			asm volatile("ecall");
			do_timer();
			count++;
		}
		// pt_regs_ptr->sepc += 4;
		return;
	}
	else
	{

		if (scause == CAUSE_STORE_PAGE_FAULT)
		{
			puts("Store Page Fault\n");
			do_page_fault(CAUSE_STORE_PAGE_FAULT);
		}
		else if (scause == CAUSE_LOAD_PAGE_FAULT)
		{
			puts("Load Page Fault\n");
			do_page_fault(CAUSE_LOAD_PAGE_FAULT);
		}
		else if (scause == CAUSE_FETCH_PAGE_FAULT)
		{
			puts("Instruction Page Fault\n");
			do_page_fault(CAUSE_FETCH_PAGE_FAULT);
		}

		// call from use mode
		else if (scause == 8)
		{
			int t;
			uint64 *tl;
			pt_regs_ptr->sepc += 4;
			switch (pt_regs_ptr->a7)
			{
			case SYS_WRITE:
				t = syscall_puts((const char *)pt_regs_ptr->a1, pt_regs_ptr->a2);
				pt_regs_ptr->a0 = t;
				break;

			case SYS_GETPID:
				t = current->pid;
				pt_regs_ptr->a0 = t;
				break;
			case SYS_MMAP:
				tl = (uint64 *)mmap((void *)pt_regs_ptr->a0, pt_regs_ptr->a1, pt_regs_ptr->a2, pt_regs_ptr->a3, pt_regs_ptr->a4, pt_regs_ptr->a5);
				pt_regs_ptr->a0 = (uint64)tl;
				break;
			case SYS_MUNMAP:
				t = munmap((void *)pt_regs_ptr->a0, pt_regs_ptr->a1);
				pt_regs_ptr->a0 = t;
				break;
			case SYS_MPROTECT:
				t = mprotect((void *)pt_regs_ptr->a0, pt_regs_ptr->a1, pt_regs_ptr->a2);
				pt_regs_ptr->a0 = t;
				break;
			case SYS_FORK:
				current->stack->sepc += 4;
				t = fork();
				pt_regs_ptr->a0 = t;
				break;
			}
		}
	}
	return;
}
