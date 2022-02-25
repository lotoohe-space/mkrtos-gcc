//
// Created by Administrator on 2022/1/12.
//

#ifndef UNTITLED1_IPC_H
#define UNTITLED1_IPC_H
#include <type.h>
#include <arch/atomic.h>
#include <sys/sem.h>


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
