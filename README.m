## MKRTOS是什么
        MKRTOS全称是Micro-Kernel Real-Time Operating System，中文名字是微内核实时操作系统。其最终的意义是实现一个小内核，并兼容Posix标准的操作系统。

### 开发背景
        Cortex-M在工业控制领域非常火，但是易用的操作系统却非常上，可能有ucos这类的操作系统，但是其功能都非常的简单，使用这类RTOS都有着一定的学习成本（我也写过一个，可以看看xTinyRTOS，非常简单）.开发MKRTOS的初衷就是兼容POSIX，以降低学习成本，并且在开发时还考虑到内存的使用，对内存使用进行优化。
### MKRTOS内核特性
1.处理器：默认支持Cortex-M3处理器。
2.进程间通信：支持信号、消息邮箱、信号量、匿名管道、共享内存。
3.移dietlibc。
4.文件系统：支持vfs，并开发了spfs文件系统，文件系统支持块缓存。
5.可执行文件：在没有MMU的处理器上支持可重定向的ELF可执行文件。
6.内存管理：简单的内存管理。
7.多任务，多线程还在开发。
8.驱动框架：字符设备与块设备。
9：常用驱动：null，tty，tty0，flash。

### 系统功能
        支持的库：dietlibc。
支持的应用：lsh，cat，ls，mkdir，uname等

### 怎么使用？
工程为Clion工程，利用CMake+arm-gcc编译，下载后使用Clion编译即可。