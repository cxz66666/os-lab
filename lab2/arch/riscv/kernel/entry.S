.section .text.entry

.global trap_s
.extern handler_s
.equ reg_size, 0x8
.align 2

trap_s:
	#save callee-saved registers and spec
	addi sp,sp, -144
	sd ra,136(sp)
	sd s0,128(sp)
	addi s0,sp,144
	sd t0,-24(s0)
	sd t1,-32(s0)
	sd t2,-40(s0)
	sd a0,-48(s0)
	sd a1,-56(s0)
	sd a2,-64(s0)
	sd a3,-72(s0)
	sd a4,-80(s0)
	sd a5,-88(s0)
	sd a6,-96(s0)
	sd a7,-104(s0)
	sd t3,-112(s0)
	sd t4,-120(s0)
	sd t5,-128(s0)
	sd t6,-132(s0)
	
	csrr t3,sepc   #这里t3已经保存过了，所以可以使用
	sd t3,-140(s0)

	csrr a0,scause
	call handler_s

	# call handler_s(scause)

	
	# load sepc and callee-saved registers
	ld t3,-140(s0)

	csrw sepc,t3
	ld t0,-24(s0)
	ld t1,-32(s0)
	ld t2,-40(s0)
	ld a0,-48(s0)
	ld a1,-56(s0)
	ld a2,-64(s0)
	ld a3,-72(s0)
	ld a4,-80(s0)
	ld a5,-88(s0)
	ld a6,-96(s0)
	ld a7,-104(s0)
	ld t3,-112(s0)
	ld t4,-120(s0)
	ld t5,-128(s0)
	ld t6,-132(s0)
	
	ld s0,128(sp)
	ld ra,136(sp)
	addi sp,sp,144

	sret
