//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <mkrtos/fs.h>
#include "mkrtos/task.h"
//#include "mkrtos/signal.h"
#include <unistd.h>
#include <fcntl.h>

extern int32_t sys_open(const char* path,int32_t flags,int32_t mode);
extern int sys_readdir(unsigned int fd, struct dirent * dirent, uint32_t count);
extern void sys_close(int fp);
extern int sys_mkdir(const char * pathname, int mode);
struct dirent dir;
void KernelTask(void*arg0, void*arg1){
    int fd;
    int res;
    if((fd=sys_open("/",O_RDONLY,0777))<0){
        while(1);
    }
    sys_mkdir("/zz",0777);
    if(sys_mkdir("/zz",0777)<0){
        sys_mkdir("/zz1",0777);
        sys_mkdir("/zz2",0777);
        sys_mkdir("/zz3",0777);
    }
    while(res>0) {
        if ((res=sys_readdir(fd, &dir, sizeof(dir)))<= 0) {
            break;
        }
    }
    sys_close(fd);


    while(1){

    }
}

//调用call_sigreturn，这里面会调用系统调用sigreturn完成用户栈恢复
extern void call_sigreturn(void);
_syscall3(int,signal,int32_t,signum, int32_t,handler, int32_t,restorer);

_syscall1(int,alarm,uint32_t,secs);
void SignalFunc(int signer){
    switch(signer){
        case SIGALRM:
            alarm(1);
            signal(SIGALRM,SignalFunc,call_sigreturn);
            break;
    }
}

void TestTask(void*arg0, void*arg1){

//    sysTasks.currentTask->signalBMap=(1<<(3-1));
//    sys_signal(3,SignalFunc,0);
    signal(SIGALRM,SignalFunc,call_sigreturn);
    alarm(5);
    while(1){

    }
}
/**
 * @brief 系统空闲任务
 */
void KernelTaskInit(void){

    devs_init();
    extern int32_t sp_mkfs(dev_t dev_no,int32_t inode_count);
    sp_mkfs(root_dev_no,30);
    static TaskCreatePar tcp;
    int32_t pid;
    tcp.taskFun=KernelTask;
    tcp.arg0=(void*)0;
    tcp.arg1=0;
    tcp.prio=6;
    tcp.userStackSize=0;
    tcp.kernelStackSize=512;
    tcp.taskName="KernelTask";

    pid=task_create(&tcp,NULL);
    if(pid<0){
        while(1);
    }
    tcp.taskFun=TestTask;
    tcp.arg0=(void*)0;
    tcp.arg1=0;
    tcp.prio=6;
    tcp.userStackSize=256;
    tcp.kernelStackSize=256;
    tcp.taskName="test";

    pid=task_create(&tcp,NULL);
    if(pid<0){
        while(1);
    }
}