
#include "arch/isr.h"
#include "type.h"
#include "arch/arch.h"
#include "mkrtos/task.h"
//只能够在特权模式调用
//获取中断号
uint32_t GetISRNum(void){
    uint32_t num;
    __asm__ __volatile__(
        "mrs %0,IPSR"
        :"=r"(num)
        :
        :
    );
	return num;
}

//中断函数列表
IsrFunc isrFuncList[59]={NULL};


/**
 * @berief 所有的外部中断都到这里,
 * @param 传入的是栈指针
 */
void ExternInter(void* sp){
	uint32_t isrNum;
	isrNum = GetISRNum();
//	if(!(isrFuncList[isrNum-ISR_START_INX].isrFlag&0x1)){
		//内核中直接调用中断
    if(isrFuncList[isrNum-ISR_START_INX].isrFunc!=NULL){
        isrFuncList[isrNum-ISR_START_INX].isrFunc();
    }else{
        printk("unreg isr %d.\r\n",isrNum);
    }

//	}else{
//		用户线程调用中断
//	}
   // extern void do_signal_isr(void* sp);
   // do_signal_isr(sp);
}

//注册中断函数
void RegIsrFunc(IsrCb isrCb,uint32_t isrNum,uint8_t inKernel){
	uint32_t t;
	t=DisCpuInter();
	isrFuncList[isrNum].isrFunc=isrCb;
	isrFuncList[isrNum].isrFlag|=inKernel;
	RestoreCpuInter(t);
}
//取消注册中断函数
void UnregIsrFunc(uint32_t isrNum){
	uint32_t t;
	t=DisCpuInter();
	isrFuncList[isrNum].isrFunc=NULL;
	RestoreCpuInter(t);
}
