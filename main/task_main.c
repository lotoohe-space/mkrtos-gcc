//
// Created by Administrator on 2022/1/9.
//
#define __LIBRARY__
#include <mkrtos/fs.h>
#include "mkrtos/task.h"
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
    printf("����������:\r\n");
    int a;
    int b;
    //scanf��������
    scanf("%d,%d",&a,&b);
    printf("\r\n�����������:%d,%d\r\n",a,b);
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
            printk("��������,%d\r\n",res);
            break;
        }else{

        }
    }
    struct new_stat st;
    sys_stat("/test.txt",&st);
    printk("д������ %d,�ļ���С %d\r\n",res,st.st_size);
    sys_lseek(fd, 0, 0);
    int r_len=0;
    while((res=sys_read(fd,r_data,sizeof(r_data)))>0){
        for(uint32_t i=0;i<sizeof (w_data);i++){
            if(w_data[i]!=r_data[i]){
                printk("д���ڴ���\r\n");
                goto end;
            }
        }
        r_len+=res;
    }
    end:
    printk("��ȡ����%d\r\n",r_len);

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

//����call_sigreturn������������ϵͳ����sigreturn����û�ջ�ָ�
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
//��������
void start_task(void* arg0,void*arg1){
    extern void fs_init(void);
    fs_init();
    //�����豸�ļ�
//    if(sys_mkdir("/dev",0777)<0){
//        fatalk("����devĿ¼ʧ�ܣ�\r\n");
//    }
    devs_init();
//    int fd;
//    int res;
//    if((fd=sys_open("/dev/tty0",O_RDONLY|O_WRONLY,0777))<0){
//        while(1);
//    }
//    res=sys_write(fd,"test\r\n",strlen("test\r\n"));
//    printk("write %d bytes.\r\n",res);
//    sys_close(fd);

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

    //

    while(1);
}
/**
 * @brief ϵͳ��������
 */
void KernelTaskInit(void){
    extern int32_t sp_mkfs(dev_t dev_no,int32_t inode_count);
    extern int32_t bk_flash_init(void);
    //��ʼ��Ĭ�ϵĴ����豸
    bk_flash_init();
    //�������ʽ���ļ�ϵͳ
    if(sp_mkfs(root_dev_no,30)<0){
        fatalk("���ļ�ϵͳ����ʧ�ܣ�\r\n");
    }
    //���洴���ں��߳�
    static TaskCreatePar tcp;
    int32_t pid;
    tcp.taskFun=start_task;
    tcp.arg0=(void*)0;
    tcp.arg1=0;
    tcp.prio=6;
    tcp.userStackSize=0;
    tcp.kernelStackSize=512;
    tcp.taskName="start_task";

    pid=task_create(&tcp,NULL);
    if(pid<0){
        while(1);
    }

}