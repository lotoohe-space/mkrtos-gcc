//
// Created by Administrator on 2022/1/12.
//

#ifndef UNTITLED1_IPC_H
#define UNTITLED1_IPC_H
#include <type.h>
#include <arch/atomic.h>

//�ȴ�����ͷ
struct wait_queue_head{
    struct task* tb;
    struct wait_queue_head *next;
};

struct sem{
    //ԭ�ӱ���
    Atomic_t lock;
    //�Ƿ�ʹ��
    Atomic_t used;
    //�ȴ�����
    uint32_t wait_num;
    //�ȴ���ȡ���Ķ���
    struct wait_queue_head wait_list;
};

//�ӵȴ�������ɾ��������-1��˵��ѹ��û�ҵ�
//���������exit������
int32_t del_into_wait_queue(struct sem *mt,struct task *tb);

int32_t sem_get(sem_t mid);
int32_t sem_alloc(void);
int32_t sem_take(sem_t mid);
int32_t sem_release(sem_t mid);

#endif //UNTITLED1_IPC_H
