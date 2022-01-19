//
// Created by Administrator on 2022/1/12.
//

#include <arch/atomic.h>
#include <mkrtos/task.h>
#include <mkrtos/mem.h>
#include <errno.h>
#include <ipc/ipc.h>


//֧�ֵ��ź�������
#define MUTEX_NUM 16
struct sem sem_list[MUTEX_NUM]={0};
static Atomic_t lock;
static uint8_t first_used=0;
//��ӵ��ȴ�������
static int32_t add_into_wait_queue(struct sem *mt,struct task *tb){

    struct wait_queue_head *wqh=&mt->wait_list;
    struct wait_queue_head *new_wqh=OSMalloc(sizeof(struct wait_queue_head));
    if(new_wqh==NULL){
        //�����ڴ���ˣ�����������һ��
        return -EAGAIN;
    }
    new_wqh->tb=tb;

    //���ݵ�������
    while(atomic_test_set(&lock,1)==0);
    new_wqh->next=wqh->next;
    mt->wait_list.next=new_wqh;
    atomic_set((&lock),0);

    mt->wait_num++;

    //��������
    tb->status=TASK_SUSPEND;
    task_sche();

    return 0;
}
//�ӵȴ�������ɾ��������-1��˵��ѹ��û�ҵ�
int32_t del_into_wait_queue(struct sem *mt,struct task *tb){
    uint32_t res=0;
    struct wait_queue_head *wqh=mt->wait_list.next;
    struct wait_queue_head *prev_wqh=&mt->wait_list;
    //���ݵ�������
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
//��������������������
static void active_all_wait(struct sem *mt){
    uint32_t res=0;
    struct wait_queue_head *wqh=mt->wait_list.next;
    struct wait_queue_head *next_wqh=mt->wait_list.next->next?mt->wait_list.next->next:NULL;
    //���ݵ�������
    while(atomic_test_set(&lock,1)==0);

    while(wqh){

        //���񱻹رջ������ڹرգ���˵���䱻ɾ����
        if(wqh->tb->status!=TASK_CLOSING
        ||wqh->tb->status!=TASK_CLOSING
        ) {
            //����Ϊ����״̬
            wqh->tb->status = TASK_RUNNING;
        }
        //�ͷ��ڴ�
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
 * ���
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
 * ����һ��
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
 * ����
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
        //����ʧ�������ȴ�����
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
        //����ͷ���֮ǰ�����㣬����Ҫ�����������������񣬲�����Щ������������޳�
        active_all_wait(&(sem_list[mid].lock));
    }

    return 0;
}


