//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include <mkrtos/sys.h>
#include <mkrtos/task.h>
//svc调用传值说明
//最大7个参数，都在寄存中进行传递
uint32_t svcHandler(uint32_t* pwdSF,uint32_t call_num) {
    uint8_t svc_number;
    uint32_t svc_r0;
    uint32_t svc_r1;
    uint32_t svc_r2;
    uint32_t svc_r3;
    uint32_t svc_r4;
    uint32_t svc_r5;
    uint32_t *psF;
    psF = pwdSF+8 ;
    svc_number = ((char *) psF[6])[-2];
    svc_r0 = ((uint32_t) psF[0]);
    svc_r1 = ((uint32_t) psF[1]);
    svc_r2 = ((uint32_t) psF[2]);
    svc_r3 = ((uint32_t) psF[3]);
    svc_r4 = ((uint32_t) psF[-8]);
    svc_r5 = ((uint32_t) psF[-7]);

    if (svc_number != 0x80) {
        psF[0] = -1;
        return 0;
    }

    if(sys_call_table[call_num]) {
        switch (call_num) {
            case 119://sys_sigreturn
            case 173:
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3)) sys_call_table[call_num])((int) (pwdSF + 8),
                                                                                                      (int) svc_r0,
                                                                                                      (int) svc_r1,
                                                                                                      (int) svc_r2);
                break;
            case 2:
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3)) sys_call_table[call_num])((int) (pwdSF),
                                                                                                      (int) svc_r0,
                                                                                                      (int) svc_r1,
                                                                                                      (int) svc_r2);
                break;
            case 117:
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3, int arg4)) sys_call_table[call_num])(
                        (int) (svc_r0),
                        (int) svc_r1,
                        (int) svc_r2,
                        (int) svc_r3,
                        (int) svc_r4
                );
                break;
            case 72:
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3)) sys_call_table[call_num])((int) (pwdSF+8),
                                                                                                      (int) svc_r0,
                                                                                                      (int) svc_r1,
                                                                                                      (int) svc_r2);
                break;
            case 179:
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3)) sys_call_table[call_num])((int) (pwdSF+8),
                                                                                                      (int) svc_r0,
                                                                                                      (int) svc_r1,
                                                                                                      (int) svc_r2);
                break;
            case 90:
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3, int arg4,
                                   int arg5)) sys_call_table[call_num])((int) (svc_r0),
                                                                        (int) svc_r1,
                                                                        (int) svc_r2,
                                                                        (int) svc_r3,
                                                                        (int) svc_r4,
                                                                        (int) svc_r5
                );
                break;
            default:
                if(svc_r0==128){
                    printk("fd is error.\n");
                }
                psF[0] = ((int (*)(int arg0, int arg1, int arg2, int arg3)) sys_call_table[call_num])((int) (svc_r0),
                                                                                                      (int) svc_r1,
                                                                                                      (int) svc_r2,
                                                                                                      (int) svc_r3);
        }
//        printk("sys remain mem %d\r\n",GetFreeMemory(1));
    }else{
        psF[0]=-1;
    }
    extern void do_signal_isr(void* sp);
    do_signal_isr(pwdSF+8);

    return 0;
}



