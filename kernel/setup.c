//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include "arch/arch.h"
#include "bsp/net_init.h"
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
int32_t sys_setup(void){
    extern void fs_init(void);
    root_mount(CUR_TASK);
#if 1
    fs_init();
#endif

//    read_user_cfg();
    devs_init();
    lwip_comm_init();
#include "net/lwiperf_interface.h"
    lwiperf_init();
    if(sp_mkfs(1,128)<0){
        fatalk("根文件系统创建失败！\r\n");
    }
    return 0;
}
int32_t arch_init(void){
    extern void Init(void);
    extern void SchedInit(void);
    extern int32_t BSPInit(void);
    BSPInit();
    SchedInit();
    ArchInit();
    Init();
    return 0;
}
