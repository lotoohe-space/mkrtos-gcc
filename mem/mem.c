/**
* @breif 内存管理
* @author 1358745329@qq.com
*/

#include "arch/arch.h"
#include "errno.h"
#include "stdio.h"
#include <string.h>
#include <mkrtos/mem.h>
#include <mkrtos/task.h>

/**
* @breif 内存池
*/
MemType __attribute__((aligned(MALLOC_BLOCK0_SIZE))) mem0[MALLOC_MEM0_SIZE]={0};
/**
* @breif 内存管理表
*/
MemTbType mem0ManTb[MALLOC_MANAGER0_TABLE]={0};


#ifndef NOT_USE_EX_MEM
uint8_t __attribute__((aligned(MALLOC_BLOCK0_SIZE))) malloc_ex_mem[MALLOC_EX_MEM_SIZE] __attribute__((section(".ext_sram")))={0};
MemTbType manager_ex_table[MALLOC_EX_MANAGER_TABLE] __attribute__((section(".ext_sram")))= { 0 };
#endif

/**
* @breif OS的内存管理项
*/
typedef struct _OSMemItem{
	
	/**
	* @breif 内存
	*/
	MemType* mem;
	/**
	* @breif 内存管理表
	*/
	MemTbType* memManTb;
	/**
	* @breif 内存大小
	*/
	uint32_t memSize;
	/**
	* @breif 内存管理表大小
	*/
	uint32_t memManTbSize;
	/**
	* @breif 块大小
	*/
	uint16_t memBlockSize;
	
	/**
	* @breif 剩余的块数量
	*/
	uint32_t freeBlockNum;
	
}*POSMemItem,OSMemItem;


typedef struct _OSMem{
	
	/**
	* @breif 内存管理项
	*/
	OSMemItem OSMemItemLs[MEM_AREA_NUM];
	
	/**
	* @breif 内存区数量
	*/
	uint16_t memNum;
	
}*POSMem,OSMem;

/**
* @breif 内存管理变量初始化
*/
OSMem osMem={
	.memNum=MEM_AREA_NUM,
	.OSMemItemLs[0].mem=mem0,
	.OSMemItemLs[0].memManTb=mem0ManTb,
	.OSMemItemLs[0].memSize=MALLOC_MEM0_SIZE,
	.OSMemItemLs[0].memManTbSize=MALLOC_MANAGER0_TABLE,
	.OSMemItemLs[0].memBlockSize=MALLOC_BLOCK0_SIZE,
	.OSMemItemLs[0].freeBlockNum = MALLOC_MANAGER0_TABLE,
	#ifndef NOT_USE_EX_MEM
	.OSMemItemLs[1].mem=malloc_ex_mem,
	.OSMemItemLs[1].memManTb=manager_ex_table,
	.OSMemItemLs[1].memSize=MALLOC_EX_MEM_SIZE,
	.OSMemItemLs[1].memManTbSize=MALLOC_EX_MANAGER_TABLE,
	.OSMemItemLs[1].memBlockSize=MALLOC_EX_BLOCK_SIZE,
	.OSMemItemLs[1].freeBlockNum = MALLOC_EX_MANAGER_TABLE,
	#endif
};



/**
* @breif 初始化内存管理
*/
void InitMem(void) {
	/*清空所有的内存管理项*/
	uint16_t i;
	for (i = 0; i < osMem.memNum; i++) {
		uint32_t j=0;
		for(j=0;j<osMem.OSMemItemLs[i].memManTbSize;j++){
			osMem.OSMemItemLs[i].memManTb[j]=0;
		}
		//memset(osMem.OSMemItemLs[i].memManTb, 0, osMem.OSMemItemLs[i].memManTbSize * sizeof(MemTbType));
	}
}
/**
* @breif 申请内存
* @param inxMem 从那一块内存进行申请
* @param size 需要申请的内存大小
*/
void* _Malloc(uint16_t inxMem, uint32_t size) {
	uint32_t i_need_block_num;//需要的block数量
	uint32_t i;
	uint32_t find_block_num = 0;//找到的空的块
	uint32_t temp;
	uint8_t flag = 0;
	uint16_t bkSize;
	MemTbType* manager_table;
	MemType* malloc_mem;
	if (inxMem >= osMem.memNum) {
		/*超出索引*/
		return NULL;
	}
	malloc_mem = osMem.OSMemItemLs[inxMem].mem;
	manager_table = osMem.OSMemItemLs[inxMem].memManTb;
	bkSize = osMem.OSMemItemLs[inxMem].memBlockSize;

	i_need_block_num = size / bkSize + ((size % bkSize == 0) ? 0 : 1);

	for (i = 0; i < osMem.OSMemItemLs[inxMem].memManTbSize;) {
		if (manager_table[i] == 0) {
			find_block_num++;
			if (find_block_num == i_need_block_num) {
				flag = 1;
				break;
			}
			i++;
		}
		else {
			find_block_num = 0;
			i += manager_table[i];
		}
	}
	if (flag != 1 || i >= osMem.OSMemItemLs[inxMem].memManTbSize) {//没有找到，或者超出了
		return NULL;
	}
	i -= i_need_block_num - 1;
	for (temp = i; temp < i + i_need_block_num; temp++) {
		if (temp == i) {
			manager_table[i] = i_need_block_num;
		}
		else {
			manager_table[temp] = 1;
		}
	}
	osMem.OSMemItemLs[inxMem].freeBlockNum -= i_need_block_num;
    printk("malloc %x.\n\n",(void*)(&(malloc_mem[bkSize * i])));
    return (void*)(&(malloc_mem[bkSize * i]));
}
/**
* @brief 释放申请的内存
* @param inxMem 从那一块内存上释放
* @param mem_addr 释放的内存首地址
*/
void _Free(uint16_t inxMem, void* mem_addr) {
	if (mem_addr == NULL) { return; }
	uint32_t free_size;
	uint16_t bkSize;
	MemTbType* manager_table;
	MemType* malloc_mem;
	uint32_t i_mem_offset;
	uint32_t i_manager_offset;
	malloc_mem = osMem.OSMemItemLs[inxMem].mem;
	manager_table = osMem.OSMemItemLs[inxMem].memManTb;
	bkSize = osMem.OSMemItemLs[inxMem].memBlockSize;

	i_mem_offset = (uint32_t)mem_addr - (uint32_t)malloc_mem;
	i_manager_offset = i_mem_offset / bkSize;
	uint32_t i;
	if (i_manager_offset > osMem.OSMemItemLs[inxMem].memManTbSize) {
		return;
	}
	
	osMem.OSMemItemLs[inxMem].freeBlockNum += manager_table[i_manager_offset];

	free_size = manager_table[i_manager_offset];
	for (i = i_manager_offset; i < free_size + i_manager_offset; i++) {
		manager_table[i] = 0;
	}
}
uint32_t GetMemSize(uint16_t inxMem, void* mem_addr){
	if (mem_addr == NULL) { return 0; }
	uint32_t free_size;
	uint16_t bkSize;
	MemTbType* manager_table;
	MemType* malloc_mem;
	uint32_t i_mem_offset;
	uint32_t i_manager_offset;
	malloc_mem = osMem.OSMemItemLs[inxMem].mem;
	manager_table = osMem.OSMemItemLs[inxMem].memManTb;
	bkSize = osMem.OSMemItemLs[inxMem].memBlockSize;

	i_mem_offset = (uint32_t)mem_addr - (uint32_t)malloc_mem;
	i_manager_offset = i_mem_offset / bkSize;
	uint32_t i;
	if (i_manager_offset > osMem.OSMemItemLs[inxMem].memManTbSize) {
		return 0;
	}
	
	return manager_table[i_manager_offset]*bkSize;
}
/**
* @breif 获取剩余的内存
* @param inxMem 内存块索引
* @return 返回剩余多少字节
*/
uint32_t GetFreeMemory(uint8_t mem_no) {
	
	/*剩余内存大小*/
	return osMem.OSMemItemLs[mem_no].freeBlockNum * osMem.OSMemItemLs[OS_USE_MEM_AREA_INX].memBlockSize;

}
uint32_t GetTotalMemory(uint8_t mem_no){
	return osMem.OSMemItemLs[mem_no].memSize;
}
/**
* @breif 申请内存
* @param size 申请的大小
* @return 返回申请到的内存，失败则返回NULL
*/
void* OSMalloc(uint32_t size) {
	int32_t st=DisCpuInter();
	void* res = _Malloc(OS_USE_MEM_AREA_INX, size);
	RestoreCpuInter(st);
    memset(res,0,size);
	return res;
}
#ifndef NOT_USE_EX_MEM
void* OSMallocEx(uint32_t size) {
	int32_t st=DisCpuInter();
	void* res = _Malloc(OS_USE_MEM_AREA_INX1, size);
	RestoreCpuInter(st);
	return res;
}
void OSFreeEx(void* mem) {
	int32_t st=DisCpuInter();
	_Free(OS_USE_MEM_AREA_INX1, mem);
	RestoreCpuInter(st);
}
void *OSReallocEx(void* mem,uint32_t size){
	int32_t st=DisCpuInter();
	void* res = _Malloc(OS_USE_MEM_AREA_INX1, size);
	if(res==NULL){
		RestoreCpuInter(st);
		return NULL;
	}
	memcpy(res,mem,GetMemSize(OS_USE_MEM_AREA_INX1,mem));
	OSFree(mem);
	RestoreCpuInter(st);
	return res;
}
#endif
void *OSRealloc(void *mem,uint32_t size){
	int32_t st=DisCpuInter();
	void* res = _Malloc(OS_USE_MEM_AREA_INX, size);
	if(res==NULL){
		RestoreCpuInter(st);
		return NULL;
	}
	memcpy(res,mem,GetMemSize(OS_USE_MEM_AREA_INX,mem));
	OSFree(mem);
	RestoreCpuInter(st);
	return res;
}

/**
* @breif 释放申请的内存
* @param mem 释放的内存的首地址
*/
void OSFree(void* mem) {
	int32_t st;
    if(!mem){
        return ;
    }
    printk("free %x.\n\n",mem);
    st=DisCpuInter();
	_Free(OS_USE_MEM_AREA_INX, mem);
	RestoreCpuInter(st);
}

static int32_t mem_add(void *mem,int32_t lenght){
    struct mem_struct* tmp;
    tmp= OSMalloc(sizeof(struct mem_struct));
    if(tmp==NULL){
        return -1;
    }
    tmp->length=lenght;
    tmp->mem_start=mem;
    tmp->next=NULL;
    uint32_t  t;
    t=DisCpuInter();
    if(!CUR_TASK->mems){
        CUR_TASK->mems=tmp;
    }else{
        tmp->next=CUR_TASK->mems;
        CUR_TASK->mems=tmp;
    }
    RestoreCpuInter(t);
    return 0;
}
static void mem_remove(void *mem){
    struct mem_struct *tmp;
    tmp=CUR_TASK->mems;
    struct mem_struct *prev=NULL;
    uint32_t t;
    t=DisCpuInter();
    while(tmp){
        if(tmp->mem_start ==mem) {
            if (prev==NULL) {
                //删除的第一个
                CUR_TASK->mems=tmp->next;
                OSFree(tmp);
                break;
            }else{
                prev->next=tmp->next;
                break;
            }
        }
        prev=tmp;
        tmp=tmp->next;
    }
    RestoreCpuInter(t);
}
/**
 * 清楚进程占用的内存，用戶的exit函數中調用
 */
void mem_clear(void){
    uint32_t t;
    struct mem_struct **tmp;
    t=DisCpuInter();
    tmp=&CUR_TASK->mems;
    while(*tmp){
        struct mem_struct *next;
        next=(*tmp)->next;
        OSFree((*tmp)->mem_start);
        OSFree((*tmp));
        *tmp=next;
    }
    CUR_TASK->mems=NULL;
    RestoreCpuInter(t);
}
void* sys_mmap(void *start, size_t length, int prot, int flags,int fd, off_t offset){
    if(fd!=-1
        ||start
    ){
        return -1;
    }
    void* res_mem = OSMalloc(length);
    if(!res_mem){
        return -1;
    }
//    printk("mmap %x start:%x length:%d prot:%d flags:%x fd:%d ost:%d\r\n",res_mem,start,length,prot,flags,fd,offset);
//    if(mem_add(res_mem,length)<0){
//        OSFree(res_mem);
//        return -ENOMEM;
//    }
    return res_mem;
}
void sys_munmap(void *start, size_t length){
//    mem_remove(start);
//    printk("munamp start:%x length:%d\r\n",start,length);

    return OSFree(start);
}
void *sys_mremap(void *__addr, size_t __old_len, size_t __new_len,
                 unsigned long __may_move){
    return OSRealloc(__addr,__new_len);
}