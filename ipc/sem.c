//
// Created by Administrator on 2022/1/12.
//

#include <arch/atomic.h>
#include <mkrtos/task.h>
#include <mkrtos/mem.h>
#include <errno.h>
#include <ipc/ipc.h>

//支持的信号量数量
#define SEM_NUM 16

static struct sem sem_list[SEM_NUM]={0};


/**
 * 获得一个sem
 * @return
 */
int32_t sem_get(sem_t mid){
    struct sem* temp;
    struct wait_queue* wq;
    struct wait_queue *pwait=NULL;

    if(mid<0||mid>=SEM_NUM){
        return -EINVAL;
    }

    wq=find_wait_queue(&sem_list[mid].s_wait,CUR_TASK,NULL);
    if(wq){
        //已经在里面了
        return mid;
    }

    pwait= OSMalloc(sizeof(struct wait_queue));
    if(pwait == NULL){
        return -ENOMEM;
    }
    pwait->task=CUR_TASK;
    pwait->next=NULL;

    atomic_inc(&sem_list[mid].s_used_count);
    //添加到等待队列中
    add_wait_queue(&sem_list[mid].s_wait, pwait);
    return mid;
}
/**
 * 放弃一个sem
 * @param mid
 * @return
 */
int32_t sem_put(sem_t mid) {
    struct sem *temp;
    struct wait_queue* wq;

    if (mid < 0 || mid >= SEM_NUM) {
        return -EINVAL;
    }
    if (atomic_test(&sem_list[mid].s_used_count,0)){
        return -EAGAIN;
    }
    //只有自己才能干掉自己
    wq=find_wait_queue(&sem_list[mid].s_wait,CUR_TASK,NULL);
    if(wq!=NULL) {
        atomic_dec(&sem_list[mid].s_used_count);
        remove_wait_queue(&sem_list[mid].s_wait, wq);
        OSFree(wq);
    }else{
        return -EACCES;
    }
    return 0;
}
/**
 * 获得信号量
 * @param mid
 * @return
 */
int32_t sem_take(sem_t mid){
    struct wait_queue* wq;

    if(mid<0||mid>=SEM_NUM){
        return -EINVAL;
    }

    //先检查进程是否申请了资源
    wq=find_wait_queue(&sem_list[mid].s_wait,CUR_TASK,NULL);
    if(wq==NULL){
        return -EACCES;
    }
    again:
    if(atomic_test(&sem_list[mid].s_sem_count,0)) {
        atomic_inc(&(sem_list[mid].s_wait_num));
        //如果为零则挂起当前任务
        task_suspend();
//        CUR_TASK->status=TASK_SUSPEND;
        task_sche();
        //醒来重新检查
        goto again;
    }

    return 0;
}
/**
 * 释放信号量
 * @param mid
 * @return
 */
int32_t sem_release(sem_t mid){
    struct wait_queue* wq;

    if(mid<0||mid>=SEM_NUM){
        return -EINVAL;
    }
    //先检查进程是否申请了资源
    wq=find_wait_queue(&sem_list[mid].s_wait,CUR_TASK,NULL);
    if(wq==NULL){
        return -EACCES;
    }
    if(atomic_cmp_hi_inc1(&(sem_list[mid].s_sem_count),sem_list[mid].s_max_count)){
        //唤醒所有挂起的任务
        wake_up((sem_list[mid].s_wait));
        atomic_set(&(sem_list[mid].s_wait_num),0);
    }else{
        return 1;
    }
    return 0;
}


