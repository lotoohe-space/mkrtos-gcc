//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <mkrtos/fs.h>
#include "mkrtos/task.h"
#include "bsp/delay.h"
#include "mkrtos/mem.h"
#include "sys/sem.h"
#include <mkrtos/stat.h>
//#include "mkrtos/signal.h"
#include <xprintf.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
extern int32_t sys_open(const char* path,int32_t flags,int32_t mode);
extern int sys_readdir(unsigned int fd, struct dirent * dirent, uint32_t count);
extern void sys_close(int fp);
extern int sys_mkdir(const char * pathname, int mode);
int sys_write (int fd,uint8_t *buf,uint32_t len);
int sys_read (int fd,uint8_t *buf,uint32_t len);
int sys_lseek(unsigned int fd, int32_t ofs, uint32_t origin);
int32_t sys_stat(char * filename, struct new_stat * statbuf);
int sys_link(const char * oldname, const char * newname);
struct dirent dir;
int32_t sys_mount(char * dev_name, char * dir_name, char * type,
                  unsigned long new_flags, void * data);
uint8_t w_data[512]={0};
uint8_t r_data[512]={0};
void KernelTask(void*arg0, void*arg1){
    int fd,fd1;
    int res;
    printf("请输入数据:\r\n");
    int a;
    int b;
    //scanf还有问题
    scanf("%d,%d",&a,&b);
    printf("\r\n输入的数据是:%d,%d\r\n",a,b);
    sys_write(0,"kernel run..\r\n",strlen("kernel run..\r\n"));

    sys_mkdir("/test",0777);
    sys_mkdir("/mnt",0777);
    sys_mount("/dev/flash","/mnt","spFS",0,0);


    if((fd=sys_open("/test.txt",O_RDWR|O_CREAT|O_APPEND,0777))<0){
        while(1);
    }
    sys_close(fd);
//    if((fd1=sys_open("/test1.txt",O_RDWR|O_CREAT|O_APPEND,0777))<0){
//        while(1);
//    }
//    sys_close(fd1);
    sys_link("/test.txt","/test1.txt");
    if((fd=sys_open("/test1.txt",O_RDWR|O_CREAT|O_APPEND,0777))<0){
        while(1);
    }
    for(uint32_t i=0;i<sizeof (w_data);i++){
        w_data[i]=(uint8_t)i;
    }
    for(uint32_t i=0;i<10;i++){
        if((res=sys_write(fd,w_data,sizeof(w_data)))<0){
            printk("发生错误,%d\r\n",res);
            break;
        }else{

        }
    }
    struct new_stat st;
    sys_stat("/test.txt",&st);
    printk("写入数据 %d,文件大小 %d\r\n",res,st.st_size);
    sys_lseek(fd, 0, 0);
    int r_len=0;
    while((res=sys_read(fd,r_data,sizeof(r_data)))>0){
        for(uint32_t i=0;i<sizeof (w_data);i++){
            if(w_data[i]!=r_data[i]){
                printk("写存在错误\r\n");
                goto end;
            }
        }
        r_len+=res;
    }
    end:
    printk("读取长度%d\r\n",r_len);

    sys_close(fd);
    printk("file write ok!\r\n");
    while(1){

    }
}
void KernelTask1(void*arg0, void*arg1){
    int fd;
    int res;
//    uint8_t data[32];
//    printk("kernel task start..\r\n");
//    if((fd=sys_open("/",O_RDONLY,0777))<0){
//        while(1);
//    }
//    int i=10;
//    while(i){
//        sprintf(data,"/mkrtos%d",i);
//        sys_mkdir(data,0777);
//        i--;
//    }
//    while(res>0) {
//        if ((res=sys_readdir(fd, &dir, sizeof(dir)))<= 0) {
//            break;
//        }
//        printk("%s\r\n",dir.d_name);
//    }
//    sys_close(fd);
    if((fd=sys_open("/test.txt",O_RDWR|O_CREAT|O_APPEND,0777))<0){
        while(1);
    }
    int i=30;
    uint8_t data[32];
    while(i){
        sprintf(data,"222%d\r\n",CUR_TASK->PID);
        sys_write(fd,data,strlen(data));
        delay_ms(10);
//        printk("write %s.\r\n",data);
        i--;
    }
    delay_ms(1000);
    sys_lseek(fd, 0, 0);

    int r_len=0;
    while((res=sys_read(fd,data,sizeof(data)-1))>0){
        data[32-1]=0;
        printk(data);
    }


    sys_close(fd);
    printk("file write ok!\r\n");

    while(1){

    }
}

//调用call_sigreturn，这里面会调用系统调用sigreturn完成用户栈恢复
extern void call_sigreturn(void);
//_syscall3(int,signal,int32_t,signum, int32_t,handler, int32_t,restorer);
//
//_syscall1(int,alarm,uint32_t,secs);
void SignalFunc(int signer){
    switch(signer){
        case SIGALRM:
//            alarm(1);
//            signal(SIGALRM,SignalFunc,call_sigreturn);
            break;
    }
}

void TestTask(void*arg0, void*arg1){

//    sysTasks.currentTask->signalBMap=(1<<(3-1));
//    sys_signal(3,SignalFunc,0);
//    signal(SIGALRM,SignalFunc,call_sigreturn);
//    alarm(5);
    while(1){

    }
}
#include <sys/wait.h>
void rc_shell_exec(void){
   int res= fork();
   if(res<0){
        printf("error exec rc shell.\r\n");
   }else if(res==0){
       extern int rc_main(int argc, char *argv[], char *envp[]) ;
       static char * argv[]={
               "./rcshell"
               ,"-l"
               ,NULL
       };
       static char *env[]={
               {"PATH=/"}
               ,NULL
       };
       rc_main(2,argv,env);
       while(1);
   }else if(res>0){
       wait(0);
   }
}
void sig_test_fuN(int signo){
    inner_set_sig(SIGHUP);
    delay_ms(200);
    printf("接收到了信号：%d\r\n",signo);
}
int init_sem(int sem_id,int init_value){
    union semun sem_union;
    sem_union.val=init_value;
    if(semctl(sem_id,0,SETVAL,sem_union)<0){
        printf("Init semaphore");
        return -1;
    }
    return 0;
}
int del_sem(int sem_id){
    union semun sem_union;
    if(semctl(sem_id,0,IPC_RMID,sem_union)<0){
        printf("Delete semaphore");
        return -1;
    }
    return 0;
}
int sem_p(int sem_id){
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=-1;
    sem_b.sem_flg=SEM_UNDO;
    if(semop(sem_id,&sem_b,1)<0){
        printf("P operation");
        return -1;
    }
    return 0;
}
int sem_v(int sem_id){
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=1;
    sem_b.sem_flg=SEM_UNDO;
    if(semop(sem_id,&sem_b,1)<0){
        printf("V operation");
        return -1;
    }
    return 0;
}
void user_task(void* arg0,void *arg1){
    int ret;
#if 0
    char line[32];
    int fd[2];
    int n;
    if(pipe(fd)<0){
        printf("pipe crate fail.\r\n");
    }
    ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        close(fd[0]);
        write(fd[1],"hello world\n",13);
        delay_ms(1000);
    }else {
        close(fd[1]);
        n=read(fd[0],line,5);
        line[n]=0;
        printf("len:%d,data:",n,line);
        n=read(fd[0],line,5);
        line[n]=0;
        printf("len:%d,data:",n,line);
        n=read(fd[0],line,5);
        line[n]=0;
        printf("len:%d,data:",n,line);
    }
    exit(0);
#endif

#if 0
    again_fork:
    ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        int fd[2];
        pipe(fd);
        delay_ms(100);
        printf("%d remain memory is %d.\r\n",getpid(),GetFreeMemory(1));
        exit(0);
    }else {

        wait(0);
        delay_ms(500);
        printf("%d remain memory is %d.\r\n",getpid(),GetFreeMemory(1));
        goto again_fork;
    }
#endif

#if 1
    int sem_id;
    sem_id=semget(123,1,IPC_CREAT);
    if(sem_id<0){
        printf("sem creating is error.\r\n");
    }
    init_sem(sem_id,0);
    ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        printf("child will wait 3S.\r\n");
        delay_ms(3000);
        printf("child PID is %d.\r\n",getpid());
        sem_v(sem_id);
    }else {
        sem_p(sem_id);
        printf("father pid is %d.\r\n");
        sem_v(sem_id);
        del_sem(sem_id);
    }
#endif
}
//启动进程
void start_task(void* arg0,void*arg1){
    extern void fs_init(void);
    root_mount(CUR_TASK);
    fs_init();
    devs_init();
    //打开三个串口输出
    open("/dev/tty", O_RDWR, 0777);
    open("/dev/tty", O_RDWR, 0777);
    open("/dev/tty", O_RDWR, 0777);

    printf("to init task.\r\n");
    printf("%d remain memory is %d.\n",getpid(),GetFreeMemory(1));
#if 1
//    int fd[2];
//    int n;
//    if(pipe(fd)<0){
//        printf("pipe crate fail.\r\n");
//    }
    int ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        user_task(0,0);
    }else {
        while(1){
           // delay_ms(1000);
           // printf("remain memory is %d.\n",GetFreeMemory(1));
        }
    }
#endif
#if 0
    int ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        while(1){
            rc_shell_exec();
        }
    }else {
       while(1);
    }
#endif
}
void idle_task(void){
    while(1);
}
#include <setjmp.h>
/**
 * @brief 系统空闲任务
 */
void KernelTaskInit(void){
    int res=0;
    extern int32_t sp_mkfs(dev_t dev_no,int32_t inode_count);
    extern int32_t bk_flash_init(void);
//    sigjmp_buf buf;
//    res=setjmp(buf);
//    longjmp(buf,1);
    //初始化默认的磁盘设备
    bk_flash_init();
    //在这里格式化文件系统
    if(sp_mkfs(root_dev_no,30)<0){
        fatalk("根文件系统创建失败！\r\n");
    }
    //下面创建内核线程
    static TaskCreatePar tcp;
    int32_t pid;
    tcp.taskFun=start_task;
    tcp.arg0=(void*)0;
    tcp.arg1=0;
    tcp.prio=6;
    tcp.userStackSize=1024;
    tcp.kernelStackSize=512;
    tcp.taskName="init";

    pid=task_create(&tcp,NULL);
    if(pid<0){
        while(1);
    }
}