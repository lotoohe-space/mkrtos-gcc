.syntax unified
.cpu cortex-m3
.thumb

//系统调用
.global call_sigreturn
.section .text.call_sigreturn
.type call_sigreturn, %function
call_sigreturn:
    mov r7,#87
    svc #0x80
