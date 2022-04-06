//
// Created by Administrator on 2022/1/21.
//
#include <mkrtos/fs.h>
#include <mkrtos/stat.h>
#include <mkrtos/task.h>
static void cp_new_stat(struct inode * inode, struct new_stat * statbuf)
{
    statbuf->st_dev = inode->i_sb->s_dev_no;
    statbuf->st_ino = inode->i_no;
    statbuf->st_mode = inode->i_type_mode;
    statbuf->st_nlink = inode->i_hlink;
    statbuf->st_uid = 0;
    statbuf->st_gid = 0;
    statbuf->st_rdev = inode->i_rdev_no;
    statbuf->st_size = inode->i_file_size;
    statbuf->st_atime = 0;
    statbuf->st_mtime = 0;
    statbuf->st_ctime = 0;
    statbuf->st_blksize = inode->i_sb->s_bk_size;
}
int32_t sys_stat(char * filename, struct new_stat * statbuf)
{
    struct inode * inode;
    int res=0;
    res = namei(filename,&inode);
    if (res<0) {
        return res;
    }
    cp_new_stat(inode,statbuf);
    puti(inode);
    return 0;
}
int32_t sys_fstat(int fd,struct new_stat* stat){
    struct file* files=CUR_TASK->files;
    if((fd<0||fd>=NR_FILE)||files[fd].used==0){
        return -EBADF;
    }
    if(!files[fd].f_inode){
        return -EINVAL;
    }
    cp_new_stat(files[fd].f_inode,stat);
    return 0;
}
int32_t sys_lstat(char * filename, struct new_stat * statbuf) {
    struct inode * inode;
    int res=0;
    res = lnamei(filename,&inode);
    if (res<0) {
        return res;
    }
    cp_new_stat(inode,statbuf);
    puti(inode);
}
