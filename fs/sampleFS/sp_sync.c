//
// Created by Administrator on 2022/1/17.
//

#include <type.h>
#include <mkrtos/fs.h>
#include <mkrtos/stat.h>
#include <mkrtos/fs.h>
#include <mkrtos/sp.h>
#include <mkrtos/bk.h>

/**
 * 同步当前文件
 * @param inode
 * @param filp
 * @return
 */
int file_fsync (struct inode *inode, struct file *filp)
{
    struct super_block *sb;
    sb=inode->i_sb;
    //同步当前的inode
    sp_sync_inode(inode);
    return sync_all_bk(sb->s_dev_no);
}
//同步一个文件
int sp_sync_file(struct inode * inode, struct file * file) {
    if (!(S_ISREG(inode->i_type_mode) || S_ISDIR(inode->i_type_mode) ||
          S_ISLNK(inode->i_type_mode)))
        return -EINVAL;
    struct sp_inode* sp_ino;
    struct super_block *sb;
    sb=inode->i_sb;
    sp_ino=inode->i_fs_priv_info;

    for(int32_t i=0;i< ARRARY_LEN(sp_ino->p_ino);i++){
        if(sp_ino->p_ino[i]){
            sync_bk(inode->i_sb->s_dev_no,sp_ino->p_ino[i]);
        }
    }

    for(int32_t i=0;i< ARRARY_LEN(sp_ino->pp_ino);i++){
        if(sp_ino->pp_ino[i]){
            struct bk_cache *tmp;
            tmp= bk_read(sb->s_dev_no,sp_ino->pp_ino[i],0,0);
            for(int32_t j=0;j<sb->s_bk_size;i+=4){
                uint32_t ino;
                ino=*((uint32_t*)(tmp->cache+i));
                if(ino) {
                    sync_bk(inode->i_sb->s_dev_no, ino);
                }else{
                    bk_release(tmp);
                    goto next;
                }
            }
            bk_release(tmp);
        }
    }
    next:
    //三级还没做
    sp_sync_inode(inode);
    return 0;
}