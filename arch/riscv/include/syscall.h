#pragma once

#define SYS_WRITE 64
#define SYS_GETPID 172
#define SYS_MMAP 222
#define SYS_MUNMAP 215
#define SYS_MPROTECT 226
#define SYS_FORK 220
#include "defs.h"
#include "sched.h"
#include "stddef.h"
int syscall_puts(const char *s, int num);
int syscall_getpid();
void *mmap(void *__addr, size_t __len, int __prot,
           int __flags, int __fd, __off_t __offset);
void *do_mmap(struct mm_struct *mm, void *start, size_t length, int prot);
int munmap(void *start, size_t length);
int mprotect(void *__addr, size_t __len, int __prot);

int fork(void);