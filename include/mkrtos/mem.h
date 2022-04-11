#ifndef _MEM_H__
#define _MEM_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "type.h"

/**
* @breif 操作系统使用的内存块
*/
#define OS_USE_MEM_AREA_INX 	1

#ifndef NOT_USE_EX_MEM
#define OS_USE_MEM_AREA_INX1 	1
#endif
/**
* @breif 系统管理的内存块的数量
*/
#ifndef NOT_USE_EX_MEM
#define MEM_AREA_NUM 2
#endif
#ifdef NOT_USE_EX_MEM
#define MEM_AREA_NUM 1
#endif

/**
* @breif 内存大小
*/
#define MALLOC_MEM0_SIZE (40*1024)
/**
* @breif 内存块大小
*/
#define MALLOC_BLOCK0_SIZE 64
/**
* @breif 内存表大小
*/
#define MALLOC_MANAGER0_TABLE (MALLOC_MEM0_SIZE/MALLOC_BLOCK0_SIZE)

#ifndef NOT_USE_EX_MEM
#define MALLOC_EX_MEM_SIZE (960*1024)
#define MALLOC_EX_BLOCK_SIZE (64)
#define MALLOC_EX_MANAGER_TABLE (MALLOC_EX_MEM_SIZE/MALLOC_EX_BLOCK_SIZE)
#endif
/**
 * 内存申请管理链表
 */
struct mem_struct {
    struct mem_struct* next;
    void *mem_start;
    struct inode * inode;//如果映射到了文件，这项将会有效，否则该项为NULL
    int32_t length;
    int32_t ofst;
};
/**
* @breif 内存类型
*/
typedef uint8_t MemType;
/**
* @breif 内存管理表类型
*/
typedef uint32_t MemTbType;


/**
* @breif 初始化内存管理
*/
void InitMem(void) ;
/**
* @breif 申请内存
* @param inxMem 从那一块内存进行申请
* @param size 需要申请的内存大小
*/
void* _Malloc(uint16_t inxMem, uint32_t size) ;
/**
* @brief 释放申请的内存
* @param inxMem 从那一块内存上释放
* @param mem_addr 释放的内存首地址
*/
int32_t _Free(uint16_t inxMem, void* mem_addr) ;
/**
* @breif 获取剩余的内存
* @param inxMem 内存块索引
* @return 返回剩余多少字节
*/
uint32_t GetFreeMemory(uint8_t mem_no);
uint32_t GetTotalMemory(uint8_t mem_no);
/**
* @breif 申请内存
* @param size 申请的大小
* @return 返回申请到的内存，失败则返回NULL
*/
void* OSMalloc(uint32_t size) ;
void* OSRealloc(void *mem,uint32_t size);
#ifndef NOT_USE_EX_MEM
void *OSReallocEx(void* mem,uint32_t size);
void* OSMallocEx(uint32_t size) ;
void OSFreeEx(void* mem) ;
#endif
/**
* @breif 释放申请的内存
* @param mem 释放的内存的首地址
*/
void OSFree(void* mem) ;
void mem_clear(void);
//#define malloc OSMalloc
//#define free OSFree
#ifdef __cplusplus
}
#endif

#endif
