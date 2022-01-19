//
// Created by Administrator on 2022/1/12.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <string.h>
#include <mkrtos/bk.h>
#include <errno.h>
#include <mkrtos/sp.h>
#define DIR_TYPE 1


/**
 * ����ĳ��Ŀ¼�µ��ļ���������inode
 * @param p_inode
 * @param file_name
 * @param len
 * @param res_inode
 * @return
 */
int sp_dir_find(struct inode* dir,const char* file_name,int len,ino_t* res_inode){
    int32_t ret;
    //Ŀ¼�ĸ���
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
            for (uint16_t j = 0; j < DIR_NUM(dir); j++) {
                struct dir_item pdi;
                if (tempi >= dir_num) {
                    return -ENOENT;
                }
                //��ȡĿ¼
                if (rbk(sb->s_dev_no, p_sp_inode->p_ino[i],
                        (uint8_t*)( &pdi), j*sizeof(struct dir_item),
                        sizeof(struct dir_item)) < 0) {
                    return -ENOENT;
                }
                if (strcmp(pdi.name, file_name) == 0 && pdi.used == TRUE) {
                    *res_inode = pdi.inode_no;
                    return tempi;
                }
                tempi++;
            }

        }
        else if (i < A_BK_NUM(p_sp_inode) + B_BK_NUM(dir->i_sb,p_sp_inode)) {

            uint32_t overNum = i - A_BK_NUM(p_sp_inode);
            uint32_t bkNo = overNum / BK_INC_X_NUM(dir->i_sb);
            uint32_t bkInx = overNum % BK_INC_X_NUM(dir->i_sb);
            uint32_t readBkInx;
            //��ȡINode
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
                if (strcmp(pdi.name, file_name) == 0 && pdi.used == TRUE) {
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
            //�����˵����ļ��Ĵ�С
            return -EFBIG;
        }
    }

    return -ENOENT;
}
/**
 * ����ָ��inode��file_name��inode��������
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
        return res;
    }
    r_inode = geti(p_inode->i_sb,res_ino);
    if(r_inode==NULL){
        return -ENOENT;
    }
    *res_inode=r_inode;
    return 0;
}
/**
 * ��һ���ļ����뵽Ŀ¼��inode��
 * @param dir
 * @param name
 * @param p_inode
 * @return
 */
int32_t add_file_to_entry(struct inode* dir, const char* name,struct inode* p_inode) {
    //�ж������ļ�
    uint32_t file_cn = 0;
    struct super_block *sb;
    //���õ�iNode����Ŀ¼
    if (FILE_TYPE(dir->i_type_mode) != DIR_TYPE) {
        return -ENOTDIR;
    }
    sb=dir->i_sb;
    file_cn = dir->i_file_size / sizeof(struct dir_item);
    //���ڣ���϶��Ǵ�����ݵ����һ����
    if (file_cn % (sb->s_bk_size / sizeof(struct dir_item)) == 0) {
        uint32_t new_bk = 0;
        int32_t ret;
        struct dir_item pdi;
        if ((ret = alloc_bk(sb,&new_bk)) < 0) {
            return ret;
        }
        strcpy(pdi.name, name);
        pdi.inode_no = p_inode->i_no;
        pdi.used = TRUE;
        dir->i_file_size += sizeof(struct dir_item);
        //д�ļ���Ϣ
        if (wbk(sb->s_dev_no, new_bk, 0, (uint8_t*)&pdi, sizeof(pdi)) < 0) {
            free_bk(sb,new_bk);
            return -1;
        }
    }
    else {
        //���һ�黹�п�λ
        uint32_t ofs_no = (file_cn % (sb->s_bk_size / sizeof(struct dir_item)));
        //��ȡ�ļ����һ��Ŀ��
        uint32_t bk_num = 0;
        struct dir_item pdi;
        if (get_bk_no_ofs(dir,dir->i_file_size, &bk_num) < 0) {
            return -1;
        }
        //��ȡ����
        if (rbk(sb->s_dev_no, bk_num, NULL, 0,  0) < 0) {
            return -1;
        }
        strcpy(pdi.name, name);
        pdi.inode_no =  p_inode->i_no;
        pdi.used = TRUE;
        //д��ȥ
        if (wbk(sb->s_dev_no, bk_num,  (uint8_t*)(&pdi),
                ofs_no * sizeof(struct dir_item),  sizeof(struct dir_item)) < 0) {
            return -1;
        }
        dir->i_file_size += sizeof(struct dir_item);
    }
    dir->i_dirt=1;
    return 0;
}
/**
 * �����Ǵ���һ���ļ�
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
    struct inode* new_inode=get_empty_inode();
    if (new_inode == NULL) {
        ret = ENOMEM;
        goto end;
    }
    new_inode->i_type_mode = mode;
    new_inode->i_dirt=1;

    //��ӵ�Ŀ¼��
    if(add_file_to_entry(dir,name,new_inode)<0) {
        ret = ERROR;
        puti(new_inode);
        //�ͷ�������ڴ�
        goto end;
    }
    *result = new_inode;
    end:
    return ret;
}
int sp_mknod(struct inode * dir, const char * name, int len, int mode, int rdev)
{

}
int sp_mkdir(struct inode * dir, const char * name, int len, int mode)
{
    bk_no_t new_bk;
    ino_t dir_ino;
    int32_t res;
    struct inode * inode;
    struct super_block *sb;
    struct sp_inode * info;
    //���ø�Ŀ¼����Ϣ
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
    inode->i_ops = NULL;
    inode->i_file_size = 2 * sizeof (struct dir_item);

    //�õ�һ���µĿ�
    if(alloc_bk(sb,&new_bk)<0){
        puti(dir);
        return -ENOSPC;
    }
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
        puti(dir);
        inode->i_hlink=0;
        puti(inode);
        return res;
    }

    dir->i_hlink++;
    dir->i_dirt=TRUE;
    puti(dir);
    puti(inode);
    return 0;
}
//�������Ŀ¼�Ƿ�Ϊ��
int empty_dir(struct inode* inode){
    //Ŀ¼�ĸ���
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
                //�ҵ�����ļ�
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
                //�ҵ�����ļ�
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
    //Ŀ¼�ĸ���
    uint32_t dirItemNum;
    uint32_t tempI = 0;
    //���õ�iNode����Ŀ¼
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
                //�ҵ�����ļ�
                if (pdi.inode_no == del->i_no && pdi.used == TRUE) {
                    //ɾ���ļ���Ϣ��
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
                //�ҵ�����ļ�
                if (pdi.inode_no == del->i_no
                    && pdi.used == TRUE
                        ) {
                    //ɾ���ļ���Ϣ��
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
            //�����˵����ļ��Ĵ�С
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

    //��Դ�ļ�������ָ����dirΪfalse
    if(del_dir_item(dir,inode)<0){
        //ɾ������
    }

    res=0;
    end:
    puti(dir);
    puti(inode);
    return res;
}