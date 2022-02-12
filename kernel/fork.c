//
// Created by Administrator on 2022/1/9.
//
#include <mkrtos/mem.h>
#include <type.h>
#include <mkrtos/task.h>
#include "arch/arch.h"
#include <string.h>

extern PTaskBlock find_task(int32_t PID);
extern int32_t add_task(PTaskBlock pTaskBlock);
//创建一个子进程
int32_t sys_fork(uint32_t *psp){
    TaskStatus oldStatus;
    uint32_t t=DisCpuInter();
    PTaskBlock ptb=find_task(-1);
    PTaskBlock newPtb=OSMalloc(sizeof(TaskBlock));
    if(newPtb==NULL){
        RestoreCpuInter(t);
        return -1;
    }
    memcpy(newPtb,ptb,sizeof(TaskBlock));
    newPtb->status=TASK_SUSPEND;
    newPtb->next=NULL;
    newPtb->nextAll=NULL;
    newPtb->memLowStack=(void *)OSMalloc(sizeof(uint32_t)*(newPtb->userStackSize+newPtb->kernelStackSize));
    if(newPtb->memLowStack==NULL){
        OSFree(newPtb);
        RestoreCpuInter(t);
        return -1;
    }
    newPtb->PID = (int16_t)atomic_read(&sysTasks.pidTemp);
#if 0
    if(ptb->taskInfo.peam!=NULL){
        newPtb->taskInfo.peam=EOS_load_item_new();
        if(newPtb->taskInfo.peam==NULL){
            OSFree(newPtb);
            OSFree(newPtb->memLowStack);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->taskInfo.peam,ptb->taskInfo.peam,sizeof(EOS_APP_MAN));
        memcpy(newPtb->taskInfo.peam->ram_data,ptb->taskInfo.peam->ram_data,ptb->taskInfo.peam->ram_size);
    }
    //复制文件描述符
    newPtb->taskInfo.fileList=(PFile)OSMalloc(sizeof(File)*FILE_MAX_NUM);
    if(newPtb->taskInfo.fileList == NULL){
        OSFree(newPtb);
        OSFree(newPtb->memLowStack);
        EOS_load_item_del(newPtb->taskInfo.peam);
        RestoreCpuInter(t);
        return -1;
    }
    memcpy(newPtb->taskInfo.fileList,ptb->taskInfo.fileList,sizeof(File)*FILE_MAX_NUM);
#endif
    int32_t err;

    /*通过优先级添加任务*/
    err = add_task(newPtb);
    if(err != 0){
        //	RestoreCpuInter(t);
        /*释放申请的内存*/
        OSFree(newPtb);
        OSFree(newPtb->memLowStack);
#if 0
        OSFree(newPtb->taskInfo.fileList);
        EOS_load_item_del(newPtb->taskInfo.peam);
#endif
        RestoreCpuInter(t);
        return -1;
    }
    atomic_inc(&sysTasks.pidTemp);
    //设置父进程
    newPtb->parentTask=ptb;
    //复制栈
    memcpy(newPtb->memLowStack,ptb->memLowStack,sizeof(uint32_t)*(newPtb->userStackSize+newPtb->kernelStackSize));

    //设置栈位置
    newPtb->skInfo.mspStack=(void*)((uint32_t)(newPtb->memLowStack)+((uint32_t)(ptb->skInfo.mspStack )- (uint32_t)(ptb->memLowStack)));
    if(newPtb->userStackSize!=0){
        newPtb->skInfo.pspStack=(void*)((uint32_t)(newPtb->memLowStack)+((uint32_t)(psp)- (uint32_t)(ptb->memLowStack)));
        ((uint32_t*)(newPtb->skInfo.pspStack))[8]=0;
    }else{
        newPtb->skInfo.pspStack=(void*)(~(0L));
    }

    newPtb->status=TASK_RUNNING;
    RestoreCpuInter(t);

    //返回pid
    return newPtb->PID;
}