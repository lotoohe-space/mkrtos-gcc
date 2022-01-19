.syntax unified
.cpu cortex-m3
.thumb

.global atomic_inc
.section .text.atomic_inc
.type atomic_inc, %function
atomic_inc:
    	MOV R3,#0
        TryInc:
        	LDREX r2, [R0]
        	ADD r2, #1
        	CMP R2,#0
        	BEQ incN
        	B incN1
        incN:
        	MOV R3,#1
        incN1:
        	STREX R1, R2, [R0]
        	CMP R1, #1	//检查 STREX 是否被驳回
        	BEQ TryInc	//重新尝试
        	MOV R0,R3
        	bx lr

.global atomic_dec
.section .text.atomic_dec
.type atomic_dec, %function
atomic_dec:
        MOV R3,#0
        TryDec:
            LDREX r2, [R0]
            ADD r2, #-1
            CMP R2,#0
            BEQ decN
            B decN1
        decN:
            MOV R3,#1
        decN1:
            STREX R1, R2, [R0]
            CMP R1, #1	//检查 STREX 是否被驳回

            BEQ TryDec	//重新尝试

            bx lr
.global atomic_set
.section .text.atomic_set
.type atomic_set, %function
atomic_set:
TrySet:
	LDREX r2, [R0]
	MOV r2, R1
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回
	BEQ TrySet	//重新尝试

	bx lr

.global AtomicRetSet
.section .text.AtomicRetSet
.type AtomicRetSet, %function
AtomicRetSet:
	push {r4}
TryRetSet:
	LDREX r2, [R0]
	mov r4,r2
	MOV r2, R1
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回
	BEQ TryRetSet	//重新尝试
	mov r0,r4
	pop {r4}
	bx lr

.global atomic_read
.section .text.atomic_read
.type atomic_read, %function
atomic_read:
	LDR R0, [R0]
	BX LR

.global AtomicAdd
.section .text.AtomicAdd
.type AtomicAdd, %function
AtomicAdd:
	MOV R3,R1
TryAdd:
	LDREX r2, [R0]
	ADD r2, R3
	STREX R1, R2, [R0]
	CMP R1, #1	//检查 STREX 是否被驳回
	BEQ TryAdd	//重新尝试
	BX LR

.global AtomicSub
.section .text.AtomicSub
.type AtomicSub, %function
AtomicSub:
	MOV R3,R1
TrySub:
	LDREX r2, [R0]
	SUB r2, R3
	STREX R1, R2, [R0]
	CMP R1, #1	//检查 STREX 是否被驳回
	BEQ TrySub	//重新尝试
	BX LR

.global atomic_test
.section .text.atomic_test
.type atomic_test, %function
atomic_test:
	push {r4}
TryTest:
	LDREX r2, [R0]
	CMP R2,r1 //比较是否等于
	BEQ FZTest //等于则跳转到赋值
	MOV R4,#0
	B FZETest
FZTest:
	MOV R4,#1
FZETest:
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回

	BEQ TryTest	//重新尝试
	MOV R0,R4
	pop {r4}
	bx lr

.global atomic_test_set
.section .text.atomic_test_set
.type atomic_test_set, %function
atomic_test_set:
	push {r4}
TryTestSet:
	LDREX r2, [R0]
	CMP R2,#0 //比较是否等于0
	BEQ FZTestSet //等于0则跳转到赋值
	MOV R4,#0
	B FZETestSet
FZTestSet:
	MOV R4,#1
	MOV r2, R1 //等于零则进行赋值
FZETestSet:
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回

	BEQ TryTestSet	//重新尝试
	MOV R0,R4
	pop {r4}
	bx lr

.global atomic_set_test
.section .text.atomic_set_test
.type atomic_set_test, %function
atomic_set_test:
	push {r4}
TrySetTest:
	LDREX r2, [R0]
	CMP R2,#0 //比较是否等于0
	BEQ FZSetTest //等于0则跳转到赋值
	MOV R4,#0
	B FZESetTest
FZSetTest:
	MOV R4,#1

FZESetTest:
    MOV r2, R1 //等于零则进行赋值
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回

	BEQ TrySetTest	//重新尝试
	MOV R0,R4
	pop {r4}
	bx lr

.global atomic_inc_test
.section .text.atomic_inc_test
.type atomic_inc_test, %function
atomic_inc_test:
        push {r4}
    TryIncTest:
        LDREX r2, [R0]
        CMP R2,#0 //比较是否等于0
        BEQ FZIncTest //等于0则跳转到赋值
        MOV R4,#0
        B FZEIncTest
    FZIncTest:
        MOV R4,#1
    FZEIncTest:
        ADD R2,R2,#1
        STREX R3, R2, [R0]
        CMP R3, #1	//检查 STREX 是否被驳回

        BEQ TryIncTest	//重新尝试
        MOV R0,R4
        pop {r4}
        bx lr


.global atomic_test_inc
.section .text.atomic_test_inc
.type atomic_test_inc, %function
atomic_test_inc:
	push {r4}
TryTestInc:
	LDREX r2, [R0]
	CMP R2,#0 //比较是否等于0
	BEQ FZTestInc //等于0则跳转到赋值
	MOV R4,#0
	B FZETestInc
FZTestInc:
	MOV R4,#1
	ADD r2,r2,#1 //等于零则进行赋值
FZETestInc:
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回

	BEQ TryTestInc	//重新尝试
	MOV R0,R4
	pop {r4}
	bx lr

.global atomic_test_dec_nq
.section .text.atomic_test_dec_nq
.type atomic_test_dec_nq, %function
atomic_test_dec_nq:
	push {r4}
TryTestDecNq:
	LDREX r2, [R0]
	CMP R2,#0 //比较是否等于0
	BEQ FZTestDecNq//等于0则跳转
	MOV R4,#0
	ADD r2, #-1 //不等于零则减一
	B FZETestDecNq
FZTestDecNq:
	MOV R4,#1
FZETestDecNq:
	STREX R3, R2, [R0]
	CMP R3, #1	//检查 STREX 是否被驳回

	BEQ TryTestDecNq	//重新尝试
	MOV R0,R4
	pop {r4}
	bx lr

