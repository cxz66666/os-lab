# RISC-V based MiniOS



一款基于RISC-V的仿Linux操作系统内核，使用C语言和RISC-V汇编编写，可运行在QEMU模拟器或RISC-V开发板上。



### Feature List

- [BootLoader](#BootLoader)

- [进程调度](#进程调度)（基于SJF，Priority，RR）
- [SV39三级页表虚拟内存机制](#虚拟内存)
- [中断委托/中断处理](#中断处理)
- [用户态进程](#用户态进程)
- [Buddy System and Slub](#buddy-system-and-slub)
- [系统调用](#系统调用)（fork/mmap/write/getpid等）
- ELF file loader（目前未实现）





### How To Use?

- 首先，您需要配置docker环境，具体请参考[官网](https://docs.docker.com/desktop/)

  个人已经打包好一份docker镜像，包含完整的开发工具（`qemu` `riscv64-unknown-elf工具链`等），以及包括 `gef插件`、 `zsh`等配套工具（具体可以参考这个[repo](https://github.com/cxz66666/OS-Dockerfile)）

  

  请使用如下命令拉取镜像

  ~~~bash
  docker pull raynor123/oslab-all:latest
  ~~~

- 使用如下命令clone本仓库

  ~~~bash
  git clone git@github.com:cxz66666/os-lab.git
  ~~~

- 假设您clone位置为`/app/os-lab`，请使用如下命令进入开发环境（注意修改您clone的位置）

  ~~~bash
  docker run -it --rm -v /app/os-lab:/home/os-lab -u root --name="oslab" --network host raynor123/oslab-all:latest zsh
  ~~~

- 此时假设您已经进入容器中，使用zsh shell

  ~~~bash
  cd /home/os-lab
  
  # 首先需要构建用户态程序
  cd user
  make # 构建用户程序hello.bin，源程序为getpid.c，您可以自行修改该程序
  mv user/hello.bin ./  # copy到根目录
  
  make # 构建kernel
  # or
  make run # 直接运行
  # or 
  make debug # 使用gdb进行调试
  
  ~~~

  ![image-20220821183234750](https://pic.raynor.top/images/2022/08/21/image-20220821183234750e384ed30cba9bf8c.png)

- 参考下文的内容，自行修改用户态程序/内核，enjoy yourself！



### 代码结构

~~~bash
├── Makefile
├── README.md
├── arch  
│   └── riscv
│       ├── Makefile
│       ├── boot
│       ├── include  #内核独自使用的部分头文件定义
│       │   ├── buddy.h #buddy system
│       │   ├── defs.h  #const define
│       │   ├── fault.h #page fault头文件
│       │   ├── sched.h 
│       │   ├── slub.h 
│       │   ├── syscall.h
│       │   └── vm.h
│       └── kernel #内核核心代码
│           ├── Makefile
│           ├── buddy.c
│           ├── entry.S #入口文件
│           ├── fault.c
│           ├── head.S
│           ├── sched.c
│           ├── slub.c
│           ├── strap.c
│           ├── syscall.c
│           ├── vm.c
│           └── vmlinux.lds #链接脚本
├── include  #工具类
│   ├── list.h
│   ├── put.h
│   ├── rand.h
│   ├── stddef.h
│   ├── string.h
│   └── types.h
├── init   #kernel init代码
│   ├── Makefile
│   └── main.c
├── lib #工具类
│   ├── Makefile
│   ├── put.c
│   ├── rand.c
│   └── string.c
└── user #用户态部分
    ├── Makefile
    ├── getpid.c #测试的c语言源码
    ├── link.ld  #链接脚本
    ├── printf.c
    ├── start.S
    ├── stddef.h
    ├── stdio.h   #类似标准的stdio.h
    ├── syscall.h #系统调用库
    └── types.h
~~~



### BootLoader

如下是RISC-V标准的BootLoader过程

![启动过程](https://mianbaoban-assets.oss-cn-shenzhen.aliyuncs.com/2020/12/eU3yIz.png) 

- ZSBL(Zeroth Stage Boot Loader)：片上ROM程序，烧录在硬件上，是芯片上电后最先运行的代码。它的作用是加载FSBL到指定位置并运行。
- FSBL(First Stage Boot Loader ）：启动PLLs和初始化DDR内存，对硬件进行初始化，加载下一阶段的bootloader。
- OpenSBI：运行在m模式下的一套软件，提供接口给操作系统内核调用，以操作硬件，实现字符输出及时钟设定等工作。OpenSBI就是一个开源的RISC-V虚拟化二进制接口的通用的规范。
- Bootloader：OpenSBI初始化结束后会通过mret指令将系统特权级切换到s模式，并跳转到操作系统内核的初始化代码。这一阶段，将会完成中断地址设置等一系列操作。之后便进入了操作系统。



为了方便开发，我们并不选择自己实现ZSML和FSBL，而是通过QEMU模拟器完成。运行QEMU时，我们使用-bios default选项将OpenSBI代码加载到0x80000000起始处。OpenSBI初始化完成后，会以M模式跳转到0x80200000处。

代码将从arch/riscv/kernel/head.S 中的_start函数开始执行，具体执行的每一句汇编的含义我已经详细的注释，简而言之，BootLoader中会进行如下的操作

- 设置若干M模式特权寄存器：
  - mstatus：控制状态寄存器
  - mtrap：m中断跳转地址
  - mscratch：在vmlinux.lds中使用` . += 0x1000`留出4KB地址用于m中断处理的内核栈
  - mtimecmp：时钟中断相关
- 设置中断委托：
  - 设置时钟中断委托
  - 设置instruction/load/store page fault委托
- 使用`mret`跳转到S模式，开始内核初始化
- 保存内核`start_kernel`函数的虚拟代码地址和虚拟栈地址
- 调用`init_buddy_system`初始化buddy system，接管所有物理内存
- 调用`paging_init`进行页表初始化，创建三个主要映射，具体请参考虚拟内存机制部分
- 设置`satp`寄存器开启虚拟内存模式，并使用内存屏障保证代码执行顺序
- 跳转到内核的`start_kernel`函数
- 调用`slub_init`初始化slub分配器
- 调用`task_init`开始创建所有的task进程并开始执行
- dead_loop



值得注意的是如果你不了解具体内核中内存布局，请使用`nm vmlinux`查看内核符号表，为Debug提供了极大方便。如下是一个样例

~~~assembly
0000000000000800 A __vdso_rt_sigreturn
ffffffe000000000 T __init_begin
ffffffe000000000 T _sinittext
ffffffe000000000 T _start
ffffffe000000040 T _start_kernel
ffffffe000000076 t clear_bss
ffffffe000000080 t clear_bss_done
ffffffe0000000c0 t relocate
ffffffe00000017c t set_reset_devices
ffffffe000000190 t debug_kernel
~~~

但是如果你想更进一步了解，请参考`arch/riscv/kernel/vmlinux.lds`，lds文件为链接脚本文件，ld（linux连接器）使用该脚本来控制ELF文件的内存布局，在其中你可以找到你想要的一些变量位置，e.g.`init_stack_top`等









### 中断处理

RISC-V将中断分为两类。一类是interrupt，它有三种标准的中断源：软件、时钟和外部来源。另外一类是同步异常(exception)，这类异常在指令执行期间产生，如访问了无效的存储器地址或执行了具有无效操作码的指令。



这里我们用异常(trap)作为硬件中断(interrupt)和同步异常(exception)的集合，另外trap指的是发生硬件中断或者同步异常时控制权转移到handler的过程。

> 后文统一用异常指代trap，中断/硬件中断指代interrupt，同步异常指代exception。



中断处理需要使用的寄存器首先有之前提到的`mstatus`，`mip`，`mie`，`mtvec`寄存器，这些寄存器需要我们操作；剩下还有`mepc`，`mcause`寄存器，这些寄存器在异常发生时**硬件会自动置位**，它们的功能如下：

* `mepc`：存放着中断或者异常发生时的指令地址，当我们的代码没有按照预期运行时，可以查看这个寄存器中存储的地址了解异常处的代码。通常指向异常处理后应该恢复执行的位置。

* `mcause`：存储了异常发生的原因。

* `mstatus`：Machine Status Register，其中m代表M模式。此寄存器中保持跟踪以及控制hart(hardware thread)的运算状态。通过对`mstatus`进行位运算，可以实现对不同bit位的设置，从而控制不同运算状态。

* `mie`、`mip`：`mie`以及`mip`寄存器是Machine Interrup Registers，用来保存中断相关的一些信息，通过`mstatus`上mie以及mip位的设置，以及`mie`和`mip`本身两个寄存器的设置可以实现对硬件中断的控制。注意mip位和`mip`寄存器并不相同。

* `mtvec`：Machine Trap-Vector Base-Address Register，主要保存M模式下的trap vector（可理解为中断向量）的设置，包含一个基地址以及一个mode。 





#### 中断委托

**RISC-V架构所有mode的异常在默认情况下都跳转到M模式处理**。为了提高性能，RISC-V支持将低权限mode产生的异常委托给对应mode处理，该过程涉及了`mideleg`和`medeleg`这两个寄存器。

* `mideleg`：Machine Interrupt Delegation。该寄存器控制将哪些Interrupt委托给S模式处理，它的结构可以参考`mip`寄存器，如`mideleg[5]`对应于 S模式的时钟中断，如果把它置位， S模式的时钟中断将会移交 S模式的异常处理程序，而不是 M模式的异常处理程序。
* `medeleg`：Machine Exception Delegation。该寄存器控制将哪些Exception委托给对应mode处理，它的各个位对应`mcause`寄存器的返回值。



你可以在head.S中找到这两个寄存器被使用的情况





#### 中断处理

硬件在处理trap之前会自动经历以下的状态转换（部分列出）

* 发生异常的时`pc`的值被存入`sepc`，且`pc`被设置为`stvec`。
* `scause`按图 10.3根据异常类型设置，`stval`被设置成出错的地址或者其它特定异
  常的信息字。
* `sstatus` CSR中的 SIE 位置零，屏蔽中断，且中断发生前的`sstatus[sie]`会被存入`sstatus[spie]`。
* 发生异常时的权限模式被保存在`sstatus[spp]`，然后设置当前模式为 S模式。 



之后我们会跳转到`trap_s`，这是一段汇编编写的代码，具体进行了如下的操作：

- ~~~assembly
  # load kernel stack ptr，之前使用的栈保存起来，使用内核进程所使用的内核栈地址
  	csrrw sp,sscratch,sp
  ~~~

- 保存所有寄存器，以及额外的控制寄存器（sepc，scause等）

- 调用handler_s代码（c语言编写），进入真正的中断处理，处理中断和异常

- 恢复保存的寄存器，以及额外的控制寄存器

- 恢复之前的用户栈

  ~~~assembly
   csrrw sp, sscratch, sp
  
   sret
  ~~~

  



#### handler_s分析

该函数是主要的中断处理函数，具体进行了如下的操作：

- copy_stack保存之前的活动上下文到task_struct结构体中，之后可以随意使用
- 针对scause不同情况采取不同的措施
  - 时钟中断：调用do_timer进行处理
  - 系统调用：根据a7寄存器的值区分不同的系统调用，进行相应的处理
  - page fault： 调用do_page_fault进行处理
- 根据是否需要返回值来修改a0寄存器







### 进程调度

具体的调度算法并不需要详述，各种地方讲的是头头是道，这里我想着重强调以下两部分



#### Task_struct分析

~~~c
/* 进程数据结构 */
struct task_struct
{
    long state;    // 进程状态 Lab3中进程初始化时置为TASK_RUNNING
    long counter;  // 运行剩余时间
    long priority; // 运行优先级 1最高 5最低
    long blocked;
    long pid; // 进程标识符
    // Above Size Cost: 40 bytes

    struct thread_struct thread; // 该进程状态段

    unsigned long long sepc;
    unsigned long long sscratch;
    struct mm_struct mm;
    struct pt_regs *stack; // fork使用
};
struct pt_regs
{
    uint64 ra, sp, gp, tp;
    uint64 t0, t1, t2, s0, s1;
    uint64 a0, a1, a2, a3, a4, a5, a6, a7;
    uint64 s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64 t3, t4, t5, t6;
    uint64 sepc;
};
/* 进程状态段数据结构 */
struct thread_struct
{
    unsigned long long ra;
    unsigned long long sp;
    unsigned long long s0;
    unsigned long long s1;
    unsigned long long s2;
    unsigned long long s3;
    unsigned long long s4;
    unsigned long long s5;
    unsigned long long s6;
    unsigned long long s7;
    unsigned long long s8;
    unsigned long long s9;
    unsigned long long s10;
    unsigned long long s11;
};
~~~

这里最重要的是问题是：**进程的上下文存在哪**？

答案是存在thread_struct结构体和pt_regs结构体中，为什么要存两边呢？

- pt_regs是在用户态->内核态下，保存所有用户态进程需要的内容
- thread_struct是在内核态->内核态下，保存上一个内核态需要的内容



等等，为什么会有内核态->内核态的情况？

可以带着这个问题来看第二部分



#### 进程切换

假设我们已经通过各种调度算法选出了下一个需要执行的task，调用`void switch_to(struct task_struct **next*)`函数进行上下文切换，之后会调用汇编编写的`__switch_to(struct task_struct\* prev, struct task_struct\* next)`函数

这里我想详细分析下这个函数

~~~assembly
__switch_to:
	li    a4,  40 # thread_struct的偏移
	add   a3, a0, a4
	add   a4, a1, a4
	#Save context into prev->thread

	#保存所有caller调用需要保存的寄存器	
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
	#该进程的用户态栈地址保存在当前的sscratch
	csrr s11,sscratch
	sd s11,120(a3)
	#Restore context from next->thread

	# 切换页表！
	ld a5,128(a4)
	srli a5,a5,12
	li t2,0x8000000000000000
	add a5,a5,t2
	csrw satp,a5
	#刷新tlb和内存屏障
	sfence.vma x0,x0

	ld s11,120(a4)
	#恢复之前的用户态栈地址
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
~~~

等等，这里最后return到哪里了呢？

分两种情况

- 如果该进程为新进程，ra被设置为了`forkret`函数，该函数为想办法从内核态返回到用户态
- 不为新进程，ra就设置成了之前的switch_to函数，程序就会直接接着从switch-to函数继续执行（内核态），就像无事发生过

后一种情况就是之前提过的内核态->内核态的进程上下文切换





### 虚拟内存

#### MMU

先提出一个问题：MMU在哪？

MMU（Memory Management Unit）是一种负责处理中央处理器（CPU）的内存访问请求的**计算机硬件**。MMU位于**处理器内核和连接高速缓存以及物理存储器的总线之间**。如果处理器没有MMU，CPU内部执行单元产生的内存地址信号将直接通过地址总线发送到芯片引脚，被内存芯片接收，这就是物理地址。如果MMU存在且启用，CPU执行单元产生的地址信号在发送到内存芯片之前将被MMU截获，这个地址信号称为虚拟地址，MMU会负责把VA翻译成相应的物理地址，然后发到内存芯片地址引脚上。

简而言之，当处理器内核取指令或者存取数据的时候，会提供一个虚拟地址，这个地址是可执行代码在编译的时候由链接器生成的。MMU负责将虚拟地址转换为物理地址，以在物理存储器中访问相应的内容。



#### SV39分页方案

Sv39使用4KB大的基页，页表项的大小是8个字节，为了保证页表大小和页面大小一致，树的基数相应地降到$2^9$，树也变为三层。 Sv39的 512 GB地址空间（虚拟地址）划分为$2^9$个 1GB大小的吉页 。每个吉页被进一步划分为$2^9$个2MB大小的巨页。每个巨页再进一步分为$2^9$个4KB大小的基页。



~~~
     38        30 29        21 20        12 11                           0
     ---------------------------------------------------------------------
    |   VPN[2]   |   VPN[1]   |   VPN[0]   |          page offset         |
     ---------------------------------------------------------------------
                            Sv39 virtual address

~~~

~~~
 55                30 29        21 20        12 11                           0
 -----------------------------------------------------------------------------
|       PPN[2]       |   PPN[1]   |   PPN[0]   |          page offset         |
 -----------------------------------------------------------------------------
                            Sv39 physical address

~~~

~~~
 63      54 53        28 27        19 18        10 9   8 7 6 5 4 3 2 1 0
 -----------------------------------------------------------------------
| Reserved |   PPN[2]   |   PPN[1]   |   PPN[0]   | RSW |D|A|G|U|X|W|R|V| 
 -----------------------------------------------------------------------
                                                     |   | | | | | | | |
                                                     |   | | | | | | | `---- V - Valid
                                                     |   | | | | | | `------ R - Readable
                                                     |   | | | | | `-------- W - Writable
                                                     |   | | | | `---------- X - Executable
                                                     |   | | | `------------ U - User
                                                     |   | | `-------------- G - Global
                                                     |   | `---------------- A - Accessed
                                                     |   `------------------ D - Dirty (0 in page directory)
                                                     `---------------------- Reserved for supervisor software
~~~



具体涉及代码的部分并不需要多说，请参考`create_mapping`和`user_paging_init` 等函数





#### Satp寄存器

一个叫satp（Supervisor Address Translation and Protection，监管者地址转换和保护）的 S模式控制状态寄存器控制了分页系统，其内容如下所示：

```c
 63      60 59                  44 43                                0
 ---------------------------------------------------------------------
|   MODE   |         ASID         |                PPN                |
 ---------------------------------------------------------------------
```

* MODE：可以开启分页并选择页表级数，8表示Sv39分配方案，0表示禁用虚拟地址映射。

* ASID (Address Space Identifier) ： 用来区分不同的地址空间，此次实验中直接置0即可。
* PPN (Physical Page Number) ：保存了根页表的物理地址，通常 `PPN = physical address >> 12`。M模式的程序在第一次进入 S模式之前会把零写入 satp以禁用分页，然后 S模式的程序在初始化页表以后会再次进行satp寄存器的写操作。







### 用户态进程

TODO，后续有时间了写





### Buddy System and Slub

TODO，同上





### 系统调用

TODO
