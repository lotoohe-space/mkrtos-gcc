.syntax unified
.cpu cortex-m3
.thumb
.text
.weak setjmp
setjmp:
.global __setjmp
__setjmp:
	mov	r1, #0
.global __sigsetjmp
__sigsetjmp:
.weak sigsetjmp
.syntax unified
sigsetjmp:
#ifndef	__SOFTFP__
	sfm	f4, 4, [r0], #48
#endif
//zhangzheng 	stmia	r0, {r4-r11, sp, lr}
	sub	r0, r0, #30
	b	__sigjmp_save

