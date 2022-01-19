//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <unistd.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
#include <mkrtos/mem.h>

extern PTaskBlock FindTask(int32_t PID);
extern void UpdateCurTask(void);
extern void TaskSche(void);
/**
* @brief ��ϵͳ��ɾ����ǰִ�е����񣬸�ɾ��ֻ������Ϊ��ʬ����
*/
void DoExit(int16_t pid,int32_t exitCode){
    PTaskBlock ptb;
    //�������ж�
    uint32_t t=DisCpuInter();
    ptb=FindTask(pid);
    if(ptb==NULL){
        ptb=sysTasks.currentTask;
    }
    //�������ڹرգ��൱�ڽ�ʬ���̣������ڲ������
    ptb->status=TASK_CLOSING;
    sysTasks.currentMaxTaskNode->taskReadyCount--;
    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
        //�������
        UpdateCurTask();
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
    TaskSche();
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
