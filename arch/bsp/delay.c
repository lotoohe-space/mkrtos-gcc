#include "delay.h"

void SysCtlDelay(u32 ulCount)
{
    __asm__ __volatile__(
    "mov r1,%0\n"
    "_SysCtlDelay:\n"
    "subs r1,#1\n"
    "bne _SysCtlDelay\n"
    :
    :"r"(ulCount)
    :"r1"
    );
}

void delay_us(u32 nus)
{		
	SysCtlDelay(12*nus);
}

void delay_ms(u16 nms)
{	 		  
    delay_us(1000*nms);
} 









































