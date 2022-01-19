//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <unistd.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
#include <mkrtos/mem.h>

extern PTaskBlock find_task(int32_t PID);
extern void update_cur_task(void);
extern void task_sche(void);
/**
* @brief ��ϵͳ��ɾ����ǰִ�е����񣬸�ɾ��ֻ������Ϊ��ʬ����
*/
void DoExit(int16_t pid,int32_t exitCode){
    PTaskBlock ptb;
    //�������ж�
    uint32_t t=DisCpuInter();
    ptb=find_task(pid);
    if(ptb==NULL){
        ptb=sysTasks.currentTask;
    }
    //�������ڹرգ��൱�ڽ�ʬ���̣������ڲ������
    ptb->status=TASK_CLOSING;
    sysTasks.currentMaxTaskNode->taskReadyCount--;
    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
        //�������
        update_cur_task();
    }
    ptb->exitCode=exitCode;
    RestoreCpuInter(t);
    //�ͷ�ջ�ռ�
    OSFree(ptb->memLowStack);

#if 0
    //�رմ򿪵��ļ������ͷų��������Ϣ
    TaskUserInfoDestory(ptb);
#endif

    t=DisCpuInter();
    ptb->status=TASK_CLOSED;
    RestoreCpuInter(t);
    /*���̽����������*/
    task_sche();
}
static inline _syscall1(int,exit,int32_t,exitCode);
/**
* @brief �������ʱ����øú���������ִ�н����������������������
*/
void TaskToEnd(int32_t exitCode){
    /*������Ҫͨ��ϵͳ���ã�����������û�����õ�*/
    exit(exitCode);
    /*for(;;);*/
}

int sys_exit(int exitCode){
    /*ɾ����ǰ����*/
    DoExit(-1,exitCode);
    return 0;
}
