//
// Created by Administrator on 2022/2/27.
//
#include <sys/types.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
//
struct sleep_time_queue{
    struct task* task;
    struct sleep_time_queue *next;
    uint32_t slp_ms;
    uint32_t cur_ms;
};
static struct sleep_time_queue *slp_tim_ls=NULL;
//struct timespec {
//    time_t tv_sec;	/* seconds */
//    long tv_nsec;		/* nanoseconds */
//};
static void add_sleep(struct sleep_time_queue **queue,struct sleep_time_queue *add){
    if(!queue){
        return ;
    }
    uint32_t t;
    t=DisCpuInter();
    if(!(*queue)){
        *queue=add;
    }else{
        add->next=*queue;
        *queue=add;
    }
    RestoreCpuInter(t);
}
static void remove_sleep(struct sleep_time_queue **queue,struct sleep_time_queue *rm){
    struct sleep_time_queue *temp;
    struct sleep_time_queue *prev=NULL;
    if(!queue){
        return ;
    }
    uint32_t t;
    t=DisCpuInter();
    temp=*queue;
    while(temp){
        if(temp==rm) {
            if (prev==NULL) {
                //ɾ���ĵ�һ��
                (*queue) = temp->next;
                break;
            }else{
                prev->next = temp->next;
                break;
            }
        }
        prev=temp;
        temp = temp->next;
    }
    RestoreCpuInter(t);
}
//ɾ��ĳ������ģ���exit.c�е���
void do_remove_sleep_tim(struct task* tk) {
    struct sleep_time_queue *temp;
    struct sleep_time_queue *prev=NULL;
    uint32_t t;
    t=DisCpuInter();
    temp=slp_tim_ls;
    while(temp){
        if(temp->task ==tk) {
            if (prev==NULL) {
                //ɾ���ĵ�һ��
                (slp_tim_ls) = temp->next;
                break;
            }else{
                prev->next = temp->next;
                break;
            }
        }
        prev=temp;
        temp = temp->next;
    }
    RestoreCpuInter(t);
}

//���tim�Ƿ�ʱ���ˣ�������ˣ�����ָ���Ľ���
//��������ڵ���ʱ��������е���
void do_check_sleep_tim(void){
    uint32_t t;
    struct sleep_time_queue *tmp;
    t=DisCpuInter();
    tmp=slp_tim_ls;
    while(tmp){
        tmp->cur_ms+=OS_WORK_PERIOD_MS;
        if(tmp->cur_ms>=tmp->slp_ms){
            task_run_1(tmp->task);
        }
        tmp=tmp->next;
    }
    RestoreCpuInter(t);
}
//��������req��ʱ�䣬������ź��жϣ���ʣ���ʱ��ŵ�rem�в�����-1
int sys_nanosleep(const struct timespec *req, struct timespec *rem){
    if(!req){
        return -EINVAL;
    }
    if(req->tv_nsec==0 && req->tv_sec==0){
        if(!rem){
            rem->tv_sec=0;
            rem->tv_nsec=0;
        }
        return 0;
    }
#define ROUND(a,b) (((a)/(b))+(((a)%(b))?1:0))
    struct sleep_time_queue stq={
            .next=NULL,
            .task=CUR_TASK,
            //���滻���ms������ȡ��
            .slp_ms=req->tv_sec*1000+ ROUND(ROUND(req->tv_nsec,1000),1000),
            .cur_ms=0
    };
    add_sleep(&slp_tim_ls,&stq);
    task_suspend();
    task_sche();
//    task_run();
    remove_sleep(&slp_tim_ls,&stq);
    if(CUR_TASK->sig_bmp){
        if(!rem){
            uint32_t rems;
            rems=stq.slp_ms-stq.cur_ms;
            rem->tv_nsec=(rems%1000)*1000*1000;
            rem->tv_sec=rems/1000;
        }
        return -1;
    }
    return 0;
}
