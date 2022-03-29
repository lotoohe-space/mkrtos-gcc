.syntax unified
.cpu cortex-m3
.thumb
.text
.global __testandset
__testandset:
    LDREX r2, [R0]
    mov r1,#1
    STREX r3, r1, [R0]
    CMP R3, #1	//检查 STREX 是否被驳回
    BEQ __testandset	//重新尝试
    mov r0,r2
	mov	pc, lr

//	mov	r2, r0
//mov	r1, #1
//swp	r0, r1, [r2]
