#include "vm.h"
#include "buddy.h"
#include "put.h"
extern uint64 text_start;
extern uint64 rodata_start;
extern uint64 data_start;

#define VPN2(num) ((num >> 30) & 0x1ff)
#define VPN1(num) ((num >> 21) & 0x1ff)
#define VPN0(num) ((num >> 12) & 0x1ff)
#define IsValid(num) (num ? 1 : 0)
#define GET_PAGE_NUM(num) (num >> 10)

const int LEVELS = 3;

const uint64 USER_SIZE = 0x100000;

uint64 *kernel_pgtbl;
struct PAGE
{
    uint64 entrys[512];
};

void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm)
{
    /*your code*/
    // stop是判断终止地址
    uint64 stop = va + sz;
    struct PAGE *now;
    uint64 newPA;
    while (va < stop)
    {
        now = (struct PAGE *)pgtbl;
        int index2 = VPN2(va), index1 = VPN1(va), index0 = VPN0(va);
        // puti(index2);
        // puti(index1);
        // puti(index0);
        uint64 pte2, pte1, pte0;

        pte2 = now->entrys[index2];
        if (!IsValid(pte2))
        {
            newPA = (uint64)alloc_pages(1) - PA2VA_OFFSET;
            pte2 = ((newPA >> 12) << 10) | 1;
            now->entrys[index2] = pte2;
        }

        now = (struct PAGE *)((pte2 >> 10) << 12);

        pte1 = now->entrys[index1];

        if (!IsValid(pte1))
        {
            newPA = (uint64)alloc_pages(1) - PA2VA_OFFSET;
            pte1 = ((newPA >> 12) << 10) | 1;
            now->entrys[index1] = pte1;
        }

        now = (struct PAGE *)((pte1 >> 10) << 12);
        //读取pte0是无意义的，这里只是为了维护形式的统一
        pte0 = now->entrys[index0];
        // putullHex(pte0);

        now->entrys[index0] = perm + ((pa >> 12) << 10);
        // putullHex(now->entrys[index0]);
        va += PGSIZE;
        pa += PGSIZE;
    }

    return;
}

void paging_init()
{

    // putullHex(&text_start);
    // putullHex(&rodata_start);

    // putullHex(&data_start);
    kernel_pgtbl = (uint64 *)((uint64)(alloc_pages(1)) - PA2VA_OFFSET);
    // putullHex(pgtbl);
    /*your code*/

    uint64 va = VM_START, pa = PHY_START;
    // text
    create_mapping(kernel_pgtbl, va, pa, U64ADDR(rodata_start) - U64ADDR(text_start), RX);
    create_mapping(kernel_pgtbl, pa, pa, U64ADDR(rodata_start) - U64ADDR(text_start), RX);

    va += U64ADDR(rodata_start) - U64ADDR(text_start);
    pa += U64ADDR(rodata_start) - U64ADDR(text_start);
    // rodata
    create_mapping(kernel_pgtbl, va, pa, U64ADDR(data_start) - U64ADDR(rodata_start), R);
    create_mapping(kernel_pgtbl, pa, pa, U64ADDR(data_start) - U64ADDR(rodata_start), R);

    // other
    va += U64ADDR(data_start) - U64ADDR(rodata_start);
    pa += U64ADDR(data_start) - U64ADDR(rodata_start);
    create_mapping(kernel_pgtbl, va, pa, PHY_SIZE - (U64ADDR(data_start) - U64ADDR(text_start)), RW);
    create_mapping(kernel_pgtbl, pa, pa, PHY_SIZE - (U64ADDR(data_start) - U64ADDR(text_start)), RW);

    va = (uint64)UART16550A_DR;
    pa = va;
    create_mapping(kernel_pgtbl, va, pa, 0x1000000, RW);
}

uint64 user_paging_init()
{
    //这里拿到的是内核地址（直接映射），千万不能忘了- offset
    uint64 *pgtbl = (uint64 *)((uint64)(alloc_pages(1)) - PA2VA_OFFSET);
    uint64 *tmp1 = pgtbl;
    uint64 *tmp2 = kernel_pgtbl;
    for (int i = 0; i < PGSIZE / sizeof(uint64); i++)
    {
        *tmp1 = *tmp2;
        ++tmp1, ++tmp2;
    }
    //先分配一页用来存新的page table

    //映射为虚拟的0-USER_SIZE 到物理的 PHY_USER_START-PHY_USER_START+USER_SIZE
    // uint64 va = 0, pa = PHY_USER_START;

    // create_mapping(pgtbl, va, pa, USER_SIZE, USER_PERM | RWX);

    // //给栈分配一个page size大小的物理内存，并且设置映射

    // //这里的pa也要设为-offset的
    // va = VM_USER_STACK_TOP - PGSIZE, pa = (uint64)(alloc_pages(1)) - PA2VA_OFFSET;
    // create_mapping(pgtbl, va, pa, PGSIZE, USER_PERM | RWX);

    return (uint64)pgtbl;
}

void free_page_tables(uint64 pagetable, uint64 va, uint64 n, int free_frame)
{
    uint64 stop = va + n * PGSIZE;
    struct PAGE *now;
    while (va < stop)
    {
        now = (struct PAGE *)pagetable;
        int index2 = VPN2(va), index1 = VPN1(va), index0 = VPN0(va);

        uint64 pte2, pte1, pte0;

        pte2 = now->entrys[index2];
        // putullHex(pte2);
        if (!IsValid(pte2))
        {
            //不存在该映射

            va += PGSIZE;
            continue;
        }

        now = (struct PAGE *)((pte2 >> 10) << 12);

        pte1 = now->entrys[index1];
        // putullHex(pte1);

        if (!IsValid(pte1))
        {
            //不存在该映射

            va += PGSIZE;
            continue;
        }

        now = (struct PAGE *)((pte1 >> 10) << 12);
        pte0 = now->entrys[index0];
        // putullHex(pte0);
        if (IsValid(pte0))
        {
            //存在映射，直接根据free_frame决定是否使用free page
            now->entrys[index0] = 0;
            if (free_frame)
            {
                free_pages((void *)(((pte0 >> 10) << 12) + PA2VA_OFFSET));
            }
        }
        // putullHex(now->entrys[index0]);
        va += PGSIZE;
    }
}

void mprotect_do(uint64 pagetable, uint64 va, size_t __len, int prot)
{
    uint64 stop = va + __len;
    struct PAGE *now = (struct PAGE *)pagetable;
    while (va < stop)
    {
        int index2 = VPN2(va), index1 = VPN1(va), index0 = VPN0(va);
        // puti(index2);
        // puti(index1);
        // puti(index0);
        uint64 pte2, pte1, pte0;

        pte2 = now->entrys[index2];
        if (!IsValid(pte2))
        {
            va += PGSIZE;
            continue;
        }

        now = (struct PAGE *)((pte2 >> 10) << 12);

        pte1 = now->entrys[index1];
        // putullHex(pte1);

        if (!IsValid(pte1))
        {
            va += PGSIZE;
            continue;
        }

        now = (struct PAGE *)((pte1 >> 10) << 12);
        //读取pte0是无意义的，这里只是为了维护形式的统一
        pte0 = now->entrys[index0];
        // putullHex(pte0);
        if (IsValid(pte0))
        {
            now->entrys[index0] = ((pte0 >> 10) << 10) | (1 + (prot << 1));
        }
        // putullHex(now->entrys[index0]);
        va += PGSIZE;
    }
    return;
}