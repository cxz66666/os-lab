#include "defs.h"
#include "riscv.h"

uint64_t sbi_call(uint64_t sbi_type, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    uint64_t ret_val;
    __asm__ volatile(
        "add x17,x0,%[arg7]\n"
        "add x10,x0,%[arg0]\n"
        "add x11,x0,%[arg1]\n"
        "add x12,x0,%[arg2]\n"
        "ecall\n"
        "add %[dest],x0,x10"
        : [dest] "=r"(ret_val)
        : [arg0] "r"(arg0), [arg1] "r"(arg1), [arg2] "r"(arg2), [arg7] "r"(sbi_type)
        : "memory");
    return ret_val;
}

void trigger_time_interrupt(unsigned long long stime_value)
{
    sbi_call(0, stime_value, 0, 0);
}
