# makefile_lab
export
CROSS_= riscv64-unknown-elf-
AR=${CROSS_}ar
GCC=${CROSS_}gcc
LD=${CROSS_}ld
OBJCOPY=${CROSS_}objcopy

ISA ?= rv64imafd
ABI ?= lp64

INCLUDE = -I$(shell pwd)/include -I$(shell pwd)/arch/riscv/include

CF = -fno-builtin -O2  -gdwarf-2 -march=$(ISA) -mabi=$(ABI) -mcmodel=medany -ffunction-sections -fdata-sections -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections 
CFLAG = ${CF} ${INCLUDE} -DSJF

.PHONY: all run debug clean
all:
	@make -C init -s
	@make -C lib -s
	@make -C arch/riscv -s

run: all
	@qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin

debug: all
	@qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -initrd hello.bin -S -s

clean:
	@make clean -C init -s
	@make clean -C lib -s
	@make clean -C arch/riscv -s
