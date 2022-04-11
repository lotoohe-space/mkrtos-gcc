//
// Created by Administrator on 2022/1/12.
//

#ifndef UNTITLED1_SP_H
#define UNTITLED1_SP_H

#include <type.h>


#define SP_MAGIC_NUM 0x11223344

//a除b向上取整数
#define ROUND_UP(a,b) ((a)/(b)+((a)%(b)?1:0))
//向下取整
#define ROUND_DOWN(a,b) ((a)/(b))
#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))
//数组长度
#define ARRARY_LEN(a) (sizeof(a)/sizeof((a)[0]))

//一个块中可以存放多少个INode
#define INODE_NUM_IN_BK(a) ((a)->s_bk_size/(a)->s_inode_size)

//sp文件系统的私有信息
#define SP_INODE(a) ((struct sp_inode*)((a)->i_fs_priv_info))
//一个BK最多可以包含多少个INode或者BK号
#define BK_INC_X_NUM(sb) ((sb)->s_bk_size / sizeof(bk_no_t))
//一个块里面有多少文件项
#define DIR_NUM(p_inode) (p_inode->i_sb->s_bk_size/sizeof(struct dir_item))
//文件使用了多少块
#define INODE_BK_NUM(p_inode) ROUND_UP(p_inode->i_file_size, p_inode->i_sb->s_bk_size)
//一级bk块个数
#define A_BK_NUM(p_inode)	ARRARY_LEN((p_inode)->p_ino)
//二级bk块个数
#define B_BK_NUM(sb,p_inode)	(BK_INC_X_NUM(sb) * ARRARY_LEN((p_inode)->pp_ino))
//三级bk块个数
#define C_BK_NUM(sb,p_inode)  BK_INC_X_NUM(sb) * BK_INC_X_NUM(sb) * ARRARY_LEN(p_inode->ppp_ino)
//文件使用了多少块
#define FILE_USED_BK_NUM(sb,p_inode) ROUND_UP(p_inode->i_file_size, sb->s_bk_size)
//一个块里面有多少文件项
#define DIR_ITEM_NUM_IN_BK(a) ((a)->s_bk_size/sizeof(struct dir_item))


#define DIR_ITEM_SIZE 128
//存储的目录项，必须是2的整数倍
struct dir_item{
    char name[DIR_ITEM_SIZE-sizeof(di_size_t)-sizeof(uint32_t)];
    di_size_t inode_no;
    uint32_t used;
};

//#if (sizeof(struct dir_item)>128)
//#error "dir_item over 128."
//#endif
//这里是sp文件系统的私有inode信息
struct sp_inode{
    ino_t p_ino[12];
    ino_t pp_ino[6];
    ino_t ppp_ino[3];
};

//sp文件系统私有的sb信息
struct sp_super_block{
//    uint32_t magicNum;
    //块总数
    uint32_t blockCount;
    //空余块数
    uint32_t blockFree;

    //iNode大小
    uint32_t iNodeSize;
    //iNode总数
    uint32_t iNodeCount;
    //iNode空余
    uint32_t iNodeFree;

    //文件系统信息块启动位置
    uint32_t FSInfoStBkInx;

    //iNode使用块启动位置
    uint32_t inode_used_bk_st_inx;
    //iNode使用块 占用多少块
    uint32_t inode_used_bk_count;

    //bk使用块启动位置
    uint32_t bkUsedBkStInx;
    //bk使用块 占用了多少块
    uint32_t bkUsedBkCount;

    //iNode数据开始位置
    uint32_t iNodeDataBkStInx;
    uint32_t iNodeDataBkCount;
    //数据块开始位置
    uint32_t dataBkStInx;

    uint32_t last_alloc_bk_inx;
};
//sp_inode.c
extern struct super_operations sp_s_ops;
struct inode * sp_alloc_inode(struct inode * p_inode);
int32_t sp_free_inode(struct inode *p_inode);
void sp_write_inode (struct inode * i_node);

struct inode* sp_new_inode(struct inode* p_inode);


struct super_block* sp_alloc_sb(struct super_block* sb);
struct super_block* sp_read_sb(struct super_block* sb);
int32_t set_bk_no_status(struct super_block *sb, uint32_t usedBkInx, uint8_t status) ;
void sp_write_super (struct super_block * sb);

int32_t alloc_bk(struct super_block* sb,bk_no_t *res_bk);
int32_t free_bk(struct super_block* sb,bk_no_t bk_no);
int32_t alloc_inode_no(struct super_block* sb,ino_t *res_ino);
int32_t free_inode_no(struct super_block*sb,ino_t ino);
int32_t check_inode_no(struct super_block*sb,ino_t ino);
int sp_sync_inode(struct inode * inode);
int32_t get_bk_no_ofs(struct inode* p_inode, uint32_t offset,uint32_t* bk_no) ;

//sp_namei.c
int sp_lookup(struct inode* p_inode,const char* file_name,int len,struct inode** res_inode);
int sp_create(struct inode *dir,const char *name,int len,int mode,struct inode ** result);
int sp_mknod(struct inode * dir, const char * name, int len, int mode, int rdev);
int sp_mkdir(struct inode * dir, const char * name, int len, int mode);
int sp_rmdir(struct inode * dir, const char * name, int len);
int sp_link(struct inode * oldinode, struct inode * dir, const char * name, int len);
int sys_unlink(const char * pathname);
int sp_symlink(struct inode * dir, const char * name, int len, const char * symname);
int sp_rename(struct inode * old_dir, const char * old_name, int old_len,
              struct inode * new_dir, const char * new_name, int new_len);
int sp_unlink(struct inode * dir, const char * name, int len);
//sp_super.c
int32_t sp_mkfs(dev_t dev_no,int32_t inode_count);

//sp_file.c
int sp_file_read(struct inode * inode, struct file * filp, char * buf, int count);
int sp_file_write(struct inode * inode, struct file * filp, char * buf, int count);

//dir.c
int sp_readdir(struct inode * inode, struct file * filp, struct dirent * dirent, int count);
int sp_dir_read(struct inode * inode, struct file * filp, char * buf, int count);

//sp_sync.c
int sp_sync_file(struct inode * inode, struct file * file) ;

//sp_file.c
extern struct inode_operations sp_file_inode_operations;
//sp_dir.c
extern struct inode_operations sp_dir_inode_operations;
//sp_rw.c
int32_t inode_alloc_new_bk(struct inode* inode, uint32_t* newBkNum);
int32_t get_ofs_bk_no(struct inode* inode, uint32_t offset,uint32_t* fpBkNum);

//sp_truncate.c
void sp_truncate(struct inode* inode,int32_t len);

//sp_link.c
extern struct inode_operations sp_symlink_inode_operations;

//sp_bmap.c
int32_t get_free_bk(struct super_block* sb);
int32_t get_free_inode(struct super_block* sb);


#endif //UNTITLED1_SP_H
