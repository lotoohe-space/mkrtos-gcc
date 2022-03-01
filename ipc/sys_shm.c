//
// Created by Administrator on 2022/3/1.
//

#include <sys/shm.h>
#include <arch/atomic.h>
#include <errno.h>
#include <mkrtos/mem.h>
#include <mkrtos/task.h>
#define SHM_NUM 4

static struct shmid_ds shmid_ds_ls[SHM_NUM]={0};
static Atomic_t shmid_used[SHM_NUM]={0};
static Atomic_t shms_lock[SHM_NUM]={0};
static Atomic_t shm_list_lk={0};

static void lock_ls(void){
    while(!atomic_test_set(&shm_list_lk,1));
}
static void unlock_ls(void){
    atomic_set(&shm_list_lk,0);
}
static void lock_shm(int shm_id){
    while(!atomic_test_set(&shms_lock[shm_id],1));
}
static void unlock_shm(int shm_id){
    atomic_set(&shms_lock[shm_id],0);
}
static int32_t inner_find_shm(key_t key){
    int i;

    lock_ls();
    for(i=0;i<SHM_NUM;i++){
        if (!atomic_test(&shmid_used[i],0)) {
            if (shmid_ds_ls[i].shm_perm.key == key) {
                unlock_ls();
                return i;
            }
        }
    }
    unlock_ls();

    return -1;
}

static int32_t inner_shm_creat(key_t key,int size,int flag){
    int i;

    lock_ls();
    for(i=0;i<SHM_NUM;i++){
        if (atomic_test_set(&shmid_used[i],1)) {
            break;
        }
    }
    unlock_ls();
    if(i==SHM_NUM) {
        return -ENOENT;
    }
    shmid_ds_ls[i].shm_unused2=OSMalloc(size);
    if(!shmid_ds_ls[i].shm_unused2) {
        atomic_set(&shmid_used[i],0);
        return -ENOMEM;
    }
    shmid_ds_ls[i].shm_perm.cgid=CUR_TASK->egid;
    shmid_ds_ls[i].shm_perm.cuid=CUR_TASK->euid;
    shmid_ds_ls[i].shm_perm.gid=CUR_TASK->rgid;
    shmid_ds_ls[i].shm_perm.uid=CUR_TASK->ruid;
    shmid_ds_ls[i].shm_perm.key=key;
    shmid_ds_ls[i].shm_perm.mode=flag;
    shmid_ds_ls[i].shm_perm.seq=0;

    shmid_ds_ls[i].shm_segsz=size;
    shmid_ds_ls[i].shm_atime=0;
    shmid_ds_ls[i].shm_dtime=0;
    shmid_ds_ls[i].shm_ctime=0;
    shmid_ds_ls[i].shm_cpid=CUR_TASK->PID;
    shmid_ds_ls[i].shm_nattch=0;
    shmid_ds_ls[i].shm_unused=0;
    shmid_ds_ls[i].shm_unused3=0;

    return i;
}
int sys_shmget(key_t key,int size,int flag){
    int id;
    if(key==IPC_PRIVATE){
        id=inner_shm_creat(key,size,flag);
        if(id<0) {
            return id;
        }
    }else{
        id=inner_find_shm(key);
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
                    id=inner_shm_creat(key,size,flag);
                    if(id<0) {
                        return id;
                    }
                    return 0;
                }
                if(id>=0){
                    return id;
                }
                id=inner_shm_creat(key,size,flag);
                return id;
            }
            return id;
        }
    }
    return -ENOMEM;
}
int sys_shmctl(int shmid,int cmd,struct shmid_ds *buf){
    int i;
    struct shmid_ds *msq;
    if(shmid<0||shmid>=SHM_NUM){
        return -1;
    }
    if(atomic_test(&shmid_used[shmid],0)){
        return -EIDRM;
    }
    msq=&shmid_ds_ls[shmid];
    switch (cmd){
        case IPC_STAT:
            lock_shm(shmid);
            memcpy(buf,&shmid_ds_ls[shmid],sizeof(struct shmid_ds));
            unlock_shm(shmid);
            break;
        case IPC_SET:
            if(CUR_TASK->euid==msq->shm_perm.cuid
               ||CUR_TASK->euid==msq->shm_perm.uid
               ||CUR_TASK->is_s_user
                    ){
                lock_shm(shmid);
                msq->shm_perm.uid=buf->shm_perm.uid;
                msq->shm_perm.gid=buf->shm_perm.gid;
                msq->shm_perm.mode=buf->shm_perm.mode;
                unlock_shm(shmid);
            }
            break;
        case IPC_RMID:
            if(CUR_TASK->euid==msq->shm_perm.cuid
               ||CUR_TASK->euid==msq->shm_perm.uid
               ||CUR_TASK->is_s_user
                    ) {
                lock_shm(shmid);
                if (!shmid_ds_ls[shmid].shm_nattch) {
                    atomic_test(&shmid_used[shmid], 0);
                    OSFree(shmid_ds_ls[shmid].shm_unused2);
                }
                unlock_shm(shmid);
            }
            break;
        case SHM_LOCK://为了兼容，返回0
            return 0;
        case SHM_UNLOCK://为了兼容，返回0
            return 0;
    }
    return 0;
}
int sys_shmat(int shmid,int flag,void** r_addr,void* addr){
    struct shmid_ds *shm_ds;
    if(shmid<0||shmid>=SHM_NUM){
        return -EINVAL;
    }
    if(atomic_test(&shmid_used[shmid],0)){
        return -EIDRM;
    }
    shm_ds=&shmid_ds_ls[shmid];
    lock_shm(shmid);
    shm_ds->shm_nattch++;
    unlock_shm(shmid);
    *r_addr=shm_ds->shm_unused2;
    //本来应该按照addr的地址进行映射，但是没有mmu的设备，并不支持该功能，所以下面直接返回就行了
    return 0;
}
int sys_shmdt(void* addr){

    for(int i=0;i<SHM_NUM;i++){
        lock_shm(i);
        if(atomic_test(&shmid_used[i],1)
        &&shmid_ds_ls[i].shm_unused2==addr){
            if(shmid_ds_ls[i].shm_nattch>0){
                shmid_ds_ls[i].shm_nattch--;
                unlock_shm(i);
                break;
            }
        }
        unlock_shm(i);
    }
    return 0;
}
