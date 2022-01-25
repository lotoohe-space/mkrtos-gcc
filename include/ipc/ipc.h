//
// Created by Administrator on 2022/1/12.
//

#ifndef UNTITLED1_IPC_H
#define UNTITLED1_IPC_H
#include <type.h>
#include <arch/atomic.h>


//sem.c
struct sem{
    //ʹ�ô���
    Atomic_t s_used_count;
    //�ȴ�����������
    Atomic_t s_wait_num;
    //������
    uint32_t s_max_count;
    //�źż���
    Atomic_t s_sem_count;
    //�ȴ�����
    struct wait_queue *s_wait;
};

int32_t sem_get(sem_t mid);
int32_t sem_put(sem_t mid) ;
int32_t sem_take(sem_t mid);
int32_t sem_release(sem_t mid);

//mutex.c
struct mutex{
    //ʹ�ô���
    Atomic_t s_used_count;
    //��ס
    Atomic_t m_lock;
    //˭����smutx
    struct task* m_who_lock;
    //����֮ǰ�����ȼ�
    uint32_t m_tk_prev_prio;
    //������ܵĵȴ�����
    struct wait_queue *m_wait;
};

int32_t mutex_get(sem_t mid);
int32_t mutex_put(sem_t mid) ;
int32_t lock_mutex(int32_t mt_l);
int32_t unlock_mutex(int32_t mt_l);

#endif //UNTITLED1_IPC_H
