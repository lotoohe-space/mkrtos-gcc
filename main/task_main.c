//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <mkrtos/fs.h>
#include "mkrtos/task.h"
#include "bsp/delay.h"
#include "mkrtos/mem.h"
#include "sys/sem.h"
#include "sys/msg.h"
#include "sys/shm.h"
#include "dirent.h"
#include "app/sysent.h"
#include <mkrtos/stat.h>
//#include "mkrtos/signal.h"
#include <xprintf.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mkrtos/user.h"
#include "paths.h"
#include "sched.h"

int32_t sys_open(const char* path,int32_t flags,int32_t mode);
int sys_write (int fd,uint8_t *buf,uint32_t len);
int sys_readdir(unsigned int fd, struct dirent * dirent, uint32_t count);
void sys_close(int fp);
int sys_mkdir(const char * pathname, int mode);

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
       exit(1);
//for(int i=0;i<100;i++){
//    malloc(1000);
//}
//printf("child run.\r\n");
//        exit(1);
   }else if(res>0){
       while(wait(0)>=0);
       printf("run exit.\n");
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
#define TEXT_SZ 32
struct shared_use_st
{
    int written;//作为一个标志，非0：表示可读，0表示可写
    char text[TEXT_SZ];//记录写入和读取的文本
};
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

#if 0
    struct timeval t_val1,t_val2;
    gettimeofday(&t_val1,NULL);
    sleep(5);
    gettimeofday(&t_val2,NULL);
    printf("%d\r\n",t_val2.tv_sec*1000*10000+t_val2.tv_usec-t_val1.tv_sec*1000*10000+t_val1.tv_usec);
#endif

#if 0
    time_t tim;
    while(1) {
        time(&tim);
        struct timeval t_val;
        gettimeofday(&t_val,NULL);
        printf("%s", asctime(gmtime(&tim)));
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec);
        sleep(1);
        void * mem=malloc(100);
        printf("0x%x remain mem %d\r\n",mem,GetFreeMemory(1));
//        struct timespec tims;
//        tims.tv_sec=0;
//        tims.tv_nsec=10000000;
//        nanosleep(&tims,NULL);
    }
#endif
#if 0
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
        sleep(3);
        printf("child PID is %d.\r\n",getpid());
        sem_v(sem_id);
    }else {
        sem_p(sem_id);
        printf("father pid is %d.\r\n");
        sem_v(sem_id);
        del_sem(sem_id);
    }
#endif
#if 1

//    int res;
//    int len;
//    int j=0;
//    res=sys_open("./test.elf",O_RDWR,0777);
//    while(( len=sys_read(res,read_test,sizeof(read_test)))>=0){
//        for(int i=0;i<len;i++){
//            if(read_test[i]!=app_test[i+j]){
//                printf("read test error.\n");
//            }
//        }
//        j+=len;
//    }
//    printf("read len of data is %d.\n");
//    sys_close(res);

//    localtime
//    time(NULL);
    char *path=getenv("PATH");
    int res= fork();
    if(res<0){
        printf("error exec rc shell.\r\n");
    }else if(res==0) {

        execvp("/bin/zsh",path);
        exit(0);
//        DIR				*dp;
//        struct dirent	*dirp;
//
//        if ((dp = opendir("./dev")) == NULL) {
//            printf("to open %s is fail.\n","./dev");
//            exit(-1);
//        }
//        putchar('\n');
//        while ((dirp = readdir(dp)) != NULL) {
//            printf("%s\t", dirp->d_name);
//        }
//        putchar('\n');
//        closedir(dp);
//        exit(0);

        static const char *argv[] = {
                "./ls",
                "./dev",
                NULL
        };
        execv("./ls", argv);
//        printf("printf test.\n");
//        printk("remain memory size is %d.\r\n",GetFreeMemory(1));
        exit(0);


//#include "loader.h"
//#include "loader_config.h"
//        static const sysent_t sysentries = { /* */
//                open, /* */
//                close, /* */
//                write, /* */
//                read, /* */
//                printf, /* */
//                scanf /* */
//        };
//
//        static const ELFSymbol_t exports[] = { { "syscalls", (void*) &sysentries } };
//        static const ELFEnv_t env = { exports, sizeof(exports) / sizeof(*exports) };
//
//        ELFExec_t *e;
//        loader_env_t loader_env;
//        loader_env.env = &env;
//        ret=load_elf("./ls", loader_env, &e);
//        if(ret<0) {
//            exit(-1);
//        }
//        static const char *argv[] = {
//                "./ls",
//                "./dev",
//                NULL
//        };
//        entry_t *entry = (entry_t*) (e->text.data + e->entry);
//        entry(2,argv,NULL);
//        unload_elf(e);
//        printk("remain memory size is %d.\r\n",GetFreeMemory(1));

//        DIR				*dp;
//        struct dirent	*dirp;
//
//        if ((dp = opendir("./")) == NULL) {
//            printf("can't open %s", "./");
//        }
//        while ((dirp = readdir(dp)) != NULL)
//            printf("%s\t", dirp->d_name);
//
//        closedir(dp);
        exit(1);
    }else{
//        printk("remain memory size is %d.\r\n",GetFreeMemory(1));
        while(wait(0)>=0);
//        printk("remain memory size is %d.\r\n",GetFreeMemory(1));

    }
//    int elf_main(void) ;
//    elf_main();
#endif
#if 0
    ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        int running = 1;//程序是否继续运行的标志
        void *shm = NULL;//分配的共享内存的原始首地址
        struct shared_use_st *shared;//指向shm
        int shmid;//共享内存标识符 //创建共享内存
        shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);
        if(shmid == -1)
        {
            fprintf(stderr, "shmget failed\n");
            exit(EXIT_FAILURE);
        }   //将共享内存连接到当前进程的地址空间
        shm = shmat(shmid, 0, 0);
        if(shm == (void*)-1)
        {
            fprintf(stderr, "shmat failed\n");
            exit(EXIT_FAILURE);
        }
        printf("\nMemory attached at %X\n", (int)shm);  //设置共享内存
        shared = (struct shared_use_st*)shm;
        shared->written = 0;
        while(running)//读取共享内存中的数据
        {       //没有进程向共享内存定数据有数据可读取
            if(shared->written != 0)
            {
                printf("You wrote: %s", shared->text);
                sleep(rand() % 3);          //读取完数据，设置written使共享内存段可写
                shared->written = 0;         //输入了end，退出循环（程序）
                if(strncmp(shared->text, "end", 3) == 0)
                    running = 0;
            }
            else//有其他进程在写数据，不能读取数据
                sleep(1);
        }   //把共享内存从当前进程中分离
        if(shmdt(shm) == -1)
        {
            fprintf(stderr, "shmdt failed\n");
            exit(EXIT_FAILURE);
        }   //删除共享内存
        if(shmctl(shmid, IPC_RMID, 0) == -1)
        {
            fprintf(stderr, "shmctl(IPC_RMID) failed\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }else {
        int running = 1;
        void *shm = NULL;
        struct shared_use_st *shared = NULL;
        char buffer[BUFSIZ + 1];//用于保存输入的文本
        int shmid;  //创建共享内存
        shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);
        if(shmid == -1)
        {
            fprintf(stderr, "shmget failed\n");
            exit(EXIT_FAILURE);
        }   //将共享内存连接到当前进程的地址空间
        shm = shmat(shmid, (void*)0, 0);
        if(shm == (void*)-1)
        {
            fprintf(stderr, "shmat failed\n");
            exit(EXIT_FAILURE);
        }
        printf("Memory attached at %X\n", (int)shm);    //设置共享内存
        shared = (struct shared_use_st*)shm;
        while(running)//向共享内存中写数据
        {       //数据还没有被读取，则等待数据被读取,不能向共享内存中写入文本
            while(shared->written == 1)
            {
                sleep(1);
                printf("Waiting...\n");
            }       //向共享内存中写入数据
            printf("Enter some text: ");
            fgets(buffer, BUFSIZ, stdin);
            strncpy(shared->text, buffer, TEXT_SZ);      //写完数据，设置written使共享内存段可读
            shared->written = 1;     //输入了end，退出循环（程序）
            if(strncmp(buffer, "end", 3) == 0) {
                running = 0;
            }
        }   //把共享内存从当前进程中分离
        if(shmdt(shm) == -1)
        {
            fprintf(stderr, "shmdt failed\n");
            exit(EXIT_FAILURE);
        }
        sleep(2);
        exit(EXIT_SUCCESS);
    }
#endif
#if 0
//    ftok
    int mqid;
    struct msgbuf {
        long mtype;     /* message type, must be > 0 */
        char mtext[32];  /* message data */
    }msg;
    struct timeval t_val;
#define SNDMSG 1
#define RCVMSG 2
    mqid = msgget(123, IPC_CREAT | IPC_EXCL | 0666);
    if (mqid == -1) {
        printf("msgget error: ");
        exit(-1);
    }
    ret=fork();
    if(ret<0){
        printf("init create error.\r\n");
    }else if(ret==0){
        key_t mqkey;

        printf("Lucy: ");
        fgets(msg.mtext, 256, stdin);
        if (strncmp("quit", msg.mtext, 4) == 0) {
            msgctl(mqid, IPC_RMID, NULL);
            exit(0);
        }
        msg.mtext[strlen(msg.mtext)-1] = '\0';
        msg.mtype = SNDMSG;
        gettimeofday(&t_val,NULL);
        msgsnd(mqid, &msg, strlen(msg.mtext) + 1, 0);
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec/1000);
        gettimeofday(&t_val,NULL);
        msgsnd(mqid, &msg, strlen(msg.mtext) + 1, 0);
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec/1000);
        gettimeofday(&t_val,NULL);
        msgsnd(mqid, &msg, strlen(msg.mtext) + 1, 0);
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec/1000);
        msgrcv(mqid, &msg, 256, RCVMSG, 0);
        printf("Peter talk: %s\n", msg.mtext);
//        printf("%d exit.\n",getpid());
    }else {
        msgrcv(mqid, &msg, 256, SNDMSG, 0);
        gettimeofday(&t_val,NULL);
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec/1000);
        msgrcv(mqid, &msg, 256, SNDMSG, 0);
        gettimeofday(&t_val,NULL);
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec/1000);
        msgrcv(mqid, &msg, 256, SNDMSG, 0);
        gettimeofday(&t_val,NULL);
        printf("%d %d\r\n",t_val.tv_sec,t_val.tv_usec/1000);

        printf("Lucy talk: %s\n", msg.mtext);
        printf("Peter: ");
        fgets(msg.mtext, 256, stdin);
        if (strncmp("quit", msg.mtext, 4) == 0) {
            exit(0);
        }
        msg.mtext[strlen(msg.mtext)-1] = '\0';
        msg.mtype = RCVMSG;
        msgsnd(mqid, &msg, strlen(msg.mtext) + 1, 0);
//        printf("%d exit.\n",getpid());
    }
#endif
}

int thread_fn(void*arg){

  //  while(1){
        sleep(1);
        printf("thread test\n");
   // }
    return 0;
}
void* myThreadID1(void* arg){
    //printf("thread_ID1\n");
    printk("knl thread test.\r\n");
    while(1);
}
void* myThreadID2(void* arg){
    //printf("thread_ID2\n");
}
#include <bsp/net_init.h>
#include <arch/arch.h>
//启动进程
void start_task(void* arg0,void*arg1){

//    int *ptr=malloc(100);
//    free(ptr);
//    static int thread_test[512];
//    clone(thread_fn,&thread_test[512-1],CLONE_VM|CLONE_FILES,NULL);

    int ret;

    ret=fork();
    if(ret<0){
        printf("init create error.\n");
    }else if(ret==0){
        setup();

#include <sys/mount.h>
        mkdir("/mnt/mmc",0777);
        int ret;
//        ret=mount("/dev/mmc", "/mnt/mmc", "sp",0, 0);
//        if(ret<0){
//            printf("挂载失败\n");
//        }
        ret=mount("/dev/mmc", "/bin", "sp",0, 0);
        if(ret<0){
            printf("挂载失败\n");
        }
        //打开三个串口输出
        open("/dev/tty", O_RDWR, 0777);
        open("/dev/tty", O_RDWR, 0777);
        open("/dev/tty", O_RDWR, 0777);

        printf("to init task.\r\n");
        printf("%d remain memory is %d.\n",getpid(),GetFreeMemory(1));

        extern void fs_w_zshell(void);
        fs_w_zshell();
        extern void fs_w_start_info(void);
        fs_w_start_info();




        extern void fs_w_clear(void);
        fs_w_clear();

        extern void fs_w_uname(void);
        fs_w_uname();

        extern void fs_w_ls_(void);
        fs_w_ls_();
#if 0
        extern void fs_w_ls(void);
        fs_w_ls();
        extern void fs_w_mkdir(void);
        fs_w_mkdir();
        extern void fs_w_cat(void);
        fs_w_cat();
#endif
        sync();
#include <test/test.h>
        fs_test();
        putenv("SHELL=/bin/zsh");
#if 0
#include <pthread.h>
        pthread_t id1,id2;
        ret= pthread_create(&id1,NULL,myThreadID1,NULL);
        if(ret<0){
            printf("Create pthread error!\n");

        }
        ret=pthread_create(&id2,NULL,myThreadID2,NULL);
        if(ret<0){
            printf("Create pthread error!\n");
        }
        pthread_join(id1,NULL);
        pthread_join(id2,NULL);
        printf("all thread done!\n");
//        extern int login_main(int argc,char *argv[]) ;
//        static const char* argv[]={
//                "root"
//                ,NULL
//        };
//        login_main(1,argv);
#endif
        extern int login(void);
        login();
        while(1) {
            int new_pid=fork();
            if(new_pid<0) {
                printf("fork error. pid:%d\n",new_pid);
            }else if(new_pid==0){
                    execv("/bin/start_info",0);
            }else{
                waitpid(new_pid,0,0);
                user_task(0,0);
            }
        }
    }else {
         nice(1);
         while(1){}
    }
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

#include <setjmp.h>
/**
 * @brief 系统空闲任务
 */
void KernelTaskInit(void){
    int res=0;
    extern int32_t sp_mkfs(dev_t dev_no,int32_t inode_count);
    extern int32_t bk_flash_init(void);
    //初始化默认的磁盘设备
    bk_flash_init();
#if 1
    //在这里格式化文件系统
    if(sp_mkfs(root_dev_no,30)<0){
        fatalk("根文件系统创建失败！\r\n");
    }

#endif
    //下面创建内核线程
    static TaskCreatePar tcp;
    int32_t pid;
    tcp.taskFun=start_task;
    tcp.arg0=(void*)0;
    tcp.arg1=0;
    tcp.prio=6;
    tcp.userStackSize=512;
    tcp.kernelStackSize=638;
    tcp.taskName="init";

    pid=task_create(&tcp);
    if(pid<0){
        while(1);
    }
}
