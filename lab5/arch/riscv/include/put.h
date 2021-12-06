#ifndef PUT_H
#define PUT_H
#define UART16550A_DR (volatile unsigned char *)0x10000000
#define UART16550A_DR_VA (volatile unsigned char *)0xffffffdf90000000

void puti(int num);
void putullHex(unsigned long long num);
int puts(const char *s);
int syscall_puts(const char *s, int num);
#endif
