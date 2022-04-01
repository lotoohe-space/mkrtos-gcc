//
// Created by Administrator on 2022/3/27.
//

#ifndef UNTITLED1_KNL_MUTEX_H
#define UNTITLED1_KNL_MUTEX_H

#include "arch/atomic.h"

struct mutex_hdl{

    Atomic_t    mutex_fg;

    struct sleep_time_queue *slp_mutex_ls;
};

struct mutex_hdl * mutex_create(void);
void mutex_init(struct mutex_hdl* mutex);
void mutex_free(struct mutex_hdl* mutex);
int mutex_lock(struct mutex_hdl *mutex,uint32_t wait);
int mutex_unlock(struct mutex_hdl *mutex);
void check_mutex_time(void);
#endif //UNTITLED1_KNL_MUTEX_H
