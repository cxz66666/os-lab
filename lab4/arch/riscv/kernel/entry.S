.align 3
.section .text.entry
.equ reg_size, 0x8
.globl trap_s
.globl __switch_to
.globl init_epc
.extern handler_s

trap_s:
	# save sepc and caller-saved registers
	# call handler(scause)
	# load sepc and caller-saved registers
	/*your code*/

	sret

init_epc:
	la t0, dead_loop
	csrw sepc, t0
	ecall
	sret

__switch_to:
	li    a4,  40 
	add   a3, a0, a4
	add   a4, a1, a4
	#Save context into prev->thread
	/*your code*/

	#Restore context from next->thread
	/*your code*/

	#return to ra
	ret
