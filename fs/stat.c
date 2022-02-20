//
// Created by Administrator on 2022/1/21.
//
#include <mkrtos/fs.h>
#include <mkrtos/stat.h>

static void cp_new_stat(struct inode * inode, struct new_stat * statbuf)
{
    statbuf->st_dev = -1;
    statbuf->st_ino = inode->i_no;
    statbuf->st_mode = inode->i_type_mode;
    statbuf->st_nlink = inode->i_hlink;
    statbuf->st_uid = -1;
    statbuf->st_gid = -1;
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

