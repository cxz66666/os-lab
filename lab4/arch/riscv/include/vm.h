#ifndef VM_H
#define VM_H
#define uint64_t unsigned long long
#define offset (0xffffffe000000000-0x80000000)
void create_mapping(uint64_t* pgtbl,uint64_t va,uint64_t pa,uint64_t sz,int perm);
extern uint64_t cur;
void paging_init();
#endif


