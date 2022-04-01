//
// Created by Administrator on 2022/3/27.
//
#include "mkrtos/knl_mutex.h"
#include <mkrtos/mem.h>
#include <errno.h>
#include <mkrtos/task.h>




#define KNL_MUTEX_MAX_SIZE 16
static struct mutex_hdl mutex_ls[KNL_MUTEX_MAX_SIZE]={0};
static uint32_t mutex_used[KNL_MUTEX_MAX_SIZE]={0};

struct mutex_hdl* alloc_mutex_hdl(void){
    for(int i=0;i< KNL_MUTEX_MAX_SIZE;i++){
        if(!(mutex_used[i])){
            mutex_used[i]=1;
            return &mutex_ls[i];
        }
    }
    return NULL;
}
void free_mutex_hdl(struct sem_hdl* sem){
    for(int i=0;i<KNL_MUTEX_MAX_SIZE;i++){
        if(mutex_ls+i==sem){
            mutex_used[i]=0;
            break;
        }
    }
}

void check_mutex_time(void){
    for(int i=0;i<KNL_MUTEX_MAX_SIZE;i++){
        if(mutex_used[i]){
            _do_check_sleep_tim(&mutex_ls[i].slp_mutex_ls);
        }
    }
}

struct mutex_hdl * mutex_create(void){
    struct mutex_hdl *mutex;
    mutex=alloc_mutex_hdl();
    if(!mutex){
        return NULL;
    }
    atomic_set(&mutex->mutex_fg,0);
    return mutex;
}
void mutex_init(struct mutex_hdl* mutex){
    if(!mutex){
        return ;
    }
    atomic_set(&mutex->mutex_fg,0);
}
void mutex_free(struct mutex_hdl* mutex){
    if(!mutex){
        return ;
    }
    free_mutex_hdl(mutex);
}
int mutex_lock(struct mutex_hdl *mutex,uint32_t wait){
    if(!mutex){
        return -EINVAL;
    }
    again:
    if(!atomic_test_set(&mutex->mutex_fg,1)){
        if(!wait){
            return -1;
        }else{
            struct timespec times;
            times.tv_sec=wait/1000;
            times.tv_nsec=(wait%1000)*1000*1000;
            struct timespec rem;
            int ret;

            again_sleep:
            //获取休眠
            ret=do_nanosleep(&mutex->slp_mutex_ls,&times,&rem,&mutex->mutex_fg,1);
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
    return 0;
}
int mutex_unlock(struct mutex_hdl *mutex){
    if(!mutex){
        return -EINVAL;
    }
    atomic_set(&mutex->mutex_fg,0);
    wake_up_sleep(&mutex->slp_mutex_ls);
    return 0;
}
