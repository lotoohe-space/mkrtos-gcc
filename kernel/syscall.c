//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include <mkrtos/sys.h>
#include <mkrtos/task.h>

uint32_t svcHandler(uint32_t* pwdSF) {
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
    if(svc_r0==119){//sys_sigreturn
        psF[0]=SysCall((void*)(svc_r0<<2),(void*)(pwdSF+8),(void*)svc_r2,(void*)svc_r3);
    }else if(svc_r0==2) {//fork
        psF[0]=SysCall((void*)(svc_r0<<2),(void*)(pwdSF),(void*)svc_r2,(void*)svc_r3);
    }else{
        psF[0]=SysCall((void*)(svc_r0<<2),(void*)svc_r1,(void*)svc_r2,(void*)svc_r3);
    }

    extern void do_signal_isr(void* sp);
    do_signal_isr(pwdSF+8);

    return 0;
}



