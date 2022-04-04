//
// Created by Administrator on 2022/2/25.
//

#include <sys/sem.h>
#include <errno.h>
#include <arch/atomic.h>
#include <mkrtos/mem.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
#define SEM_NUM 4
static struct semid_ds semid_ds_ls[SEM_NUM]={0};
static Atomic_t semid_ds_used[SEM_NUM]={0};
static Atomic_t sem_list_lk={0};

static void lock_ls(void){
    while(!atomic_test_set(&sem_list_lk,1));
}
static void unlock_ls(void){
    atomic_set(&sem_list_lk,0);
}
static void lock_sem(struct sem* _sem){
    while(!atomic_test_set(&_sem->semlock,1));
}
static void unlock_sem(struct sem* _sem){
    atomic_set(&_sem->semlock,0);
}
static int32_t inner_find_sem(key_t key){
    int i;

    lock_ls();
    for(i=0;i<SEM_NUM;i++){
        if (!atomic_test(&semid_ds_used[i],0)) {
            if (semid_ds_ls[i].sem_perm.key == key) {
                atomic_set(&sem_list_lk, 0);
                return i;
            }
        }
    }
    unlock_ls();

    return -1;
}

static int32_t inner_sem_creat(key_t key,int nsems,int flag){
    int i;

    lock_ls();
    for(i=0;i<SEM_NUM;i++){
        if (atomic_test_set(&semid_ds_used[i],1)) {
            break;
        }
    }
    unlock_ls();
    if(i==SEM_NUM) {
        return -ENOENT;
    }
    semid_ds_ls[i].sem_base=OSMalloc(nsems);
    if(!semid_ds_ls[i].sem_base) {
        atomic_set(&semid_ds_used[i],0);
        return -ENOMEM;
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

//唤醒队列中所有的任务
void sem_wake_up(struct sem_queue *queue,int semnum){
    uint32_t t;
    t=DisCpuInter();
    while(queue){
        if(queue->task){
            if(
                    ( queue->semid==semnum ||semnum<0)
                    &&(queue->task->status==TASK_SUSPEND
                        ||queue->task->status==TASK_UNINTR)
                    ){
                task_run_1(queue->task);
            }
        }
        queue=queue->next;
    }
    RestoreCpuInter(t);
}

//添加一个到等待队列中
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
//移除一个等待的
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
                //删除的第一个
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
            return id;
        }
    }else{
        id=inner_find_sem(key);
        if(!flag){
            if(id<0) {
                return -ENOENT;
            }
            return id;
        }else {
            //获取一个存在的
            if (flag & IPC_CREAT) {
                if (flag & IPC_EXCL) {
                    if(id>=0) {
                        return -ENOENT;
                    }
                    id=inner_sem_creat(key,nsems,flag);
                    if(id<0) {
                        return id;
                    }
                    return 0;
                }
                if(id>=0){
                    return id;
                }
                id=inner_sem_creat(key,nsems,flag);
                return id;
            }
            return id;
        }
    }
    return -ENOMEM;
}
static Atomic_t sem_ds_lock[SEM_NUM]={0};
static void lock_sem_ds(int ds_no){
    while(atomic_test_set(&sem_ds_lock[ds_no],1));
}
static void unlock_sem_ds(int ds_no){
    atomic_set(&sem_ds_lock[ds_no],0);
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
            lock_sem_ds(semid);
            memcpy(arg.buf,&semid_ds_ls[semid],sizeof(struct semid_ds));
            unlock_sem_ds(semid);
            break;
        case IPC_SET:
            lock_sem_ds(semid);
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
            unlock_sem_ds(semid);
            break;
        case IPC_RMID:
            //唤醒所有等待该信号量的进程
            sem_wake_up(semid_ds_ls[semid].sem_pending,-1);
            //删除该信号量
            atomic_set(&semid_ds_used[semid],0);
            //释放内存
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
            lock_sem(&semid_ds_ls[semid].sem_base[semnum]);
            semid_ds_ls[semid].sem_base[semnum].semval=arg.val;
            unlock_sem(&semid_ds_ls[semid].sem_base[semnum]);
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
                lock_sem(&semid_ds_ls[semid].sem_base[i]);
                semid_ds_ls[semid].sem_base[i].semval=arg.array[i];
                unlock_sem(&semid_ds_ls[semid].sem_base[i]);
            }
            break;
    }

    return 0;
}

int sys_semop(int semid,struct sembuf semoparray[],size_t ops){
    struct semid_ds *sem_ds;
    struct sem* _sem;
    if(semid<0||semid>=SEM_NUM){
        return -1;
    }
    if(atomic_test(&semid_ds_used[semid],0)){
        return -EIDRM;
    }
    sem_ds=&semid_ds_ls[semid];

    for(int i=0;i<ops;i++){
        _sem=&semid_ds_ls[semid].sem_base[semoparray[i].sem_num];

        //最后一次操作的进程pid
        sem_ds->sem_base[semoparray[i].sem_num].sempid=CUR_TASK->PID;
        if(semoparray[i].sem_op>0){
            lock_sem(_sem);
            if(semoparray[i].sem_num>=semid_ds_ls[semid].sem_nsems){
                return -1;
            }
            if(semoparray->sem_flg&SEM_UNDO){
                //撤销则减去
                _sem->semval-=semoparray[i].sem_op;
            }else{
                //反之加上
                _sem->semval+=semoparray[i].sem_op;
            }

            //检查是否在等待某个资源到位
            if(_sem->semval>=ABS(semoparray[i].sem_op)) {
                //值减1
                if(_sem->semncnt>0){
                    _sem->semncnt--;
                }
                //进程释放了资源，唤醒等待的进程
                sem_wake_up(semid_ds_ls[semid].sem_pending,semoparray[i].sem_num);
            }

            //检查信号量是否到零
            if(_sem->semval==0) {
                //值减1
                if(_sem->semzcnt>0){
                    _sem->semzcnt--;
                }
                //进程释放了资源，唤醒等待的进程
                sem_wake_up(semid_ds_ls[semid].sem_pending,semoparray[i].sem_num);
            }
            unlock_sem(_sem);
        }else if(semoparray[i].sem_op==0){
            again_0:
            lock_sem(_sem);
            if(!_sem->semval){
                unlock_sem(_sem);
                return 0;
            }
            //希望等待到该信号量值变成0
            if(semid_ds_ls[semid].sem_perm.mode&IPC_NOWAIT){
                unlock_sem(_sem);
                return -EAGAIN;
            }
            _sem->semzcnt++;
            unlock_sem(_sem);
            //挂起这个进程
            struct sem_queue sem_q={semoparray[i].sem_num,CUR_TASK,NULL};
            sem_add_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);
            task_suspend();
            task_sche();
            task_run();
            sem_remove_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);

            //信号被删除了，则返回ERMID
            if(atomic_test(&semid_ds_used[semid],0)){
                return -ERMID;
            }
            //有信号，返回EINTR
            if(CUR_TASK->sig_bmp[0]
               ||CUR_TASK->sig_bmp[1]){
                _sem->semzcnt--;
                return -EINTR;
            }
            goto again_0;
        } if(semoparray[i].sem_op<0){
            again_get:
            lock_sem(_sem);
            //大于绝对值
            //下面应该原子操作的
            //锁住当前的信号集
            if(_sem->semval>=ABS(semoparray[i].sem_op)) {
                //则直接进行操作SOFTFPU
                if (semoparray->sem_flg & SEM_UNDO) {
                    _sem->semval += ABS(semoparray[i].sem_op);
                } else {
                    _sem->semval -= ABS(semoparray[i].sem_op);
                }
            }else{
                //反之说明资源不够
                if(semid_ds_ls[semid].sem_perm.mode&IPC_NOWAIT){
                    unlock_sem(_sem);
                    return -EAGAIN;
                }
                //要挂其进程了
                _sem->semncnt++;
                unlock_sem(_sem);
                //挂起这个进程
                struct sem_queue sem_q={semoparray[i].sem_num,CUR_TASK,NULL};
                sem_add_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);
                task_suspend();
                task_sche();
                task_run();
                sem_remove_wait_queue(&semid_ds_ls[semid].sem_pending,&sem_q);

                //信号量被删除了，则返回ERMID
                if(atomic_test(&semid_ds_used[semid],0)){
                    return -ERMID;
                }
                //有信号，返回EINTR
                if(CUR_TASK->sig_bmp[0]
                   ||CUR_TASK->sig_bmp[1]){
                    _sem->semncnt--;
                    return -EINTR;
                }
                goto again_get;
            }
        }
    }
    return 0;
}

