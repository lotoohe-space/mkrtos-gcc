#ifndef _ISR_H__
#define _ISR_H__

#include "type.h"

#define ISR_START_INX 15
//中断从第16开始，前面是系统的异常
typedef void (*IsrCb)(void);
typedef struct IsrFunc{
	//中断的回调函数
	IsrCb isrFunc;
	//0bit:isr在属于内核还是用户
	uint8_t isrFlag;
}*PIsrFunc,IsrFunc;

//注册中断函数
void RegIsrFunc(IsrCb isrCb,uint32_t isrNum,uint8_t inKernel);
//取消注册中断函数
void UnregIsrFunc(uint32_t isrNum);

#endif
