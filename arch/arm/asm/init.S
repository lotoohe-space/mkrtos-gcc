.syntax unified
.cpu cortex-m3
.thumb

.global Init
.section .text.Init
.type Init, %function
Init:
CPSID   I
	//首先设置pendsv为最低优先级
	//设置pendsv的中断优先级
	ldr r0,=0xE000ED22
	//最低优先级,pendsv和systick优先级
	ldr r1,=0xa0a0
	strh r1,[r0]

	ldr r0,=0xE000ED1F
	ldr r1,=0xf0
	strb r1,[r0]
	//设置psp为0,用于判断是否第一次任务调度
	MOVS R0, #0 //R0 = 0
	MSR PSP, R0 //PSP = R0

	LDR R0, =0xE000ED04
	LDR R1, =0x10000000
	//进入pendsv中断
	STR R1, [R0]
	//打开中断
    CPSIE I ;
	//用户线程模式,用户以非特权模式运行
	MOV R0,#1
	MSR CONTROL, R0
	BX LR


/**
* @brief 软中断函数
*/

.global SVC_Handler
.section .text.SVC_Handler
.type SVC_Handler, %function
SVC_Handler:
	TST.W LR, #4
	ITE EQ
 	MRSEQ R0, MSP
	MRSNE R0, PSP
	//入栈r4-r11
	SUB R0, R0, #0x20
	STM R0, {R4-R11}
    MOV R1, R7
	B svcHandler
