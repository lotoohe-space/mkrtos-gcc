//
// Created by Administrator on 2022/2/25.
//

#include <sys/sem.h>
#include <errno.h>
#include <arch/atomic.h>
#include <mkrtos/mem.h>
#include <mkrtos/task.h>
#define SEM_NUM 4
static struct semid_ds semid_ds_ls[SEM_NUM]={0};
static Atomic_t semid_ds_used[SEM_NUM]={0};
static Atomic_t sem_list_lk={0};

static int32_t inner_find_sem(key_t key){
    int i;

   while(!atomic_test_set(&sem_list_lk,1));
    for(i=0;i<SEM_NUM;i++){
        if (!atomic_test(&semid_ds_used,0)) {
            if (semid_ds_ls[i].sem_perm.key == key) {
                atomic_set(&sem_list_lk, 0);
                return i;
            }
        }
    }
    atomic_set(&sem_list_lk,0);

    return -1;
}

static int32_t inner_sem_creat(key_t key,int nsems,int flag){
    int i;

    while(!atomic_test_set(&sem_list_lk,1));
    for(i=0;i<SEM_NUM;i++){
        if (atomic_test_set(&semid_ds_used[i],1)) {
            break;
        }
    }
    atomic_set(&sem_list_lk,0);
    if(i==SEM_NUM) {
        return -ENOENT;
    }
    semid_ds_ls[i].sem_base=OSMalloc(nsems);
    if(!semid_ds_ls[i].sem_base) {
        atomic_set(&semid_ds_used[i],0);
    }
    semid_ds_ls[i].sem_perm.cgid=CUR_TASK->egid;
    semid_ds_ls[i].sem_perm.cuid=CUR_TASK->euid;
    semid_ds_ls[i].sem_perm.gid=CUR_TASK->rgid;
    semid_ds_ls[i].sem_perm.uid=CUR_TASK->ruid;
    semid_ds_ls[i].sem_perm.key=key;
    semid_ds_ls[i].sem_perm.mode=flag;
    semid_ds_ls[i].sem_perm.seq=0;

    semid_ds_ls[i].sem_ctime=0;
    semid_ds_ls[i].sem_nsems=nsems;
    semid_ds_ls[i].sem_otime=0;
    semid_ds_ls[i].sem_pending=0;
    semid_ds_ls[i].sem_pending_last=0;
    semid_ds_ls[i].undo=0;
//    atomic_set(&sem_list_lk,0);

    return i;
}

#define ABS(a) ((a)<0?(-(a)):(a))
#include <arch/arch.h>
//���Ѷ��������е�����
void sem_wake_up(struct sem_queue *queue,int semnum){
    uint32_t t;
    t=DisCpuInter();
    while(queue){
        if(queue->task){
            if(
                    ( queue->semid==semnum ||semnum<0)
                    &&queue->task->status==TASK_SUSPEND
                    ){
                task_run_1(queue->task);
            }
        }
        queue=queue->next;
    }
    RestoreCpuInter(t);
}

//���һ�����ȴ�������
void sem_add_wait_queue(struct sem_queue** queue,struct sem_queue* add_queue){
    uint32_t t;
    t=DisCpuInter();
    if(*queue==NULL){
        *queue=add_queue;
    }else{
        add_queue->next = (*queue);
        *queue=add_queue;
    }
    RestoreCpuInter(t);
}
//�Ƴ�һ���ȴ���
void sem_remove_wait_queue(struct sem_queue ** queue,struct sem_queue* add_queue){
    struct sem_queue *temp=*queue;
    struct sem_queue *prev=NULL;
    uint32_t t;
    if(!add_queue){
        return ;
    }
    t=DisCpuInter();
    while(temp){
        if(temp==add_queue) {
            if (prev==NULL) {
                //ɾ���ĵ�һ��
                *queue=temp->next;
                break;
            }else{
                prev->next=temp->next;
                break;
            }
        }
        prev=temp;
        temp=temp->next;
    }
    RestoreCpuInter(t);
}


int32_t sys_semget(key_t key,int nsems,int flag){
    struct semid_ds *sem_ds;
    int id;
    if(key==IPC_PRIVATE){
        id=inner_sem_creat(key,nsems,flag);
        if(id<0) {
            return -ENOMEM;
        }
    }else{
        if(!flag){
            id=inner_find_sem(key);
            if(id<0) {
                return -ENOENT;
            }
            return id;
        }else {
            //��ȡһ�����ڵ�
            if (flag & IPC_CREAT) {
                if (flag & IPC_EXCL) {
                } else {
                }
            }
        }
    }
    return -ENOMEM;
}

int32_t sys_semctl(int semid,int semnum,int cmd,union semun arg){
    int i;
    if(semid<0||semid>=SEM_NUM){
        return -1;
    }
    if(atomic_test(&semid_ds_used[semid],0)){
        return -EIDRM;
    }

    switch (cmd){
        case IPC_STAT:
            memcpy(arg.buf,&semid_ds_ls[semid],sizeof(struct semid_ds));
            break;
        case IPC_SET:
            if(CUR_TASK->ruid==semid_ds_ls[semid].sem_perm.cuid
            ||CUR_TASK->ruid==semid_ds_ls[semid].sem_perm.uid
            ||CUR_TASK->is_s_user
            ){
                struct semid_ds *ds;
                ds=(struct semid_ds*)arg.buf;
                semid_ds_ls[semid].sem_perm.uid=ds->sem_perm.uid;
                semid_ds_ls[semid].sem_perm.gid=ds->sem_perm.gid;
                semid_ds_ls[semid].sem_perm.mode=ds->sem_perm.mode;
            }
            break;
        case IPC_RMID:
            //�������еȴ����ź����Ľ���
            sem_wake_up(&semid_ds_ls[semid].sem_pending,-1);
            //ɾ�����ź���
            atomic_set(&semid_ds_used[semid],0);
            //�ͷ��ڴ�
            OSFree(semid_ds_ls[semid].sem_base);
            break;
        case GETVAL:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            return semid_ds_ls[semid].sem_base[semnum].semval;
//            break;
        case SETVAL:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            semid_ds_ls[semid].sem_base[semnum].semval=arg.val;
            break;
        case GETPID:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            return semid_ds_ls[semid].sem_base[semnum].sempid;
//            break;
        case GETNCNT:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            return semid_ds_ls[semid].sem_base[semnum].semncnt;
//            break;
        case GETZCNT:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            return semid_ds_ls[semid].sem_base[semnum].semzcnt;
//            break;
        case GETALL:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            for(i=0;i<semid_ds_ls[semid].sem_nsems;i++){
                arg.array[i]=semid_ds_ls[semid].sem_base[i].semval;
            }
            break;
        case SETALL:
            if(semnum>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            for(i=0;i<semid_ds_ls[semid].sem_nsems;i++){
                semid_ds_ls[semid].sem_base[i].semval=arg.array[i];
            }
            break;
    }

    return 0;
}

int semop(int semid,struct sembuf semoparray[],size_t ops){
    if(semid<0||semid>=SEM_NUM){
        return -1;
    }
    if(atomic_test(&semid_ds_used[semid],0)){
        return -EIDRM;
    }
    for(int i=0;i<ops;i++){
        if(semoparray[i].sem_op>0){
            if(semoparray[i].sem_num>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            if(semoparray->sem_flg&SEM_UNDO){
                //�������ȥ
                semid_ds_ls[semid]
                        .sem_base[semoparray[i].sem_num]
                        .semval-=semoparray[i].sem_op;
            }else{
                //��֮����
                semid_ds_ls[semid]
                .sem_base[semoparray[i].sem_num]
                .semval+=semoparray[i].sem_op;
            }

            //����Ƿ��ڵȴ�ĳ����Դ��λ
            if(semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semval>=ABS(semoparray[i].sem_op)) {
                //ֵ��1
                if(semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semncnt>0){
                    semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semncnt--;
                }
                //�����ͷ�����Դ�����ѵȴ��Ľ���
                sem_wake_up(&semid_ds_ls[semid].sem_pending,semoparray[i].sem_num);
            }

            //����ź����Ƿ���
            if(semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semval==0) {
                //ֵ��1
                if(semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semzcnt>0){
                    semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semzcnt--;
                }
                //�����ͷ�����Դ�����ѵȴ��Ľ���
                sem_wake_up(&semid_ds_ls[semid].sem_pending,semoparray[i].sem_num);
            }

        }else if(semoparray[i].sem_op==0){
            again_0:
            if(!semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semval){
                return 0;
            }
            //ϣ���ȴ������ź���ֵ���0
            if(semid_ds_ls[semid].sem_perm.mode&IPC_NOWAIT){
                return -EAGAIN;
            }
            semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semzcnt++;

            //�����������
            struct sem_queue sem_q={semoparray[i].sem_num,CUR_TASK,NULL};
            sem_add_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);
            task_suspend();
            task_sche();
            sem_remove_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);

            //�źű�ɾ���ˣ��򷵻�ERMID
            if(atomic_test(&semid_ds_used[semid],0)){
                return -ERMID;
            }
            //���źţ�����EINTR
            if(CUR_TASK->sig_bmp){
                semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semzcnt--;
                return -EINTR;
            }
            goto again_0;
        } if(semoparray[i].sem_op<0){
            again_get:
            //���ھ���ֵ
            if(semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semval>=ABS(semoparray[i].sem_op)) {
                //��ֱ�ӽ��в���
                if (semoparray->sem_flg & SEM_UNDO) {
                    semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semval += ABS(semoparray[i].sem_op);
                } else {
                    semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semval -= ABS(semoparray[i].sem_op);
                }
            }else{
                //��֮˵����Դ����
                if(semid_ds_ls[semid].sem_perm.mode&IPC_NOWAIT){
                    return -EAGAIN;
                }
                //Ҫ���������
                semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semncnt++;

                //�����������
                struct sem_queue sem_q={semoparray[i].sem_num,CUR_TASK,NULL};
                sem_add_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);
                task_suspend();
                task_sche();
                sem_remove_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);

                //�źű�ɾ���ˣ��򷵻�ERMID
                if(atomic_test(&semid_ds_used[semid],0)){
                    return -ERMID;
                }
                //���źţ�����EINTR
                if(CUR_TASK->sig_bmp){
                    semid_ds_ls[semid].sem_base[semoparray[i].sem_num].semncnt--;
                    return -EINTR;
                }

                goto again_get;
            }
        }
    }
    return 0;
}

