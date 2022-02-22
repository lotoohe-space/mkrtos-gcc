//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <unistd.h>
#include <mkrtos/task.h>
#include "arch/arch.h"
#include "stdlib.h"
#include <mkrtos/mem.h>

//signal.c
extern void sig_chld(void);
//shced.c
extern void update_cur_task(void);
extern void task_sche(void);
//fs.h
extern void sys_close(int fp);
/**
* @brief ��ϵͳ��ɾ����ǰִ�е����񣬸�ɾ��ֻ������Ϊ��ʬ����
*/
void DoExit(int32_t exitCode){
    //�������ж�
    uint32_t t;
//    t=DisCpuInter();
//    //�������ڹرգ��൱�ڽ�ʬ���̣������ڲ������
//    CUR_TASK->status=TASK_CLOSING;
//    sysTasks.currentMaxTaskNode->taskReadyCount--;
//    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
//        //�������
//        update_cur_task();
//    }
//    CUR_TASK->exitCode=exitCode;
//    CUR_TASK->status=TASK_RUNNING;
//    RestoreCpuInter(t);
#if 0
    //�رմ򿪵��ļ������ͷų��������Ϣ
    TaskUserInfoDestory(ptb);
#endif
    //�ر����е��ļ�
    for( int i=3;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used){
            sys_close(i);
        }
    }
    mem_clear();
    t=DisCpuInter();
    //����chld��������
    sig_chld();
    //�ͷ�ջ�ռ�
    OSFree(CUR_TASK->memLowStack);
    //���������Ѿ��ر���
    CUR_TASK->status=TASK_CLOSED;
    //���ѵȴ�������йرյ�
    wake_up(CUR_TASK->close_wait);

    sysTasks.currentMaxTaskNode->taskReadyCount--;
    if(sysTasks.currentMaxTaskNode->taskReadyCount==0){
        //�������
        update_cur_task();
    }

    /*���̽����������*/
    task_sche();
    RestoreCpuInter(t);

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
