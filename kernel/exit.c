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
* @brief ��ϵͳ��ɾ����ǰִ�е����񣬸�ɾ��ֻ������Ϊ��ʬ����
*/
void DoExit(int32_t exitCode){
    //�������ж�
    uint32_t t=DisCpuInter();
    //�������ڹرգ��൱�ڽ�ʬ���̣������ڲ������
    CUR_TASK->status=TASK_CLOSING;
    sysTasks.currentMaxTaskNode->taskReadyCount--;
    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
        //�������
        update_cur_task();
    }
    CUR_TASK->exitCode=exitCode;
    RestoreCpuInter(t);
    //�ͷ�ջ�ռ�
    OSFree(CUR_TASK->memLowStack);

#if 0
    //�رմ򿪵��ļ������ͷų��������Ϣ
    TaskUserInfoDestory(ptb);
#endif
    //�ر����е��ļ�
    for( int i=0;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used){
            sys_close(i);
        }
    }
    mem_clear();
    t=DisCpuInter();
    //���������Ѿ��ر���
    CUR_TASK->status=TASK_CLOSED;
    //���ѵȴ�������йرյ�
    wake_up(CUR_TASK->close_wait);
    RestoreCpuInter(t);
    /*���̽����������*/
    task_sche();
}
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
    DoExit(exitCode);
    return 0;
}
