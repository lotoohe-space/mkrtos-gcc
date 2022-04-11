
#include <type.h>
#include <mkrtos/fs.h>
#include <string.h>
#include <mkrtos/bk.h>
#include <mkrtos/task.h>
#include <mkrtos/dev.h>

#define CH_DEV_MAX_NUM 10
#define BK_DEV_MAX_NUM 10

//默认的目录所使用的设备
dev_t root_dev_no=0;

struct ch_dev{
    //驱动名字
    const char* d_name;
    //文件操作函数，字符设备下面的函数需要自己写，块设备默认已经写好了
    struct file_operations *d_fops;
    //设备类型
    dev_t d_type;
};

struct bk_dev{
    //驱动名字
    const char* d_name;
    //文件操作函数，字符设备下面的函数需要自己写，块设备默认已经写好了
    struct file_operations *d_fops;
    //设备类型
    dev_t d_type;

    //块数
    uint32_t bk_count;
    //块设备缓存
    struct bk_cache* bk_cache_ls;
    //块设备的操作函数
    struct bk_operations* bk_ops;
    //缓存大小
    uint32_t bk_cache_count;
    //块大小
    uint32_t bk_size;
    //锁缓存表
    Atomic_t bk_ch_ls_lock;
    //等待队列
    struct wait_queue *b_ch_ls_wait;
};
/**
 * 字符设备驱动列表
 */
struct ch_dev devs_char[CH_DEV_MAX_NUM]={0};

/**
 * 块设备驱动列表
 */
struct bk_dev devs_bk[BK_DEV_MAX_NUM]={0};


int sync_all_bk_dev(void) {
    for(int i=0;i<BK_DEV_MAX_NUM;i++) {
        if(devs_bk[i].bk_ops) {
            sync_all_bk(i);
        }
    }
    return 0;
}
/**
 * 请求一个块设备号
 * @param dev_no
 * @return
 */
int32_t request_bk_no(dev_t dev_no){
    if(dev_no>=BK_DEV_MAX_NUM){
        return -1;
    }
    if(devs_bk[dev_no].bk_ops==NULL){
        return 0;
    }
    return -1;
}
/**
 * 分配一个bk号码
 * @param dev_no
 * @return
 */
int32_t alloc_bk_no(){
    uint32_t i;
    for(i=0;i<BK_DEV_MAX_NUM;i++){
        if(devs_bk[i].bk_ops==NULL){
            return i;
        }
    }
    return -1;
}
/**
 * 请求一个字符设备号
 * @param dev_no
 * @return
 */
int32_t request_char_no(dev_t dev_no){
    if(dev_no>=BK_DEV_MAX_NUM){
        return -1;
    }
    if(devs_char[dev_no].d_fops==NULL){
        return 0;
    }
    return -1;
}
/**
 * 返回驱动的操作符
 * @param major_no
 * @return
 */
struct file_operations* get_ch_dev_ops(dev_t major_no){
    if(major_no>=CH_DEV_MAX_NUM){
        return NULL;
    }
    if(devs_char[major_no].d_fops==NULL){
        return NULL;
    }
    return devs_char[major_no].d_fops;
}
/**
 * 返回设备的cache
 * @param major_no 
 * @return 
 */
struct bk_cache* get_bk_dev_cache(dev_t major_no,uint32_t *res_cache_len){
    if(major_no>=BK_DEV_MAX_NUM){
        return NULL;
    }
    *res_cache_len=devs_bk[major_no].bk_cache_count;
    return devs_bk[major_no].bk_cache_ls;
}
/**
 * 获取块大小
 * @param major_no
 * @return
 */
uint32_t get_bk_size(dev_t major_no) {
    if(major_no>=BK_DEV_MAX_NUM){
        return 0;
    }
    return devs_bk[major_no].bk_size;
}
/**
 * 返回块设备的操作函数
 * @param major_no
 * @return
 */
struct bk_operations* get_bk_ops(dev_t major_no){
    if(major_no>=BK_DEV_MAX_NUM){
        return NULL;
    }
    if(devs_bk[major_no].bk_ops==NULL){
        return NULL;
    }
    return devs_bk[major_no].bk_ops;
}
/**
 * 获取块数量
 * @param major_no
 * @return
 */
uint32_t get_bk_count(dev_t major_no){
    if(major_no>=BK_DEV_MAX_NUM){
        return -1;
    }

    return devs_bk[major_no].bk_count;
}
/**
 * 注册字符设备
 * @param major_no
 * @param name
 * @param d_fops
 * @return
 */
int32_t reg_ch_dev(dev_t major_no,const char* name ,struct file_operations *d_fops){
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(devs_char[major_no].d_fops){
        return -EBUSY;
    }
    devs_char[major_no].d_name=name;
    devs_char[major_no].d_fops=d_fops;
    return 0;
}
/**
 * 取消注册字符设备
 * @param major_no
 * @param name
 * @return
 */
int32_t unreg_ch_dev(dev_t major_no,const char* name){
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(strcmp(devs_char[major_no].d_name,name)!=0){
        return -EINVAL;
    }
    devs_char[major_no].d_name=NULL;
    devs_char[major_no].d_fops=NULL;
    return 0;
}
int32_t bk_dev_reg_param(dev_t major_no,uint32_t bk_count,uint32_t cache_len,uint32_t bk_size) {
    uint32_t tmp_ch_len;
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(!devs_bk[major_no].bk_ops){
        return -EBUSY;
    }
    if((tmp_ch_len=bk_cache_init(&(devs_bk[major_no].bk_cache_ls),cache_len,bk_size))<=0) {
        return -1;
    }
    devs_bk[major_no].bk_count=bk_count;
    devs_bk[major_no].bk_cache_count=tmp_ch_len;
    devs_bk[major_no].bk_size=bk_size;
    return 0;
}
int32_t bk_dev_unreg_param(dev_t major_no) {
    uint32_t tmp_ch_len;
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(devs_bk[major_no].bk_ops){
        return -EBUSY;
    }
    if(devs_bk[major_no].bk_cache_count) {
        bk_cache_destory(&(devs_bk[major_no].bk_cache_ls), devs_bk[major_no].bk_cache_count);
        devs_bk[major_no].bk_cache_count = 0;
    }
    return 0;
}
/**
 * 注册字符设备
 * @param major_no
 * @param name
 * @param d_fops
 * @return
 */
int32_t reg_bk_dev(
        dev_t major_no,
        const char* name ,
        struct bk_operations *bk_ops,
        struct file_operations *file_ops
//                ,
//        uint32_t bk_count,
//        uint32_t cache_len,
//        uint32_t bk_size
        ){
//    uint32_t tmp_ch_len;
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(devs_bk[major_no].bk_ops){
        return -EBUSY;
    }
//    if((tmp_ch_len=bk_cache_init(&(devs_bk[major_no].bk_cache_ls),cache_len,bk_size))<=0) {
//        return -1;
//    }
//    devs_bk[major_no].bk_count=bk_count;
//    devs_bk[major_no].bk_cache_count=tmp_ch_len;
    devs_bk[major_no].d_name=name;
    devs_bk[major_no].bk_ops=bk_ops;
//    devs_bk[major_no].bk_size=bk_size;
    return 0;
}
int32_t get_bk_cn(dev_t major_no,uint32_t *res_bk_no) {
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(!devs_bk[major_no].bk_ops){
        return -EBUSY;
    }
    *res_bk_no= devs_bk[major_no].bk_count;
    return 0;
}
/**
 * 取消注册字符设备
 * @param major_no
 * @param name
 * @return
 */
int32_t unreg_bk_dev(dev_t major_no,const char* name){
    if(major_no>=BK_DEV_MAX_NUM){
        return -EINVAL;
    }
    if(strcmp(devs_bk[major_no].d_name,name)!=0){
        return -EINVAL;
    }
    devs_bk[major_no].d_name=NULL;
    devs_bk[major_no].bk_ops=NULL;
    return 0;
}

static void __wait_on_bk_ch_ls(dev_t bk_dev_no){
    struct wait_queue wait = {CUR_TASK , NULL };

    add_wait_queue(&(devs_bk[bk_dev_no].b_ch_ls_wait), &wait);
    again:
    task_suspend();
    if (atomic_read(&(devs_bk[bk_dev_no].bk_ch_ls_lock))) {
        task_sche();
        goto again;
    }
    remove_wait_queue(&(devs_bk[bk_dev_no].b_ch_ls_wait), &wait);
    task_run();
}

void wait_on_bk_ch_ls(dev_t bk_dev_no){
    if(atomic_read(&devs_bk[bk_dev_no].bk_ch_ls_lock)){
        __wait_on_bk_ch_ls(bk_dev_no);
    }
}
void lock_bk_ls(dev_t bk_dev_no){
    wait_on_bk_ch_ls(bk_dev_no);
    atomic_set(&devs_bk[bk_dev_no].bk_ch_ls_lock,1);
}

void unlock_bk_ls(dev_t bk_dev_no){
    atomic_set(&devs_bk[bk_dev_no].bk_ch_ls_lock,0);
    wake_up(devs_bk[bk_dev_no].b_ch_ls_wait);
}


#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
extern const unsigned int DevRegList$$Base;
	extern const unsigned int DevRegList$$Limit;
#elif defined(__ICCARM__) || defined(__ICCRX__)
#pragma section="DevRegList"
#elif defined(__GNUC__)
extern uint64_t *_dev_reg_item_start;
extern uint64_t *_dev_reg_item_end;
#endif
/**
 * 初始化所有的设备
 */
void devs_init(void){
    uint32_t errCount=0;
    #if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
        PDevRegItem pscList= (PDevRegItem )(&DevRegList$$Base);
            uint32_t lsLen = ((uint32_t)(&DevRegList$$Limit)- (uint32_t)(&DevRegList$$Base)) / sizeof(DevRegItem);
            for(uint32_t i=0;i<lsLen;i++){
                if(pscList[i].initFun()<0){
                    errCount++;
                }
            }
    #elif defined(__ICCARM__) || defined(__ICCRX__)
    #error not supported compiler, please use command table mode
    #elif defined(__GNUC__)
        struct dev_reg* devs_reg= (struct dev_reg* )(&_dev_reg_item_start);
        uint32_t lsLen = ((uint32_t)(&_dev_reg_item_end)- (uint32_t)(&_dev_reg_item_start)) / sizeof(struct dev_reg);
        for(uint32_t i=0;i<lsLen;i++){
            if(devs_reg[i].init() <0 ) {
                errCount++;
            }else {
                //这里添加设备的文件
            }
        }
    #else
    #error not supported compiler, please use command table mode
    #endif
}

/* 打开字符设备 */
int chrdev_open(struct inode * inode, struct file * filp)
{
    if(MAJOR(inode->i_rdev_no) >=BK_DEV_MAX_NUM){
        return -ENODEV;
    }
    //通过主设备号获取字符设备的操作函数
    filp->f_op = devs_char[MAJOR(inode->i_rdev_no) ].d_fops;
    if (filp->f_op && filp->f_op->open) {
        return filp->f_op->open(inode, filp);
    }
    return 0;
}
/* 默认的字符设备操作函数集 */
struct file_operations def_chr_fops = {
        NULL,		/* lseek */
        NULL,		/* read */
        NULL,		/* write */
        NULL,		/* readdir */
        NULL,		/* select */
        NULL,		/* ioctl */
        NULL,		/* mmap */
        NULL,
        chrdev_open,	/* open */
        NULL,		/* release */
};

struct inode_operations chrdev_inode_operations = {
        &def_chr_fops,		/* default file operations */
        NULL,			/* create */
        NULL,			/* lookup */
        NULL,			/* link */
        NULL,			/* unlink */
        NULL,			/* symlink */
        NULL,			/* mkdir */
        NULL,			/* rmdir */
        NULL,			/* mknod */
        NULL,			/* rename */
        NULL,			/* readlink */
        NULL,			/* follow_link */
        NULL,			/* bmap */
        NULL,			/* truncate */
        NULL			/* permission */
};
/**
 * bk如果当成文件打开则
 * @param inode
 * @param filp
 * @return
 */
int blkdev_open(struct inode * inode, struct file * filp)
{
    if(MAJOR(inode->i_rdev_no) >=BK_DEV_MAX_NUM){
        return -ENODEV;
    }
    filp->f_op = devs_bk[MAJOR(inode->i_rdev_no) ].d_fops;
    if (filp->f_op && filp->f_op->open) {
        return filp->f_op->open(inode, filp);
    }
    return 0;
}
struct file_operations def_blk_fops = {
        NULL,		/* lseek */
        NULL,		/* read */
        NULL,		/* write */
        NULL,		/* readdir */
        NULL,		/* select */
        NULL,		/* ioctl */
        NULL,		/* mmap */
        NULL,
        blkdev_open,	/* open */
        NULL,		/* release */
};

struct inode_operations blkdev_inode_operations = {
        &def_blk_fops,		/* default file operations */
        NULL,			/* create */
        NULL,			/* lookup */
        NULL,			/* link */
        NULL,			/* unlink */
        NULL,			/* symlink */
        NULL,			/* mkdir */
        NULL,			/* rmdir */
        NULL,			/* mknod */
        NULL,			/* rename */
        NULL,			/* readlink */
        NULL,			/* follow_link */
        NULL,			/* bmap */
        NULL,			/* truncate */
        NULL			/* permission */
};
