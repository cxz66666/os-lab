#include "vm.h"
#include "put.h"

extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;

uint64_t page_num = 0;

#define VPN2(num) ((num >> 30) & 0x1ff)
#define VPN1(num) ((num >> 21) & 0x1ff)
#define VPN0(num) ((num >> 12) & 0x1ff)

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

    while (va < stop)
    {
        now = (struct PAGE *)pgtbl;
        int index2 = VPN2(va), index1 = VPN1(va), index0 = VPN0(va);
        puti(index2);
        puti(index1);
        puti(index0);
        uint64_t pte2, pte1, pte0;

        pte2 = now->entrys[index2];
        putullHex(pte2);
        if (!IsValid(pte2))
        {
            pte2 = (++page_num) << 10 + 1;
            now->entrys[index2] = pte2;
        }

        now = (struct PAGE *)(GET_PAGE_NUM(pte2) * PAGE_SIZE + pgtbl);

        pte1 = now->entrys[index1];
        putullHex(pte1);

        if (!IsValid(pte1))
        {
            pte1 = (++page_num) << 10 + 1;
            now->entrys[index1] = pte1;
        }

        now = (struct PAGE *)(GET_PAGE_NUM(pte1) * PAGE_SIZE + pgtbl);
        //读取pte0是无意义的，这里只是为了维护形式的统一
        pte0 = now->entrys[index0];
        putullHex(pte0);

        now->entrys[index0] = (1 + (perm & 7) << 1) + ((pa >> 12) & (0xfffffffffff)) << 10;

        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }

    return;
}

void paging_init()
{
    uint64_t *pgtbl = &_end;
    putullHex(pgtbl);
    /*your code*/

    uint64_t va = 0xffffffe000000000, pa = 0x80000000;
    create_mapping(pgtbl, va, pa, 0x1000000, 7);

    va = pa;
    create_mapping(pgtbl, va, pa, 0x1000000, 7);

    va = (uint64_t)UART16550A_DR;
    pa = va;
    create_mapping(pgtbl, va, pa, 0x1000000, 7);
}
