/**
  *************** (C) COPYRIGHT 2017 STMicroelectronics ************************
  * @file      startup_stm32f103xe.s
  * @author    MCD Application Team
  * @brief     STM32F103xE Devices vector table for Atollic toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Configure the clock system
  *                - Configure external SRAM mounted on STM3210E-EVAL board
  *                  to be used as data memory (optional, to be enabled by user)
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M3 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* start address for the initialization values of the .data section.
defined in linker script */
.word _sidata
/* start address for the .data section. defined in linker script */
.word _sdata
/* end address for the .data section. defined in linker script */
.word _edata
/* start address for the .bss section. defined in linker script */
.word _sbss
/* end address for the .bss section. defined in linker script */
.word _ebss

.equ  BootRAM,        0xF1E0F85F
/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*/

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:

/* Copy the data segment initializers from flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

/* Zero fill the bss segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

/* Call the clock system intitialization function.*/
    bl  SystemInit
/* Call static constructors */
  //  bl __libc_init_array
/* Call the application's entry point.*/
  bl main
  bx lr
.size Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors


g_pfnVectors:

  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word isr_handler//1

  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//5
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//10
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//15
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//20
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//25
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//30
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//35
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//40
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//45
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//50
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//55
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler//60
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word isr_handler
  .word BootRAM       /* @0x1E0. This is for boot in RAM mode for
                         STM32F10x High Density devices. */

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_Handler

  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler

  .weak MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler

  .weak BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler

  .weak UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler

 // .weak SVC_Handler
 // .thumb_set SVC_Handler,Default_Handler

  .weak DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler

  .weak PendSV_Handler
  .thumb_set PendSV_Handler.s,Default_Handler

 // .weak SysTick_Handler
 // .thumb_set SysTick_Handler,Default_Handler

  .weak isr_handler
  .thumb_set isr_handler,Default_Handler


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
