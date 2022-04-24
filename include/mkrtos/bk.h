/*
 * @Author: your name
 * @Date: 2021-11-27 14:34:47
 * @LastEditTime: 2021-11-28 21:10:22
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \ufsCmake\ufs\bkCache.h
 */
#ifndef _BK_CACHE_H__
#define _BK_CACHE_H__

#include <type.h>
#include <arch/atomic.h>
#include <mkrtos/fs.h>

int32_t bk_cache_init(struct bk_cache** p_bk_ch_ls,uint32_t cache_len,uint32_t bk_size);
int32_t bk_cache_destory(struct bk_cache* p_bk_ch_ls,uint32_t cache_len);

int32_t wbk(dev_t dev,uint32_t bk_inx,uint8_t *data,uint32_t ofs,uint32_t bk_size) ;
int32_t rbk(dev_t dev,uint32_t bk_inx,uint8_t *data,uint32_t ofs,uint32_t bk_size) ;

void lock_bk(struct bk_cache* bk);
void unlock_bk(struct bk_cache* bk);
void wait_on_bk(struct bk_cache* bk);
struct bk_cache* bk_read(dev_t dev_no,uint32_t bk_no,uint32_t may_write,int32_t not_r);
void bk_release(struct bk_cache* bk_tmp);
struct bk_cache* sync_rand_bk(dev_t dev_no) ;
void sync_bk(dev_t dev_no,uint32_t bk_no);
int32_t sync_all_bk(dev_t dev_no);
int32_t sync_all_bk_raw(struct bk_cache* bk_cache_ls,struct bk_operations *bk_ops,int cache_len);
int file_fsync (struct inode *inode, struct file *filp);
struct bk_cache* occ_bk(dev_t dev_no,bk_no_t bk_no,void* addr);
int32_t rel_bk(dev_t dev_no,bk_no_t bk_no);
int32_t rel_bk1(dev_t dev_no,void* addr);
#endif 
