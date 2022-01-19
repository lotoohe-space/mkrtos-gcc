//
// Created by Administrator on 2022/1/9.
//
#include <type.h>
#include <errno.h>
#include <string.h>
#include <arch/arch.h>
#include <mkrtos/mem.h>
#include <mkrtos/task.h>

/**
 * @brief 创建任务
 * @param tcp 任务创建操作
 * @param progInfo 任务的代码信息
 * @return
 */
int32_t task_create(PTaskCreatePar tcp,void* progInfo){
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
        errno=ERROR;
        return -1;
    }
    void *memStack = NULL;
    PTaskBlock pTaskBlock=NULL;
    pTaskBlock=(PTaskBlock)OSMalloc(sizeof(TaskBlock));
    if(pTaskBlock == NULL){
        errno=ENOMEM;
        return -1;
    }
    memset(pTaskBlock,0,sizeof(TaskBlock));
    pTaskBlock->taskName = taskName;

    /*申请堆栈的内存*/
    memStack = (void *)OSMalloc(sizeof(uint32_t)*(userStackSize+kernelSize));
    if(memStack == NULL){
        OSFree(pTaskBlock);
        errno=ENOMEM;
        return -1;
    }

    /*初始化任务控制表*/
    pTaskBlock->runCount=0;
    pTaskBlock->delayCount=0;
    pTaskBlock->PID = (int16_t)atomic_read(&sysTasks.pidTemp);
    pTaskBlock->memLowStack=memStack;
    if(userStackSize!=0){
        pTaskBlock->skInfo.pspStack=(void*)(&(((uint32_t*)memStack)[userStackSize+kernelSize-1]));
    }else{
        pTaskBlock->skInfo.pspStack=(void*)(~(0L));
    }
    pTaskBlock->skInfo.mspStack=(void*)(&(((uint32_t*)memStack)[kernelSize-1]));
    pTaskBlock->skInfo.stackType=1;
    pTaskBlock->prio=prio;
    pTaskBlock->userStackSize=userStackSize;
    pTaskBlock->kernelStackSize=kernelSize;
    pTaskBlock->status=TASK_SUSPEND;

    pTaskBlock->next=NULL;
    pTaskBlock->nextBk=NULL;
    pTaskBlock->nextAll=NULL;
    if(userStackSize!=0){
        /*设置栈的初始化寄存器*/
        pTaskBlock->skInfo.pspStack=
                OSTaskSetReg(pTaskBlock->skInfo.pspStack,taskFun,arg0,arg1);
    }
    pTaskBlock->skInfo.mspStack=
            OSTaskSetReg(pTaskBlock->skInfo.mspStack,taskFun,arg0,arg1);

    if(userStackSize==0){
        //线程在内核模式;
        pTaskBlock->skInfo.stackType = 2;
    }else{
        pTaskBlock->skInfo.stackType=1;
    }

    /*通过优先级添加任务*/
    int32_t err = AddTask(pTaskBlock);
    if(err != 0){
        //	RestoreCpuInter(t);
        /*释放申请的内存*/
        OSFree(pTaskBlock);
        OSFree(memStack);
        return -1;
    }
#if 0
    //设置程序信息
    pTaskBlock->taskInfo.peam=progInfo;
    if( TaskUserInfoCreate(pTaskBlock) <0 ){
        TaskAllLinksDel(pTaskBlock);
        TaskPrioLinksDel(pTaskBlock);
        //	RestoreCpuInter(t);
        OSFree(pTaskBlock);
        OSFree(memStack);
        err= ERROR;
        return -1;
    }

    TaskUserFileInit(pTaskBlock->PID);
#else
    for(int32_t i=0;i<NR_FILE;i++){
        pTaskBlock->files[i].used=0;
    }
    root_mount(pTaskBlock);
#endif

    atomic_inc(&sysTasks.pidTemp);

    //最后设置为运行模式，之前都是挂起的
    uint32_t t=DisCpuInter();
    pTaskBlock->status=TASK_RUNNING;
    RestoreCpuInter(t);
    err= ERROR;

    return pTaskBlock->PID;
}