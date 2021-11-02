#include "riscv.h"

extern void clock_init(void);
extern void task_init(void);
void intr_enable(void)
{
    //设置sstatus[sie]=1,打开s模式的中断开关
    // your code
    unsigned long res = set_csr(sstatus, 2);
    unsigned long ss = read_csr(sstatus);
}

void intr_disable(void)
{
    //设置sstatus[sie]=0,关闭s模式的中断开关
    // your code
    unsigned long res = clear_csr(sstatus, 2);
}

void idt_init(void)
{
    extern void trap_s(void);
    //向stvec寄存器中写入中断处理后跳转函数的地址
    // your code
    write_csr(stvec, &trap_s);
    unsigned long st = read_csr(stvec);
}

void init(void)
{
    idt_init();
    intr_enable();
    clock_init();
    task_init();
}
