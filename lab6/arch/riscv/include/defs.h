#ifndef _DEFS_H
#define _DEFS_H

#define csr_read(csr)                            \
    (                                            \
        {                                        \
            register uint64 __v;                 \
            asm volatile(                        \
                "csrr %0," #csr                  \
                : "=r"(__v)                      \
                :                                \
                : "memory"); /* unimplemented */ \
            __v;                                 \
        })

#define csr_write(csr, val)                  \
    (                                        \
        {                                    \
            uint64 __v = (uint64)(val);      \
            asm volatile("csrw " #csr ", %0" \
                         :                   \
                         : "r"(__v)          \
                         : "memory");        \
        })

#endif

#define PHY_START 0x0000000080000000
#define PHY_SIZE 128 * 1024 * 1024 // 128MB， QEMU 默认内存大小

//用户程序的物理地址
#define PHY_USER_START 0x84000000
#define VM_USER_STACK_TOP 0xffffffdf80000000
#define PHY_END (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000 // 4KB
#define PGROUNDUP(addr) ((addr + PGSIZE - 1) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) ((addr >> 12) << 12)

#define VM_START (0xffffffe000000000)
#define VM_END (0xffffffff00000000)
#define VM_SIZE (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)

#define max(a, b) a > b ? a : b
#define U64ADDR(num) ((uint64)&num)

// #define size_t unsigned long
#define __off_t unsigned long

#define PROT_NONE 0x0
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
