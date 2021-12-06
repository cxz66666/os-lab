#include "put.h"

int syscall_puts(const char *s, int num)
{
    int index = 0;
    while (num--)
    {
        *UART16550A_DR_VA = (unsigned char)(*s);
        s++;
        index++;
    }
    return index;
}
int puts(const char *s)
{
    while (*s != '\0')
    {
        *UART16550A_DR_VA = (unsigned char)(*s);
        s++;
    }
    return 0;
}
static char itoch(int x)
{
    if (x >= 0 && x <= 9)
    {
        return (char)(x + 48);
    }
    else if (x >= 10 && x <= 15)
    {
        return 'a' + x - 10;
    }
    return 0;
}
void puti(int x)
{
    int digit = 1, tmp = x;
    while (tmp >= 10)
    {
        digit *= 10;
        tmp /= 10;
    }
    while (digit >= 1)
    {
        *UART16550A_DR_VA = (unsigned char)itoch(x / digit);
        x %= digit;
        digit /= 10;
    }
    return;
}
void putullHex(unsigned long long x)
{
    puts("0x");
    unsigned long long digit = 1, tmp = x;
    while (tmp >= 16)
    {
        digit *= 16;
        tmp /= 16;
    }
    while (digit >= 1)
    {
        *UART16550A_DR_VA = (unsigned char)itoch(x / digit);
        x %= digit;
        digit /= 16;
    }

    return;
}
