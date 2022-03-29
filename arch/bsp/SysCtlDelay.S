.syntax unified
.cpu cortex-m3
.thumb

.global SysCtlDelay
.section .text.SysCtlDelay
.type SysCtlDelay, %function
SysCtlDelay:
    _again:
    subs r0,#1
    bne _again
    bx lr