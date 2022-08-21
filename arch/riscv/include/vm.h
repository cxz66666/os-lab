#ifndef VM_H
#define VM_H

#define VALID 1
#define READ_PERM 2
#define WRITE_PERM 4
#define EXEC_PERM 8
#define USER_PERM 16

#define RWX (READ_PERM | WRITE_PERM | EXEC_PERM | VALID)
#define RW (READ_PERM | WRITE_PERM | VALID)
#define R (READ_PERM | VALID)
#define RX (READ_PERM | EXEC_PERM | VALID)

#include "defs.h"
#include "stddef.h"
#include "types.h"

void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, uint64 perm);
void paging_init();
void free_page_tables(uint64 pagetable, uint64 va, uint64 n, int free_frame);
void mprotect_do(uint64 pagetable, uint64 va, size_t __len, int prot);
#define VM_NONE 0x00000000

#define VM_READ 0x00000001 /* currently active flags */
#define VM_WRITE 0x00000002
#define VM_EXEC 0x00000004
#define VM_SHARED 0x00000008
#endif
