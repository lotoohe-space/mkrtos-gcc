//
// Created by Administrator on 2022/1/12.
//

#ifndef UNTITLED1_IPC_H
#define UNTITLED1_IPC_H
#include <type.h>
#include <arch/atomic.h>

//等待队列头
struct wait_queue_head{
    struct task* tb;
    struct wait_queue_head *next;
};

struct sem{
    //原子变量
    Atomic_t lock;
    //是否使用
    Atomic_t used;
    //等待数量
    uint32_t wait_num;
    //等待获取锁的队列
    struct wait_queue_head wait_list;
};

//从等待链表中删除，返回-1，说明压根没找到
//这个函数给exit函数用
int32_t del_into_wait_queue(struct sem *mt,struct task *tb);

int32_t sem_get(sem_t mid);
int32_t sem_alloc(void);
int32_t sem_take(sem_t mid);
int32_t sem_release(sem_t mid);

#endif //UNTITLED1_IPC_H
