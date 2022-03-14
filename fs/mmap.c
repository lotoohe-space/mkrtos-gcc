//
// Created by Administrator on 2022/3/13.
//

#include "mkrtos/fs.h"
#include "mkrtos/sp.h"
#include "mkrtos/bk.h"
#include "arch/atomic.h"
int general_mumap(struct inode * inode,void * start ,size_t len){
    int bk_num;
    struct super_block *sb;
    sb=inode->i_sb;
    bk_num=len/sb->s_bk_size +(len%sb->s_bk_size?1:0);

    for(int i=0;i<bk_num;i++){
        rel_bk1(sb->s_dev_no,(uint8_t*)start+i*sb->s_bk_size);
    }

    return 0;
}
int general_mmap(struct inode * inode, struct file * fp, void* addr, size_t len , int mask, unsigned long off){
    struct super_block  *sb;
    int start_bk;
    int bk_num;
    sb=inode->i_sb;
    off+=1;
//    start_bk=off/sb->s_bk_size +(off%sb->s_bk_size?1:0);
//    bk_num=len/sb->s_bk_size +(len%sb->s_bk_size?1:0);
    int res;
    bk_no_t bk_no;
    int bk_i=0;
    for(int i=off;i<off+len;i+=sb->s_bk_size) {
        res = get_ofs_bk_no(inode, i ,&bk_no);
        if (res < 0) {
            return res;
        }

        if(occ_bk(sb->s_dev_no,bk_no,addr+bk_i*sb->s_bk_size)==NULL){
            //失败则撤销之前映射的所有
            for(int j=off;j<=i;j++){
                res = get_ofs_bk_no(inode, i, &bk_no);
                if (res < 0) {
                    return res;
                }
                rel_bk(sb->s_dev_no,bk_no);
            }
            return -ENOMEM;
        }
        bk_i++;
    }
    return 0;
}
