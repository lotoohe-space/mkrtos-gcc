//
// Created by Administrator on 2022/2/11/011.
//

#include <mkrtos/fs.h>
#include <mkrtos/dev.h>
#include <stdio.h>
#include <mkrtos/stat.h>
#include "fcntl.h"
#include "paths.h"

//导入外部的两个函数
extern int sys_mknod(const char * filename, int mode, dev_t dev);
extern int sys_mkdir(const char * pathname, int mode);
int32_t sys_open(const char* path,int32_t flags,int32_t mode);
int sys_write (int fd,uint8_t *buf,uint32_t len);
int sys_readdir(unsigned int fd, struct dirent * dirent, uint32_t count);
void sys_close(int fp);
int sys_read (int fd,uint8_t *buf,uint32_t len);

void fs_init(void){
    //创建设备目录
    sys_mkdir("/dev",0777);
    sys_mkdir("/mnt",0777);
    sys_mkdir("/bin",0777);
    sys_mkdir("/etc",0777);
    //null设备
    sys_mknod(_PATH_DEVNULL,MK_MODE(S_IFCHR,0777),MKDEV(NULL_MAJOR,0));
    //创建两个终端设备
    sys_mknod("/dev/tty0",MK_MODE(S_IFCHR,0777),MKDEV(TTY_MAJOR,0));
    sys_mknod("/dev/tty1",MK_MODE(S_IFCHR,0777),MKDEV(TTY_MAJOR,1));

    sys_mknod("/dev/tty",MK_MODE(S_IFCHR,0777),MKDEV(TTYMAUX_MAJOR,0));
    sys_mknod(_PATH_CONSOLE,MK_MODE(S_IFCHR,0777),MKDEV(TTYMAUX_MAJOR,0));
    sys_mknod("/dev/mmc",MK_MODE(S_IFBLK,0777),MKDEV(MMC_MAJOR,0));

    int res;
    res=sys_open(_PATH_PASSWD,O_RDWR|O_CREAT,0777);
    if(res>=0){
        static const char* tmp="root:root:0:0:null:/root:/bin/zsh";
        sys_write(res,tmp,strlen(tmp));
        sys_close(res);
    }
    res=sys_open(_PATH_GROUP,O_RDWR|O_CREAT,0777);
    if(res>=0){
        static const char* tmp="root:x:0:root";
        sys_write(res,tmp,strlen(tmp));
        sys_close(res);
    }
}

