//
// Created by Administrator on 2022/3/27.
//
//内核里面使用的msg，不兼容posix，只是为了提高速度

#include "mkrtos/knl_msg.h"
#include <mkrtos/mem.h>
#include <errno.h>
#include <string.h>

#define KNL_MSG_MAX_SIZE 16
static struct msg_hdl msg_ls[KNL_MSG_MAX_SIZE]={0};
static uint32_t msg_used[KNL_MSG_MAX_SIZE]={0};

static struct msg_hdl* alloc_msg_hdl(void){
    for(int i=0;i< KNL_MSG_MAX_SIZE;i++){
        if(!msg_used[i]){
            msg_used[i]=1;
            return &msg_ls[i];
        }
    }
    return NULL;
}
static void free_msg_hdl(struct msg_hdl* msg){
    for(int i;i<KNL_MSG_MAX_SIZE;i++){
        if(msg_ls+i==msg){
            msg_used[i]=0;
        }
    }
}
void check_msg_time(void){
    for(int i;i<KNL_MSG_MAX_SIZE;i++){
        if(msg_used[i]){
            _do_check_sleep_tim(msg_ls[i].put_slp);
            _do_check_sleep_tim(msg_ls[i].get_slp);
        }
    }
}

struct msg_hdl* msg_create(uint32_t max_len,uint32_t msg_size){
    struct msg_hdl *msg;
    msg=(struct msg_hdl*)alloc_msg_hdl();
    if(!msg){
        return NULL;
    }

    msg->msg=OSMalloc(sizeof(uint8_t)*msg_size*max_len);
    if((msg->msg)==NULL){
        OSFree(msg);
        return NULL;
    }
    msg->max_len=max_len;
    msg->msg_size=msg_size;
    msg->rear=msg->front=0;
    spin_lock_init(&msg->slh);
    return msg;
}
void msg_init(struct msg_hdl *msg,uint32_t max_len,uint32_t msg_size){
    if(!msg){
        return ;
    }
    msg->max_len=max_len;
    msg->msg_size=msg_size;
    msg->rear=msg->front=0;
    spin_lock_init(&msg->slh);
}
void msg_free(struct msg_hdl* msg){
    if(!msg){
        return ;
    }
    OSFree(msg->msg);
    free_msg_hdl(msg);
}
int32_t msg_is_empty(struct msg_hdl *msg){
    if(!msg){
        return -1;
    }
    uint32_t res;
    res=(msg->front==msg->rear);
    return res;
}
int32_t msg_is_full(struct msg_hdl *msg) {
    if (!msg) {
        return -1;
    }
    uint32_t res;
    res = ((msg->rear + 1) % msg->max_len == msg->front);
    return res;
}
int32_t msg_len(struct msg_hdl *msg) {
    if (!msg) {
        return -1;
    }
    uint32_t len;
    len = (msg->rear - msg->front + msg->max_len) % msg->max_len;
    return len;
}
int msg_get(struct msg_hdl*msg,uint8_t *data,uint32_t wait){
    uint32_t t;
    if(!msg){
        return -EINVAL;
    }
    again:
    if(msg_is_empty(msg)){
        if(!wait) {
            return -1;
        }else{
            struct timespec times;
            times.tv_sec=wait/1000;
            times.tv_nsec=(wait%1000)*1000*1000;
            struct timespec rem;
            int ret;

            again_sleep:
            //获取休眠
            ret=do_nanosleep(&msg->get_slp,&times,&rem);
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
    }else{
        spin_lock(&msg->slh);

        if(msg_is_empty(msg)){
            spin_unlock(&msg->slh);
            return -1;
        }
        msg->front=(msg->front+1)%msg->max_len;
        memcpy(data,
               msg->msg+ msg->front * msg->msg_size
                ,msg->msg_size);

        spin_unlock(&msg->slh);
        wake_up_sleep(msg->put_slp);
    }
    return 0;
}
int msg_put(struct msg_hdl *msg,uint8_t *data,uint32_t wait){
    uint32_t t;
    if(!msg){
        return -EINVAL;
    }
    again:
    if(msg_is_full(msg)){
        /*满了，放入失败，挂起任务，休眠并等待释放*/
        if(wait==0){
            return -1;
        }else{
            struct timespec times;
            times.tv_sec=wait/1000;
            times.tv_nsec=(wait%1000)*1000*1000;
            struct timespec rem;
            int ret;

            again_sleep:
            //获取休眠
            ret=do_nanosleep(&msg->put_slp,&times,&rem);
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
    }else{
        spin_lock(&msg->slh);
        /*一定要二次检查*/
        if(msg_is_full(msg)){
            spin_unlock(&msg->slh);
            return -1;
        }
        msg->rear=(msg->rear+1)%msg->max_len ;
        memcpy(
                /*拷贝的开始地址*/
                msg->msg+ msg->rear * msg->msg_size
                ,data
                ,msg->msg_size
        );
        spin_unlock(&msg->slh);
        /*唤醒所有因为MsgGetBlock的任务*/
        wake_up_sleep(msg->get_slp);
    }

    return 0;
}
