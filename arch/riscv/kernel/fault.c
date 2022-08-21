
#include "fault.h"
#include "buddy.h"
#include "defs.h"
#include "put.h"
#include "sched.h"
#include "stddef.h"
#include "types.h"
#include "vm.h"
extern struct task_struct *current;
extern uint64 USER_SIZE;

void do_page_fault(int scause)
{

    uint64 stval = csr_read(stval);
    struct vm_area_struct *begin = current->mm.vm_area;
    printk("[S] PAGE_FAULT: PID: %d, scause: %d, sepc: %lx, badaddr: %lx\n", current->pid, scause, current->stack->sepc, stval);
    while (begin != NULL)
    {
        stval = csr_read(stval);

        if (stval >= begin->vm_start && stval < begin->vm_end)
        {

            switch (scause)
            {
            case CAUSE_FETCH_PAGE_FAULT:
                if (!(begin->vm_flags & VM_EXEC))
                {
                    puts("[S] Invalid vm area in page fault [exec]\n");
                    return;
                }
                break;

            case CAUSE_LOAD_PAGE_FAULT:
                if (!(begin->vm_flags & VM_READ))
                {
                    puts("[S] Invalid vm area in page fault [load]\n");
                    return;
                }
                break;
            case CAUSE_STORE_PAGE_FAULT:
                if (!(begin->vm_flags & VM_WRITE))
                {
                    puts("[S] Invalid vm area in page fault [write]\n");
                    return;
                }
                break;
            default:
                puts("[S] Invalid fault type!\n");
                return;
            }

            uint64 va = PGROUNDDOWN(stval), pa;
            printk("va is %lx\n", va);
            if (va < USER_SIZE)
            {
                pa = PHY_USER_START + va;
            }
            else if (va >= VM_USER_STACK_TOP - PGSIZE && va < VM_USER_STACK_TOP)
            {
                pa = (uint64)current->mm.user_stack_begin - PA2VA_OFFSET;
            }
            else
            {
                printk("[S] A weird address in page fault %lx\n", va);
                pa = (uint64)alloc_pages(1) - PA2VA_OFFSET;
            }

            printk("[S] mapped PA :%lx to VA :%lx with size :%d,perm:%d \n",
                   pa, va, PGSIZE, begin->vm_flags);
            create_mapping((uint64 *)current->mm.pgtbl, va, pa, PGSIZE, USER_PERM | (begin->vm_flags << 1) | 1);

            return;
        }
        begin = begin->vm_next;
    }
    puts("[S] Invalid vm area in page fault [not exist]\n");
    return;
}