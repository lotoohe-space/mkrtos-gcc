
#include "mkrtos/smem_knl.h"
#include <ipc/spin_lock.h>
#include <mkrtos/mem.h>

#define SHARE_MEM_SIZE 2048

/**
* @breif 共享内存
*/
static uint8_t *share_mem=NULL;

static spinlock_handler share_mem_lock_handl;

/**
* @breif 共享存储，解决局部需要使用大内存时使用malloc导致性能降低，而直接定义导致栈溢出
*/
int32_t share_mem_init(void){
    share_mem=OSMallocEx(SHARE_MEM_SIZE);
	if(share_mem==NULL){
		return -1;
	}
	spin_lock_init(&share_mem_lock_handl);
	return 0;
}
/**
* @breif 共享内存锁
*/
uint8_t* share_mem_lock(uint32_t hope_size){
	if(hope_size>SHARE_MEM_SIZE){
		return NULL;
	}
	spin_lock(&share_mem_lock_handl);
	return share_mem;
}
/**
* @breif 共享内存解锁
*/
void share_mem_unlock(void){
	spin_unlock(&share_mem_lock_handl);
}

