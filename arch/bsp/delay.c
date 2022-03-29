#include "delay.h"

extern void SysCtlDelay(u32 ulCount);

void delay_us(u32 nus)
{		
	SysCtlDelay(12*nus);
}

void delay_ms(u16 nms)
{	 		  
    delay_us(1000*nms);
} 









































