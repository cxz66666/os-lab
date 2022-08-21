.align 3
.section .text.entry
.equ reg_size, 0x8
.globl trap_s
.globl __switch_to
.globl init_epc
.globl ret_from_fork
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


#########################
	addi sp,sp,-256
	sd ra,0(sp)
	#sd sp,8(sp)
	sd gp,16(sp)
	sd tp,24(sp)
	sd t0,32(sp)
	sd t1,40(sp)
	sd t2,48(sp)
	sd s0,56(sp)
	sd s1,64(sp)	
	sd a0,72(sp)
	sd a1,80(sp)
	sd a2,88(sp)
	sd a3,96(sp)
	sd a4,104(sp)
	sd a5,112(sp)
	sd a6,120(sp)
	sd a7,128(sp)
	sd s2,136(sp)
	sd s3,144(sp)
	sd s4,152(sp)
	sd s5,160(sp)
	sd s6,168(sp)
	sd s7,176(sp)
	sd s8,184(sp)
	sd s9,192(sp)
	sd s10,200(sp)
	sd s11,208(sp)
	sd t3,216(sp)
	sd t4,224(sp)
	sd t5,232(sp)
	sd t6,240(sp)

	csrr t3,sepc   #这里t3已经保存过了，所以可以使用

	sd t3,248(sp)

	csrr a0,scause
	csrr a1,sepc
	mv a2,sp
	#如果是中断 返回1 如果是异常 返回0，同时把sepc+4
	call handler_s

	# call handler_s(scause)

	
	# load sepc and callee-saved registers
	ld t3,248(sp)
	#bnez a0,NOT_ADD  
	#addi t3,t3,4
NOT_ADD:
	#判断是否是data段内，在lab5中不进行
	#la t2,data_start
	#blt t3,t2,NOT_DATA_SECTION
	#la t2,data_end
	#ld t3,0(t2)
NOT_DATA_SECTION:

	csrw sepc,t3
	ld ra,0(sp)
	ld gp,16(sp)
	ld tp,24(sp)
	ld t0,32(sp)
	ld t1,40(sp)
	ld t2,48(sp)
	ld s0,56(sp)
	ld s1,64(sp)	
	ld a0,72(sp)
	ld a1,80(sp)
	ld a2,88(sp)
	ld a3,96(sp)
	ld a4,104(sp)
	ld a5,112(sp)
	ld a6,120(sp)
	ld a7,128(sp)
	ld s2,136(sp)
	ld s3,144(sp)
	ld s4,152(sp)
	ld s5,160(sp)
	ld s6,168(sp)
	ld s7,176(sp)
	ld s8,184(sp)
	ld s9,192(sp)
	ld s10,200(sp)
	ld s11,208(sp)
	ld t3,216(sp)
	ld t4,224(sp)
	ld t5,232(sp)
	ld t6,240(sp)
	#ld sp,8(sp)

	addi sp,sp,256


    csrrw sp, sscratch, sp

	sret


##############################


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

ret_from_fork:
	ld t3,248(a0)
	csrw sepc,t3

	#切换权限
	li t1, 0x100            #t1=0001 0000 0000 
	csrc sstatus, t1         #将mstatus第8位清0 


	ld ra,0(a0)
	ld sp,8(a0)
	ld gp,16(a0)
	ld tp,24(a0)
	ld t0,32(a0)
	ld t1,40(a0)
	ld t2,48(a0)
	ld s0,56(a0)
	ld s1,64(a0)	
	ld a1,80(a0)
	ld a2,88(a0)
	ld a3,96(a0)
	ld a4,104(a0)
	ld a5,112(a0)
	ld a6,120(a0)
	ld a7,128(a0)
	ld s2,136(a0)
	ld s3,144(a0)
	ld s4,152(a0)
	ld s5,160(a0)
	ld s6,168(a0)
	ld s7,176(a0)
	ld s8,184(a0)
	ld s9,192(a0)
	ld s10,200(a0)
	ld s11,208(a0)
	ld t3,216(a0)
	ld t4,224(a0)
	ld t5,232(a0)
	ld t6,240(a0)	
	ld a0,72(a0)

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