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
//pipe.c
void do_fork_pipe(struct inode *inode,struct task* new_task,int fd);
//����һ���ӽ���
int32_t sys_fork(uint32_t *psp){
    TaskStatus oldStatus;
    uint32_t t=DisCpuInter();
    PTaskBlock ptb=CUR_TASK;
    PTaskBlock newPtb=OSMalloc(sizeof(TaskBlock));
    if(newPtb==NULL){
        RestoreCpuInter(t);
        return -1;
    }
    memcpy(newPtb,ptb,sizeof(TaskBlock));
    newPtb->status=TASK_SUSPEND;
    newPtb->runCount=0;
    newPtb->next=NULL;
    newPtb->nextAll=NULL;
    newPtb->memLowStack=(void *)OSMalloc(sizeof(uint32_t)*(newPtb->userStackSize+newPtb->kernelStackSize));
    if(newPtb->memLowStack==NULL){
        OSFree(newPtb);
        RestoreCpuInter(t);
        return -1;
    }
    newPtb->PID = (pid_t)atomic_read(&sysTasks.pidTemp);
#if 1
    if(ptb->exec){
        void* exec_tmp=newPtb->exec;
        //���¸���Ӧ����Ϣ
        newPtb->exec=OSMalloc(sizeof(ELFExec_t));
        if(!newPtb->exec){
            OSFree(newPtb);
            OSFree(newPtb->memLowStack);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec,exec_tmp,sizeof(ELFExec_t));
        newPtb->exec->data.data = OSMalloc(ptb->exec->data.sh_size);
        if(!newPtb->exec->data.data){
            OSFree(newPtb);
            OSFree(ptb->exec);
            OSFree(newPtb->memLowStack);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec->data.data,ptb->exec->data.data,ptb->exec->data.sh_size);
        newPtb->exec->bss.data = OSMalloc(ptb->exec->bss.sh_size);
        if(!newPtb->exec->bss.data){
            OSFree(newPtb);
            OSFree(ptb->exec);
            OSFree(newPtb->memLowStack);
            OSFree(newPtb->exec->data.data);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec->bss.data,ptb->exec->bss.data,ptb->exec->bss.sh_size);
        (*(ptb->exec->used_count))++;
    }
#endif
    int32_t err;
    newPtb->parent=NULL;
    /*ͨ�����ȼ��������*/
    err = add_task(newPtb);
    if(err != 0){
        //	RestoreCpuInter(t);
        /*�ͷ�������ڴ�*/
        OSFree(newPtb);
        OSFree(ptb->exec);
        OSFree(newPtb->memLowStack);
        OSFree(newPtb->exec->data.data);
        OSFree(newPtb->exec->bss.data);
        RestoreCpuInter(t);
        return -1;
    }

    atomic_inc(&sysTasks.pidTemp);
    //���ø�����
    newPtb->parentTask=ptb;
    //����ջ
    memcpy(newPtb->memLowStack,ptb->memLowStack,sizeof(uint32_t)*(newPtb->userStackSize+newPtb->kernelStackSize));

    //����ջλ��
    newPtb->skInfo.mspStack=(void*)(&(((uint32_t*)newPtb->memLowStack)[ptb->kernelStackSize-1]));
    if(newPtb->userStackSize!=0){
        newPtb->skInfo.pspStack=(void*)((uint32_t)(newPtb->memLowStack)+((uint32_t)(psp)- (uint32_t)(ptb->memLowStack)));
        ((uint32_t*)(newPtb->skInfo.pspStack))[8]=0;
    }else{
        newPtb->skInfo.pspStack=(void*)(~(0L));
    }
    //����Ϊ�û�ģʽ
    newPtb->skInfo.svcStatus=0;
    newPtb->skInfo.stackType=1;

    //���ڴ򿪵��ļ�������Ӧ������inode�����ý���+1�Ĳ�����
    for(int i=0;i<NR_FILE;i++){
        if(newPtb->files[i].used){
            struct inode *tmp;
            tmp=newPtb->files[i].f_inode;
            if(tmp){
                if(i<3){
                    tmp->i_ops->default_file_ops->open(tmp,&newPtb->files[i]);
                }
                //��pipe����fork
                do_fork_pipe(tmp,newPtb,i);
                atomic_inc(&(tmp->i_used_count));
            }
        }
    }
    newPtb->del_wait=NULL;
    newPtb->close_wait=NULL;
    newPtb->sig_bmp=0;
    for(int i=0;i<_NSIG;i++) {
        newPtb->signals[i]._u._sa_handler=SIG_DFL;
    }
    //mem��Ϣ������fork
    newPtb->mems=NULL;
    newPtb->status=TASK_RUNNING;
    RestoreCpuInter(t);

    //����pid
    return newPtb->PID;
}