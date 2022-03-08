
#include <errno.h>
#include "syscalls.h"
//直接这样也行，哈哈哈
syscall(mmap,mmap)
/*
.syntax unified
.cpu cortex-m3
.thumb
.text

@
@ mmap takes 6 parameters - ie more than can be passed in registers via the
@ regular syscall interface. Instead, parameters are passed on the stack.
@
@ On entry, the compiler will have already placed the fifth and sixth
@ parameters on the stack - all we need do here is push the first four and
@ call the syscall.
@ 这个系统调用用6个参数，md，还没有处理
//TODO:

.global mmap

mmap:
	stmdb	sp!, {r0, r1, r2, r3}
    push {r7};
    mov r7,__NR_mmap
	mov	r0, sp
	svc	128
    pop {r7};
	add	sp, sp, #16
	b	__unified_syscall

*/