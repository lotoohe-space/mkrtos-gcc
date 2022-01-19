
#include <mkrtos/bk.h>
#include <type.h>
#include <mkrtos/task.h>
#include <mkrtos/fs.h>
#include <string.h>
#include <stdlib.h>
#include <mkrtos/mem.h>
#include <ipc/ipc.h>
//设置BIT
#define SET_BIT(a,b) ((a)|=1<<(b))
//清楚BIT
#define CLR_BIT(a,b) ((a)&=~(1<<(b)))
#define GET_BIT(a,b) (((a)>>(b))&0x1)
#define ABS(a) ((a)<0?-(a):(a))


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
/**
 * 随机同步一个块
 * @param dev_no
 * @param bk_ch
 * @return
 */
static struct bk_cache* sync_bk(dev_t dev_no,struct bk_cache* bk_cache_ls,uint32_t cache_len) {
    struct bk_operations *bk_ops;
    struct bk_cache *sync_cache;
    uint32_t sync_bk_no;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }

    //随机进行同步
    sync_bk_no=ABS(rand())%cache_len;

    sync_cache=&bk_cache_ls[sync_bk_no];

    while(sem_take((sync_cache->sem_lock))<0);

    if (GET_BIT(sync_cache->flag, 1)) {
        //先擦除
        if(bk_ops->erase_bk(sync_cache->bk_no)<0){
            sync_cache->flag=0;
            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        if(bk_ops->write_bk(sync_cache->bk_no,sync_cache->cache)<0){
            sync_cache->flag=0;
            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
    }
    sync_cache->flag=0;
    while(sem_release((sync_cache->sem_lock)));

    return sync_cache;
}
static int32_t sync_all_bk(dev_t dev_no){
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

    for(i=0;i<cache_len;i++){
        if(GET_BIT(bk_cache_ls[i].flag,7)){
            continue;
        }
        while(sem_take((bk_cache_ls[i].sem_lock))<0);
        if (GET_BIT(bk_cache_ls[i].flag,0)) {
            if(bk_ops->erase_bk(bk_cache_ls[i].bk_no)<0){
//                bk_cache_ls[i].flag=0;
//                while(1);
            }
        }
        if (GET_BIT(bk_cache_ls[i].flag, 1)) {
            if(bk_ops->read_bk(bk_cache_ls[i].bk_no,bk_cache_ls[i].cache)<0){
//                bk_cache_ls[i].flag=0;
//                while(1);
            }
        }
        bk_cache_ls[i].flag=0;
        while(sem_release((bk_cache_ls[i].sem_lock)));
    }
    return 0;
}
/**
 * 在表中找到指定的cache
 * @param bk_cache_ls
 * @param bk_no
 * @param cache_len
 * @return
 */
static struct bk_cache* find_bk_cache(struct bk_cache* bk_cache_ls,uint32_t bk_no,uint32_t cache_len){
    uint32_t i;
    int32_t prev_i=-1;
    //这里用hash时最快的，后面优化
    for(i=0;i<cache_len;i++){
        if(!GET_BIT(bk_cache_ls[i].flag,7)){
            prev_i=i;
            continue;
        }
        if(bk_cache_ls[i].bk_no!=bk_no){
            continue;
        }
//        bk_cache_ls[i].flag=0x80;
       return &bk_cache_ls[i];
    }
    if(prev_i!=-1){
        //不等于-1则说明没有，而且有空的块
        bk_cache_ls[prev_i].flag=0x80;
        bk_cache_ls[prev_i].bk_no=bk_no;
//        bk_cache_ls[prev_i].used_count++;
        return &(bk_cache_ls[prev_i]);
    }

    return NULL;
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
    uint32_t i;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    struct bk_cache* bk_tmp;
    bk_tmp=find_bk_cache(bk_cache_ls,bk_no,cache_len);
    if(bk_tmp==NULL){
        //没有则释放一个
        bk_tmp=sync_bk(dev_no,bk_cache_ls,cache_len);
        bk_tmp->bk_no=bk_no;
        bk_tmp->flag=0x80;
    }

    while(sem_take((bk_tmp->sem_lock))<0);
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
    while(sem_release((bk_tmp->sem_lock)));

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
    uint32_t i;

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
    struct bk_cache* bk_tmp;
    bk_tmp=find_bk_cache(bk_cache_ls,bk_no,cache_len);
    if(bk_tmp==NULL){
        //没有则释放一个
        bk_tmp=sync_bk(dev_no,bk_cache_ls,cache_len);
        bk_tmp->flag=0x80;
    }
    while(sem_take((bk_tmp->sem_lock))<0);
    if (!GET_BIT(bk_tmp->flag, 2)) {
        if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
//            fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
        }
        SET_BIT(bk_tmp->flag, 2);
    }
    if(ofs+size>=bk_tmp->bk_size){
        //写超出了边界，则死机
        fatalk("%s %s 致命错误",__FUNCTION__ ,__LINE__);
    }
    memcpy(data,bk_tmp->cache+ofs,size);
    while(sem_release((bk_tmp->sem_lock)));

    return 0;
}
