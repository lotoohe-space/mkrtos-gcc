//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include <mkrtos/sys.h>
#include <mkrtos/task.h>
//svc调用传值说明
//第一个参数为调用号
//第二-四个参数为系统的参数
//更多的参数放到栈中传递
uint32_t svcHandler(uint32_t* pwdSF,uint32_t call_num) {
    static uint8_t flag = 0;
    uint8_t svc_number;
    uint32_t svc_r0;
    uint32_t svc_r1;
    uint32_t svc_r2;
    uint32_t svc_r3;
    int32_t retVal; //用于存储返回值
    uint32_t *psF;
    psF = pwdSF+8 ;
    svc_number = ((char *) psF[6])[-2];
    svc_r0 = ((uint32_t) psF[0]);
    svc_r1 = ((uint32_t) psF[1]);
    svc_r2 = ((uint32_t) psF[2]);
    svc_r3 = ((uint32_t) psF[3]);

    if (svc_number != 0x80) {
        psF[0] = -1;
        return 0;
    }

    extern int SysCall(void *callNo,void*arg1,void *arg2,void*arg3);
    if(call_num==119||call_num==173){//sys_sigreturn
        psF[0]=((int(*)(int arg0,int arg1,int arg2,int arg3))sys_call_table[call_num])((int)(pwdSF+8),(int)svc_r0,(int)svc_r1,(int)svc_r2);
    }else if(call_num==2) {//fork
        psF[0]=((int(*)(int arg0,int arg1,int arg2,int arg3))sys_call_table[call_num])((int)(pwdSF),(int)svc_r0,(int)svc_r1,(int)svc_r2);
    }else{
        if(sys_call_table[call_num]) {
            psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3)) sys_call_table[call_num])((int) (svc_r0),
                                                                                                  (int) svc_r1,
                                                                                                  (int) svc_r2,
                                                                                                  (int) svc_r3);
        }else {//没有实现的直接返回-1
            psF[0]=-1;
        }
    }
    extern void do_signal_isr(void* sp);
    do_signal_isr(pwdSF+8);

    return 0;
}



