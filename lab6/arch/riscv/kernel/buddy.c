#include "buddy.h"
#include "defs.h"
#include "put.h"
struct buddy buddy_item;

extern uint64 _end;
extern uint64 text_start;
void init_buddy_system(void)
{
    buddy_item.size = PHY_SIZE / PGSIZE;
    uint64 needPage = buddy_item.size * 2 * sizeof(unsigned int) / PGSIZE;
    unsigned int *begin = (unsigned int *)(U64ADDR(_end));

    buddy_item.bitmap = begin;
    begin[0] = buddy_item.size;

    for (int i = 1; i < 2 * buddy_item.size - 1; i++)
    {
        begin[i] = begin[(i - 1) / 2] / 2;
    }

    //申请需要的page
    needPage += (U64ADDR(_end) - U64ADDR(text_start)) / PGSIZE;
    alloc_pages(needPage);
}
uint64 ROUNDUP(uint64 num)
{
    if (num <= 1)
        return 1;
    uint64 iteration = 1;
    while ((iteration << 1) < num)
    {
        iteration <<= 1;
    }
    return iteration << 1;
}
void *alloc_pages(int num)
{
    int need = ROUNDUP(num);
    if (buddy_item.bitmap[0] < need)
    {
        for (int i = 0; i < 100; i++)
        {
            puti(buddy_item.bitmap[i]);
            puts(" ");
        }
        puts("What fuck ?\n");
        return '\0';
    }
    int index = 0;
    while (1)
    {
        if (index + 1 >= buddy_item.size)
        {
            break;
        }
        if (buddy_item.bitmap[index * 2 + 1] >= need)
        {
            index = index * 2 + 1;
        }
        else if (buddy_item.bitmap[index * 2 + 2] >= need)
        {
            index = index * 2 + 2;
        }
        else
        {
            break;
        }
    }
    uint64 *va = (uint64 *)(VM_START + PGSIZE * (buddy_item.bitmap[index] * (index + 1) - buddy_item.size));
    buddy_item.bitmap[index] = 0;
    while (index)
    {
        index = (index - 1) / 2;
        buddy_item.bitmap[index] = max(buddy_item.bitmap[index * 2 + 1], buddy_item.bitmap[index * 2 + 2]);
        // if (!index)
        //     printk("[] %d %d %d %d\n", index, buddy_item.bitmap[index], buddy_item.bitmap[2 * index + 1], buddy_item.bitmap[index * 2 + 2]);
    }

    return va;
}
void free_pages(void *ptr)
{
    int index = ((uint64)ptr - VM_START) / PGSIZE;
    int tmp = index;
    int nowIndex = index + buddy_item.size - 1;
    int nowSize = 1;

    while (1)
    {
        if (!buddy_item.bitmap[nowIndex])
        {
            buddy_item.bitmap[nowIndex] = nowSize;
            break;
        }
        nowIndex = (nowIndex - 1) / 2;
        nowSize *= 2;
    }

    while (nowIndex)
    {
        nowIndex = (nowIndex - 1) / 2;
        if (buddy_item.bitmap[nowIndex * 2 + 1] == nowSize && buddy_item.bitmap[nowIndex * 2 + 2] == nowSize)
        {
            buddy_item.bitmap[nowIndex] = 2 * nowSize;
        }
        else
        {
            buddy_item.bitmap[nowIndex] = max(buddy_item.bitmap[nowIndex * 2 + 1], buddy_item.bitmap[nowIndex * 2 + 2]);
        }
        nowSize *= 2;
    }
    return;
}