/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "mkrtos/task.h"


void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
    unsigned char *mfsr=(void*)0xe000ed28;
    unsigned char *bfsr=(void*)0xe000ed29;
    unsigned short *ufsr=(void*)0xe000ed2a;
    unsigned int *hard_fsr=(void*)0xe000ed2c;
    unsigned char *dfsr=(void*)0xe000ed30;
    printk("----------------------%s----------------\r\n",__FUNCTION__);
    printk("mfsr 0x%x\r\n",*mfsr);
    printk("bfsr 0x%x\r\n",*bfsr);
    printk("ufsr 0x%x\r\n",*ufsr);
    printk("hard_fsr 0x%x\r\n",*hard_fsr);
    printk("dfsr 0x%x\r\n",*dfsr);
    printk("----------------------------------------\r\n");
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
 

void DebugMon_Handler(void)
{
}

//void PendSV_Handler.s(void)
//{
//}
// 
//void SysTick_Handler(void)
//{
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
