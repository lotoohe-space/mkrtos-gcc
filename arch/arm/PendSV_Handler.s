.syntax unified
.cpu cortex-m3
.thumb

.global PendSV_Handler
.section .text.Reset_Handler
.type PendSV_Handler, %function
PendSV_Handler:
	CPSID   I

	//psp和msp作为任务切换的参数
	MRS R0,PSP
	MRS R1,MSP
	cmp r0,#0

	//如果不是第一次调度了，则保存r4-r11到栈内
	TST LR,#4
	ITE EQ
	MRSEQ r3, MSP
	MRSNE r3, PSP
	ITE EQ
	MOVEQ r2, #0
	MOVNE r2, #1


	//检测psp如果为0，说明是第一次进行调度，则立刻进行一次调度
	CMP r0,#0
	CBZ r0, thread_change

	SUB R3, R3, #0x20 //R0 -= 0x20
	STM R3, {R4-R11}
	//参数2 设置使用栈的为psp
	//mov r2,#1
	//#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	//;支持FPU
	//		SUB     R3, R3, #0x40
	//		VSTM    R3, {D8-D15}
	//#endif
	//将更改的值放回要传的参数中去
	TST LR,#4
	ITE EQ
	MOVEQ r1,r3
	MOVNE r0,r3
	ITE EQ
	MSREQ MSP, r3
	MSRNE PSP, r3

thread_change://任务调度
	//r0:psp r1:msp  r2:(0为msp 1 psp)
	ldr.w r3,=sys_task_sche
	blx r3
	//取得下一个任务的参数 r1 psp r2 msp
	ldrd r1,r2,[r0]
	//用户使用的栈类型r3 0为msp 1为psp 2标志任务为内核任务，默认使用msp
	ldr r3,[R0,#8]

	//检测使用的寄存器
	tst r3,#1
	ITE EQ
	MOVEQ R0,R2
	MOVNE R0,R1//不等于零则使用psp
	//#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	//;支持FPU
	//		VLDM 		R0, {D8-D15} //恢复新的r4-r11的值
	//		ADDS 		R0, R0, #0x40 //R0 += 0x40
	//#endif
	LDM r0, {R4-R11} 		//恢复新的r4-r11的值
	ADDS r0, r0, #0x20 	//R0 += 0x20

	//#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	//BIC.W   LR, LR, #0x10
	//#endif
	//保存栈的值回去
	MSR MSP,R2
	MSR PSP,R1

	//如果等于2，则说明是内核线程
	//则跳转到re
	cmp r3,#2
	beq ree

	//检查svc是否被嵌套
	tst r3,#0x00010000
	bne reeSvc//嵌套了则执行其它操作

	//没有嵌套，则检查所使用的栈
	cmp r3,#1
	beq userMode//psp模式则跳转
	b ree
userMode:
//psp模式则保存
	MSR PSP, r0
	//切换到用户线程模式
	MOV R1,#1
	MSR CONTROL, R1
	//lr 的第2位为1时自动切换到PSP
	MOV LR,#0xfffffffd
	b ree1

////内核线程
reeSvc:
	//如果是2代表内核线程被中断
	tst r3,#0x00000002
	bne setKenTher //跳转设置为特权模式
	//否则设置为非特权模式
	MOV R1,#3
	MSR CONTROL, R1
	b setKenTherNext
setKenTher:
	MOV R1,#0
 	MSR CONTROL, R1
setKenTherNext:
//	ldr.w r1,=SetSvcStatus
//	blx r1
	MSR MSP,R0
	MOV LR,#0xfffffff1
	b ree1

ree: //更新下msp的值
	MSR MSP,R0
	MOV LR,#0xfffffff9
	//内核线程，设置为特权模式
	MOV R0,#0
	MSR CONTROL, R0
ree1:
	//开中断
	cpsie I
	BX 	LR