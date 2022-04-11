//
// Created by Administrator on 2022/1/18.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <mkrtos/sp.h>
#include <mkrtos/bk.h>
#include <fcntl.h>
#include <string.h>
#include <mkrtos/stat.h>
#include "mkrtos/debug.h"

int sp_file_read(struct inode * inode, struct file * filp, char * buf, int count){
    uint32_t fileUsedBKNum;
    uint32_t rLen = 0;
    int32_t r_inx = 0;
    struct super_block *sb;
    struct sp_inode *sp_ino;
    //读这一块时最大的大小
    uint32_t bkRemainSize;
    //偏移在块中开始的位置
    uint32_t bInx;
    //偏移开始的位置
    uint32_t ofstBkInx;
    if (count == 0) {
        return 0;
    }
    r_inx=filp->f_ofs;
    if (r_inx >= inode->i_file_size) {
        return -1;
    }
    sb = inode->i_sb;
    sp_ino = (struct sp_inode *)(inode->i_fs_priv_info);

    ofstBkInx= ROUND_DOWN(r_inx, sb->s_bk_size);
    bInx= r_inx % sb->s_bk_size;
    bkRemainSize= sb->s_bk_size - bInx;
    fileUsedBKNum = FILE_USED_BK_NUM(sb, inode);

    for (uint32_t i = ofstBkInx; i < fileUsedBKNum && rLen<count && (rLen+r_inx)< inode->i_file_size; i++) {
        uint32_t remainSize = count - rLen;
        uint32_t rSize = 0;
        uint32_t readFileRamainSize;
        readFileRamainSize = inode->i_file_size - (rLen + r_inx);
        rSize = MIN(remainSize, sb->s_bk_size);
        rSize = MIN(rSize, readFileRamainSize);
        if (i == ofstBkInx) {
            rSize = MIN(rSize, bkRemainSize);
        }
        else {
            bInx = 0;
            bkRemainSize = sb->s_bk_size;
        }
        if (i < A_BK_NUM(sp_ino)) {
            struct bk_cache *tmp;
            tmp= bk_read(sb->s_dev_no,sp_ino->p_ino[i],0,0);
            memcpy(buf+rLen,tmp->cache+bInx,rSize);
            rLen += rSize;
            filp->f_ofs+=rSize;
            bk_release(tmp);
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {

            uint32_t overNum = i - A_BK_NUM(sp_ino);
            uint32_t bkNo = overNum / BK_INC_X_NUM(sb);
            uint32_t bkInx = overNum % BK_INC_X_NUM(sb);
            uint32_t readBkInx;
            struct bk_cache *tmp;
            struct bk_cache *tmp1;

            tmp= bk_read(sb->s_dev_no,sp_ino->pp_ino[bkNo],0,0);
            memcpy((uint8_t*)(&readBkInx),tmp->cache+bkInx*sizeof(uint32_t),sizeof(uint32_t));
            bk_release(tmp);

            tmp1= bk_read(sb->s_dev_no,readBkInx,0,0);
            memcpy(buf + rLen,tmp1->cache+bInx,rSize);
            rLen += rSize;
            filp->f_ofs+=rSize;
            bk_release(tmp1);


        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {
            struct bk_cache *tmp;
            int a,b;
            //三级大小
            uint32_t overANum = i - A_BK_NUM(sp_ino)-B_BK_NUM(sb, sp_ino);
            //在初级块中的偏移
            uint32_t pFileBksInx = overANum/(BK_INC_X_NUM(sb)*BK_INC_X_NUM(sb));
            //计算二级偏移，等于超出部分大小，除以
            uint32_t ppFileBksInx = (overANum%(BK_INC_X_NUM(sb)*BK_INC_X_NUM(sb)))/BK_INC_X_NUM(sb);
            uint32_t pppFileBksInx= overANum%BK_INC_X_NUM(sb);

            tmp= bk_read(sb->s_dev_no,sp_ino->ppp_ino[pFileBksInx],0,0);
            memcpy((uint8_t*)(&a),tmp->cache+(ppFileBksInx<<2),sizeof(uint32_t));
            bk_release(tmp);

            tmp= bk_read(sb->s_dev_no,a,0,0);
            memcpy(&b,tmp->cache+(pppFileBksInx<<2),sizeof(uint32_t));
            bk_release(tmp);

            tmp= bk_read(sb->s_dev_no,b,0,0);
            memcpy(buf + rLen,tmp->cache+bInx,rSize);
            rLen += rSize;
            filp->f_ofs+=rSize;
            bk_release(tmp);
        }
        else {
            return -1;
        }
    }

    return rLen;
}
/**
 * 获取指定偏移所在的块号
 * @param inode
 * @param offset
 * @param fpBkNum
 * @return
 */
int32_t get_ofs_bk_no(struct inode* inode, uint32_t offset,uint32_t* fpBkNum) {
    struct super_block *sb;
    struct sp_inode *sp_ino;
    if(inode==NULL){
        return -1;
    }

    sb=inode->i_sb;
    sp_ino=(struct sp_inode *)(inode->i_fs_priv_info);
    if(offset > inode->i_file_size){
        return -1;
    }
    uint32_t usedBkNum = ROUND_UP(offset, sb->s_bk_size)-1;
    if (usedBkNum < A_BK_NUM(sp_ino)) {
        *fpBkNum = sp_ino->p_ino[usedBkNum ];
        return 0;
    }
    else if (usedBkNum < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {
        //超过A部分的大小
        uint32_t overANum = usedBkNum - A_BK_NUM(sp_ino);
        //二级大小
        uint32_t pFileBksInx = overANum / BK_INC_X_NUM(sb);
        uint32_t pFileBksi = overANum % BK_INC_X_NUM(sb);
        uint32_t bkNum;

        struct bk_cache* tmp;
        tmp=bk_read(sb->s_dev_no,sp_ino->pp_ino[pFileBksInx],0,0);
        memcpy((uint8_t*)(&bkNum), tmp->cache+pFileBksi*sizeof(uint32_t),sizeof(uint32_t));
        bk_release(tmp);
        *fpBkNum = bkNum;
        return 0;
    }
    else if (usedBkNum < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {
        struct bk_cache *tmp;
        //三级大小
        uint32_t overANum = usedBkNum - A_BK_NUM(sp_ino)-B_BK_NUM(sb, sp_ino);
        //在初级块中的偏移
        uint32_t pFileBksInx = overANum/(BK_INC_X_NUM(sb)*BK_INC_X_NUM(sb));
        //计算二级偏移，等于超出部分大小，除以
        uint32_t ppFileBksInx = (overANum%(BK_INC_X_NUM(sb)*BK_INC_X_NUM(sb)))/BK_INC_X_NUM(sb);
        uint32_t pppFileBksInx= overANum%BK_INC_X_NUM(sb);

        if(!sp_ino->ppp_ino[pFileBksInx]){
            return -ENOSPC;
        }
        int a;
        tmp=bk_read(sb->s_dev_no,sp_ino->ppp_ino[pFileBksInx],0,0);
        memset(&a,tmp->cache+(ppFileBksInx<<2),sizeof(a));
        bk_release(tmp);
        if(!a){
            return -ENOSPC;
        }
        int b;
        tmp=bk_read(sb->s_dev_no,a,0,0);
        memset(&b,tmp->cache+(pppFileBksInx<<2),sizeof(b));
        bk_release(tmp);
        if(!b){
            return -ENOSPC;
        }
        *fpBkNum = b;
        return 0;
    }
    else {
       return -1;
    }
    return -1;
}
int32_t inode_alloc_new_bk(struct inode* inode, uint32_t* newBkNum){
    struct super_block* sb;
    struct sp_inode *sp_ino;
    sb=inode->i_sb;
    sp_ino=(struct sp_inode *)(inode->i_fs_priv_info);
    uint32_t usedNewBkNum;
    if(inode->i_file_size==0){
        usedNewBkNum =0;
    }else{
        usedNewBkNum= FILE_USED_BK_NUM(sb, inode);
    }

    if (usedNewBkNum < A_BK_NUM(sp_ino)) {

            if(alloc_bk(sb,newBkNum)<0){
                return -1;
            }
            sp_ino->p_ino[usedNewBkNum] = *newBkNum;
            inode->i_dirt=1;

    }
    else if (usedNewBkNum < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {
        //超过A部分的大小
        uint32_t overANum = usedNewBkNum - A_BK_NUM(sp_ino);
        //二级大小
        uint32_t pFileBksInx = overANum / BK_INC_X_NUM(sb);
        uint32_t pFileBksi = overANum % BK_INC_X_NUM(sb);
        struct bk_cache* tmp;

        uint32_t a;
        if (sp_ino->pp_ino[pFileBksInx] == 0) {
            if(alloc_bk(sb,&a)<0){
                return -1;
            }
            //全部设置为0
            tmp=bk_read(sb->s_dev_no,a,1,1);
            memset(tmp->cache,0,sb->s_bk_size);
            bk_release(tmp);
            sp_ino->pp_ino[pFileBksInx] = a;
            inode->i_dirt=1;
        }
        uint32_t b;
        if(alloc_bk(sb,&b)<0){
            return -1;
        }
        tmp=bk_read(sb->s_dev_no,sp_ino->pp_ino[pFileBksInx],1,0);
        memcpy(tmp->cache+pFileBksi * 4, (uint8_t*)&b,sizeof(b));
        bk_release(tmp);
        *newBkNum = b;
    }
    else if (usedNewBkNum < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {
        //三级大小
        uint32_t overANum = usedNewBkNum - A_BK_NUM(sp_ino)-B_BK_NUM(sb, sp_ino);
        //在初级块中的偏移
        uint32_t pFileBksInx = overANum/(BK_INC_X_NUM(sb)*BK_INC_X_NUM(sb));
        //计算二级偏移，等于超出部分大小，除以
        uint32_t ppFileBksInx = (overANum%(BK_INC_X_NUM(sb)*BK_INC_X_NUM(sb)))/BK_INC_X_NUM(sb);
        uint32_t pppFileBksInx= overANum%BK_INC_X_NUM(sb);
        uint32_t a;
        struct bk_cache* tmp;
        if(!sp_ino->ppp_ino[pFileBksInx]){
            if(alloc_bk(sb,&a)<0){
                return -1;
            }
            //全部设置为0
            tmp=bk_read(sb->s_dev_no,a,1,1);
            memset(tmp->cache,0,sb->s_bk_size);
            bk_release(tmp);
            sp_ino->ppp_ino[pFileBksInx] = a;
            inode->i_dirt=1;
        }
        //得到二级的值
        uint32_t b;
        tmp=bk_read(sb->s_dev_no,sp_ino->ppp_ino[pFileBksInx],1,0);
        memcpy(&b, tmp->cache+ppFileBksInx * 4,sizeof(b));
        if(!b) {
            if (alloc_bk(sb, &b) < 0) {
                sp_ino->ppp_ino[pFileBksInx] = 0;
                free_bk(sb, a);
                bk_release(tmp);
                return -1;
            }
            memcpy( tmp->cache+ppFileBksInx * 4,&b,sizeof(b));
            bk_release(tmp);
            //全部设置为0
            tmp=bk_read(sb->s_dev_no,b,1,1);
            memset(tmp->cache,0,sb->s_bk_size);
            inode->i_dirt=1;
        }else{
            bk_release(tmp);
            tmp=bk_read(sb->s_dev_no,b,1,0);
        }
//        bk_release(tmp);
        uint32_t c;
        if(alloc_bk(sb,&c)<0){
            sp_ino->ppp_ino[pFileBksInx]=0;

            free_bk(sb,a);
            free_bk(sb,b);
            return -1;
        }
//        tmp=bk_read(sb->s_dev_no,b,1);
        memcpy(tmp->cache+pppFileBksInx * 4, (uint8_t*)&c,sizeof(c));
        bk_release(tmp);
        *newBkNum=c;
        return 0;
    }
    else {
       return -1;
    }

    return 0;
}
int sp_file_write(struct inode * inode, struct file * filp, char * buf, int count){
    uint32_t usedBkNum;
    uint32_t upSize;
    uint32_t wSize = 0;
    uint8_t flag = 0;
    uint32_t wLen = 0;
    //写入的偏移位置
    uint32_t wOffset = 0;
    struct super_block *sb;
    struct bk_cache* tmp=NULL;
    if(!S_ISREG(inode->i_type_mode)){
        return -EISDIR;
    }

    if (count == 0) {
        return 0;
    }
    if(filp->f_flags&O_APPEND){
        filp->f_ofs=inode->i_file_size;
    }
    wOffset = filp->f_ofs;
    //写入的偏移超过了文件大小
    if(wOffset > inode->i_file_size){
        return -1;
    }

    sb=inode->i_sb;
    //文件占用多少块
    usedBkNum = ROUND_UP(wOffset, sb->s_bk_size);

    //使用块总共的内存
    upSize = usedBkNum * sb->s_bk_size;
    //写入数据
    while (wLen < count) {

        if (flag == 0) {//先写最后一块
            if (upSize > wOffset && upSize != wOffset) {//不能相等，相等说明刚好写到了一块
                //写入最后一块剩余的空间
                uint32_t last_bk_no;
                if (get_ofs_bk_no(inode,wOffset, &last_bk_no) < 0) {
                    return -1;
                }
                wSize = count > (sb->s_bk_size - (wOffset % sb->s_bk_size))
                        ? (sb->s_bk_size - (wOffset % sb->s_bk_size)) : count;

                tmp=bk_read(sb->s_dev_no,last_bk_no,1,0);
                memcpy(tmp->cache+wOffset % sb->s_bk_size,buf+wLen,wSize);

                wLen += wSize;
                wOffset += wSize;

                if(wOffset > inode->i_file_size){
                    //更新文件大小
                    inode->i_file_size = wOffset;
                    filp->f_ofs=wOffset;
                    inode->i_dirt = 1;
                }
                bk_release(tmp);
            }
            flag = 1;
        }
        else {
            //剩余的大小
            uint32_t remainSize;
            //分配的新的块
            uint32_t needWBk;
            if(wOffset >= inode->i_file_size){
                //如果写入偏移大于或者等于文件大小，申请一块新的
                if (inode_alloc_new_bk(inode, &needWBk) < 0) {
                    return -ENOSPC;
                }
            }else{
                //否则获取当前偏移的块号
                if (get_ofs_bk_no(inode,wOffset, &needWBk) < 0) {
                    return -1;
                }
            }
            remainSize = count - wLen;
            //计算还需要写入多少
            wSize = remainSize > sb->s_bk_size ?  sb->s_bk_size : remainSize;
            tmp=bk_read(sb->s_dev_no,needWBk,1,wSize==sb->s_bk_size);
            memcpy(tmp->cache,buf+wLen,wSize);
            wLen += wSize;
            wOffset += wSize;
            if(wOffset > inode->i_file_size){
                //更新文件大小
                inode->i_file_size = wOffset;
                filp->f_ofs=wOffset;
                inode->i_dirt = 1;
            }
            bk_release(tmp);
        }
    }
//    DEBUG("fs",INFO,"剩余块数:%d.",get_free_bk(sb));
//    DEBUG("fs",INFO,"剩余inode:%d.",get_free_inode(sb));
    trace("file size %d\r\n",inode->i_file_size);
    return wLen;
}
