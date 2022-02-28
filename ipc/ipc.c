//
// Created by Administrator on 2022/2/25.
//
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

//sys_sem.c
int32_t sys_semget(key_t key,int nsems,int flag);
int32_t sys_semctl(int semid,int semnum,int cmd,union semun arg);
int sys_semop(int semid,struct sembuf semoparray[],size_t ops);
//sys_msg.c
int sys_msgget(key_t key,int flag);
int sys_msgctl(int msgid,int cmd,struct msqid_ds *buf);
int sys_msgrcv(int msgid,void *ptr,size_t nbytes,long type,int flag);
int sys_msgsnd(int msgid,const void *ptr,size_t nbytes,int flag);

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
        case MSGGET:
            return sys_msgget(a,b);
        case MSGCTL:
            return sys_msgctl(a,b,(struct msqid_ds *)c);
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