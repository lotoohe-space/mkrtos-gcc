//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include <errno.h>
#include <string.h>
#include "arch/arch.h"
#include <mkrtos/mem.h>
#include <mkrtos/task.h>
#include "fcntl.h"

/**
 * @brief 创建任务
 * @param tcp 任务创建操作
 * @param progInfo 任务的代码信息
 * @return
 */
int32_t task_create(PTaskCreatePar tcp){
    TaskFun taskFun;
    void *arg0;
    void *arg1;
    uint8_t prio;
    uint16_t userStackSize;
    uint16_t kernelSize;
    const char* taskName;

    if(tcp == NULL){
        return -1;
    }

    taskFun=tcp->taskFun;
    arg0=tcp->arg0;
    arg1=tcp->arg1;
    prio=tcp->prio;
    userStackSize=tcp->userStackSize;
    kernelSize=tcp->kernelStackSize;
    taskName=tcp->taskName;

    if(taskFun == NULL){
        errno=EINVAL;
        return -1;
    }
    void *memStack = NULL;
    void *knlMemStack = NULL;
    PTaskBlock pTaskBlock=NULL;
    pTaskBlock=(PTaskBlock)OSMalloc(sizeof(TaskBlock));
    if(pTaskBlock == NULL){
        errno=ENOMEM;
        return -1;
    }
    memset(pTaskBlock,0,sizeof(TaskBlock));
    pTaskBlock->taskName = taskName;

    /*申请堆栈的内存*/
    memStack = (void *)OSMalloc(sizeof(uint32_t)*(userStackSize));
    if(memStack == NULL){
        OSFree(pTaskBlock);
        errno=ENOMEM;
        return -1;
    }
    knlMemStack= (void *)OSMalloc(sizeof(uint32_t)*(kernelSize));
    if(knlMemStack == NULL){
        OSFree(pTaskBlock);
        OSFree(memStack);
        errno=ENOMEM;
        return -1;
    }
    /*初始化任务控制表*/
    pTaskBlock->runCount=0;
    pTaskBlock->PID = (pid_t)atomic_read(&sysTasks.pidTemp);
    pTaskBlock->tpid=-1;
    if(atomic_test(&(sysTasks.pidTemp),1)) {
        sysTasks.init_task=pTaskBlock;
    }

    pTaskBlock->memLowStack=memStack;
    if(userStackSize!=0){
        pTaskBlock->skInfo.pspStack=(void*)(&(((uint32_t*)memStack)[userStackSize-1]));
    }else{
        pTaskBlock->skInfo.pspStack=(void*)(~(0L));
    }
    pTaskBlock->knl_low_stack=knlMemStack;
    pTaskBlock->skInfo.mspStack=(void*)(&(((uint32_t*)knlMemStack)[kernelSize-1]));
    pTaskBlock->skInfo.stackType=1;
    pTaskBlock->prio=prio;
    pTaskBlock->userStackSize=userStackSize;
    pTaskBlock->kernelStackSize=kernelSize;
    pTaskBlock->status=TASK_SUSPEND;
    pTaskBlock->parentTask=pTaskBlock;
    pTaskBlock->is_s_user=1;
    pTaskBlock->next=NULL;
    strncpy(pTaskBlock->pwd_path,"/",sizeof(pTaskBlock->pwd_path));
//    pTaskBlock->nextBk=NULL;
    pTaskBlock->nextAll=NULL;
    if(userStackSize!=0){
        /*设置栈的初始化寄存器*/
        pTaskBlock->skInfo.pspStack=
                OSTaskSetReg(pTaskBlock->skInfo.pspStack,taskFun,arg0,arg1,0);
    }else{
        pTaskBlock->skInfo.mspStack=
                OSTaskSetReg(pTaskBlock->skInfo.mspStack,taskFun,arg0,arg1,0);
    }


    if(userStackSize==0){
        //线程在内核模式;
        pTaskBlock->skInfo.stackType = 2;
    }else{
        pTaskBlock->skInfo.stackType=1;
    }
    pTaskBlock->skInfo.svcStatus=0;
    /*通过优先级添加任务*/
    int32_t err = add_task(pTaskBlock,1);
    if(err != 0){
        //	RestoreCpuInter(t);
        /*释放申请的内存*/
        OSFree(pTaskBlock);
        OSFree(memStack);
        OSFree(knlMemStack);
        return -1;
    }

    for(int32_t i=0;i<NR_FILE;i++){
        pTaskBlock->files[i].used=0;
    }
#if 0

    root_mount(pTaskBlock);
    if(CUR_TASK) {
        //打开三个串口输出
        extern int32_t do_open(struct file *files, const char *path, int32_t flags, int32_t mode);
        do_open(pTaskBlock->files, "/dev/tty", O_RDWR, 0777);
        do_open(pTaskBlock->files, "/dev/tty", O_RDWR, 0777);
        do_open(pTaskBlock->files, "/dev/tty", O_RDWR, 0777);
    }
#endif


    atomic_inc(&sysTasks.pidTemp);
    //最后设置为运行模式，之前都是挂起的9
//    uint32_t t=DisCpuInter();
    pTaskBlock->status=TASK_RUNNING;
//    RestoreCpuInter(t);
    err= 0;

    return pTaskBlock->PID;
}