//
// Created by Administrator on 2022/1/17.
//

#include <mkrtos/fs.h>
#include <mkrtos/sp.h>

static struct file_operations sp_file_operations = {
        NULL,			/* lseek - default */
        sp_file_read,		/* read */
        sp_file_write,			/* write */
        NULL,		/* readdir */
        NULL,			/* select - default */
        NULL,			/* ioctl - default */
        general_mmap,			/* mmap */
        general_mumap,/*mumap*/
        NULL,			/* no special open code */
        NULL,			/* no special release code */
        sp_sync_file		/* default fsync */
};

/*
 * directories can handle most operations...
 */
struct inode_operations sp_file_inode_operations = {
        &sp_file_operations,	/* default directory file-ops */
        NULL,		/* create */
        NULL,		/* create */
        NULL,		/* lookup */
        NULL,		/* link */
        NULL,		/* unlink */
        NULL,		/* symlink */
        NULL,		/* mkdir */
        NULL,		/* rmdir */
        NULL,		/* mknod */
        NULL,		/* rename */
        NULL,			/* readlink */
        NULL,			/* follow_link */
        NULL,			/* bmap */
        sp_truncate,		/* truncate */
        NULL			/* permission */
};

