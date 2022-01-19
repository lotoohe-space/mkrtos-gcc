//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include <arch/arch.h>
extern void Init(void);
extern void SchedInit(void);
int32_t sys_setup(void){
    SchedInit();
    ArchInit();
    Init();
    return 0;
}
