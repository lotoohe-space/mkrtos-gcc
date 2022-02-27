//
// Created by Administrator on 2022/2/25.
//
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

extern int32_t sys_semget(key_t key,int nsems,int flag);
int32_t sys_semctl(int semid,int semnum,int cmd,union semun arg);
int sys_semop(int semid,struct sembuf semoparray[],size_t ops);

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
            break;
        case SEMGET:
            return sys_semget(a,b,c);
        case SEMCTL:
            return sys_semctl(a,b,c,(union semun)d);
        case SEMOP:
            return sys_semop(a,(struct sembuf*)d,b);
    }
    return -ENOSYS;
}