#ifndef _OS_SHARE_MEM_H__
#define _OS_SHARE_MEM_H__
#include "type.h"

/**
* @breif 共享存储，解决局部需要使用大内存时使用malloc导致性能降低
*/
int32_t share_mem_init(void);
/**
* @breif 共享内存锁
*/
uint8_t* share_mem_lock(uint32_t hope_size);
/**
* @breif 共享内存解锁
*/
void share_mem_unlock(void);

#endif
