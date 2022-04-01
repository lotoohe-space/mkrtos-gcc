//
// Created by Administrator on 2022/3/27.
//

#ifndef UNTITLED1_KNL_SEM_H
#define UNTITLED1_KNL_SEM_H

#include "arch/atomic.h"
#include "task.h"
#include <ipc/spin_lock.h>
struct sem_hdl{

    Atomic_t    sem_cnt;
    Atomic_t    max_cnt;


    struct sleep_time_queue *slp_sem_ls;
};

struct sem_hdl *sem_create(uint32_t init_cnt,uint32_t max_cnt) ;
void sem_init(struct sem_hdl *sem,uint32_t init_cnt,uint32_t max_cnt);
void sem_free(struct sem_hdl * sem);
int sem_pend(struct sem_hdl* sem,uint32_t wait);
int sem_post(struct sem_hdl *sem);
void check_sem_time(void);
#endif //UNTITLED1_KNL_SEM_H
