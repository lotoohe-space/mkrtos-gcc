
#include <mkrtos/bk.h>
#include <type.h>
#include <mkrtos/task.h>
#include <mkrtos/fs.h>
#include <string.h>
#include <stdlib.h>
#include <mkrtos/mem.h>
#include <ipc/ipc.h>
#include "arch/arch.h"
//����BIT
#define SET_BIT(a,b) ((a)|=1<<(b))
//���BIT
#define CLR_BIT(a,b) ((a)&=~(1<<(b)))
#define GET_BIT(a,b) (((a)>>(b))&0x1)
#define ABS(a) ((a)<0?-(a):(a))

static int32_t sync_all_bk(dev_t dev_no);
/**
 * �黺���ʼ��
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
* @brief ����BkCache
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


int file_fsync (struct inode *inode, struct file *filp)
{
    struct super_block *sb;
    sb=inode->i_sb;
    return sync_all_bk(sb->s_dev_no);
}
/**
 * ���ͬ��һ����
 * @param dev_no
 * @param bk_ch
 * @return
 */
struct bk_cache* sync_rand_bk(dev_t dev_no,uint32_t new_bk) {
    struct bk_operations *bk_ops;
    struct bk_cache *sync_cache;
    uint32_t cache_len;
    struct bk_cache* bk_cache_ls;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);
    again:
    //�������ͬ�������У������������Ҫ����ͬһ��bk��������������Ĳ�һ������ôҪ�ȴ���Ҳ��һ�����������ͷ��˲�ͬ��bk��
    sync_cache=&bk_cache_ls[ABS(rand()) % cache_len];
    if(atomic_read(&sync_cache->b_lock)){
        //�����Ѿ��������ģ����������һ��
        task_sche();
        goto again;
    }
    lock_bk(sync_cache);
    if (GET_BIT(sync_cache->flag, 1)) {
        //�Ȳ���
        if(bk_ops->erase_bk(sync_cache->bk_no)<0){
            sync_cache->flag=0;
            fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
        }
        if(bk_ops->write_bk(sync_cache->bk_no,sync_cache->cache)<0){
            sync_cache->flag=0;
            fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
        }
    }
//    sync_cache->bk_no=new_bk;
    sync_cache->flag=0x0;
    unlock_bk(sync_cache);
    unlock_bk_ls(dev_no);
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
    lock_bk_ls(dev_no);
    for(i=0;i<cache_len;i++){
        if(GET_BIT(bk_cache_ls[i].flag,7)){
            continue;
        }
        lock_bk(bk_cache_ls+i);
        if (GET_BIT(bk_cache_ls[i].flag,0)) {
            if(bk_ops->erase_bk(bk_cache_ls[i].bk_no)<0){
                fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
            }
        }
        if (GET_BIT(bk_cache_ls[i].flag, 1)) {
            if(bk_ops->read_bk(bk_cache_ls[i].bk_no,bk_cache_ls[i].cache)<0){
                fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
            }
        }
        bk_cache_ls[i].flag=0;
        unlock_bk(bk_cache_ls+i);
    }
    unlock_bk_ls(dev_no);
    return 0;
}
/**
 * �ڱ����ҵ�ָ����cache
 * @param bk_cache_ls
 * @param bk_no
 * @param cache_len
 * @return
 */
static struct bk_cache* find_bk_cache(dev_t dev_no,uint32_t bk_no){
    uint32_t i;
    int32_t prev_i=-1;
    uint32_t cache_len;
    struct bk_cache* bk_cache_ls;
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);

    //������hashʱ���ģ������Ż�
    for(i=0;i<cache_len;i++){
        if(!GET_BIT(bk_cache_ls[i].flag,7)){
            prev_i=i;
            continue;
        }
        if(bk_cache_ls[i].bk_no!=bk_no){
            continue;
        }
//        lock_bk(&bk_cache_ls[i]);
//        bk_cache_ls[i].flag=0x80;
        unlock_bk_ls(dev_no);
       return &bk_cache_ls[i];
    }
    if(prev_i!=-1){
//        lock_bk(&bk_cache_ls[prev_i]);
        //������-1��˵��û�У������пյĿ�
        bk_cache_ls[prev_i].flag=0x80;
        bk_cache_ls[prev_i].bk_no=bk_no;
//        bk_cache_ls[prev_i].used_count++;
        unlock_bk_ls(dev_no);

        return &(bk_cache_ls[prev_i]);
    }
    unlock_bk_ls(dev_no);

    return NULL;
}
struct bk_cache* sync_bk(dev_t dev_no,uint32_t bk_no){
    uint32_t i;
    int32_t prev_i=-1;
    uint32_t cache_len;
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        return NULL;
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);

    lock_bk_ls(dev_no);

    //������hashʱ���ģ������Ż�
    for(i=0;i<cache_len;i++){
        if(!GET_BIT(bk_cache_ls[i].flag,7)){
            prev_i=i;
            continue;
        }
        if(bk_cache_ls[i].bk_no!=bk_no){
            continue;
        }
        struct bk_cache *sync_cache;
        sync_cache=&(bk_cache_ls[i]);
        lock_bk(sync_cache);
        if (GET_BIT(sync_cache->flag, 1)) {
            //�Ȳ���
            if(bk_ops->erase_bk(sync_cache->bk_no)<0){
                sync_cache->flag=0;
                fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
            }
            if(bk_ops->write_bk(sync_cache->bk_no,sync_cache->cache)<0){
                sync_cache->flag=0;
                fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
            }
        }
        sync_cache->flag=0x0;
        unlock_bk(sync_cache);
    }

    unlock_bk_ls(dev_no);

    return NULL;
}
/**
 * д��
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
    struct bk_cache* bk_tmp;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    again:
    bk_tmp=find_bk_cache(dev_no,bk_no);
    if(bk_tmp==NULL){
        //û�����ͷ�һ��
        bk_tmp=sync_rand_bk(dev_no,bk_no);
        goto again;
    }
    lock_bk(bk_tmp);
    if(bk_tmp->bk_no!=bk_no){
        unlock_bk(bk_tmp);
        //�����ͬ���˿飬��˵��
        goto again;
    }
    if(ofs==0 && size==bk_tmp->bk_size) {
        //����պ�дһ�飬��û��Ҫ��
        SET_BIT(bk_tmp->flag,2);
    }else {
        //�������һ��
        if (!GET_BIT(bk_tmp->flag, 2)) {
            if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
//                fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
            }
            SET_BIT(bk_tmp->flag, 2);
        }
    }

    if(ofs+size>bk_tmp->bk_size){
        //д�����˱߽磬������
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    memcpy(bk_tmp->cache+ofs,data,size);
    SET_BIT(bk_tmp->flag,1);
    unlock_bk(bk_tmp);

    return 0;
}


/**
 * ����
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
    struct bk_cache* bk_tmp;

    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
        return -1;
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);

        return -1;
    }
    again:
    bk_tmp=find_bk_cache(dev_no,bk_no);
    if(bk_tmp==NULL){
        //û�����ͷ�һ��
        bk_tmp=sync_rand_bk(dev_no,bk_no);
        goto again;

    }
    lock_bk(bk_tmp);
    if(bk_tmp->bk_no!=bk_no){
        //��ʱ�����Ǹ�bk cache�Ѿ����ı��ˣ����ʱ����Ҫ���»�ȡһ���µĻ���
        unlock_bk(bk_tmp);
        goto again;
    }
    if (!GET_BIT(bk_tmp->flag, 2)) {
        if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
//            fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
        }
        SET_BIT(bk_tmp->flag, 2);
    }
    if(ofs+size>bk_tmp->bk_size){
        //д�����˱߽磬������
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    memcpy(data,bk_tmp->cache+ofs,size);
    unlock_bk(bk_tmp);

    return 0;
}

/**
 * ��ȡ����
 * @param dev_no
 * @param bk_no
 * @return
 */
struct bk_cache* bk_read(dev_t dev_no,uint32_t bk_no,uint32_t may_write){
    struct bk_cache* bk_cache_ls;
    struct bk_operations *bk_ops;
    struct bk_cache* bk_tmp;
    uint32_t cache_len=0;
    bk_ops=get_bk_ops(dev_no);
    if(bk_ops==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    bk_cache_ls = get_bk_dev_cache(dev_no,&cache_len);
    if(bk_cache_ls==NULL){
        fatalk("%s %s ��������",__FUNCTION__ ,__LINE__);
    }
    again:
    bk_tmp=find_bk_cache(dev_no,bk_no);
    if(bk_tmp==NULL){
        //û�����ͷ�һ��
        bk_tmp=sync_rand_bk(dev_no,bk_no);
        goto again;

    }
    lock_bk(bk_tmp);
    if(bk_tmp->bk_no!=bk_no){
        //��ʱ�����Ǹ�bk cache�Ѿ����ı��ˣ����ʱ����Ҫ���»�ȡһ���µĻ���
        unlock_bk(bk_tmp);
        goto again;
    }
    if (!GET_BIT(bk_tmp->flag, 2)) {
        if (bk_ops->read_bk(bk_no, bk_tmp->cache) < 0) {
        }
        SET_BIT(bk_tmp->flag, 2);
    }
    if(may_write){
        SET_BIT(bk_tmp->flag,1);
    }
    return bk_tmp;
}
/**
 * �ͷŻ���
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
    CUR_TASK->status = TASK_SUSPEND;
    if (atomic_read(&( bk->b_lock))) {
        task_sche();
        goto again;
    }
    remove_wait_queue(&bk->b_wait, &wait);
    CUR_TASK->status = TASK_RUNNING;
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
