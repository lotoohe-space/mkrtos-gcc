.syntax unified
.cpu cortex-m3
.thumb

//系统调用
.global SysCall
.section .text.SysCall
.type SysCall, %function
SysCall:
    push {r4,r5,lr}
    ldr r4,=sys_call_table
    add r4,r0
    ldr r5,[r4]
    mov r0,r1
    mov r1,r2
    mov r2,r3
    blx r5
    pop {r4,r5,pc}


.global _syscall
.section .text._syscall
.type _syscall, %function
_syscall:
    push {lr}
    svc #0x80
    pop {pc}
