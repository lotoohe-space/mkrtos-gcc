//
// Created by Administrator on 2022/1/12.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <string.h>
#include <mkrtos/bk.h>
#include <errno.h>
#include <mkrtos/sp.h>
#include <mkrtos/task.h>
#include "mkrtos/stat.h"

#define DIR_TYPE 1


int32_t match(register char* str1,register const char *str2,int32_t len){
    uint32_t i;
    for(i=0;str2[i] && i<len;i++){
        if(str1[i]!=str2[i]){
            return str1[i]-str2[i];
        }
    }
    if(str1[i]!='\0'){
        return str1[i];
    }
    return 0;
}

/**
 * 查找某个目录下的文件，并返回inode
 * @param p_inode
 * @param file_name
 * @param len
 * @param res_inode
 * @return
 */
int sp_dir_find(struct inode* dir,const char* file_name,int len,ino_t* res_inode){
    int32_t ret;
    //目录的个数
    uint32_t dir_num;
    uint32_t inode_bk_num;
    uint32_t tempi = 0;
    struct sp_inode *p_sp_inode;
    struct super_block* sb;
    if (FILE_TYPE(dir->i_type_mode) != DIR_TYPE) {
        return -ENOTDIR;
    }
    sb=dir->i_sb;
    p_sp_inode=SP_INODE(dir);

    dir_num = dir->i_file_size / sizeof(struct dir_item);
    inode_bk_num = INODE_BK_NUM(dir);

    for (uint32_t i = 0; i < inode_bk_num; i++) {
        if (i < A_BK_NUM(p_sp_inode)) {
            struct bk_cache* bk_tmp;
            bk_tmp=bk_read(sb->s_dev_no,p_sp_inode->p_ino[i],0,0);
            for (uint16_t j = 0; j < DIR_NUM(dir); j++) {
                struct dir_item *pdi;
                if (tempi >= dir_num) {
                    bk_release(bk_tmp);
                    return -ENOENT;
                }
                //读取目录
//                if (rbk(sb->s_dev_no, p_sp_inode->p_ino[i],
//                        (uint8_t*)( &pdi), j*sizeof(struct dir_item),
//                        sizeof(struct dir_item)) < 0) {
//                    return -ENOENT;
//                }
                pdi=bk_tmp->cache+sizeof(struct dir_item)*j;
                if (match(pdi->name, file_name,len) == 0 && pdi->used == TRUE) {
                    *res_inode = pdi->inode_no;
                    bk_release(bk_tmp);
                    return tempi;
                }
                tempi++;
            }
            bk_release(bk_tmp);
        }
        else if (i < A_BK_NUM(p_sp_inode) + B_BK_NUM(dir->i_sb,p_sp_inode)) {

            uint32_t overNum = i - A_BK_NUM(p_sp_inode);
            uint32_t bkNo = overNum / BK_INC_X_NUM(dir->i_sb);
            uint32_t bkInx = overNum % BK_INC_X_NUM(dir->i_sb);
            uint32_t readBkInx;
            //读取INode
            if (rbk(sb->s_dev_no,
                    p_sp_inode->pp_ino[bkNo],
                    (uint8_t*)(&readBkInx),
                    bkInx*sizeof(uint32_t),
                    sizeof(uint32_t)) < 0
                    ) {
                return -ENOENT;
            }

            for (uint16_t j = 0; j < DIR_NUM(dir); j++) {
                struct dir_item pdi;
                if (tempi >= dir_num) {
                    return -ENOENT;
                }
                if (rbk(sb->s_dev_no, readBkInx,  (uint8_t*)(&pdi), j*sizeof(struct dir_item), sizeof(struct dir_item)) < 0) {
                    return -ENOENT;
                }
                if (match(pdi.name, file_name,len)  == 0 && pdi.used == TRUE) {
                    *res_inode = pdi.inode_no;
                    return tempi;
                }
                tempi++;
            }
        }
//        else if (i < A_BK_NUM(inode) + B_BK_NUM(pFsInfo, inode) + C_BK_NUM(pFsInfo, inode)) {
//
//        }
        else {
            //超过了单个文件的大小
            return -EFBIG;
        }
    }

    return -ENOENT;
}
/**
 * 查找指定inode下file_name的inode，并返回
 * @param p_inode
 * @param file_name
 * @param len
 * @param res_inode
 * @return
 */
int sp_lookup(struct inode* p_inode,const char* file_name,int len,struct inode** res_inode){
    ino_t res_ino;
    int res;
    struct inode* r_inode;
    res = sp_dir_find(p_inode,file_name,len,&res_ino);
    if(res<0){
        puti(p_inode);
        return res;
    }
    r_inode = geti(p_inode->i_sb,res_ino);
    if(r_inode==NULL){
        puti(p_inode);
        return -ENOENT;
    }
    puti(p_inode);

    if(res_inode) {
        *res_inode = r_inode;
    }
    return 0;
}
/**
 * 吧一个文件加入到目录的inode中
 * @param dir
 * @param name
 * @param p_inode
 * @return
 */
int32_t add_file_to_entry(struct inode* dir, const char* name,struct inode* p_inode) {
    //有多少项文件
    uint32_t file_cn = 0;
    struct super_block *sb;
    struct bk_cache* tmp_bk_ch;
    uint32_t file_size;
    //放置的iNode不是目录
    if (FILE_TYPE(dir->i_type_mode) != DIR_TYPE) {
        return -ENOTDIR;
    }
    sb=dir->i_sb;

    again:
    file_size=dir->i_file_size;
    file_cn = dir->i_file_size / sizeof(struct dir_item);
    //大于，则肯定是存放数据的最后一个块
    if (file_cn % (sb->s_bk_size / sizeof(struct dir_item)) == 0) {
        uint32_t new_bk = 0;
        int32_t ret;
        struct dir_item pdi;
        //给这个inode分配一个新的块
        if ((ret = inode_alloc_new_bk(dir,&new_bk)) < 0) {
            return ret;
        }
        strcpy(pdi.name, name);
        pdi.inode_no = p_inode->i_no;
        pdi.used = TRUE;
        tmp_bk_ch=bk_read(sb->s_dev_no,new_bk,1,0);
        if(dir->i_file_size!=file_size){
            bk_release(tmp_bk_ch);
            goto again;
        }
        printk("%s %d new bk %d\r\n",__FUNCTION__ ,__LINE__,new_bk);

        memcpy(tmp_bk_ch->cache,&pdi,sizeof(pdi));
        dir->i_file_size += sizeof(struct dir_item);
        bk_release(tmp_bk_ch);
    }
    else {
        //最后一块还有空位
        uint32_t ofs_no;
        //获取文件最后一块的块号
        uint32_t bk_num = 0;
        struct dir_item *pdi;
        uint32_t bk_num_tmp;

        if (get_bk_no_ofs(dir,dir->i_file_size, &bk_num) < 0) {
            return -1;
        }

        tmp_bk_ch=bk_read(sb->s_dev_no,bk_num,1,0);
        trace("旧文件大小%d,新文件大小%d\r\n",file_size,dir->i_file_size);
        if(file_size != dir->i_file_size){
            //需要重新检查块号
            if (get_bk_no_ofs(dir,dir->i_file_size, &bk_num_tmp) < 0) {
                bk_release(tmp_bk_ch);
                return -1;
            }
            if(bk_num!=bk_num_tmp){
                trace("块已经被修改\r\n");
                bk_release(tmp_bk_ch);
                //块号已经变了，重新计算写入位置
                goto again;
            }
            file_cn = dir->i_file_size / sizeof(struct dir_item);
        }
        trace("创建文件%s\r\n",name);

        ofs_no= (file_cn % (sb->s_bk_size / sizeof(struct dir_item)));

        //检查文件是否存在了
        for(uint32_t i=0;i<ofs_no;i++){
            pdi=(struct dir_item*)(tmp_bk_ch->cache+i * sizeof(struct dir_item));
            if(strncmp(pdi->name,name,sizeof(struct dir_item))==0&&pdi->used==TRUE){
                bk_release(tmp_bk_ch);
                return -EEXIST;
            }
        }
        pdi=(struct dir_item*)(tmp_bk_ch->cache+ofs_no * sizeof(struct dir_item));
        strcpy(pdi->name, name);
        pdi->inode_no =  p_inode->i_no;
        pdi->used = TRUE;
#if 1
#include <mkrtos/task.h>
        trace("进程%d,写块%d,偏移%d,旧文件大小%d,新文件大小%d\r\n",CUR_TASK->PID ,bk_num,ofs_no*sizeof(struct dir_item),file_size,dir->i_file_size);
#endif
        dir->i_file_size += sizeof(struct dir_item);
        bk_release(tmp_bk_ch);
    }
    dir->i_dirt=1;
    dir->i_hlink++;
    return 0;
}
/**
 * 这里是创建一个文件
 * @param dir
 * @param name
 * @param len
 * @param mode
 * @param result
 * @return
 */
int sp_create(struct inode *dir,const char *name,int len,int mode,struct inode ** result){
    int32_t ret = 0;
    struct super_block *p_sb;
    struct inode* new_inode=sp_new_inode(dir);
    if (new_inode == NULL) {
        puti(dir);
        ret = -ENOMEM;
        goto end;
    }
    new_inode->i_type_mode = mode&0xffff;
    new_inode->i_dirt=1;
    new_inode->i_ops=&sp_file_inode_operations;

    //添加到目录中
    if((ret=add_file_to_entry(dir,name,new_inode))<0) {
        new_inode->i_hlink--;
        puti(dir);
        puti(new_inode);
        //释放申请的内存
        goto end;
    }
    puti(dir);
    *result = new_inode;
    end:
    return ret;
}
int sp_mknod(struct inode * dir, const char * name, int len, int mode, int rdev)
{
    int error;
    struct inode * inode;
    int res;
    ino_t res_ino;
    if (!dir)
        return -ENOENT;
    res = sp_dir_find(dir,name,len,&res_ino);
    if (res>=0) {
        puti(dir);
        return -EEXIST;
    }
    inode = sp_new_inode(dir);
    if (!inode) {
        puti(dir);
        return -ENOSPC;
    }
//    inode->i_uid = current->euid;
    inode->i_type_mode = mode;
    inode->i_ops = NULL;
    if (IS_FILE(inode->i_type_mode))
        inode->i_ops = &sp_file_inode_operations;
    else if (IS_DIR_FILE(inode->i_type_mode)) {
        inode->i_ops = &sp_dir_inode_operations;
//        if (dir->i_type_mode & S_ISGID)
//            inode->i_mode |= S_ISGID;
    }
//    else if (S_ISLNK(inode->i_mode))
//        inode->i_op = &minix_symlink_inode_operations;
    else if (IS_CHAR_FILE(inode->i_type_mode))
        inode->i_ops = &chrdev_inode_operations;
    else if (IS_BK_FILE(inode->i_type_mode))
        inode->i_ops = &blkdev_inode_operations;
//    else if (S_ISFIFO(inode->i_mode))
//        init_fifo(inode);
    if (IS_CHAR_FILE(inode->i_type_mode) || IS_BK_FILE(inode->i_type_mode))
        inode->i_rdev_no = rdev;
    inode->i_dirt = 1;
    error = add_file_to_entry(dir, name, inode);
    if (error) {
        inode->i_hlink--;
        inode->i_dirt = 1;
        puti(inode);
        puti(dir);
        return error;
    }
    puti(dir);
    puti(inode);
    return 0;
}
int sp_mkdir(struct inode * dir, const char * name, int len, int mode)
{
    bk_no_t new_bk;
    ino_t dir_ino;
    int32_t res;
    struct inode * inode;
    struct super_block *sb;
    struct sp_inode * info;
    //设置根目录的信息
    struct dir_item di;
    if (!dir || !dir->i_sb) {
        puti(dir);
        return -EINVAL;
    }
    sb=dir->i_sb;
//    info = ( struct sp_super_block * )(dir->i_fs_priv_info);
    if(sp_dir_find(dir,name,len,&dir_ino)>=0){
        puti(dir);
        return -EEXIST;
    }
    inode = sp_new_inode(dir);
    if (!inode) {
        puti(dir);
        return -ENOSPC;
    }
    inode->i_ops = &sp_dir_inode_operations;
    inode->i_file_size = 2 * sizeof (struct dir_item);
    inode->i_type_mode=1<<16;
    //得到一个新的块
    if(alloc_bk(sb,&new_bk)<0){
        puti(dir);
        return -ENOSPC;
    }
    trace("进程%d,函数%s,行%d,新块号%d\r\n",CUR_TASK->PID,__FUNCTION__ ,__LINE__,new_bk);
    strcpy(di.name, ".");
    di.used = TRUE;
    di.inode_no = inode->i_no;

    if (wbk(sb->s_dev_no, new_bk,
            (uint8_t*)(&di),
            0,
            sizeof(di)) < 0) {
    }
    strcpy(di.name, "..");
    di.used = TRUE;
    di.inode_no = dir->i_no;
    if (wbk(sb->s_dev_no,
            new_bk,
            (uint8_t*)(&di),
            sizeof(di),
            sizeof(di)) < 0) {
    }

    info = (struct sp_inode*)(inode->i_fs_priv_info);
    info->p_ino[0]=new_bk;
    inode->i_dirt=TRUE;
    inode->i_hlink=2;
    if((res=add_file_to_entry(dir,name,inode))<0) {
        inode->i_hlink=0;
        puti(dir);
        puti(inode);
        return res;
    }

    dir->i_hlink++;
    dir->i_dirt=TRUE;
//    sp_sync_inode(dir);
//    sync_all_bk(sb->s_dev_no);
    puti(dir);
    puti(inode);
    return 0;
}
//计算这个目录是否为空
int empty_dir(struct inode* inode){
    //目录的个数
    uint32_t dirItemNum;
    uint32_t tempI = 0;
    struct super_block* sb;
    struct sp_inode* sp_ino;
    if(!IS_DIR_FILE(inode->i_type_mode)){
        return 0;
    }
    if(inode->i_file_size==0){
        return 1;
    }
    sb=inode->i_sb;
    sp_ino=inode->i_fs_priv_info;
    dirItemNum = inode->i_file_size / sizeof(struct dir_item);
    uint32_t fileUsedBKNum = FILE_USED_BK_NUM(sb, inode);

    for (uint32_t i = 0; i < fileUsedBKNum; i++) {
        if (i < A_BK_NUM(sp_ino)) {
            for (uint16_t j = 0; j < DIR_ITEM_NUM_IN_BK(sb); j++) {
                if (tempI >= dirItemNum) {
                    return 0;
                }
                struct dir_item pdi;
                if (rbk(sb->s_dev_no, sp_ino->p_ino[i], (uint8_t*)( &pdi), j*sizeof(struct dir_item),  sizeof(struct dir_item)) < 0) {
                    return 0;
                }
                //找到这个文件
                if (pdi.used == TRUE) {
                    return 0;
                }

                tempI++;
            }
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {

            uint32_t overNum = i - A_BK_NUM(sp_ino);
            uint32_t bkNo = overNum / BK_INC_X_NUM(sb);
            uint32_t bkInx = overNum % BK_INC_X_NUM(sb);
            uint32_t delBkInx;
            if (rbk(sb->s_dev_no, sp_ino->pp_ino[ bkNo],  (uint8_t*)(&delBkInx), bkInx*sizeof(uint32_t),  sizeof(uint32_t)) < 0) {
                return 0;
            }

            for (uint16_t j = 0; j < DIR_ITEM_NUM_IN_BK(sb); j++) {
                if (tempI >= dirItemNum) {
                   return 0;
                }
                struct dir_item pdi;
                if (rbk(sb->s_dev_no,delBkInx,  (uint8_t*)(&pdi), j*sizeof(struct dir_item), sizeof(struct dir_item)) < 0) {
                    return 0;
                }
                //找到这个文件
                if ( pdi.used == TRUE) {
                    return 0;
                }
                tempI++;
            }
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {

        }
        else {
            return 0;
        }
    }

    return 1;
}

int32_t del_dir_item(struct inode *ino,struct inode* del){
    struct sp_inode* sp_ino;
    struct super_block *sb;
    //目录的个数
    uint32_t dirItemNum;
    uint32_t tempI = 0;
    //放置的iNode不是目录
    if (IS_DIR_FILE(ino->i_type_mode)) {
        return -1;
    }
    sb=ino->i_sb;
    sp_ino=(struct sp_inode*)(ino->i_fs_priv_info);
    dirItemNum = ino->i_file_size / sizeof(struct dir_item);
    uint32_t fileUsedBKNum = FILE_USED_BK_NUM(sb, ino);

    for (uint32_t i = 0; i < fileUsedBKNum; i++) {
        if (i < A_BK_NUM(sp_ino)) {
            for (uint16_t j = 0; j < DIR_ITEM_NUM_IN_BK(sb); j++) {
                if (tempI >= dirItemNum) {
                    return -1;
                }
                struct dir_item pdi;
                if (rbk(sb->s_dev_no, sp_ino->p_ino[i], (uint8_t*)( &pdi), j*sizeof(struct dir_item),  sizeof(struct dir_item)) < 0) {
                    return -1;
                }
                //找到这个文件
                if (pdi.inode_no == del->i_no && pdi.used == TRUE) {
                    //删除文件信息，
                    pdi.used = FALSE;
                    if (wbk(sb->s_dev_no, sp_ino->pp_ino[i],   (uint8_t*)(&pdi), j*sizeof(struct dir_item),  sizeof(struct dir_item)) < 0) {
                        return -1;
                    }
                    return 0;
                }

                tempI++;
            }
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino)) {

            uint32_t overNum = i - A_BK_NUM(sp_ino);
            uint32_t bkNo = overNum / BK_INC_X_NUM(sb);
            uint32_t bkInx = overNum % BK_INC_X_NUM(sb);
            uint32_t delBkInx;
            if (rbk(sb->s_dev_no, sp_ino->pp_ino[ bkNo],  (uint8_t*)(&delBkInx), bkInx*sizeof(uint32_t),  sizeof(uint32_t)) < 0) {
                return -1;
            }

            for (uint16_t j = 0; j < DIR_ITEM_NUM_IN_BK(sb); j++) {
                if (tempI >= dirItemNum) {
                    return -1;
                }
                struct dir_item pdi;
                if (rbk(sb->s_dev_no,  delBkInx,  (uint8_t*)(&pdi), j*sizeof(struct dir_item), sizeof(struct dir_item)) < 0) {
                    return -1;
                }
                //找到这个文件
                if (pdi.inode_no == del->i_no
                    && pdi.used == TRUE
                        ) {
                    //删除文件信息，
                    pdi.used = FALSE;
                    if (wbk(sb->s_dev_no, delBkInx,  (uint8_t*)(&pdi), j*sizeof(struct dir_item), sizeof(struct dir_item)) < 0) {
                        return -1;
                    }
                    return 0;
                }

                tempI++;
            }
        }
        else if (i < A_BK_NUM(sp_ino) + B_BK_NUM(sb, sp_ino) + C_BK_NUM(sb, sp_ino)) {

        }
        else {
            //超过了单个文件的大小
            return -1;
        }
    }

    return -1;
}


int sp_rmdir(struct inode * dir, const char * name, int len)
{
    int32_t res;
    ino_t dir_ino;
    struct inode * inode;
    res=-ENOENT;
    if((res=sp_dir_find(dir,name,len,&dir_ino))<0){
        goto end;
    }
    if((inode=geti(dir->i_sb,dir_ino))==NULL){
        res=-EPERM;
        goto end;
    }
    if(inode->i_sb->s_dev_no!=dir->i_sb->s_dev_no){
        goto end;
    }
    if(!IS_DIR_FILE(inode->i_type_mode)){
        res=-ENOENT;
        goto end;
    }
    if(!empty_dir(dir)){
        res=-ENOTEMPTY;
        goto end;
    }

    if(atomic_read(&(inode->i_used_count))>1){
        res=-EBUSY;
        goto end;
    }
    if(inode->i_hlink!=2){
        //
    }
    inode->i_hlink=0;
    inode->i_dirt=1;
    dir->i_hlink--;
    dir->i_dirt=1;

    //在源文件中设置指定的dir为false
    if(del_dir_item(dir,inode)<0){
        //删除错误
    }

    res=0;
    end:
    puti(dir);
    puti(inode);
    return res;
}
//断开一次硬链接
int sp_unlink(struct inode * dir, const char * name, int len){
    int retval;
    struct inode * inode;
    ino_t de;

    repeat:
    retval = -ENOENT;
    inode = NULL;
    retval = sp_dir_find(dir,name,len,&de);
    if (retval<0)
        goto end_unlink;
    if (!(inode = geti(dir->i_sb, de)))
        goto end_unlink;
    retval = -EPERM;
    if (IS_DIR_FILE(inode->i_type_mode))
        goto end_unlink;
    if (de != inode->i_no) {
        puti(inode);
        task_sche();
        goto repeat;
    }
//    if ((dir->i_mode & S_ISVTX) && !suser() &&
//        current->euid != inode->i_uid &&
//        current->euid != dir->i_uid)
//        goto end_unlink;
    if (de != inode->i_no) {
        retval = -ENOENT;
        goto end_unlink;
    }
    if (!inode->i_hlink) {
        printk("删除不存在的文件 (%04x:%lu), %d\n", inode->i_sb->s_dev_no,inode->i_no,inode->i_hlink);
        inode->i_hlink=1;
    }
//    dir->i_ctime = dir->i_mtime = CURRENT_TIME;
    dir->i_dirt = 1;
    inode->i_hlink--;
//    inode->i_ctime = dir->i_ctime;
    inode->i_dirt = 1;
    retval = 0;
    end_unlink:
    puti(inode);
    puti(dir);
    return retval;
}
int sp_symlink(struct inode * dir, const char * name, int len, const char * symname)
{
    ino_t ino;
    struct inode * inode = NULL;
    int i;
    char c;
    int res;
    struct bk_cache *tmp;

    if (!(inode = sp_new_inode(dir))) {
        puti(dir);
        return -ENOSPC;
    }
    inode->i_type_mode = S_IFLNK | 0777;
    inode->i_ops = &sp_symlink_inode_operations;

    bk_no_t no_t;
    if(alloc_bk(dir->i_sb,&no_t)<0){
        free_inode_no(dir->i_sb,inode->i_no);
        puti(inode);
        return -ENOSPC;
    }
    ((struct sp_inode*)inode->i_fs_priv_info)->p_ino[0]=no_t;
    tmp=bk_read(dir->i_sb->s_dev_no,no_t,1,0);
    i = 0;
    while (i < dir->i_sb->s_bk_size && (c=*(symname++)))
        tmp->cache[i++] = c;
    tmp->cache[i] = 0;
    bk_release(tmp);

    inode->i_file_size = i;
    inode->i_dirt = 1;
    res = sp_dir_find(dir,name,len,&ino);
    if (res>=0) {
        //目录下已经有文件了
        inode->i_hlink--;
        inode->i_dirt = 1;
        puti(inode);
        puti(dir);
        return -EEXIST;
    }
    i = add_file_to_entry(dir, name, inode);
    if (i<0) {
        inode->i_hlink--;
        inode->i_dirt = 1;
        puti(inode);
        puti(dir);
        return i;
    }
    puti(dir);
    puti(inode);
    return 0;
}
int sp_link(struct inode * oldinode, struct inode * dir, const char * name, int len)
{
    int error;
    ino_t ino;
    if (IS_DIR_FILE(oldinode->i_type_mode)) {
        puti(oldinode);
        puti(dir);
        return -EPERM;
    }

    error = sp_dir_find(dir,name,len,&ino);
    if (error>=0) {
        puti(dir);
        puti(oldinode);
        return -EEXIST;
    }
    error = add_file_to_entry(dir, name, oldinode);
    if (error<0) {
        puti(dir);
        puti(oldinode);
        return error;
    }
    puti(dir);
    oldinode->i_hlink++;
    oldinode->i_dirt = 1;
    puti(oldinode);
    return 0;
}
int sp_rename(struct inode * old_dir, const char * old_name, int old_len,
                 struct inode * new_dir, const char * new_name, int new_len){

    return -ENOSYS;
}