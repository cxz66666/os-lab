#ifndef VM_H
#define VM_H
#define uint64_t unsigned long long

#define VALID 1
#define READ_PERM 2
#define WRITE_PERM 4
#define EXEC_PERM 8
#define USER_PERM 16

#define RWX (READ_PERM | WRITE_PERM | EXEC_PERM | VALID)
#define RW (READ_PERM | WRITE_PERM | VALID)
#define R (READ_PERM | VALID)
#define RX (READ_PERM | EXEC_PERM | VALID)

uint64_t offset = (0xffffffe000000000 - 0x80000000);
void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, uint64_t perm);
extern uint64_t cur;
void paging_init();

#endif
