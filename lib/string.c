#include "string.h"
#include "defs.h"
#include "put.h"
void *memset(void *dst, int c, uint64 n)
{

    // printk("%lx %d %ld\n", dst, c, n);
    char *cdst = (char *)dst;
    for (uint64 i = 0; i < n; i++)
    {
        cdst[i] = c;
    }

    return dst;
}

void *memmove(void *dst, const void *src, uint n)
{
    char *cdst = (char *)dst;
    char *csrc = (char *)src;
    for (uint64 i = 0; i < n; ++i)
        cdst[i] = csrc[i];

    return dst;
}