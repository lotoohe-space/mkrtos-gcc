//
// Created by Administrator on 2022/1/10.
//

#ifndef UNTITLED1_FS_H
#define UNTITLED1_FS_H

#include <errno.h>
#include <type.h>
#include <arch/atomic.h>

//当前进程根目录INODE
#define ROOT_INODE ((struct inode*)(sysTasks.currentTask->root_inode))
//当前进程工作目录INODE
#define PWD_INODE ((struct inode*)(sysTasks.currentTask->pwd_inode))

struct file_operations;
struct inode_operations;
struct super_block;

#define MK_MODE(a,b) (((a))|(b))
//取得文件类型
#define FILE_TYPE(a) (((a)>>16)&0xffff)
//取得文件模式
#define FILE_MODE(a) ((a)&0xffff)

typedef struct {
    long    val[2];
} fsid_t;

/* 文件系统的信息结构 */
struct statfs {
    long f_type;
    long f_bsize;
    long f_blocks;
    long f_bfree;
    long f_bavail;
    long f_files;
    long f_ffree;
    fsid_t f_fsid;
    long f_namelen;
    long f_spare[6];
};

struct wait_queue;
//INode节点
typedef struct inode {
    //文件类型与权限
    //
    uint32_t i_type_mode;
    //文件大小
    uint32_t i_file_size;
    //自己的Inode号码
    uint32_t i_no;
    //硬连接数
    uint32_t i_hlink;
    //如果是设备文件，则这个代表设备号
    uint32_t i_rdev_no;

    //上面的数据需要存到磁盘
    struct inode *i_mount;
    struct wait_queue *i_wait_q;//这个inode的等待队列
    Atomic_t i_lock;//用来锁这个inode
    //打开计数
//    Atomic_t i_open_count;
    Atomic_t i_used_count;//使用计数
    struct wait_queue *poll_wait;//poll_wait的inode 的队列
//    struct wait_queue *w_wait;//等待写这个inode 的队列
    uint8_t i_dirt;//是否被修改过
    struct super_block *i_sb; //inode的sb
    struct inode_operations* i_ops;//inode的ops
    //文件系统编号，通过该编号查找到对应的文件系统
//    uint32_t i_fs_no;
    void *i_fs_priv_info;//文件系统拥有的私有信息
}*p_inode, inode;

//SupberBlock节点
typedef struct super_block {
    //魔数
    uint32_t s_magic_num;
    //块大小
    uint32_t s_bk_size;
    //块设备的设备号
    uint32_t s_dev_no;
    //inode写入到磁盘的大小
    uint16_t s_inode_size;

    //文件系统编号，通过该编号查找到对应的文件系统
    uint32_t s_fs_no;
    //文件系统根节点的inode
    struct inode *root_inode;
    struct inode *s_covered;
    //是否修改过
    uint8_t s_dirt;
    //super操作函数
    struct super_operations* s_ops;
    //文件系统拥有的私有信息
    void *s_sb_priv_info;
}*p_super_block, super_block;


struct file {
    uint32_t f_mode;		    /* 文件不存在时，创建文件的权限 */
    int32_t f_ofs;            /* 文件读写偏移量 */
    dev_t  f_rdev;             /* 这是tty设备所对于的字符设备驱动号 */
//    uint32_t f_count;           /*这个file被使用了多少次,暂时用不上*/
    unsigned short f_flags; /* 以什么样的方式打开文件，如只读，只写等等 */
    struct inode * f_inode;		/* 文件对应的inode */
    struct file_operations * f_op; /*文件对应的操作符*/
    uint32_t used;   /*是否被使用标记*/

    int fo_flag;

    int net_sock;/*网络sock*/
    uint8_t net_file;/*网络文件*/
//    uint32_t sock_used_cn;/*网络文件使用计数*/
};

#include <dirent.h>
//struct dirent
//{
//    long d_ino; /*索引节点号 */
//    int32_t d_off; /*在目录文件中的偏移 */
//    uint16_t d_file_name_len; /*文件名长 */
////    unsigned char d_type; /*文件类型 */
//    char d_name [128+1]; /*文件名，最长128字符 */
//};

/* 对inode对应文件的操作
 */
struct file_operations {
    int (*lseek) (struct inode *, struct file *, off_t, int);
    int (*read) (struct inode *, struct file *, char *, int);
    int (*write) (struct inode *, struct file *, char *, int);
    int (*readdir) (struct inode *, struct file *, struct dirent *, int);
//    int (*select) (struct inode *, struct file *, int, uint32_t *);
    int (*poll) (struct inode * inode,struct file *fp, void *wait_tb,struct timeval *timeout);
    int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
    int (*mmap) (struct inode * inode, struct file * fp, unsigned long addr, size_t len , int mask, unsigned long off);
    int (*mumap)(struct inode * inode,void * start ,size_t len);
    int (*open) (struct inode * inode, struct file * fp);
    void (*release) (struct inode * ino, struct file * f);
    int (*fsync) (struct inode *, struct file *);
};

/* 对inode的操作
 */
struct inode_operations {
    struct file_operations * default_file_ops;
    int (*create) (struct inode *dir,const char *name,int len,int mode,struct inode **result);
    int (*lookup) (struct inode * dir,const char * file_name,int len,struct inode ** ret_inode);
    int (*link) (struct inode *,struct inode *,const char *,int);
    int (*unlink) (struct inode *,const char *,int);
    int (*symlink) (struct inode *,const char *,int,const char *);
    int (*mkdir) (struct inode *,const char *,int,int);
    int (*rmdir) (struct inode *,const char *,int);
    int (*mknod) (struct inode *,const char *,int,int,int);
    int (*rename) (struct inode *,const char *,int,struct inode *,const char *,int);
    int (*readlink) (struct inode *,char *,int);
    int (*follow_link) (struct inode *,struct inode *,int,int,struct inode **);
    int (*bmap) (struct inode *,int);
    void (*truncate) (struct inode *,int);
    int (*permission) (struct inode *, int);
};

struct super_operations {
    //申请指定文件系统的inode
    struct inode * (*alloc_inode)(struct inode * p_inode);
    //释放指定文件系统的inode
    int32_t (*free_inode)(struct inode *p_inode);

    int32_t (*read_inode) (struct inode * p_r_ino);
    int (*notify_change) (int flags, struct inode *);
    void (*write_inode) (struct inode *);
    void (*put_inode) (struct inode *);
    void (*put_super) (struct super_block *);
    void (*write_super) (struct super_block *);
    void (*statfs) (struct super_block *, struct statfs *);
    int (*remount_fs) (struct super_block *, int *, char *);
};



struct fs_type{
    //文件系统的名字
    const char* f_name;
    //是否需要设备号
    uint8_t req_dev_no;
    //读这个文件系统的操作块
    struct super_block * (*read_sb)(struct super_block * sb);
    struct super_block* (*alloc_sb)(struct super_block* sb);
    void (*free_sb)(struct super_block* sb);
};

//fs_type.c文件系统类型相关
extern struct fs_type fs_type_list[];
extern int32_t fs_type_len;
struct fs_type *find_fs_type(const char *fs_name);

//open.c
//
//void sys_close(int fd);

//inode.c 根目录的设备号
void lose_inode(struct inode* p_inode);
struct inode* get_empty_inode(void);
struct inode* geti(struct super_block* p_sb,ino_t ino);
int32_t puti(struct inode* put_inode);

void wait_on_inode(struct inode* inode);
void lock_inode(struct inode* inode);
void unlock_inode(struct inode* inode);
int sync_all_inode(void);


//namei.c
int namei(const char * pathname, struct inode ** res_inode);
int lnamei(const char * pathname, struct inode ** res_inode);
int32_t dir_namei(const char * pathname, int32_t * namelen, const char ** name,
                  struct inode * base, struct inode ** res_inode);
int32_t open_namei(const char* file_path,int32_t flags,int32_t mode,struct inode **res_inode,struct inode *base_dir);


//dev.c
extern dev_t root_dev_no;
extern struct inode_operations chrdev_inode_operations;
extern struct inode_operations blkdev_inode_operations;
struct bk_operations{
    int32_t (*open)(uint32_t bk_no);
    int32_t (*read_bk)(uint32_t bk_no,uint8_t *data);
    int32_t (*write_bk)(uint32_t bk_no,uint8_t *data);
    int32_t (*erase_bk)(uint32_t bk_no);
    void (*release)(uint32_t bk_no);
};
int32_t get_bk_cn(dev_t major_no,uint32_t *res_bk_no);
uint32_t get_bk_size(dev_t major_no) ;
struct bk_operations* get_bk_ops(dev_t major_no);
uint32_t get_bk_count(dev_t major_no);
struct bk_cache* get_bk_dev_cache(dev_t major_no,uint32_t *res_cache_len);
int32_t reg_bk_dev(
        dev_t major_no,
        const char* name ,
        struct bk_operations *bk_ops,
        struct file_operations *file_ops
//                ,
//        uint32_t bk_count,
//        uint32_t cache_len,
//        uint32_t bk_size
);
int32_t bk_dev_reg_param(dev_t major_no,uint32_t bk_count,uint32_t cache_len,uint32_t bk_size) ;
int32_t bk_dev_unreg_param(dev_t major_no) ;
int32_t unreg_bk_dev(dev_t major_no,const char* name);
struct file_operations* get_ch_dev_ops(dev_t major_no);
int32_t reg_ch_dev(dev_t major_no,const char* name ,struct file_operations *d_fops);
int32_t unreg_ch_dev(dev_t major_no,const char* name);
int32_t request_bk_no(dev_t dev_no);
int32_t alloc_bk_no();
int32_t request_char_no(dev_t dev_no);
void lock_bk_ls(dev_t bk_dev_no);
void unlock_bk_ls(dev_t bk_dev_no);
void devs_init(void);
int sync_all_bk_dev(void) ;
/////////

//bk.c
struct wait_queue;

//块缓存相关
struct bk_cache {
    //块的Inx
    uint32_t bk_no;
    uint32_t bk_size;
    //缓存擦除不需要缓存，则为Null
    uint8_t* cache;
    uint8_t* oldcache;
    Atomic_t b_lock;
    //使用计数
    uint32_t used_cn;
    //等待队列
    struct wait_queue* b_wait;
    //擦除标记 1bit写入 2bit读取 (4bit 永久锁住，除非被使用者释放掉) (5bit mmap映射)  (6bit 7bit 00为空 01被使用 10被删除)
    uint8_t flag;
};
#define KEY_ACCESS(a) ((a)&0xC0)
//是否支持某种访问条件
#define IS_NULLKEY(a)   (KEY_ACCESS((a).flag)==0x00)
#define IS_USEDKEY(a)   (KEY_ACCESS((a).flag)==0x40)
#define IS_DELKEY(a)    (KEY_ACCESS((a).flag)==0x80)

//是否被mmap了
#define IS_MMAP(a)      ((a).flag&((1<<5)))
//是否冻结了
#define IS_FREEZE(a)    ((a).flag&((1<<4)))

#define SET_FREEZE(a)   (a).flag&=(~(1<<4));(a).flag|=(1<<4)
//mmap位设置后freeze一定要设置
#define SET_MMAP(a)     SET_FREEZE(a);(a).flag&=(~(1<<5));(a).flag|=(1<<5)

#define SET_NULLKEY(a)  (a).flag&=~0xC0
#define SET_USEDKEY(a)  (a).flag&=~0xC0;(a).flag|=0x40
#define SET_DELKEY(a)   (a).flag&=~0xC0;(a).flag|=0x80
#define CLR_KEY(a)  (a).flag=0;
////////


//super.c
struct super_block* get_empty_sb(void);
void free_sb(struct super_block* sb);
int sync_all_sb(void);

//super.c
struct task;
void root_mount(struct task* task);

//file_name.c
int32_t _getname(const char* file_name,uint32_t len,char** to);
int32_t getname(const char* file_name, char** to);
void putname(const char* file_name);
char* path_name(char* file_path);

//mmap.c
int general_mmap(struct inode * inode, struct file * fp, void* addr, size_t len , int mask, unsigned long off);
int general_mumap(struct inode * inode,void * start ,size_t len);

//bk_rw.c
int bk_file_open(struct inode * ino, struct file * filp);
int bk_file_read(struct inode *ino, struct file *fp, char * buf, int count);
int bk_file_write(struct inode *ino, struct file * fp, char * buf, int count);
int bK_file_fsync(struct inode *inode, struct file *filp);
int bk_file_release(struct inode * ino, struct file * fp);

/**
 * @breif: 不同编译器Setion的定义
 */
#ifndef SECTION
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
#define SECTION(x)                  __attribute__((section(x)))
#elif defined(__ICCARM__) || defined(__ICCRX__)
#define SECTION(x)                  @ x
#elif defined(__GNUC__)
#define SECTION(x)                  __attribute__((section(x)))
#else
#define SECTION(x)
#endif
#endif

struct dev_reg{
    int32_t (*init)(void);
    int32_t (*exit)(void);
};
/**
 * @breif dev快速初始化
 */
#define DEV_BK_EXPORT(_initFun,_exitFun,_name) \
        const struct dev_reg \
        dev_reg_##_name SECTION("DevRegList") = {\
			.init=_initFun,\
			.exit=_exitFun\
		}

void trace(const char* fmt, ...);
void printk(const char *fmt, ...);
void fatalk(const char* fmt, ...);
void console_write(const char* str);

#endif //UNTITLED1_FS_H
