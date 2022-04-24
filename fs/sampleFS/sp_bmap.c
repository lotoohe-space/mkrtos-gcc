//
// Created by Administrator on 2022/1/12.
//
#include <mkrtos/mem.h>
#include <mkrtos/fs.h>
#include <type.h>
#include <mkrtos/sp.h>
#include <mkrtos/bk.h>
#include <string.h>
/**
 * 设置bk块的占用状态
 * @param sb
 * @param usedBkInx
 * @param status
 * @return
 */
int32_t set_bk_no_status(struct super_block *sb, uint32_t usedBkInx, uint8_t status) {
    uint32_t bkInx;
    struct sp_super_block * sp_sb;
    sp_sb=sb->s_sb_priv_info;
    //先读Inode used block
    bkInx = ROUND_DOWN(usedBkInx, sb->s_bk_size * 8);
    uint8_t rByte;
    if (rbk(sb->s_dev_no, sp_sb->bkUsedBkStInx + bkInx, &rByte, ROUND_DOWN(usedBkInx, 8)% sb->s_bk_size,  1) <0) {
        return -1;
    }
    if (!status) {
        rByte|=~(1<<( usedBkInx % 8));
    }
    else {
        rByte&=(1<<( usedBkInx % 8));
    }
    //回写
    if (wbk(sb->s_dev_no, sp_sb->bkUsedBkStInx + bkInx, &rByte, ROUND_DOWN(usedBkInx, 8)% sb->s_bk_size, 1) < 0) {
        return -1;
    }

    return 0;
}
int32_t get_free_bk(struct super_block* sb){
    struct sp_super_block * sp_sb=sb->s_sb_priv_info;
    uint32_t free_bk=0;
    int fn_inx=0;
    uint32_t j;
    uint8_t r;
    uint8_t m;
    struct bk_cache *bk_tmp;
    for (uint32_t i = 0; i <sp_sb->bkUsedBkCount; i++) {
        bk_tmp=bk_read(sb->s_dev_no,sp_sb->bkUsedBkStInx + i,1,0);
        trace("bk map %x %x\n",bk_tmp->cache[0],bk_tmp->cache[1]);
        for (j = 0; j < sb->s_bk_size; j++) {
            r = bk_tmp->cache[j];
            if (r != 0) {
                //有空的
                for (m = 0; m < 8; m++) {
                    fn_inx++;
                    if (fn_inx >= sp_sb->blockCount) {
                        bk_release(bk_tmp);
                        return free_bk;
                    }
                    if (((r) & (1 << m))) {
                        free_bk++;
                    }
                }
            }
            else {
                fn_inx+=8;
            }
        }
        bk_release(bk_tmp);
    }
    return free_bk;
}
/**
 * 从磁盘获得一个bk号码
 * @param sb
 * @param res_bk
 * @return
 */
int32_t alloc_bk(struct super_block* sb,bk_no_t *res_bk){
    struct sp_super_block * sp_sb=sb->s_sb_priv_info;
    uint32_t free_bk=0;
    uint32_t fn_inx=0;
    uint32_t j;
    uint8_t r;
    uint8_t m;
    struct bk_cache *bk_tmp;
//    if(sp_sb->blockFree==0) {
//        return -1;
//    }
    uint32_t st_bk;
    uint32_t st_byte_inx;
    st_bk=sp_sb->last_alloc_bk_inx/(sb->s_bk_size<<3);
    fn_inx+=st_bk*(sb->s_bk_size<<3);
    st_byte_inx=(sp_sb->last_alloc_bk_inx%(sb->s_bk_size<<3))>>3;
    fn_inx+=st_byte_inx<<3;
    for (uint32_t i = st_bk; i <sp_sb->bkUsedBkCount; i++) {
        bk_tmp=bk_read(sb->s_dev_no,sp_sb->bkUsedBkStInx + i,1,0);
//        SET_FREEZE(*bk_tmp);
        for (j = st_byte_inx; j < sb->s_bk_size; j++) {
            r = bk_tmp->cache[j];
            if (r) {
                //有空的
                for (m = 0; m < 8; m++) {
                    if (((r) & (1 << m))) {
                        //找到为1的空块
                        free_bk = fn_inx;
                        if ((free_bk+1) >= sp_sb->blockCount) {
                            bk_release(bk_tmp);
                            return -1;
                        }
                        goto next;
                    }
                    else {
                        fn_inx++;
                    }
                }
            }
            else {
                fn_inx += 8;
            }
        }
        st_byte_inx=0;
        bk_release(bk_tmp);
    }
    return -1;
    next:
    sp_sb->last_alloc_bk_inx=free_bk;
    r &= ~(1<<( free_bk % 8));
    bk_tmp->cache[j] = r;

    if(sp_sb->blockFree) {
        sp_sb->blockFree--;
    }
    *res_bk=free_bk;
    sb->s_dirt=1;
    bk_release(bk_tmp);
    return 0;
}
/**
 * 从磁盘释放一个bk no
 * @param sb
 * @param bk
 * @return
 */
int32_t free_bk(struct super_block* sb,bk_no_t bk_no){
    uint32_t bk_inx;
    struct sp_super_block * sp_sb=sb->s_sb_priv_info;
    bk_inx = ROUND_DOWN(bk_no, sb->s_bk_size * 8);
    uint8_t rByte;
    if (rbk(sb->s_dev_no,
            sp_sb->bkUsedBkStInx + bk_inx,
            &rByte,
            ROUND_DOWN(bk_no, 8)% sb->s_bk_size ,
            1) <0) {
        return -1;
    }
    rByte|=(1<<( bk_no % 8));
    //回写
    if (wbk(sb->s_dev_no,
            sp_sb->bkUsedBkStInx + bk_inx,
            &rByte,
            ROUND_DOWN(bk_no, 8)% sb->s_bk_size,
            1) < 0) {
        return -1;
    }
    sp_sb->blockFree++;
    sb->s_dirt=1;
    return 0;
}
int32_t get_free_inode(struct super_block* sb){
    uint32_t i;
    uint32_t free_inode=0;
    uint32_t fn_inx = 0;
    uint32_t j;
    uint8_t m;
    uint8_t r;
    struct sp_super_block * sp_sb;
    struct bk_cache* bk_tmp;
    sp_sb=sb->s_sb_priv_info;
    //先从bitmap中查找空闲的inode
    for (i = 0; i < sp_sb->inode_used_bk_count; i++) {
        bk_tmp=bk_read(sb->s_dev_no,sp_sb->inode_used_bk_st_inx + i,1,0);

        for (j = 0; j < sb->s_bk_size; j++) {
            r=bk_tmp->cache[j];
            if (r != 0) {
                //有空的
                for (m = 0; m < 8; m++) {
                    fn_inx++;
                    if (fn_inx >= sp_sb->iNodeCount) {
                        bk_release(bk_tmp);
                        return free_inode;
                    }
                    if (((r) & (1 << m))) {
                        free_inode++;
                    }
                }
            }
            else {
                fn_inx+=8;
            }
        }
        bk_release(bk_tmp);
        return free_inode;
    }
}
/**
 * 从磁盘申请一个inode号
 * @param sb
 * @param res_ino
 * @return
 */
int32_t alloc_inode_no(struct super_block* sb,ino_t *res_ino){
    uint32_t i;
    uint32_t bk_inx=0;
    uint32_t free_inode=0;
    uint32_t f_inx = 0;
    uint32_t j;
    uint8_t m;
    uint8_t r;
    struct sp_super_block * sp_sb;
    struct bk_cache* bk_tmp;
    sp_sb=sb->s_sb_priv_info;
//    if(sp_sb->iNodeFree==0) {
//        return -1;
//    }
    //先从bitmap中查找空闲的inode
    for (i = 0; i < sp_sb->inode_used_bk_count; i++) {
        bk_tmp=bk_read(sb->s_dev_no,sp_sb->inode_used_bk_st_inx + i,1,0);
//        SET_FREEZE(*bk_tmp);
        for (j = 0; j < sb->s_bk_size; j++) {
            r=bk_tmp->cache[j];
            if (r != 0) {
                //有空的
                for (m = 0; m < 8; m++) {
                    if (((r) & (1 << m))) {
//                        f_inx += m;
                        free_inode = f_inx;
                        if ((free_inode+1) >= sp_sb->iNodeCount) {
                            bk_release(bk_tmp);
                            return -1;
                        }
                        goto next;
                    }else {
                        f_inx++;
                    }
                }
            }
            else {
                f_inx += 8;
            }
        }
        bk_release(bk_tmp);
    }
    return -1;
    next:
    r&=~(1<<( free_inode % 8));
    bk_tmp->cache[j] = r;
    if(sp_sb->iNodeFree) {
        sp_sb->iNodeFree--;
    }
    *res_ino=free_inode;
    sb->s_dirt=1;
    bk_release(bk_tmp);
    trace("分配新inode %d\r\n",free_inode);

    return 0;
}
/**
 * 从磁盘释放一个inode号
 * @param sb
 * @param ino
 * @return
 */
int32_t free_inode_no(struct super_block*sb,ino_t ino){
    int32_t bk_inx;
    struct sp_super_block * sp_sb=sb->s_sb_priv_info;


    //先读Inode used block
    bk_inx = ROUND_DOWN(ino, sb->s_bk_size * 8);
    uint8_t rByte;
    if (rbk(sb->s_dev_no,
            sp_sb->inode_used_bk_st_inx + bk_inx,
            &rByte,
            ROUND_DOWN(ino, 8) % sb->s_bk_size,
            1) < 0) {
        return -1;
    }
    rByte|=(1<<( ino % 8));
    //回写
    if (wbk(sb->s_dev_no,
            sp_sb->inode_used_bk_st_inx + bk_inx,
            &rByte,
            ROUND_DOWN(ino, 8) % sb->s_bk_size,
            1) < 0) {
        return -1;
    }
    sp_sb->iNodeFree++;
    sb->s_dirt=1;
    return 0;
}
/**
 * 检查磁盘上的inode是否存在
 * @param sb
 * @param ino
 * @return
 */
int32_t check_inode_no(struct super_block*sb,ino_t ino){
    //在INode 使用统计块的偏移
    uint32_t inode_used_bk_ofs;
    //在块中的偏移
    uint32_t inode_ofs_in_bk;
    uint8_t ofs_byte;
    struct sp_super_block * sp_sb=sb->s_sb_priv_info;

    inode_used_bk_ofs = sp_sb->inode_used_bk_st_inx+ino/sb->s_bk_size;
    inode_ofs_in_bk = ino % sb->s_bk_size;
    if (rbk(sb->s_dev_no,
            inode_used_bk_ofs,
            &ofs_byte,
            inode_ofs_in_bk / 8,
            1) <0) {
        return -1;
    }

    if (ofs_byte >> (inode_ofs_in_bk % 8) == 1) {
        //Inode没有被使用
        return 0;
    }
    return 1;
}
/**
 * 获取inode文件最后位置所在的bk号码
 * @param p_inode
 * @param offset
 * @param bk_no
 * @return
 */
int32_t get_bk_no_ofs(struct inode* p_inode, uint32_t offset,uint32_t* bk_no) {
    struct super_block *sb;
    struct sp_inode *sp_inode;
    uint32_t used_bk_num;
    if(offset > p_inode->i_file_size){
        return -1;
    }
    sb=p_inode->i_sb;
    sp_inode= SP_INODE(p_inode);
    used_bk_num= ROUND_UP(offset+1, sb->s_bk_size);
    if (used_bk_num <= A_BK_NUM(sp_inode)) {
        *bk_no = sp_inode->p_ino[used_bk_num - 1];
        return 0;
    }
    else if (used_bk_num <= A_BK_NUM(sp_inode) + B_BK_NUM(sb, sp_inode)) {
        //超过A部分的大小
        uint32_t over_size = used_bk_num - A_BK_NUM(sp_inode)-1;
        //二级大小
        uint32_t pp_bk_no = over_size / BK_INC_X_NUM(sb);
        uint32_t pp_bk_ofs = over_size % BK_INC_X_NUM(sb);
        uint32_t bk_num;
        //二级大小
        if (rbk(sb->s_dev_no, sp_inode->pp_ino[pp_bk_no],
                         (uint8_t*)(&bk_num), pp_bk_ofs*sizeof(bk_no_t), sizeof(bk_no_t))) {
            return -1;
        }
        *bk_no = bk_num;
        return 0;
    }
//    else if (usedBkNum <= A_BK_NUM(pINode) + B_BK_NUM(pFsInfo, pINode) + C_BK_NUM(pFsInfo, pINode)) {
//        //三级部分的大小
//        uint32_t overANum = usedBkNum - A_BK_NUM(pINode)-B_BK_NUM(pFsInfo, pINode)-1;
//        //得到一级偏移
//        uint32_t pFileBksInx = overANum/BK_INC_X_NUM(pFsInfo)*BK_INC_X_NUM(pFsInfo);
//        //计算二级偏移，等于超出部分大小，除以
//        uint32_t ppFileBksInx = (overANum%(BK_INC_X_NUM(pFsInfo)*BK_INC_X_NUM(pFsInfo)))/BK_INC_X_NUM(pFsInfo);
//        uint32_t pppFileBksInx= (overANum%(BK_INC_X_NUM(pFsInfo)*BK_INC_X_NUM(pFsInfo)))%BK_INC_X_NUM(pFsInfo);
//        uint32_t bkNum;
//        //获得二级块偏移
//        if (BkCacheAddOp(pFsInfo->bkDev, 3, pINode->ppFileBks[pFileBksInx],
//                         (uint8_t*)(&bkNum), ppFileBksInx*sizeof(uint32_t), sizeof(uint32_t))) {
//            return FsNotFindErr;
//        }
//        //获得最终偏移
//        if (BkCacheAddOp(pFsInfo->bkDev, 3, bkNum,  (uint8_t*)(&bkNum), pppFileBksInx*sizeof(uint32_t), sizeof(uint32_t))) {
//            return FsNotFindErr;
//        }
//        *fpBkNum = bkNum;
//        return FsNoneErr;
//    }
    else {
        //超过了单个文件的大小
//        printk("超过了单个文件大小\r\n");
        return -1;
    }
    return -1;
}


extern struct super_operations sp_sb;


