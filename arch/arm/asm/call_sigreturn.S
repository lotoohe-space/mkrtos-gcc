.syntax unified
.cpu cortex-m3
.thumb

//ϵͳ����
.global call_sigreturn
.section .text.call_sigreturn
.type call_sigreturn, %function
call_sigreturn:
    mov r7,#87
    svc #0x80
