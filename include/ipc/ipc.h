//
// Created by Administrator on 2022/1/12.
//

#ifndef UNTITLED1_IPC_H
#define UNTITLED1_IPC_H
#include <type.h>
#include <arch/atomic.h>


//sem.c
struct sem{
    //使用次数
    Atomic_t s_used_count;
    //等待的任务数量
    Atomic_t s_wait_num;
    //最大计数
    uint32_t s_max_count;
    //信号计数
    Atomic_t s_sem_count;
    //等待链表
    struct wait_queue *s_wait;
};

int32_t sem_get(sem_t mid);
int32_t sem_put(sem_t mid) ;
int32_t sem_take(sem_t mid);
int32_t sem_release(sem_t mid);

//mutex.c
struct mutex{
    //使用次数
    Atomic_t s_used_count;
    //锁住
    Atomic_t m_lock;
    //谁锁的smutx
    struct task* m_who_lock;
    //任务之前的优先级
    uint32_t m_tk_prev_prio;
    //任务可能的等待队列
    struct wait_queue *m_wait;
};

int32_t mutex_get(sem_t mid);
int32_t mutex_put(sem_t mid) ;
int32_t lock_mutex(int32_t mt_l);
int32_t unlock_mutex(int32_t mt_l);

#endif //UNTITLED1_IPC_H
