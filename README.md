## MKRTOS 是什么

- MKRTOS 全称是 Micro-Kernel Real-Time Operating System，中文名字是微内核实时操作系统。其最终的意义是实现一个小内核，并兼容 Posix 标准的操作系统，使其完全适用于各类嵌入式场合。

### 开发背景

- Cortex-M 在工业控制领域非常火，但是易用的操作系统却非常上，可能有 ucos 这类的操作系统，但是其功能都非常的简单，使用这类 RTOS 都有着一定的学习成本（我也写过一个，可以看看 xTinyRTOS，非常简单）.开发 MKRTOS 的初衷就是兼容 POSIX，以降低学习成本，并且在开发时还考虑到内存的使用，对内存使用进行优化。

### MKRTOS 内核特性

- 1.处理器：默认支持 Cortex-M3 处理器。
- 2.进程间通信：支持信号、消息邮箱、信号量、匿名管道、共享内存。
- 3.文件系统：支持 vfs，并开发了 spfs 文件系统，文件系统支持块缓存。
- 4.可执行文件：在没有 MMU 的处理器上支持可重定向的 ELF 可执行文件。
- 5.内存管理：简单的内存管理（后期可能会更换为slab算法）。
- 6.多任务，多线程。 
- 7.驱动框架：字符设备与块设备。
- 8.常用驱动：null，tty，tty0，flash。
- 9.以太网支持（部分支持，其它正在开发），采用lwip。

### 系统功能

- 支持的库：dietlibc。
- 支持的应用：lsh，cat，ls，mkdir，uname 等

### 怎么使用？

- 工程为 Clion 工程，利用 CMake+arm-gcc 编译，下载后使用 Clion 编译即可。

### 我的博客

- 欢迎到博客交流（还没有申请域名^-^）：[MKRTOS博客](http://124.222.90.143/)

### 笔记
1. dietlibc移植说明，去掉不需要的cpu相关的文件夹，syscalls.h内__ARGS_mmap改为1 ，并修改mmap.S文件。
2. 修改setjump.S等文件。
3. 所有的.S文件需要增加编译的头。
4. lwip移植
    1.需要完成clone函数的实现。
    2.需要完成信号量、互斥锁、消息邮箱。
    3.需要给struct sock添加引用计数，防止多进程时出现问题，需要对socket，accept，close这3个函数进行处理。
    4.一些其它的配置。
5. 其它大量修改（想不起来了）。
6. 2022/4/4 修复了一些使用上的bug，比如在用户线程修改了特权模式的寄存器，导致直接直接异常。
7. 未来工作：完善文件系统测试，进程通信测试，完成显示驱动支持。</br>
显示驱动支持想法：直接将显示缓存给用户操作是最直接的，速度最快，其它的细节交给用户处理就行了，直接需要提供驱动框架，以及一些配置的相关的接口即可。

### 工作规划
1. 完善用户管理，用户登录相关系统调用。
2. 完善spFS以及vfs文件系统。（1）mmap等的支持，考虑是否采用分页内存管理，分页比较费内存。（2）完善文件的权限管理
3. 重新实现自己的网络协议栈。
4. 显示驱动完善。
5. 移植最新版本的dietlibc。
6. 驱动的动态插入与卸载。
### Cortex-M3
## Cortex‐M3 的 CONTROL 寄存器
- Cortex-M3的CONTROL寄存器
CONTROL[1] 堆栈指针选择
0=选择主堆栈指针 MSP（复位后缺省值）
1=选择进程堆栈指针 PSP
在线程或基础级（没有在响应异常——译注），可以使用 PSP。在 handler 模式下，
只允许使用 MSP，所以此时不得往该位写 1。
CONTROL[0] 0=特权级的线程模式
1=用户级的线程模式
Handler 模式永远都是特权级的。
- CONTROL[1]
  在 Cortex‐M3 的 handler 模式中，CONTROL[1]总是 0。在线程模式中则可以为 0 或 1。
  仅当处于特权级的线程模式下，此位才可写，其它场合下禁止写此位。改变处理器的模
  式也有其它的方式：在异常返回时，通过修改 LR 的位 2，也能实现模式切换。这将在第 5
  章中展开论述。
- CONTROL[0]
  仅当在特权级下操作时才允许写该位。一旦进入了用户级，唯一返回特权级的途径，就
  是触发一个（软）中断，再由服务例程改写该位。
  CONTROL 寄存器也是通过 MRS 和 MSR 指令来操作的：
  MRS R0, CONTROL
  MSR CONTROL, R0
