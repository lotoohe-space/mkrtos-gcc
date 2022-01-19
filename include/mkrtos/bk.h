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

#endif 
