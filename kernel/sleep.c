//
// Created by Administrator on 2022/2/27.
//
#include <sys/types.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
//

static struct sleep_time_queue *slp_tim_ls=NULL;

void add_sleep(struct sleep_time_queue **queue,struct sleep_time_queue *add){
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
void remove_sleep(struct sleep_time_queue **queue,struct sleep_time_queue *rm){
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
                //删除的第一个
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
//删除某个任务的，在exit.c中调用
void do_remove_sleep_tim(struct task* tk) {
    struct sleep_time_queue *temp;
    struct sleep_time_queue *prev=NULL;
    uint32_t t;
    t=DisCpuInter();
    temp=slp_tim_ls;
    while(temp){
        if(temp->task ==tk) {
            if (prev==NULL) {
                //删除的第一个
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
void wake_up_sleep(struct sleep_time_queue **temp){
    uint32_t t;
    struct sleep_time_queue *queue;
    t=DisCpuInter();
    queue=*temp;
    while(queue){
        if(queue->task){
            if(queue->task->status==TASK_SUSPEND
            ||queue->task->status==TASK_UNINTR
            ){
                task_run_1(queue->task);
//                queue->task->status=TASK_RUNNING;
            }
        }
        queue=queue->next;
    }
    RestoreCpuInter(t);
}
//检测tim是否到时间了，如果到了，则唤醒指定的进程
//这个函数在调度时钟里面进行调用
void _do_check_sleep_tim(struct sleep_time_queue **slp_ls){
    uint32_t t;
    struct sleep_time_queue *tmp;
    t=DisCpuInter();
    tmp=*slp_ls;
    while(tmp){
        tmp->cur_ms+=OS_WORK_PERIOD_MS;
        if(tmp->cur_ms>=tmp->slp_ms){
            task_run_1(tmp->task);
        }
        tmp=tmp->next;
    }
    RestoreCpuInter(t);
}
void do_check_sleep_tim(void){
    _do_check_sleep_tim(&slp_tim_ls);
}
#include <ipc/spin_lock.h>

int do_nanosleep(struct sleep_time_queue **slp_ls,const struct timespec *req,struct timespec *rem,Atomic_t *lock,int check_val
        ){
    if(!req){
        return -EINVAL;
    }
    if(req->tv_nsec==0 && req->tv_sec==0){
        if(rem){
            rem->tv_sec=0;
            rem->tv_nsec=0;
        }
        return 0;
    }
#define ROUND(a,b) (((a)/(b))+(((a)%(b))?1:0))
    struct sleep_time_queue stq={
            .next=NULL,
            .task=CUR_TASK,
            //下面换算成ms，向上取整
            .slp_ms=req->tv_sec*1000+ ROUND(ROUND(req->tv_nsec,1000),1000),
            .cur_ms=0
    };
    uint32_t t;
    //
    add_sleep(slp_ls,&stq);
    if(lock) {
        t = DisCpuInter();
        if(atomic_test(lock,check_val)) {
            task_suspend();
            task_sche();
        }
        RestoreCpuInter(t);
    }else{
        task_suspend();
        task_sche();
    }
    remove_sleep(slp_ls,&stq);
    if(rem){
        uint32_t rems;
        rems=stq.slp_ms-stq.cur_ms;
        rem->tv_nsec=(rems%1000)*1000*1000;
        rem->tv_sec=rems/1000;
    }
    if(CUR_TASK->sig_bmp[0]
    ||CUR_TASK->sig_bmp[1]
    ){
        return -EINTR;
    }
    return 0;
}
//请求休眠req的时间，如果被信号中断，则剩余的时间放到rem中并返回-1
int sys_nanosleep(const struct timespec *req, struct timespec *rem){
   return do_nanosleep(&slp_tim_ls,req,rem,0,0);
}
//这个函数可以使用另一个级别等于或高于当前线程的线程先运行。如果没有符合条件的线程，那么这个函数将会立刻返回然后继续执行当前线程的程序。
int sys_sched_yield(void){
    task_sche();
    return 0;
}