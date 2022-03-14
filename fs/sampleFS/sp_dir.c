//
// Created by Administrator on 2022/1/17.
//

#include <type.h>
#include <errno.h>
#include <mkrtos/fs.h>
#include <mkrtos/sp.h>
#include <mkrtos/bk.h>
#include <string.h>
int sp_dir_read(struct inode * inode, struct file * filp, char * buf, int count)
{
    return -EISDIR;
}
int sp_readdir(struct inode * inode, struct file * filp,
                  struct dirent * dirent, int count)
{
    struct dir_item di;
    int ret=0;
    again:
    if((ret=sp_file_read(inode, filp,(uint8_t*)(&di), sizeof( struct dir_item)))<=0){
        return ret;
    }
    if(di.used==0){
        goto again;
    }

    strncpy(dirent->d_name,di.name,sizeof(di.name));
    dirent->d_reclen=strlen(di.name);
    dirent->d_ino=di.inode_no;
    dirent->d_off=filp->f_ofs-sizeof(struct dir_item);
    return sizeof(di);
}


static struct file_operations sp_dir_operations = {
        NULL,			/* lseek - default */
        sp_dir_read,		/* read */
        NULL,			/* write - bad */
        sp_readdir,		/* readdir */
        NULL,			/* select - default */
        NULL,			/* ioctl - default */
        NULL,			/* mmap */
        NULL,
        NULL,			/* no special open code */
        NULL,			/* no special release code */
        file_fsync		/* default fsync */
};

/*
 * directories can handle most operations...
 */
struct inode_operations sp_dir_inode_operations = {
        &sp_dir_operations,	/* default directory file-ops */
        sp_create,		/* create */
        sp_lookup,		/* lookup */
        sp_link,		/* link */
        sp_unlink,		/* unlink */
        sp_symlink,		/* symlink */
        sp_mkdir,		/* mkdir */
        sp_rmdir,		/* rmdir */
        sp_mknod,		/* mknod */
        sp_rename,		/* rename */
        NULL,			/* readlink */
        NULL,			/* follow_link */
        NULL,			/* bmap */
        sp_truncate,		/* truncate */
        NULL			/* permission */
};
