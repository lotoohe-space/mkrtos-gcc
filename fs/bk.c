
#include <mkrtos/bk.h>
#include <type.h>
#include <mkrtos/task.h>
#include <mkrtos/fs.h>
#include <string.h>
#include <stdlib.h>
#include <mkrtos/mem.h>
#include <ipc/ipc.h>
#include "arch/arch.h"
#include <mkrtos/debug.h>

//设置BIT
#define SET_BIT(a,b) ((a)|=1<<(b))
//清楚BIT
#define CLR_BIT(a,b) ((a)&=~(1<<(b)))
#define GET_BIT(a,b) (((a)>>(b))&0x1)
#define ABS(a) ((a)<0?-(a):(a))

static struct bk_cache* find_bk_cache(dev_t dev_no,uint32_t bk_no);
//static struct bk_cache* sync_one_bk(dev_t dev_no);
/**
 * 块缓存初始化
 * @param p_bk_ch_ls
 * @param cache_len
 * @return
 */
int32_t bk_cache_init(struct bk_cache** p_bk_ch_ls,uint32_t cache_len,uint32_t bk_size){
    uint32_t i;

    *p_bk_ch_ls = OSMalloc(sizeof(struct bk_cache)*cache_len);
    if(*p_bk_ch_ls == NULL){
        return -1;
    }
    for(i=0;i<cache_len;i++){
        struct bk_cache* pBkCache=&((*p_bk_ch_ls)[i]);
        memset(pBkCache,0,sizeof(struct bk_cache));
        pBkCache->cache=OSMalloc(bk_size);
        if(pBkCache->cache==NULL){
            break;
        }
        pBkCache->bk_size=bk_size;
    }
    if(i==0){
        OSFree(*p_bk_ch_ls);
    }
    return i;
}
/**
* @brief 销毁BkCache
*/
int32_t bk_cache_destory(struct bk_cache* p_bk_ch_ls,uint32_t cache_len){
    if(p_bk_ch_ls==NULL){
        return -1;
    }

    for(uint32_t i=0;i<cache_len;i++){
        OSFree((p_bk_ch_ls[i].cache));
    }
    OSFree(p_bk_ch_ls);
    return 0;
}
/**
 * hash函數
 * @param bk_no
 * @param max
 * @return
 */
inline int bk_hash(int bk_no,int max){
    return bk_no%max;
}
/**
 * hash查找一个bk，请确保ls被锁住
 * @param dev_no
 * @param bk_no
 * @return
 */
static struct bk_cache* search_bk(struct bk_cache* bk_cache_ls,uint32_t cache_len,dev_t dev_no,uint32_t bk_no){
    int i;
    int inx;
    i=0;
    //先查找
    inx = bk_hash(bk_no, cache_len);
    while(!IS_NULLKEY(bk_cache_ls[inx])) {
        //没有被删除并且被使用，则直接返回
        if (!IS_DELKEY(bk_cache_ls[inx])
            && bk_cache_ls[inx].bk_no == bk_no) {
            return &bk_cache_ls[inx];
        }
        inx = bk_hash(inx + 1, cache_len);
        i++;
        if(i>=cache_len){
            break;
        }
    }
    return NULL;
}
/**
 * 占用指定bk，不会自动的解除占用，除非进程自己释放或者进程关闭
 * @param dev_no
 * @return
 */
struct bk_cache* occ_bk(dev_t dev_no,bk_no_t bk_no,void* addr){
    struct bk_cache* bk_ch;
    struct bk_operations *bk_ops;
    if(!addr){
        return NULL;
    }
    bk_ops=get_bk_ops(dev_no);
    again:
    bk_ch=find_bk_cache(dev_no,bk_no);
    if(bk_ch==NULL){
        //没有则释放一个
        bk_ch=sync_rand_bk(dev_no);
        if(bk_ch==NULL) {
            return NULL;
        }
        goto again;
    }
    lock_bk(bk_ch);
    //释放之前的缓存，并使用addr提供的缓存地址，这样能够保证对用户的缓存连续性
    bk_ch->oldcache = bk_ch->cache;
    bk_ch->cache = addr;
    if (!GET_BIT(bk_ch->flag, 2)) {
        if (bk_ops->read_bk(bk_no, bk_ch->cache) < 0) {
        }
        SET_BIT(bk_ch->flag, 2);
    }
    SET_BIT(bk_ch->flag,1);
    SET_MMAP(*bk_ch);
    unlock_bk(bk_ch);
    return bk_ch;
}
int32_t rel_bk1(dev_t dev_no,void* addr){
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    int cache_len;
    int i;
    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);
    for(i=0;i<cache_len;i++){
        lock_bk(bk_cache_ls+i);
        if(
                bk_cache_ls[i].cache==addr
                && IS_MMAP(bk_cache_ls[i])
                ) {
            if (GET_BIT(bk_cache_ls[i].flag, 1)) {
                //先擦除
                if(bk_ops->erase_bk(bk_cache_ls[i].bk_no)<0){
                    fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
                }
                if(bk_ops->write_bk(bk_cache_ls[i].bk_no,bk_cache_ls[i].cache)<0){
                    fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
                }
                CLR_BIT(bk_cache_ls[i].flag,0);
                CLR_BIT(bk_cache_ls[i].flag,1);
            }
            CLR_KEY(bk_cache_ls[i]);
            bk_cache_ls[i].cache = bk_cache_ls[i].oldcache;
            bk_cache_ls[i].oldcache = NULL;

            unlock_bk(bk_cache_ls + i);
            unlock_bk_ls(dev_no);
            return 0;
        }
    }
    unlock_bk_ls(dev_no);
    return 0;
}
int32_t rel_bk(dev_t dev_no,bk_no_t bk_no){
    struct bk_cache* bk_cache_ls;
    int cache_len;
    int i;

    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);
    for(i=0;i<cache_len;i++) {
        lock_bk(bk_cache_ls + i);
        if (
                bk_cache_ls[i].bk_no == bk_no
                && IS_MMAP(bk_cache_ls[i])
                ) {
            CLR_KEY(bk_cache_ls[i]);
            bk_cache_ls[i].cache = bk_cache_ls[i].oldcache;
            bk_cache_ls[i].oldcache = NULL;
            unlock_bk(bk_cache_ls + i);
            unlock_bk_ls(dev_no);
            return 0;
        }
        unlock_bk(bk_cache_ls + i);
    }
    unlock_bk_ls(dev_no);
    return 0;
}
void sync_bk(dev_t dev_no,uint32_t bk_no){
    uint32_t i;
    uint32_t cache_len;
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    struct bk_cache* sync_cache;
    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        return ;
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);

    sync_cache= search_bk(bk_cache_ls,cache_len,dev_no,bk_no);
    if(sync_cache==NULL){
        unlock_bk_ls(dev_no);
        return ;
    }
    lock_bk(sync_cache);
    if (GET_BIT(sync_cache->flag, 1)) {
        //先擦除
        if(bk_ops->erase_bk(sync_cache->bk_no)<0){
            // sync_cache->flag=0;
            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        if(bk_ops->write_bk(sync_cache->bk_no,sync_cache->cache)<0){
            //  sync_cache->flag=0;
            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        CLR_BIT(bk_cache_ls[i].flag,0);
        CLR_BIT(bk_cache_ls[i].flag,1);
    }
    if(!IS_FREEZE(*sync_cache)) {
        CLR_KEY(*sync_cache);
        sync_cache->used_cn=0;
    }
    unlock_bk(sync_cache);
    unlock_bk_ls(dev_no);
}
/**
 * 随机同步一个块
 * @param dev_no
 * @param bk_ch
 * @return
 */
struct bk_cache* sync_rand_bk(dev_t dev_no) {
    struct bk_operations *bk_ops;
    struct bk_cache *sync_cache;
    uint32_t cache_len;
    struct bk_cache* bk_cache_ls;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);
    again:

    //随机进行同步，不行，两个进程如果要操作同一个bk，但是随机出来的不一样，那么要等待的也不一样，两个则释放了不同的bk块
    sync_cache=&bk_cache_ls[sysTasks.sysRunCount % cache_len];
    if(atomic_read(&sync_cache->b_lock)){
        //遇到已经被锁定的，则重新随机一个
        task_sche();
        goto again;
    }

    lock_bk(sync_cache);
    if (GET_BIT(sync_cache->flag, 1)) {
        //先擦除
        if(bk_ops->erase_bk(sync_cache->bk_no)<0){
            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        if(bk_ops->write_bk(sync_cache->bk_no,sync_cache->cache)<0){
            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        CLR_BIT(sync_cache->flag,0);
        CLR_BIT(sync_cache->flag,1);
    }
    if(!IS_FREEZE(*sync_cache)) {
        CLR_KEY(*sync_cache);
        (*sync_cache).used_cn=0;
    }else{
        unlock_bk(sync_cache);
        goto again;
    }
    unlock_bk(sync_cache);
    unlock_bk_ls(dev_no);
    return sync_cache;
}
int32_t sync_all_bk(dev_t dev_no){
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    uint32_t cache_len;
    uint32_t i;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        return -1;
    }
    bk_cache_ls= get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        return -1;
    }
    lock_bk_ls(dev_no);
    for(i=0;i<cache_len;i++){
        if(!IS_USEDKEY(bk_cache_ls[i])){
            continue;
        }
        lock_bk(bk_cache_ls+i);
        if (GET_BIT(bk_cache_ls[i].flag, 1)) {
            //先擦除
            if(bk_ops->erase_bk(bk_cache_ls[i].bk_no)<0){
               // bk_cache_ls[i].flag=0;
                fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
            }
            if(bk_ops->write_bk(bk_cache_ls[i].bk_no,bk_cache_ls[i].cache)<0){
              //  bk_cache_ls[i].flag=0;
                fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
            }
            CLR_BIT(bk_cache_ls[i].flag,0);
            CLR_BIT(bk_cache_ls[i].flag,1);
        }
        if(!IS_FREEZE(bk_cache_ls[i])) {
            CLR_KEY(bk_cache_ls[i]);
            bk_cache_ls[i].used_cn=0;
        }
        unlock_bk(bk_cache_ls+i);
    }
    unlock_bk_ls(dev_no);
    return 0;
}

/**
 * 在表中找到指定的cache，如果没有空的则返回NULL
 * @param bk_cache_ls
 * @param bk_no
 * @param cache_len
 * @return
 */
static struct bk_cache* find_bk_cache(dev_t dev_no,uint32_t bk_no){
    uint32_t i=0;
    uint32_t cache_len;
    int inx;

    struct bk_cache* bk_cache_ls;
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);
    //先查找
    inx = bk_hash(bk_no, cache_len);
    while(!IS_NULLKEY(bk_cache_ls[inx])) {
        //没有被删除并且被使用，则直接返回
        if (!IS_DELKEY(bk_cache_ls[inx])
        && bk_cache_ls[inx].bk_no == bk_no) {
            bk_cache_ls[inx].used_cn++;
            unlock_bk_ls(dev_no);
            return &bk_cache_ls[inx];
        }
        inx = bk_hash(inx + 1, cache_len);
        i++;
        if(i>=cache_len){
            break;
        }
    }
    i=0;
    inx = bk_hash(bk_no, cache_len);
    if(!IS_NULLKEY(bk_cache_ls[inx]) && !IS_DELKEY(bk_cache_ls[inx])){
        do{
            inx = bk_hash(inx + 1, cache_len);
            i++;
        }while(!IS_NULLKEY(bk_cache_ls[inx]) && !IS_DELKEY(bk_cache_ls[inx]) && i<cache_len);
        if(i==cache_len){
            //大哥没空的了
            unlock_bk_ls(dev_no);
            return NULL;
        }
    }
    SET_USEDKEY(bk_cache_ls[inx]);
    bk_cache_ls[inx].bk_no=bk_no;
    atomic_set(&bk_cache_ls[inx].b_lock.counter,0);
    bk_cache_ls[inx].used_cn++;
    unlock_bk_ls(dev_no);
    return &(bk_cache_ls[inx]);
}

/**
 * 写块
 * @param dev
 * @param data
 * @param bk_size
 * @return
 */
int32_t wbk(dev_t dev_no,uint32_t bk_no,uint8_t *data,uint32_t ofs,uint32_t size) {
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    uint32_t cache_len;
    struct bk_cache* bk_tmp;
    uint32_t bk_cn;
    if(get_bk_cn(dev_no,&bk_cn)<0){
        return NULL;
    }
    if(bk_no>=bk_cn){
        //超过了可读写的块范围
        return NULL;
    }
    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    again:
    bk_tmp=find_bk_cache(dev_no,bk_no);
    if(bk_tmp==NULL){
        //没有则释放一个
        bk_tmp=sync_rand_bk(dev_no);
        goto again;
    }
    lock_bk(bk_tmp);
    if(bk_tmp->bk_no!=bk_no){
        unlock_bk(bk_tmp);
        //如果是同步了块，则说明
        goto again;
    }
    if(ofs==0 && size==bk_tmp->bk_size) {
        //如果刚好写一块，则没必要读
        SET_BIT(bk_tmp->flag,2);
    }else {
        //否则读这一块
        if (!GET_BIT(bk_tmp->flag, 2)) {
            if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
//                fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
            }
            SET_BIT(bk_tmp->flag, 2);
        }
    }

    if(ofs+size>bk_tmp->bk_size){
        //写超出了边界，则死机
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    memcpy(bk_tmp->cache+ofs,data,size);
    SET_BIT(bk_tmp->flag,1);
    unlock_bk(bk_tmp);

    return 0;
}


/**
 * 读块
 * @param dev
 * @param bk_inx
 * @param bk_size
 * @return
 */
int32_t rbk(dev_t dev_no,uint32_t bk_no,uint8_t *data,uint32_t ofs,uint32_t size) {
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    uint32_t cache_len;
    struct bk_cache* bk_tmp;
    uint32_t bk_cn;
    if(get_bk_cn(dev_no,&bk_cn)<0){
        return NULL;
    }
    if(bk_no>=bk_cn){
        //超过了可读写的块范围
        return NULL;
    }
    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        return -1;
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);

        return -1;
    }
    again:
    bk_tmp=find_bk_cache(dev_no,bk_no);
    if(bk_tmp==NULL){
        //没有则释放一个
        bk_tmp=sync_rand_bk(dev_no);
        goto again;

    }
    lock_bk(bk_tmp);
    if(bk_tmp->bk_no!=bk_no){
        //当时锁的那个bk cache已经被改变了，这个时候需要重新获取一个新的缓存
        unlock_bk(bk_tmp);
        goto again;
    }
    if (!GET_BIT(bk_tmp->flag, 2)) {
        if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
//            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        SET_BIT(bk_tmp->flag, 2);
    }
    if(ofs+size>bk_tmp->bk_size){
        //写超出了边界，则死机
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    memcpy(data,bk_tmp->cache+ofs,size);
    unlock_bk(bk_tmp);

    return 0;
}

/**
 * 获取缓存
 * @param dev_no
 * @param bk_no
 * @param not_r 这个参数为1，代表不进行读，意义在于如果直接写一块数据，则不需要读，但是函数任然会给bk_cache结构体设置已读标志
 * @return
 */
struct bk_cache* bk_read(dev_t dev_no,uint32_t bk_no,uint32_t may_write,int32_t not_r){
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    struct bk_cache* bk_tmp;
    uint32_t cache_len=0;
    uint32_t bk_cn;
    if(get_bk_cn(dev_no,&bk_cn)<0){
        return NULL;
    }
    if(bk_no>=bk_cn){
        DEBUG("bk",ERR,"bk_no %d,bk_cn %d.",bk_no,bk_cn);
        //超过了可读写的块范围
        return NULL;
    }
    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }

    again:
    bk_tmp=find_bk_cache(dev_no,bk_no);
    if(bk_tmp==NULL){
        //没有则释放一个
        bk_tmp=sync_rand_bk(dev_no);
        goto again;

    }
    lock_bk(bk_tmp);
    if(bk_tmp->bk_no!=bk_no){
        //当时锁的那个bk cache已经被改变了，这个时候需要重新获取一个新的缓存
        unlock_bk(bk_tmp);
        goto again;
    }
    if (!not_r && !GET_BIT(bk_tmp->flag, 2)) {
        if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
        }

    }
    SET_BIT(bk_tmp->flag, 2);
    if(may_write){
        SET_BIT(bk_tmp->flag,1);
    }
    return bk_tmp;
}
/**
 * 释放缓存
 * @param bk_tmp
 */
void bk_release(struct bk_cache* bk_tmp){
    if(bk_tmp==NULL){
        return ;
    }
    unlock_bk(bk_tmp);
}

static void __wait_on_bk(struct bk_cache* bk){
    struct wait_queue wait = {CUR_TASK , NULL };

    add_wait_queue(&bk->b_wait, &wait);
    again:
    task_suspend();
//    CUR_TASK->status = TASK_SUSPEND;
    if (atomic_read(&( bk->b_lock))) {
        task_sche();
        goto again;
    }
    remove_wait_queue(&bk->b_wait, &wait);
    task_run();
//    CUR_TASK->status = TASK_RUNNING;
}

void wait_on_bk(struct bk_cache* bk){
    if(atomic_read(&bk->b_lock)){
        __wait_on_bk(bk);
    }
}
void lock_bk(struct bk_cache* bk){
    wait_on_bk(bk);
    atomic_set(&bk->b_lock,1);
}

void unlock_bk(struct bk_cache* bk){
    atomic_set(&bk->b_lock,0);
    wake_up(bk->b_wait);
}
