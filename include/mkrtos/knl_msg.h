//
// Created by Administrator on 2022/3/27.
//

#ifndef UNTITLED1_KNL_MSG_H
#define UNTITLED1_KNL_MSG_H

#include "ipc/spin_lock.h"
#include "sys/types.h"
#include "task.h"

struct msg_hdl{
    int32_t front;
    int32_t rear;
    uint32_t max_len;
    uint32_t msg_size;
    uint8_t* msg;

    Atomic_t is_empty;//是不是空的
    Atomic_t is_full;//是不是满的
    spinlock_handler slh;

    struct sleep_time_queue *get_slp;
    struct sleep_time_queue *put_slp;
};

struct msg_hdl* msg_create(uint32_t max_len,uint32_t msg_size);
void msg_init(struct msg_hdl *msg,uint32_t max_len,uint32_t msg_size);
void msg_free(struct msg_hdl* msg);
//int32_t msg_is_empty(struct msg_hdl *msg);
//int32_t msg_is_full(struct msg_hdl *msg) ;
//int32_t msg_len(struct msg_hdl *msg) ;
int msg_get(struct msg_hdl*msg,uint8_t *data,uint32_t wait);
int msg_put(struct msg_hdl *msg,uint8_t *data,uint32_t wait);
void check_msg_time(void);

#endif //UNTITLED1_KNL_MSG_H
