//
// Created by Administrator on 2022/2/25.
//
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//ipc中的函數，只有通过get获取了id后，才能利用这个id操作，操作之前还得检测，当前进程是否调用get函数进行登记了。
//

//sys_sem.c
int32_t sys_semget(key_t key,int nsems,int flag);
int32_t sys_semctl(int semid,int semnum,int cmd,union semun arg);
int sys_semop(int semid,struct sembuf semoparray[],size_t ops);
//sys_msg.c
int sys_msgget(key_t key,int flag);
int sys_msgctl(int msgid,int cmd,struct msqid_ds *buf);
int sys_msgrcv(int msgid,void *ptr,size_t nbytes,long type,int flag);
int sys_msgsnd(int msgid,const void *ptr,size_t nbytes,int flag);

//sys_shm.c
int sys_shmget(key_t key,int size,int flag);
int sys_shmctl(int shmid,int cmd,struct shmid_ds *buf);
int sys_shmat(int shmid,int flag,void** r_addr,void* addr);
int sys_shmdt(void* addr);

struct ipc_kludge {
    struct msgbuf *msgp;
    long msgtyp;
};

/**
 * 系统的IPC调用
 * @param func
 * @param a
 * @param b
 * @param c
 * @param d
 * @return
 */
int sys_ipc(int func,int a,int b,int c,int d){
    switch(func){
        case SHMGET:
            return sys_shmget(a,b,c);
        case SHMCTL:
            return sys_shmctl(a,b,(void*)d);
        case SHMAT:
            return sys_shmat(a,b,(void**)c,(void*)d);
        case SHMDT:
            return sys_shmdt((void*)d);
        case MSGGET:
            return sys_msgget(a,b);
        case MSGCTL:
            return sys_msgctl(a,b,(void *)c);
        case MSGRCV:
            return sys_msgrcv(a,((struct ipc_kludge*)d)->msgp,b,((struct ipc_kludge*)d)->msgtyp,c);
        case MSGSND:
            return sys_msgsnd(a,(const void *)d,b,c);
        case SEMGET:
            return sys_semget(a,b,c);
        case SEMCTL:
            return sys_semctl(a,b,c,(union semun)d);
        case SEMOP:
            return sys_semop(a,(struct sembuf*)d,b);
    }
    return -ENOSYS;
}