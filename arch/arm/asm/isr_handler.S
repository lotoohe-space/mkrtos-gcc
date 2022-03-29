.syntax unified
.cpu cortex-m3
.thumb

.global isr_handler
.section .text.isr_handler
.type isr_handler, %function
isr_handler:
//	TST.W LR, #4
//	ITE EQ
 //	MRSEQ R0, MSP
//	MRSNE R0, PSP
//	SUB R0, R0, #0x20
//	STM R0, {R4-R11}
	MRS R0, PSP
	B ExternInter