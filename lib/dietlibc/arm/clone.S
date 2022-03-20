
//#include <errno.h>
#include "syscalls.h"
syscall_weak(clone,clone,__clone)
/*
.syntax unified
.cpu cortex-m3
.thumb
.text
.weak 	clone
.global __clone

@
@ Some slightly tricky stuff here... edit with care :-)
@

clone:
__clone:
	movs	r12, r0			@ check function pointer
	cmpne	r1, #0			@ if function check for stack pointer
	moveq	r0, #-EINVAL		@ if one is not available set errno value
	beq	__unified_syscall	@ handle as if error was returned by the syscall

	stmdb	r1!, { r3, r12 } 	@ store function param (r3) and function ptr (r12)
					@ into what will become the childs stack.

	mov	r0, r2			@ move flags argument to r0
    push {r7}
    mov r7,__NR_clone
	svc	128		@ call the syscall
    pop {r7};
	movs	r0, r0			@ set condition codes
	blt	__unified_syscall	@ (return code < 0): handle as an error
	movne	pc, lr			@ (return code > 0): return to parent

	mov	fp, #0			@ clear the frame pointer
	adr	lr, 1f			@ setup return address
	ldmia	sp!, { r0, pc } 	@ load function param and jump to thread function

1:	b	_exit			@ branch to _exit (PIC safe)

*/