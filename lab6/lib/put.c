#include "put.h"
int puts(const char *s)
{
    while (*s != '\0')
    {
        *UART16550A_DR = (unsigned char)(*s);
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
        *UART16550A_DR = (unsigned char)itoch(x / digit);
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
        *UART16550A_DR = (unsigned char)itoch(x / digit);
        x %= digit;
        digit /= 16;
    }
    *UART16550A_DR = '\n';
    return;
}

static int vprintfmt(void (*putch)(char), const char *fmt, va_list vl)
{
    int in_format = 0, longarg = 0;
    size_t pos = 0;
    for (; *fmt; fmt++)
    {
        if (in_format)
        {
            switch (*fmt)
            {
            case 'l':
            {
                longarg = 1;
                break;
            }

            case 'x':
            {
                long num = longarg ? va_arg(vl, long) : va_arg(vl, int);

                int hexdigits = 2 * (longarg ? sizeof(long) : sizeof(int)) - 1;
                for (int halfbyte = hexdigits; halfbyte >= 0; halfbyte--)
                {
                    int hex = (num >> (4 * halfbyte)) & 0xF;
                    char hexchar = (hex < 10 ? '0' + hex : 'a' + hex - 10);
                    putch(hexchar);
                    pos++;
                }
                longarg = 0;
                in_format = 0;
                break;
            }

            case 'd':
            {
                long num = longarg ? va_arg(vl, long) : va_arg(vl, int);
                if (num < 0)
                {
                    num = -num;
                    putch('-');
                    pos++;
                }
                int bits = 0;
                char decchar[25] = {'0', 0};
                for (long tmp = num; tmp; bits++)
                {
                    decchar[bits] = (tmp % 10) + '0';
                    tmp /= 10;
                }

                for (int i = bits; i >= 0; i--)
                {
                    putch(decchar[i]);
                }
                pos += bits + 1;
                longarg = 0;
                in_format = 0;
                break;
            }

            case 'u':
            {
                unsigned long num = longarg ? va_arg(vl, long) : va_arg(vl, int);
                int bits = 0;
                char decchar[25] = {'0', 0};
                for (long tmp = num; tmp; bits++)
                {
                    decchar[bits] = (tmp % 10) + '0';
                    tmp /= 10;
                }

                for (int i = bits; i >= 0; i--)
                {
                    putch(decchar[i]);
                }
                pos += bits + 1;
                longarg = 0;
                in_format = 0;
                break;
            }

            case 's':
            {
                const char *str = va_arg(vl, const char *);
                while (*str)
                {
                    putch(*str);
                    pos++;
                    str++;
                }
                longarg = 0;
                in_format = 0;
                break;
            }

            case 'c':
            {
                char ch = (char)va_arg(vl, int);
                putch(ch);
                pos++;
                longarg = 0;
                in_format = 0;
                break;
            }
            default:
                break;
            }
        }
        else if (*fmt == '%')
        {
            in_format = 1;
        }
        else
        {
            putch(*fmt);
            pos++;
        }
    }
    return pos;
}
void put_char(char c)
{
    *UART16550A_DR = (unsigned char)c;
}
int printk(const char *s, ...)
{
    int res = 0;
    va_list vl;
    va_start(vl, s);
    res = vprintfmt(put_char, s, vl);
    va_end(vl);
    return res;
}
