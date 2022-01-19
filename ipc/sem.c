//
// Created by Administrator on 2022/1/12.
//

#include <arch/atomic.h>
#include <mkrtos/task.h>
#include <mkrtos/mem.h>
#include <errno.h>
#include <ipc/ipc.h>


//支持的信号量数量
#define MUTEX_NUM 16
struct sem sem_list[MUTEX_NUM]={0};
static Atomic_t lock;
static uint8_t first_used=0;
//添加到等待链表中
static int32_t add_into_wait_queue(struct sem *mt,struct task *tb){

    struct wait_queue_head *wqh=&mt->wait_list;
    struct wait_queue_head *new_wqh=OSMalloc(sizeof(struct wait_queue_head));
    if(new_wqh==NULL){
        //灭有内存的了，可以重新试一试
        return -EAGAIN;
    }
    new_wqh->tb=tb;

    //短暂的链表保护
    while(atomic_test_set(&lock,1)==0);
    new_wqh->next=wqh->next;
    mt->wait_list.next=new_wqh;
    atomic_set((&lock),0);

    mt->wait_num++;

    //挂起任务
    tb->status=TASK_SUSPEND;
    task_sche();

    return 0;
}
//从等待链表中删除，返回-1，说明压根没找到
int32_t del_into_wait_queue(struct sem *mt,struct task *tb){
    uint32_t res=0;
    struct wait_queue_head *wqh=mt->wait_list.next;
    struct wait_queue_head *prev_wqh=&mt->wait_list;
    //短暂的链表保护
    while(atomic_test_set(&lock,1)==0);

    while(wqh){

        if(wqh->tb==tb){
            prev_wqh->next=wqh->next;
            OSFree(wqh);
            goto end;
        }

        prev_wqh=wqh;
        wqh=wqh->next;
    }
    res=-1;
    end:
    atomic_set((&lock),0);
    return res;
}
//激活并清楚所有阻塞的任务
static void active_all_wait(struct sem *mt){
    uint32_t res=0;
    struct wait_queue_head *wqh=mt->wait_list.next;
    struct wait_queue_head *next_wqh=mt->wait_list.next->next?mt->wait_list.next->next:NULL;
    //短暂的链表保护
    while(atomic_test_set(&lock,1)==0);

    while(wqh){

        //任务被关闭或者正在关闭，则说明其被删除了
        if(wqh->tb->status!=TASK_CLOSING
        ||wqh->tb->status!=TASK_CLOSING
        ) {
            //设置为运行状态
            wqh->tb->status = TASK_RUNNING;
        }
        //释放内存
        OSFree(wqh);

        wqh=next_wqh;
        if(next_wqh) {
            next_wqh = next_wqh->next;
        }
    }
    mt->wait_num=0;
    end:
    atomic_set((&lock),0);
}

/**
 * 获得
 * @return
 */
int32_t sem_get(sem_t mid){
    struct sem* temp;
    if(mid<0||mid>=MUTEX_NUM){
        return -1;
    }

    temp = sem_list + mid;
    if(temp->wait_num>0){
        sem_release(mid);
    }
    if(atomic_read(&(sem_list[mid].used))==0) {
        atomic_set(&(sem_list[mid].used), 1);
        atomic_set(&(temp->lock), 1);
    }

    return mid;
}
/**
 * 分配一个
 * @param mid
 * @return
 */
int32_t sem_alloc(void){
    int32_t i;
    struct sem* temp;
    for(i=0;i<MUTEX_NUM;i++){
        temp = sem_list + i;
        if(atomic_test_set(&(sem_list[i].used),1)) {
            return i;
        }
    }
    return -1;
}
/**
 * 放弃
 * @param mid
 * @return
 */
int32_t sem_put(uint32_t mid){
    struct sem* temp;
    if(mid<0||mid>=MUTEX_NUM){
        return -1;
    }
    temp = sem_list + mid;
    if(atomic_read(&(sem_list[mid].used))==1) {
        temp->wait_list.tb = NULL;
        temp->wait_list.next = NULL;
        atomic_set(&(sem_list[mid].used), 1);
    }
}
int32_t sem_take(sem_t mid){
    if(mid<0||mid>=MUTEX_NUM){
        return -1;
    }
    if(!atomic_test_set(&(sem_list[mid].lock),1)) {
        //设置失败则加入等待队列
        if(add_into_wait_queue(&sem_list[mid],sysTasks.currentTask)<0){
            return -EAGAIN;
        }
    }

    return 0;
}
int32_t sem_release(sem_t mid){
    if(mid<0||mid>=MUTEX_NUM){
        return -1;
    }
    if(!atomic_set_test(&(sem_list[mid].lock),0)){
        //如果释放锁之前不是零，则需要激活所有阻塞的任务，并将这些任务从链表中剔除
        active_all_wait(&(sem_list[mid].lock));
    }

    return 0;
}


