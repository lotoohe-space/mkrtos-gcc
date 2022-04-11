//
// Created by Administrator on 2022/1/22.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <mkrtos/stat.h>
#include <mkrtos/bk.h>
#include <mkrtos/sp.h>
static int sp_follow_link(struct inode * dir, struct inode * inode,
                             int flag, int mode, struct inode ** res_inode)
{
    int error;
    struct sp_inode* sp_ino;
    struct bk_cache *tmp;
    *res_inode = NULL;
    if (!dir) {
        dir = CUR_TASK->root_inode;
        atomic_inc(&(dir->i_used_count));
    }
    if (!inode) {
        puti(dir);
        return -ENOENT;
    }
    if (!S_ISLNK(inode->i_type_mode)) {
        puti(dir);
        *res_inode = inode;
        return 0;
    }
//    if (CUR_TASK->link_count > 5) {
//        puti(inode);
//        puti(dir);
//        return -ELOOP;
//    }
    if(sp_ino->p_ino[0]==0){
        return -1;
    }
    tmp=bk_read(inode->i_sb,sp_ino->p_ino[0],0,0);

    puti(inode);
//    CUR_TASK->link_count++;
    error = open_namei(tmp->cache,flag,mode,res_inode,dir);
//    CUR_TASK->link_count--;
    bk_release(tmp);
    return error;
}

static int sp_readlink(struct inode * inode, char * buffer, int buflen)
{
    struct bk_cache * bh;
    int i;
    char c;

    if (!S_ISLNK(inode->i_type_mode)) {
        puti(inode);
        return -EINVAL;
    }
    if (buflen > inode->i_sb->s_bk_size) {
        buflen = inode->i_sb->s_bk_size;
    }
    struct sp_inode *sp_ino;
    sp_ino=inode->i_fs_priv_info;

    bh = bk_read(inode,sp_ino->p_ino[0], 0,0);
    puti(inode);
    i = 0;
    while (i<buflen && (c = bh->cache[i])) {
        i++;
        *(buffer++)=c;
    }
    bk_release(bh);
    return i;
}

struct inode_operations sp_symlink_inode_operations = {
        NULL,			/* no file-operations */
        NULL,			/* create */
        NULL,			/* lookup */
        NULL,			/* link */
        NULL,			/* unlink */
        NULL,			/* symlink */
        NULL,			/* mkdir */
        NULL,			/* rmdir */
        NULL,			/* mknod */
        NULL,			/* rename */
        sp_readlink,		/* readlink */
        sp_follow_link,	/* follow_link */
        NULL,			/* bmap */
        NULL,			/* truncate */
        NULL			/* permission */
};

