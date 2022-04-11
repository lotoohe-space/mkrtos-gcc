//
// Created by Administrator on 2022/4/7.
//
#include <mkrtos/fs.h>
#include <mkrtos/bk.h>
#include <fcntl.h>
#include <string.h>
/**
 * 打开块设备
 * @param inode
 * @param filp
 * @return
 */
int bk_file_open(struct inode * ino, struct file * filp){
    int ret;
    struct bk_operations *bk_ops;
    bk_ops=get_bk_ops(ino->i_rdev_no);
    if(!bk_ops){
        return -1;
    }
    if(bk_ops->open) {
        ret = bk_ops->open(ino->i_rdev_no);
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}
#define MIN(a,b) ((a)>(b)?(b):(a))
//读块设备
int bk_file_read(struct inode *ino, struct file *fp, char * buf, int count){
    struct bk_operations *bk_ops;
    struct super_block *sb;
    struct bk_cache* tmp_bk;

    int w_ofs;
    int w_ofs_bk;
    int dev_no;
    int w_size=0;

    int w_len=0;
    int cur_bk_rm_size=0;
    int cur_w_ofs=0;
    dev_no=ino->i_rdev_no;
    bk_ops=get_bk_ops(dev_no);
    if(!bk_ops){
        return -1;
    }
    sb=ino->i_sb;
    if(fp->f_flags&O_APPEND){
        w_ofs=ino->i_file_size;
    }else {
        w_ofs = fp->f_ofs;
    }
    ino->i_file_size+=count;
    while(w_len<count) {
        w_ofs += w_len;//0
        //获取开始的块偏移
        w_ofs_bk = w_ofs / sb->s_bk_size;//0
        //当前读取块剩余的大小
        cur_bk_rm_size = sb->s_bk_size - (w_ofs % sb->s_bk_size);//2048
        tmp_bk = bk_read(dev_no, w_ofs_bk, 1,0);
        if (!tmp_bk) {
            ino->i_file_size-=count;
            //运行到这里说明当前的块设备没有存储空间了
            return -ENOSPC;
        }
        //获得需要读取的大小
        w_size = MIN((count - w_len), sb->s_bk_size);
        w_size = MIN(w_size, cur_bk_rm_size);
        //计算当前需要读取的偏移
        cur_w_ofs = w_ofs % sb->s_bk_size;
        memcpy(buf+w_len,tmp_bk->cache + cur_w_ofs,w_size);
        bk_release(tmp_bk);

        //读了多少数据
        w_len += w_size;
    }
    return w_len;
}

//写块设备
int bk_file_write(struct inode *ino, struct file * fp, char * buf, int count){
    struct bk_operations *bk_ops;
    struct super_block *sb;
    struct bk_cache* tmp_bk;

    int w_ofs;
    int w_ofs_bk;
    int dev_no;
    int w_size=0;

    int w_len=0;
    int cur_bk_rm_size=0;
    int cur_w_ofs=0;
    dev_no=ino->i_rdev_no;
    bk_ops=get_bk_ops(dev_no);
    if(!bk_ops){
        return -1;
    }
    sb=ino->i_sb;
    if(fp->f_flags&O_APPEND){
        w_ofs=ino->i_file_size;
    }else {
        w_ofs = fp->f_ofs;
    }
    ino->i_file_size+=count;
    while(w_len<count) {
        w_ofs += w_len;//0
        //获取开始的块偏移
        w_ofs_bk = w_ofs / sb->s_bk_size;//0
        //当前写入块剩余的大小
        cur_bk_rm_size = sb->s_bk_size - (w_ofs % sb->s_bk_size);//2048
        tmp_bk = bk_read(dev_no, w_ofs_bk, 1,0);
        if (!tmp_bk) {
            ino->i_file_size-=count;
            //运行到这里说明当前的块设备没有存储空间了
            return -ENOSPC;
        }
        //获得需要写入的大小
        w_size = MIN((count - w_len), sb->s_bk_size);
        w_size = MIN(w_size, cur_bk_rm_size);
        //计算当前需要写入的偏移
        cur_w_ofs = w_ofs % sb->s_bk_size;
        memcpy(tmp_bk->cache + cur_w_ofs, buf + w_len, w_size);
        bk_release(tmp_bk);

        //写入了数据
        w_len += w_size;
    }
    return w_len;
}
int bK_file_fsync(struct inode *inode, struct file *filp)
{
    sync_all_bk(inode->i_rdev_no);
    return 0;
}
int bk_file_release(struct inode * ino, struct file * fp){
    struct bk_operations *bk_ops;
    bk_ops=get_bk_ops(ino->i_rdev_no);
    if(!bk_ops){
        return -1;
    }
    if(bk_ops->release) {
        bk_ops->release(ino->i_rdev_no);
    }
    return 0;
}
//struct file_operations bk_file_ops={
////        .open=bk_file_open,
//        .read=bk_file_read,
//        .write=bk_file_write,
//        .fsync=bK_file_fsync
////        .release=bk_file_release
//};
