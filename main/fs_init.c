//
// Created by Administrator on 2022/2/11/011.
//

#include <mkrtos/fs.h>
#include <mkrtos/dev.h>
#include <stdio.h>
#include <mkrtos/stat.h>
//导入外部的两个函数
extern int sys_mknod(const char * filename, int mode, dev_t dev);
extern int sys_mkdir(const char * pathname, int mode);

void fs_init(void){
    //创建设备目录
    sys_mkdir("/dev",0777);
    sys_mkdir("/mnt",0777);
    //null设备
    sys_mknod("/dev/null",MK_MODE(S_IFCHR,0777),MKDEV(NULL_MAJOR,0));
    //创建两个终端设备
    sys_mknod("/dev/tty0",MK_MODE(S_IFCHR,0777),MKDEV(TTY_MAJOR,0));
    sys_mknod("/dev/tty1",MK_MODE(S_IFCHR,0777),MKDEV(TTY_MAJOR,1));

    sys_mknod("/dev/tty",MK_MODE(S_IFCHR,0777),MKDEV(TTYMAUX_MAJOR,0));
}

