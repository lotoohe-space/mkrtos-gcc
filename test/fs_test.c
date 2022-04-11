//
// Created by Administrator on 2022/4/7.
//

#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
static void fs_write_test(void*arg){
    int fd=open("/mnt/txt1",O_CREAT|O_RDWR,0);
    if(fd<0){
        printf("error fd is %d.\n",fd);
        exit(-1);
    }
    for(int i=0;i<100;i++) {
        int w_len;
        w_len = write(fd, arg, strlen(arg));
        if (w_len <= 0) {
            printf("write error is %d.\n", w_len);
        }
    }
    close(fd);
}
static char read_cache[128];
static void fs_read_test(void){
    int fd=open("/mnt/txt1",O_RDONLY,0);
    if(fd<0){
        printf("error fd is %d.\n",fd);
        exit(-1);
    }
    int r_len;
    while((r_len=read(fd,read_cache,sizeof(read_cache)-1))>0){
        read_cache[r_len-1]=0;
        printf("%s",read_cache);
    }
    close(fd);
}
static void new_proc(void(*fn)(void*arg),void* arg){
    int new_pid;
    new_pid=fork();
    if(new_pid<0){
        printf("fork error.");
    }else if(new_pid==0){
        fn(arg);
    }else if(new_pid){
        waitpid(new_pid,0,0);
    }
}
uint8_t rbuf[4096];
void fs_rw_test(const char *file_name,uint8_t *data,uint32_t len,int w_cn){
//#define RW_TEST_CN 20
    int rwlen=0;
    int fd;
    fd=open(file_name,O_CREAT|O_RDWR,0777);
    if(fd<0){
        printf("文件打开错误.\n");
        return ;
    }
    struct timeval tv;
    struct timeval tv1;
    gettimeofday(&tv,NULL);
    for(int i=0;i<w_cn;i++) {
        int ret=write(fd, data, len);
        if(ret<0){
            close(fd);
            printf("写入错误:%d\n",ret);
            return ;
        }
        rwlen+=ret;
        write(0,".",1);
    }
    close(fd);
    gettimeofday(&tv1,NULL);
    printf("单次写入数据长度:%d，总共写如:%d，写入费时:%dms.\n",sizeof(rbuf),rwlen,(tv1.tv_sec-tv.tv_sec)*1000+(tv1.tv_usec/1000-tv.tv_usec/1000));
    fd=open(file_name,O_RDONLY,0777);
    if(fd<0){
        printf("文件打开错误.\n");
        return ;
    }
    rwlen=0;
    gettimeofday(&tv,NULL);
    for(int i=0;i<w_cn;i++) {
        int ret;
        ret=read(fd, rbuf, sizeof(rbuf));
        rwlen +=ret;
        for (int i = 0; i < len; i++) {
            if (rbuf[i] != 0) {
                printf("读写数据存在错误.\n");
                break;
            }
        }
    }
    printf("读取长度:%d\n",rwlen);
    close(fd);
    gettimeofday(&tv1,NULL);
    printf("读取费时:%dms.\n",(tv1.tv_sec-tv.tv_sec)*1000+(tv1.tv_usec/1000-tv.tv_usec/1000));
    printf("文件读写测试成功.\n");
}
void fs_big_test(void){
    fs_rw_test("/bin/bigtxt.txt", rbuf, sizeof(rbuf),256);
}
//static const char * testw1;
//static const char * testw2;
//const uint8_t wbuf[4096];
void fs_test(void){
//    for(int i=0;i<sizeof(wbuf);i++){
//        wbuf[i]=i;
//    }
#if 0
    uint8_t file_name[32];
    for(int i=0;i<17;i++) {
        sprintf(file_name, "mnt/txt%d", i);
        fs_rw_test(file_name, wbuf, sizeof(wbuf),20);
    }
    for(int i=0;i<17;i++) {
        sprintf(file_name, "mnt/txt%d", i);
        unlink(file_name);
    }
#endif

#if 1
    fs_big_test();
#endif
//    testw1="12345678902345678901234567890\r\n";
//    testw2="qwertyuiopasdfghjklklzxcvbnm\r\n";
//    int new_pid;
//    new_pid=fork();
//    if(new_pid<0){
//        printf("fork error.");
//    }else if(new_pid==0){
//        new_proc(fs_write_test,testw1);
//    }else if(new_pid){
//        new_proc(fs_write_test,testw2);
//        waitpid(new_pid,0,0);
//    }
//    fs_read_test();
}

