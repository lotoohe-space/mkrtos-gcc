//
// Created by zz on 2022/2/13.
//

#include <mkrtos/task.h>
#include <arch/arch.h>
#include "mkrtos/mem.h"
#include <sys/wait.h>
/**
 * �������
 * @param wake_tasks
 */
void clear_task_q(struct task **wake_tasks){
    uint32_t t;
    t=DisCpuInter();
    while(*wake_tasks){
        *wake_tasks=(*wake_tasks)->del_wait;
    }
    RestoreCpuInter(t);
}
void add_wait_task_q(struct task** queue,struct task* add_queue){
    uint32_t t;
    t=DisCpuInter();
    if(*queue==NULL){
        *queue=add_task;
    }else{
        add_queue->del_wait = (*queue);
        *queue=add_queue;
    }
    RestoreCpuInter(t);
}
/**
 * �Ƴ��ȴ�������
 * @param queue ��Ҫ�Ƴ����������
 * @param tk �Ƴ�������
 * @param max_prio ���صȴ������������ȼ�������ΪNULL�򲻷���
 * @return
 */
//�Ƴ�һ���ȴ���
void remove_wait_task_q(struct task ** queue,struct task* add_queue){
    struct task *temp=*queue;
    struct task *prev=NULL;
    uint32_t t;
    if(!add_queue){
        return ;
    }
    t=DisCpuInter();
    while(temp){
        if(temp==add_queue) {
            if (prev==NULL) {
                //ɾ���ĵ�һ��
                *queue=temp->del_wait;
                break;
            }else{
                prev->del_wait=temp->del_wait;
                break;
            }
        }
        prev=temp;
        temp=temp->del_wait;
    }
    RestoreCpuInter(t);
}

//pid_t waitpid(pid_t pid,int *status,int options);
//�ɹ����������������ӽ���ID��ʧ�ܣ�-1�����ӽ��̣�
//��������ͷ��������
//����pid��
//       >0 ����ָ��ID���ӽ���
//       -1 ���������ӽ��̣��൱��wait��
//       0 ���պ͵�ǰ����waitpidһ����������ӽ���
//       < -1 ����ָ���������ڵ������ӽ���
//����0������3ΪWNOHANG�����ӽ�����������
static void wait_task(struct wait_queue **wait_c){
    uint32_t t;
    RestoreCpuInter(t);
    struct wait_queue wait={CUR_TASK,NULL};
    //���������̻�û�н���ر�״̬����ȴ��ر��������
    add_wait_queue(wait_c,&wait);
    CUR_TASK->status=TASK_SUSPEND;
    task_sche();
    remove_wait_queue(wait_c,&wait);
    CUR_TASK->status=TASK_RUNNING;
}
/**
 * �ȴ�pid
 * @param pid
 * @param statloc
 * @param options
 * @return
 */
pid_t waitpid(pid_t pid,int32_t *statloc,int32_t options){
    uint32_t t;
    struct task *ls;
    struct task *close_task=0;
    uint32_t child_run_cn=0;
    uint32_t child_all_cn=0;
    again:
    child_run_cn=0;
    child_all_cn=0;
    t=DisCpuInter();
    ls=sysTasks.allTaskList;
    while(ls){
        if(ls->parentTask== CUR_TASK) {

            if (pid == -1) {
                if (ls->status == TASK_CLOSED) {
                    int32_t res_pid;
                    del_task(NULL, ls);
                    del_task(&sysTasks.allTaskList, ls);
                    res_pid = ls->PID;
                    OSFree(ls);
                    RestoreCpuInter(t);
                    return res_pid;
                } else {
                    //�����ӵ�һ�������У�Ȼ���ڽ�β�������ж�
                    add_wait_task_q(&close_task, ls);
                    //�����е��ӽ�������
                    child_run_cn++;
                }
                //���е��ӽ�������
                child_all_cn++;

            } else if (pid > 0) {
                if (ls->PID == pid) {
                    if (ls->status == TASK_CLOSED) {
                        int32_t res_pid;
                        del_task(NULL, ls);
                        del_task(&sysTasks.allTaskList, ls);
                        res_pid = ls->PID;
                        OSFree(res_pid);
                        RestoreCpuInter(t);
                        return res_pid;
                    } else {
                        if(options&WNOHANG ){
                            //������
                            return 0;
                        }else {
                            wait_task(&ls->close_wait);
                            goto again;
                        }
                    }
                }
            } else if (pid == 0) {

            } else if (pid < -1) {

            }
        }
        ls=ls->nextAll;
    }
    RestoreCpuInter(t);
    if(pid==-1){
        if(child_all_cn==0){
            //û���ӽ���
            return -1;
        }else {
            //�ӽ�����������δ�رյĽ���������ôӦ�õȴ�
            if (child_all_cn == child_run_cn) {
                if(options&WNOHANG ){
                    //������
                    clear_task_q(&close_task);
                    return 0;
                }
                pid_t pid=-1;
                //�ȴ�����
                struct wait_queue wait_c={CUR_TASK,NULL};
                struct task *tmp;
                tmp=close_task;
                while(tmp){
                    //���ӵ��ȴ�����
                    add_wait_queue(&tmp->close_wait,&wait_c);
                    tmp=tmp->del_wait;
                }

                //��ʼ�ȴ�
                CUR_TASK->status=TASK_SUSPEND;
                task_sche();
                tmp=close_task;
                while(tmp){//�Ƴ����еĵȴ�����
                    if(tmp->status==TASK_CLOSED){
                        pid=tmp->PID;
                    }
                    remove_wait_queue(&tmp->close_wait,&wait_c);
                    tmp=tmp->del_wait;
                }
                //�������
                clear_task_q(&close_task);
                CUR_TASK->status=TASK_RUNNING;
                return pid;
            }
        }
    }

    return -1;
}