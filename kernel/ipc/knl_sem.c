//
// Created by Administrator on 2022/3/27.
//

#include "mkrtos/knl_sem.h"
#include "mkrtos/mem.h"
#include <errno.h>
#define KNL_SEM_MAX_SIZE 16
static struct sem_hdl sem_ls[KNL_SEM_MAX_SIZE]={0};
static uint32_t sem_used[KNL_SEM_MAX_SIZE]={0};

struct sem_hdl* alloc_sem_hdl(void){
    for(int i=0;i< KNL_SEM_MAX_SIZE;i++){
        if(!sem_used[i]){
            sem_used[i]=1;
            return &sem_ls[i];
        }
    }
    return NULL;
}
void free_sem_hdl(struct sem_hdl* sem){
    for(int i=0;i<KNL_SEM_MAX_SIZE;i++){
        if(&sem_ls[i]==sem){
            sem_used[i]=0;
            break;
        }
    }
}

struct sem_hdl *sem_create(uint32_t init_cnt,uint32_t max_cnt) {
    struct sem_hdl *sem;
    sem = (struct sem_hdl *) alloc_sem_hdl();
    if (!sem) {
        return NULL;
    }
    atomic_set(&sem->sem_cnt, init_cnt);
    atomic_set(&sem->max_cnt, max_cnt);
    return sem;
}
void sem_init(struct sem_hdl *sem,uint32_t init_cnt,uint32_t max_cnt){
    if(!sem){
        return ;
    }
    atomic_set(&sem->sem_cnt, init_cnt);
    atomic_set(&sem->max_cnt, max_cnt);
}

void check_sem_time(void){
    for(int i=0;i<KNL_SEM_MAX_SIZE;i++){
        if(sem_used[i]){
            if(sem_ls[i].slp_sem_ls) {
                _do_check_sleep_tim(&(sem_ls[i].slp_sem_ls));
            }
        }
    }
}

void sem_free(struct sem_hdl * sem){
    if(!sem){
        return ;
    }
    free_sem_hdl(sem);
}

int sem_pend(struct sem_hdl* sem,uint32_t wait){
    int old_wait;
    if(sem==NULL){
        return -EINVAL;
    }
    old_wait=wait;
    again:
    if(atomic_test_dec_nq(&sem->sem_cnt)){
        /*加锁失败，挂起任务，休眠并等待被解锁*/
        if(wait==0){
            return -1;
        }else{
            struct timespec times;
            times.tv_sec=wait/1000;
            times.tv_nsec=(wait%1000)*1000*1000;
            struct timespec rem;
            int ret;

        again_sleep:
            ret=do_nanosleep(&sem->slp_sem_ls,&times,&rem,&sem->sem_cnt,0);
            if(ret==-EINTR){
                //这里需要重新设定延时
                times.tv_nsec=rem.tv_nsec;
                times.tv_sec=rem.tv_sec;
                rem.tv_sec=0;
                rem.tv_nsec=0;
                goto again_sleep;
            }
            //剩余的时间放到wait中
            wait=rem.tv_sec*1000+rem.tv_nsec/1000/1000;
            goto again;
        }
    }
    return old_wait-wait;
}
int sem_post(struct sem_hdl *sem){
    if(!sem){
        return -EINVAL;
    }
    if(atomic_read(&sem->sem_cnt)>=atomic_read(&sem->max_cnt)){
        //唤醒所有等待休眠的进程
        wake_up_sleep(&sem->slp_sem_ls);
        return 0;
    }
    atomic_inc(&sem->sem_cnt);
    //唤醒所有等待休眠的进程
    wake_up_sleep(&sem->slp_sem_ls);

    return 0;
}