.align 3
.section .text.entry
.equ reg_size, 0x8
.globl trap_s
.globl __switch_to
.globl init_epc
.extern handler_s
.extern _end
.extern current

trap_s:

# load kernel stack ptr
	csrrw sp,sscratch,sp

	# save sepc and caller-saved registers
	# call handler(scause)
	# load sepc and caller-saved registers
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
	sd t6,-136(s0)
	
	csrr t3,sepc   #这里t3已经保存过了，所以可以使用
	sd t3,-144(s0)


	csrr a0,scause
	csrr a1,sepc
	mv a2,sp
	#如果是中断 返回1 如果是异常 返回0，同时把sepc+4
	call handler_s

	# call handler_s(scause)

	
	# load sepc and callee-saved registers
	ld t3,-144(s0)
	bnez a0,NOT_ADD  
	addi t3,t3,4
NOT_ADD:
	#判断是否是data段内，在lab5中不进行
	#la t2,data_start
	#blt t3,t2,NOT_DATA_SECTION
	#la t2,data_end
	#ld t3,0(t2)
NOT_DATA_SECTION:

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
	ld t6,-136(s0)
	
	ld s0,128(sp)
	ld ra,136(sp)
	addi sp,sp,144


    csrrw sp, sscratch, sp

	sret

init_epc:
	li t0, 0
	csrw sepc, t0
	 
	#切换权限
	li t1, 0x100            #t1=0001 0000 0000 
	csrc sstatus, t1         #将mstatus第8位清0 
	
	#设置sscratch为内核栈，同时设置当前sp为用户栈
	#实际上在task init已经设置的很正确了，初始的sp直接是用户栈，初始的sscratch是内核栈


	#更新页表  mm.pgtbl 实际上已经在__switch_to干过了


	sret

__switch_to:
	li    a4,  40 
	add   a3, a0, a4
	add   a4, a1, a4
	#Save context into prev->thread
	/*your code*/

	sd ra,0(a3)
	sd sp,8(a3)
	sd s0,16(a3)
	sd s1,24(a3)
	sd s2,32(a3)
	sd s3,40(a3)
	sd s4,48(a3)
	sd s5,56(a3)
	sd s6,64(a3)
	sd s7,72(a3)
	sd s8,80(a3)
	sd s9,88(a3)
	sd s10,96(a3)
	sd s11,104(a3)

	csrr s11,sscratch
	sd s11,120(a3)
	#Restore context from next->thread
	/*your code*/


	# 切换页表
	ld a5,128(a4)
	srli a5,a5,12
	li t2,0x8000000000000000
	add a5,a5,t2
	csrw satp,a5
	sfence.vma x0,x0

	ld s11,120(a4)
	csrw sscratch,s11
	ld ra,0(a4)
	ld sp,8(a4)
	ld s0,16(a4)
	ld s1,24(a4)
	ld s2,32(a4)
	ld s3,40(a4)	
	ld s4,48(a4)
	ld s5,56(a4)
	ld s6,64(a4)
	ld s7,72(a4)
	ld s8,80(a4)
	ld s9,88(a4)
	ld s10,96(a4)
	ld s11,104(a4)
	

	#return to ra
	ret