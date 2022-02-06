.syntax unified
.cpu cortex-m3
.thumb

//系统调用
.global SysCall
.section .text.SysCall
.type SysCall, %function
SysCall:
    push {r7,lr}
    ldr r7,=sys_call_table
    ldr r7,[r7]
    blx r7
    pop {r7,pc}


.global _syscall
.section .text._syscall
.type _syscall, %function
_syscall:
    push {lr}
    svc #0x80
    pop {pc}
