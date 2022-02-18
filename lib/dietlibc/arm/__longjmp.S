.syntax unified
.cpu cortex-m3
.thumb
.text
.global __longjmp
.type __longjmp,function
__longjmp:
	mov	ip, r0
	movs	r0, r1
	moveq	r0, #1
#ifndef	__SOFTFP__
	lfm	f4, 4, [ip], #48
#endif
	ldmia.w   ip!, {r4-r11}
    ldmia.w   ip!, {r2}
    mov sp,r2
    ldmia.w   ip!, {pc};
