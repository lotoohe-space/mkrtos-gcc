//
// Created by Administrator on 2022/2/18/018.
//
#include <mkrtos/dev.h>
#include <mkrtos/fs.h>
#include <string.h>
static int null_read(struct inode *ino, struct file *fp, char * buf, int count){
    memset(buf,0,count);
    return count;
}
static int null_write(struct inode *ino, struct file * fp, char * buf, int count){
    return count;
}
static struct file_operations null_ops={
        .read=null_read,
        .write=null_write
};

int null_init(void){
    if(reg_ch_dev(NULL_MAJOR,
                  "null",
                  &null_ops
    )<0){
        return -1;
    }
    return 0;
}
int null_close(void){
    unreg_ch_dev(NULL_MAJOR,"null");
    return 0;
}

DEV_BK_EXPORT(null_init,null_close,null);