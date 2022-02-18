//
// Created by Administrator on 2022/2/11/011.
//

#include <mkrtos/fs.h>
#include <mkrtos/dev.h>
#include <stdio.h>
#include <mkrtos/stat.h>
//�����ⲿ����������
extern int sys_mknod(const char * filename, int mode, dev_t dev);
extern int sys_mkdir(const char * pathname, int mode);

void fs_init(void){
    //�����豸Ŀ¼
    sys_mkdir("/dev",0777);
    sys_mkdir("/mnt",0777);
    //null�豸
    sys_mknod("/dev/null",MK_MODE(S_IFCHR,0777),MKDEV(NULL_MAJOR,0));
    //���������ն��豸
    sys_mknod("/dev/tty0",MK_MODE(S_IFCHR,0777),MKDEV(TTY_MAJOR,0));
    sys_mknod("/dev/tty1",MK_MODE(S_IFCHR,0777),MKDEV(TTY_MAJOR,1));

    sys_mknod("/dev/tty",MK_MODE(S_IFCHR,0777),MKDEV(TTYMAUX_MAJOR,0));
}

