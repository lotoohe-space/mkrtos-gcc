//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <unistd.h>
#include <mkrtos/task.h>
#include "arch/arch.h"
#include "stdlib.h"
#include "sys/wait.h"
#include <mkrtos/mem.h>

//extern PTaskBlock find_task(int32_t PID);
extern void update_cur_task(void);
extern void task_sche(void);
extern void sys_close(int fp);
/**
* @brief 在系统中删除当前执行的任务，该删除只是设置为僵尸进程
*/
void DoExit(int32_t exitCode){
    //关所有中断
    uint32_t t=DisCpuInter();
    //任务正在关闭，相当于僵尸进程，任务不在参与调度
    CUR_TASK->status=TASK_CLOSING;
    sysTasks.currentMaxTaskNode->taskReadyCount--;
    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
        //任务更新
        update_cur_task();
    }
    CUR_TASK->exitCode=exitCode;
    RestoreCpuInter(t);
    //释放栈空间
    OSFree(CUR_TASK->memLowStack);

#if 0
    //关闭打开的文件，并释放程序相关信息
    TaskUserInfoDestory(ptb);
#endif
    //关闭所有的文件
    for( int i=0;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used){
            sys_close(i);
        }
    }
    mem_clear();
    t=DisCpuInter();
    //设置任务已经关闭了
    CUR_TASK->status=TASK_CLOSED;
    //唤醒等待这个队列关闭的
    wake_up(CUR_TASK->close_wait);
    RestoreCpuInter(t);
    /*立刻进行任务调度*/
    task_sche();
}
/**
* @brief 任务结束时会调用该函数，任务执行结束，在这里销毁这个任务
*/
void TaskToEnd(int32_t exitCode){
    /*这里需要通过系统调用，这个函数是用户层调用的*/
    exit(exitCode);
    /*for(;;);*/
}

int sys_exit(int exitCode){
    /*删除当前任务*/
    DoExit(exitCode);
    return 0;
}
