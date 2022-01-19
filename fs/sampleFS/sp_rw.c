//
// Created by Administrator on 2022/1/18.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <mkrtos/sp.h>
#include <mkrtos/bk.h>
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
            //读取一块数据
            if (rbk(sb->s_dev_no,sp_ino->p_ino[i], buf+rLen, bInx,  rSize) < 0) {
                return -1;
            }
            rLen += rSize;
            filp->f_ofs+=rSize;
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {

            uint32_t overNum = i - A_BK_NUM(sp_ino);
            uint32_t bkNo = overNum / BK_INC_X_NUM(sb);
            uint32_t bkInx = overNum % BK_INC_X_NUM(sb);
            uint32_t readBkInx;
            if (rbk(sb->s_dev_no, sp_ino->pp_ino[bkNo],  (uint8_t*)(&readBkInx), bkInx*sizeof(uint32_t), sizeof(uint32_t))< 0) {
                return -1;
            }
            if (rbk(sb->s_dev_no, readBkInx, buf + rLen, bInx, rSize) < 0) {
                return -1;
            }
            rLen += rSize;
            filp->f_ofs+=rSize;
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {
            return -1;
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
    uint32_t usedBkNum = ROUND_UP(offset+1, sb->s_bk_size);
    if (usedBkNum <= A_BK_NUM(sp_ino)) {
        *fpBkNum = sp_ino->p_ino[usedBkNum - 1];
        return 0;
    }
    else if (usedBkNum <= A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {
        //超过A部分的大小
        uint32_t overANum = usedBkNum - A_BK_NUM(sp_ino)-1;
        //二级大小
        uint32_t pFileBksInx = overANum / BK_INC_X_NUM(sb);
        uint32_t pFileBksi = overANum % BK_INC_X_NUM(sb);
        uint32_t bkNum;
        //二级大小
        if (rbk(sb->s_dev_no, sp_ino->p_ino[pFileBksInx],
                         (uint8_t*)(&bkNum), pFileBksi*sizeof(uint32_t), sizeof(uint32_t))) {
            return -1;
        }
        *fpBkNum = bkNum;
        return 0;
    }
    else if (usedBkNum <= A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {
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
        return -1;
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
    uint32_t usedNewBkNum = FILE_USED_BK_NUM(sb, inode) + 1;
    int32_t ret = 0;

    if (usedNewBkNum <= A_BK_NUM(sp_ino)) {
        //一级大小
        for (uint32_t i = 0; i < A_BK_NUM(sp_ino); i++) {
            if (sp_ino->p_ino[i] != 0) {
                continue;
            }
            if(alloc_bk(sb,newBkNum)<0){
                return -1;
            }
            sp_ino->p_ino[i] = *newBkNum;
            inode->i_dirt=1;
            break;
        }
    }
    else if (usedNewBkNum <= A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {
        //超过A部分的大小
        uint32_t overANum = usedNewBkNum - A_BK_NUM(sp_ino)-1;
        //二级大小
        uint32_t pFileBksInx = overANum / BK_INC_X_NUM(sb);
        uint32_t pFileBksi = overANum % BK_INC_X_NUM(sb);
        uint32_t a;
        if (sp_ino->pp_ino[pFileBksInx] == 0) {
            if(alloc_bk(sb,&a)<0){
                return -1;
            }
            sp_ino->pp_ino[pFileBksInx] = a;
            inode->i_dirt=1;
        }
        uint32_t b;
        if(alloc_bk(sb,&b)<0){
//            free_bk(sb,a);
            return -1;
        }
        //这里还有点问题
        //填充二级
        if (wbk(sb->s_dev_no, sp_ino->pp_ino[pFileBksInx], pFileBksi * 4, (uint8_t*)&b, sizeof(b)) != 0) {
//            free_bk(sb,a);
            free_bk(sb,b);
            return -1;
        }
        *newBkNum = b;
    }
    else if (usedNewBkNum <= A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {
//        //三级大小
//        uint32_t overANum = usedNewBkNum - A_BK_NUM(pINode)-B_BK_NUM(pFsInfo, pINode)-1;
//        //得到一级偏移
//        uint32_t pFileBksInx = overANum/(BK_INC_X_NUM(pFsInfo)*BK_INC_X_NUM(pFsInfo));
//        //计算二级偏移，等于超出部分大小，除以
//        uint32_t ppFileBksInx = (overANum%(BK_INC_X_NUM(pFsInfo)*BK_INC_X_NUM(pFsInfo)))/BK_INC_X_NUM(pFsInfo);
//        uint32_t pppFileBksInx= (overANum%(BK_INC_X_NUM(pFsInfo)*BK_INC_X_NUM(pFsInfo)))%BK_INC_X_NUM(pFsInfo);
//        //填充一级
//        uint32_t a;
//        if (pINode->ppFileBks[pFileBksInx] == BK_IDLE_NUM) {
//
//            FIND_BK_TO_SET_USED(a);
//            pINode->ppFileBks[pFileBksInx] = a;
//
//            uint8_t *temp=ShareMemLock(pFsInfo->blockSize);
//            if (BkCacheAddOp(pFsInfo->bkDev, 2, a,  temp, 0, pFsInfo->blockSize) < 0) {ShareMemUnlock();
//                ShareMemUnlock();
//                return -1;
//            }
//            ShareMemUnlock();
//
//            //写INode
//            pFsInfo->fsOper->inodeOper.DirtyInode(pINode);
//        }
//
//        uint32_t pBkNum;
//        if (BkCacheAddOp(pFsInfo->bkDev, 3, pINode->ppFileBks[pFileBksInx],
//                         (uint8_t*)(&pBkNum), ppFileBksInx*sizeof(uint32_t), sizeof(uint32_t)) < 0) {
//            return -1;
//        }
//        uint32_t b;
//        if(	pBkNum == BK_IDLE_NUM){
//            if(FIND_BK_TO_SET_USED(b)<0){
//                ToSetBKUsed(pFsInfo, a, FALSE);
//                pINode->pFileBks[pFileBksInx]=BK_IDLE_NUM;
//                return -1;
//            }
//
//            pBkNum=b;
//            if (BKWrite(pFsInfo, pINode->ppFileBks[pFileBksInx], ppFileBksInx*sizeof(uint32_t), (uint8_t*)&b, sizeof(b)) != 0) {
//                return -1;
//            }
//        }
//        uint32_t c;
//        if(FIND_BK_TO_SET_USED(c)<0){
//            ToSetBKUsed(pFsInfo, a, FALSE);
//            ToSetBKUsed(pFsInfo, b, FALSE);
//            pINode->pFileBks[pFileBksInx]=BK_IDLE_NUM;
//        }
//
//        if (BKWrite(pFsInfo, pBkNum, pppFileBksInx * 4, (uint8_t*)&c, sizeof(c)) != 0) {
//            //printk("%s %s 写入数据失败:%d\r\n", __DATE__, __FUNCTION__, pINode->pFileBks[pFileBksInx]);
//            ToSetBKUsed(pFsInfo, a, FALSE);
//            ToSetBKUsed(pFsInfo, b, FALSE);
//            ToSetBKUsed(pFsInfo, c, FALSE);
//            pINode->pFileBks[pFileBksInx]=BK_IDLE_NUM;
//            return FsWriteBkErr;
//        }
//        *newBkNum = c;
        return -1;
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

    if(!IS_FILE(inode->i_type_mode)){
        return -EISDIR;
    }

    if (count == 0) {
        return 0;
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
            if (upSize > wOffset
                //不能相等，相等说明刚好写到了一块
                && upSize != wOffset
                    ) {
                //写入最后一块剩余的空间
                uint32_t last_bk_no;
                if (get_ofs_bk_no(inode,wOffset, &last_bk_no) < 0) {
                    return -1;
                }
                wSize = count > (sb->s_bk_size - (wOffset % sb->s_bk_size))
                        ? (sb->s_bk_size - (wOffset % sb->s_bk_size)) : count;
                //写入文件
                if (wbk(sb->s_dev_no, last_bk_no,wOffset % sb->s_bk_size, buf+wLen, wSize) < 0) {
                    return -1;
                }
                wLen += wSize;
                wOffset += wSize;
            }
            flag = 1;
        }
        else {
            //剩余的大小
            uint32_t remainSize;

            //分配的新的块
            uint32_t needWBk;

            remainSize = count - wLen;
            if(wOffset >= inode->i_file_size){
                //如果写入偏移大于或者等于文件大小，申请一块新的
                if (inode_alloc_new_bk(inode, &needWBk) < 0) {
                    return -1;
                }
            }else{
                //否则获取当前偏移的块号
                if (get_ofs_bk_no(inode,wOffset, &needWBk) < 0) {
                    return -1;
                }
            }
            //计算还需要写入多少
            wSize = remainSize > sb->s_bk_size ?  sb->s_bk_size : remainSize;
            if (wbk(sb->s_dev_no, needWBk, 0, buf + wLen, wSize) < 0) {
//                return -1;
            }
            wLen += wSize;
            wOffset += wSize;
        }

        if(wOffset > inode->i_file_size){
            //更新文件大小
            inode->i_file_size = wOffset;
            inode->i_dirt = 1;
        }
    }
    return wLen;
}