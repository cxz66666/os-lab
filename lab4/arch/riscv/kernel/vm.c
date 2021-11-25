#include "vm.h"
#include "put.h"

extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;
uint64_t cur = 0;

#define VPN2(num) ((num >> 30) & 0x1ff)
#define VPN1(num) ((num >> 21) & 0x1ff)
#define VPN0(num) ((num >> 12) & 0x1ff)
#define U64ADDR(num) ((uint64_t)&num)
#define IsValid(num) (num ? 1 : 0)
#define GET_PAGE_NUM(num) (num >> 10)

const int LEVELS = 3;
const uint64_t PAGE_SIZE = 0x1000;
struct PAGE
{
    uint64_t entrys[512];
};

void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm)
{
    /*your code*/
    // stop是判断终止地址
    uint64_t stop = va + sz;
    struct PAGE *now;
    struct PAGE *begin = (struct PAGE *)pgtbl;
    while (va < stop)
    {
        now = begin;
        int index2 = VPN2(va), index1 = VPN1(va), index0 = VPN0(va);
        // puti(index2);
        // puti(index1);
        // puti(index0);
        uint64_t pte2, pte1, pte0;

        pte2 = now->entrys[index2];
        // putullHex(pte2);
        if (!IsValid(pte2))
        {
            ++cur;
            pte2 = (((uint64_t)(begin + cur) >> 12) << 10) | 1;
            now->entrys[index2] = pte2;
        }

        now = (struct PAGE *)((pte2 >> 10) << 12);

        pte1 = now->entrys[index1];
        // putullHex(pte1);

        if (!IsValid(pte1))
        {
            ++cur;
            pte1 = (((uint64_t)(begin + cur) >> 12) << 10) | 1;
            now->entrys[index1] = pte1;
        }

        now = (struct PAGE *)((pte1 >> 10) << 12);
        //读取pte0是无意义的，这里只是为了维护形式的统一
        pte0 = now->entrys[index0];
        // putullHex(pte0);

        now->entrys[index0] = ((1 + ((perm & 7) << 1)) + ((pa >> 12) << 10));
        // putullHex(now->entrys[index0]);
        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }

    return;
}

void paging_init()
{

    // putullHex(&text_start);
    // putullHex(&rodata_start);

    // putullHex(&data_start);
    uint64_t *pgtbl = &_end;
    // putullHex(pgtbl);
    /*your code*/

    uint64_t va = 0xffffffe000000000, pa = 0x80000000;
    // text
    create_mapping(pgtbl, va, pa, U64ADDR(rodata_start) - U64ADDR(text_start), 5);
    create_mapping(pgtbl, pa, pa, U64ADDR(rodata_start) - U64ADDR(text_start), 5);

    va += U64ADDR(rodata_start) - U64ADDR(text_start);
    pa += U64ADDR(rodata_start) - U64ADDR(text_start);
    // rodata
    create_mapping(pgtbl, va, pa, U64ADDR(data_start) - U64ADDR(rodata_start), 1);
    create_mapping(pgtbl, pa, pa, U64ADDR(data_start) - U64ADDR(rodata_start), 1);

    // other
    va += U64ADDR(data_start) - U64ADDR(rodata_start);
    pa += U64ADDR(data_start) - U64ADDR(rodata_start);
    create_mapping(pgtbl, va, pa, 0x1000000 - (U64ADDR(data_start) - U64ADDR(text_start)), 3);
    create_mapping(pgtbl, pa, pa, 0x1000000 - (U64ADDR(data_start) - U64ADDR(text_start)), 3);

    va = (uint64_t)UART16550A_DR;
    pa = va;
    create_mapping(pgtbl, va, pa, 0x1000000, 3);
    puts("Done\n");
}
