#include "defs.h"
extern sbi_call();

const uint64_t SBI_CONSOLE_PUTCHAR = 1;
const uint64_t SBI_CONSOLE_GETCHAR = 2;
int puts(char *str)
{
    // your code
    while (*str)
    {
        sbi_call(SBI_CONSOLE_PUTCHAR, *str, 0, 0);
        str++;
    }
    return 0;
}

int put_num(uint64_t n)
{
    if (n < 10)
    {
        sbi_call(SBI_CONSOLE_PUTCHAR, n + '0', 0, 0);
        return 0;
    }
    put_num(n / 10);
    sbi_call(SBI_CONSOLE_PUTCHAR, (n % 10) + '0', 0, 0);
    return 0;
}
