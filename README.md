# ICS2021 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System) in College of Intelligence and Computing, Tianjin Univerisity.

This project is introduced from Nanjin University in 2016. Thank you for  Prof. Chunfeng Yuan of NJU and Dr. Zihao Yu of ICT.

The following subprojects/components are included. Some of them are not fully implemented.
* NEMU
* testcase
* uClibc
* kernel
* typing game
* NEMU-PAL

## NEMU

NEMU(NJU Emulator) is a simple but complete full-system x86 emulator designed for teaching. It is the main part of this programming assignment. Small x86 programs can run under NEMU. The main features of NEMU include
* a small monitor with a simple debugger //一个简单调试器带有一个监视器
 * single step
 * register/memory examination //寄存或者记忆检验
 * expression evaluation with the support of symbols //表达式求值
 * watch point
 * backtrace //栈回溯
* CPU core with support of most common used x86 instructions in protected mode
 * real mode is not supported
 * x87 floating point instructions are not supported
* DRAM with row buffer and burst
* two-level unified cache
* IA-32 segmentation and paging with TLB
 * protection is not supported
* IA-32 interrupt and exception
 * protection is not supported
* 6 devices
 * timer, keyboard, VGA, serial, IDE, i8259 PIC
 * most of them are simplified and unprogrammable
* 2 types of I/O
 * port-mapped I/O and memory-mapped I/O

## testcase

Some small C programs to test the implementation of NEMU.

## uClibc

uClibc(https://www.uclibc.org/) is a C library for embedding systems. It requires much fewer run-time support than glibc and is very friendly to NEMU.

## kernel

This is the simplified version of Nanos(http://cslab.nju.edu.cn/opsystem). It is a uni-tasking kernel with the following features.
* 2 device drivers
 * Ramdisk
 * IDE
* ELF32 loader
* memory management with paging
* a simple file system
 * with fix number and size of files
 * without directory
* 6 system calls
 * open, read, write, lseek, close, brk

## typing game

This is a fork of the demo of NJU 2013 oslab0(the origin repository has been deleted, but we have a fork of it -- https://github.com/nju-ics/os-lab0). It is ported to NEMU.

## NEMU-PAL

This is a fork of Wei Mingzhi's SDLPAL(https://github.com/CecilHarvey/sdlpal). It is obtained by refactoring the original SDLPAL, as well as porting to NEMU.

思考题1：cpu_exec()函数调用的参数是uint32_t，也就是unsigned int类型，-1隐式转换为0xffffffff，在函数体内部的for()循环内部一直执行下去，同时在循环体内部调用exec(cpu.eip)，即取指令后执行，exec通过查询opcode_table判断执行操作，如果是0xd6，nemu_trap指令，就会进入nemu_trap内部，将nemu_state赋值为END，改变状态，然后在cpu_exec()的循环体中判断nemu_state!=RUNNING，退出执行函数，结束程序。即-1指的是运行直到程序结束。

思考题2：static作用是让wp_pool的作用域限定在watchpoint.c内部，防止外部代码更改操作池

思考题3：CF位是无符号运算进位标志位，产生加法进位或者减法借位的时候CF位1，否则为0，内容在Chapter2 Basic Programming Model 2.3.4 Flags Register部分
ModR/M是1字节的寻址说明字节，在操作码之后。分为 3 个字段：mod：和r/m配合，指定寻址模式；reg/opcode：指定寄存器，或是操作码扩展；r/m：寄存器编号或者内存寻址模式。内容在Chapter17 INSTRUCTION SET 17.2 Instruction Format部分
mov机器码结构,操作码[+ModR/M][+SIB][+偏移displacement][+立即数immediate]内容在Chapter17 INSTRUCTION SET，MOV — Move Data
shell命令find . -name "*.c" -o -name "*.h" | xargs wc -l
去除空行find . -name "*.c" -o -name "*.h" | xargs cat | grep -v "^$$" | wc -l
-Wall,开启 gcc 绝大多数编译警告。会把很多潜在风险提示出来：未使用变量、类型隐式转换、数组越界、隐式函数声明等问题。警告默认不会阻止编译，但提示代码隐患。
-Werror,将所有警告全部升级为错误,出现警告，编译直接终止。

PA2思考题1：程序从main返回以后回到10000f:	b8 00 00 00 00       	mov    $0x0,%eax ,100014:	d6    也就是将eax清零,然后通过nemu_trap结束程序
思考题2：FLOAT比float拥有均匀的绝对精度和硬件设计更加简单，但是不能表示非常大或者非常小的数