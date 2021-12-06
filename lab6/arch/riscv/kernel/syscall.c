#include "syscall.h"
#include "put.h"
#include "sched.h"
#include "slub.h"
#include "vm.h"
extern struct task_struct *current;

int syscall_puts(const char *s, int num)
{
    int index = 0;
    while (num--)
    {
        *UART16550A_DR = (unsigned char)(*s);
        s++;
        index++;
    }
    return index;
}

int syscall_getpid()
{
    return current->pid;
}

unsigned long get_unmapped_area(size_t length)
{
    unsigned long begin = 0;
    struct vm_area_struct *tmp = current->mm.vm_area;
    while (1)
    {
        int conflict = 0;
        while (tmp != NULL)
        {
            if (begin < tmp->vm_end && tmp->vm_start < begin + length)
            {
                conflict = 1;
                begin = tmp->vm_end;
                break;
            }
            if (tmp->vm_start >= begin + length)
            {
                break;
            }
            tmp = tmp->vm_next;
        }
        if (!conflict)
        {
            break;
        }
    }

    return begin;
}
void *do_mmap(struct mm_struct *mm, void *start, size_t length, int prot)
{
    struct vm_area_struct *t = kmalloc(sizeof(struct vm_area_struct));
    t->vm_start = (unsigned long)start;
    t->vm_end = t->vm_start + PGROUNDUP(length);
    t->vm_flags = prot;
    t->vm_next = NULL;
    t->vm_prev = NULL;
    t->vm_mm = mm;
    if (mm->vm_area == NULL)
    {
        mm->vm_area = t;
        return (void *)t->vm_start;
    }

    struct vm_area_struct *begin = mm->vm_area;
    struct vm_area_struct *prev;
    if (begin->vm_start >= t->vm_end)
    {
        begin->vm_prev = t;
        t->vm_next = begin;
        mm->vm_area = t;
        return (void *)t->vm_start;
    }
    int confilct = 0;
    while (begin != NULL)
    {
        if (!confilct)
        {
            if (t->vm_start < begin->vm_end && begin->vm_start < t->vm_end)
            {
                confilct = 1;
            }
        }
        prev = begin;
        begin = begin->vm_next;
    }
    if (confilct)
    {
        t->vm_start = get_unmapped_area(length);
        t->vm_end = t->vm_start + length;
    }
    //寻找插入的位置
    begin = mm->vm_area;
    while (begin != NULL)
    {
        if (begin->vm_start >= t->vm_end)
        {
            prev->vm_next = t;
            t->vm_next = begin;
            t->vm_prev = prev;
            begin->vm_prev = t;
            return (void *)t->vm_start;
        }
        prev = begin;
        begin = begin->vm_next;
    }
    prev->vm_next = t;
    t->vm_prev = prev;
    printk("[S] New vm_area_struct: start %lx, end %lx, prot [r:%d,w:%d,x:%d]\n",
           t->vm_start, t->vm_end, t->vm_flags & VM_READ, t->vm_flags & VM_WRITE, t->vm_flags & VM_EXEC);
    return (void *)t->vm_start;
}
//
void *mmap(void *__addr, size_t __len, int __prot,
           int __flags, int __fd, __off_t __offset)
{
    return do_mmap(&current->mm, __addr, __len, __prot);
    //如果有冲突的
}
int munmap(void *start, size_t length)
{
    struct vm_area_struct *t = current->mm.vm_area;
    struct vm_area_struct *prev = NULL;
    while (t != NULL)
    {
        if (t->vm_start == (unsigned long)start && t->vm_end == t->vm_start + PGROUNDUP(length))
        {
            free_page_tables(current->mm.pgtbl, t->vm_start, (t->vm_end - t->vm_start) / PGSIZE, 1);
            if (!prev)
            {
                current->mm.vm_area = t->vm_next;
            }
            else
            {
                prev->vm_next = t->vm_next;
            }
            kfree(t);
            return 0;
        }
        prev = t;
        t = t->vm_next;
    }
    return -1;
}

int mprotect(void *__addr, size_t __len, int __prot)
{
    mprotect_do(current->mm.pgtbl, (uint64)__addr, __len, __prot);
    return 0;
}

int fork(void)
{
    return create_fork_task();
}