//
// Created by Administrator on 2022/1/16.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <mkrtos/sp.h>
#include <mkrtos/mem.h>
#include <mkrtos/bk.h>
#include <string.h>


/**
 * sp的sb申请
 * @param sb
 * @return
 */
struct super_block* sp_alloc_sb(struct super_block* sb){
    sb->s_sb_priv_info= OSMalloc(sizeof(struct sp_super_block));
    if(sb->s_sb_priv_info==NULL){
        return NULL;
    }
    memset(sb->s_sb_priv_info,0,sizeof(struct sp_super_block));
    return sb;
}


/**
 * 格式化sp文件系统
 * @param dev_no
 * @param inode_num
 * @return
 */
int32_t sp_mkfs(dev_t dev_no,int32_t inode_count){
    bk_no_t new_bk;
    ino_t new_inode;
    int32_t res=0;
    struct sp_super_block* sp_sb;
    struct super_block* sb;
    sb= OSMalloc(sizeof(struct super_block));
    sb->s_sb_priv_info=OSMalloc(sizeof(struct sp_super_block));
    sp_sb= sb->s_sb_priv_info;

    sb->s_dirt=1;
    sb->s_ops=&sp_s_ops;
    sb->s_dev_no=dev_no;
    sp_sb->iNodeCount= sb->s_inode_size=128;
    sb->s_bk_size=get_bk_size(dev_no);

    sp_sb->blockCount=get_bk_count(dev_no);
    //super_block开始的块位置
    sp_sb-> FSInfoStBkInx = 1;

    if(inode_count<=0){
        //自动inode个数为30%的块数
        sp_sb->iNodeCount = (get_bk_count(dev_no)*3)/10;
    }else {
        sp_sb->iNodeCount = inode_count;
    }

    //iNode统计使用需要多少块
    uint32_t iNodeUsedBkCn;
    iNodeUsedBkCn = (sp_sb->iNodeCount / 8 / sp_sb->iNodeSize) +
                    (((sp_sb->iNodeCount / 8) + (sp_sb->iNodeCount % 8) ? 1 : 0) / sp_sb->iNodeSize
                     + (((sp_sb->iNodeCount / 8) + (sp_sb->iNodeCount % 8) ? 1 : 0) % sp_sb->iNodeSize) ? 1 : 0)
            ;
    sp_sb->inode_used_bk_count = iNodeUsedBkCn;
    sp_sb->inode_used_bk_st_inx = sp_sb->FSInfoStBkInx + 1;

    //BK统计使用需要多少块
    uint32_t bkUsedBkCn;
    bkUsedBkCn = (sp_sb->blockCount / 8 / sb->s_bk_size) +
                 (((sp_sb->blockCount / 8) + (sp_sb->blockCount % 8) ? 1 : 0) / sb->s_bk_size
                  + (((sp_sb->blockCount / 8) + (sp_sb->blockCount % 8) ? 1 : 0) % sb->s_bk_size) ? 1 : 0)
            ;
    sp_sb->bkUsedBkCount = bkUsedBkCn;
    sp_sb->bkUsedBkStInx = sp_sb->inode_used_bk_count + sp_sb->inode_used_bk_st_inx;

    uint8_t* temp_mem= OSMalloc((sb->s_bk_size));
    memset(temp_mem,0xff,sb->s_bk_size);
    //写iNode使用块信息
    for (uint32_t i = 0; i < iNodeUsedBkCn; i++) {
        wbk(sb->s_dev_no, sp_sb->inode_used_bk_st_inx+i, temp_mem, 0, sb->s_bk_size);
    }

    //写bk使用块信息
    for (uint32_t i = 0; i < bkUsedBkCn; i++) {
        wbk(sb->s_dev_no,sp_sb->bkUsedBkStInx+i, temp_mem, 0, sb->s_bk_size);
    }

    //擦除iNode所占用的数据
    uint32_t eCn;
    eCn = (sp_sb->iNodeCount * sb->s_inode_size) / sb->s_bk_size
          + (((sp_sb->iNodeCount * sb->s_inode_size) % sb->s_bk_size) ? 1 : 0);

    sp_sb->iNodeDataBkStInx = sp_sb->bkUsedBkStInx + sp_sb->bkUsedBkCount;
    sp_sb->iNodeDataBkCount = eCn;

    //数据的开始位置
    sp_sb->dataBkStInx = sp_sb->iNodeDataBkStInx + sp_sb->iNodeDataBkCount;
    sp_sb->last_alloc_bk_inx=0;
    //写FSInfo到FS_INFO_ST_BK_INX块
    sb->s_magic_num=SP_MAGIC_NUM;

    sp_write_super(sb);

    //标记被FS默认使用的块
    for (uint32_t i = 0; i < sp_sb->dataBkStInx; i++) {
        set_bk_no_status(sb, i,1);
    }

    sb->s_dev_no=dev_no;

    //新建. .. 文件，并且都指向自己
    struct dir_item di;
    struct inode* r_inode;
    struct sp_inode *sp_ino;
    if(alloc_bk(sb,&new_bk)<0){
       res=-1;
       goto end;
    }
    if(alloc_inode_no(sb,&new_inode)<0){
        free_bk(sb,new_bk);
        res=-1;
        goto end;
    }
    r_inode = OSMalloc(sizeof(struct inode));
    if(r_inode==NULL){
        res=-1;
        //申请失败
        goto end1;
    }
    r_inode->i_fs_priv_info=NULL;
    //填充参数
    r_inode->i_file_size=sizeof(struct dir_item)*2;
    //设置为目录
    r_inode->i_type_mode=1<<16;
    //设置sb
    r_inode->i_sb=sb;
    r_inode->i_dirt=1;
    r_inode->i_no=new_inode;
    //硬链接数等于2
    r_inode->i_hlink=2;
    atomic_set(&(r_inode->i_used_count),1);
    //申请一个inode
    if(sp_alloc_inode(r_inode) == NULL){
        res=-1;
        //申请失败
        goto end2;
    }

    sp_ino=r_inode->i_fs_priv_info;
    //填充新的bk号
    sp_ino->p_ino[0]=new_bk;
    //根目录下的. .. 目录
    strcpy(di.name, ".");
    di.used = TRUE;
    di.inode_no = 0;

    if (wbk(sb->s_dev_no, new_bk,  (uint8_t*)(&di), 0, sizeof(di)) < 0) {
        res=-1;
        goto end2;
    }
    strcpy(di.name, "..");
    di.used = TRUE;
    di.inode_no = 0;

    if (wbk(sb->s_dev_no, new_bk,  (uint8_t*)(&di), sizeof(di), sizeof(di)) < 0) {
        res=-1;
        goto end2;
    }
    //写inode
    sp_write_inode(r_inode);
    //同步所有的块
    sync_all_bk(sb->s_dev_no);

    OSFree(r_inode);
    goto end;
    end2:
    sp_free_inode(r_inode);
    OSFree(r_inode);
    end1:
    free_bk(sb,new_bk);
    free_bk(sb,new_inode);
    end:
    OSFree(sb->s_sb_priv_info);
    OSFree(sb);
    return 0;
}
