//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <unistd.h>
#include <mkrtos/task.h>
#include "arch/arch.h"
#include "stdlib.h"
#include <mkrtos/mem.h>
#include <loader.h>
//signal.c
extern void sig_chld(void);
//shced.c
extern void update_cur_task(void);
extern void task_sche(void);
//fs.h
extern void sys_close(int fp);

//半关闭
void pre_exit(void){
//关闭所有的文件
    for( int i=0;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used){
            sys_close(i);
        }
    }
    mem_clear();
    do_remove_sleep_tim(CUR_TASK);
}
/**
* @brief 在系统中删除当前执行的任务，该删除只是设置为僵尸进程
*/
void DoExit(int32_t exitCode){
    //关所有中断
    uint32_t t;
#if 0
    //关闭打开的文件，并释放程序相关信息
    TaskUserInfoDestory(ptb);
#endif
    pre_exit();
    t=DisCpuInter();
    //当前进程结束了，应该吧当前进程的子进程全部移交给初始化进程
    struct task* tmp=sysTasks.allTaskList;
    while(tmp){
        if(tmp->parentTask==CUR_TASK){
            tmp->parentTask=sysTasks.init_task;
        }
        tmp=tmp->nextAll;
    }
    //发送chld给父进程
    sig_chld();
    //唤醒等待这个队列关闭的
    wake_up(CUR_TASK->close_wait);
    //设置任务已经关闭了
    CUR_TASK->status=TASK_CLOSED;
    sysTasks.currentMaxTaskNode->taskReadyCount--;
    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
        //任务更新
        update_cur_task();
    }
    unload_elf(CUR_TASK->exec);
    CUR_TASK->exec=NULL;
    //释放栈空间
    OSFree(CUR_TASK->memLowStack);
    CUR_TASK->memLowStack=NULL;
    task_sche();
    RestoreCpuInter(t);

}
/**
* @brief 任务结束时会调用该函数，任务执行结束，在这里销毁这个任务
*/
void TaskToEnd(int32_t exitCode){
//    printk("exit %d\n",exitCode);
    /*这里需要通过系统调用，这个函数是用户层调用的*/
    exit(exitCode);
    /*for(;;);*/
}

int sys_exit(int exitCode){
    /*删除当前任务*/
    inner_set_sig(SIGKILL);
    return 0;
}
