#ifndef _MEM_H__
#define _MEM_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#include "type.h"

/**
* @breif ����ϵͳʹ�õ��ڴ��
*/
#define OS_USE_MEM_AREA_INX 	0

#ifndef NOT_USE_EX_MEM
#define OS_USE_MEM_AREA_INX1 	1
#endif
/**
* @breif ϵͳ������ڴ�������
*/
#ifndef NOT_USE_EX_MEM
#define MEM_AREA_NUM 2
#endif
#ifdef NOT_USE_EX_MEM
#define MEM_AREA_NUM 1
#endif

/**
* @breif �ڴ��С
*/
#define MALLOC_MEM0_SIZE (20*1024)
/**
* @breif �ڴ���С
*/
#define MALLOC_BLOCK0_SIZE 64
/**
* @breif �ڴ���С
*/
#define MALLOC_MANAGER0_TABLE (MALLOC_MEM0_SIZE/MALLOC_BLOCK0_SIZE)

#ifndef NOT_USE_EX_MEM
#define MALLOC_EX_MEM_SIZE (960*1024)
#define MALLOC_EX_BLOCK_SIZE (64)
#define MALLOC_EX_MANAGER_TABLE (MALLOC_EX_MEM_SIZE/MALLOC_EX_BLOCK_SIZE)
#endif

/**
* @breif �ڴ�����
*/
typedef uint8_t MemType;
/**
* @breif �ڴ���������
*/
typedef uint32_t MemTbType;


/**
* @breif ��ʼ���ڴ����
*/
void InitMem(void) ;
/**
* @breif �����ڴ�
* @param inxMem ����һ���ڴ��������
* @param size ��Ҫ������ڴ��С
*/
void* _Malloc(uint16_t inxMem, uint32_t size) ;
/**
* @brief �ͷ�������ڴ�
* @param inxMem ����һ���ڴ����ͷ�
* @param mem_addr �ͷŵ��ڴ��׵�ַ
*/
void _Free(uint16_t inxMem, void* mem_addr) ;
/**
* @breif ��ȡʣ����ڴ�
* @param inxMem �ڴ������
* @return ����ʣ������ֽ�
*/
uint32_t GetFreeMemory(void) ;
uint32_t GetTotalMemory(void);
/**
* @breif �����ڴ�
* @param size ����Ĵ�С
* @return �������뵽���ڴ棬ʧ���򷵻�NULL
*/
void* OSMalloc(uint32_t size) ;
void* OSRealloc(void *mem,uint32_t size);
#ifndef NOT_USE_EX_MEM
void *OSReallocEx(void* mem,uint32_t size);
void* OSMallocEx(uint32_t size) ;
void OSFreeEx(void* mem) ;
#endif
/**
* @breif �ͷ�������ڴ�
* @param mem �ͷŵ��ڴ���׵�ַ
*/
void OSFree(void* mem) ;

//#define malloc OSMalloc
//#define free OSFree
#ifdef __cplusplus
}
#endif

#endif
