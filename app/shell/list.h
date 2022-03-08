

#ifndef _OS_LIST_H__
#define _OS_LIST_H__
#include "type.h"
#include "ipc/spin_lock.h"
/*链表节点*/
typedef struct _OSListNode{
    uint8_t* data;
    struct _OSListNode* next;
}*POSListNode,OSListNode;

typedef void (*_OSListAddNtData)(uint8_t**data, void *newData);
typedef void (*_OSListDelNtData)(uint8_t**data);
typedef int32_t (*_OSListFindCompare)(POSListNode pOSListNode,void* toCompareVal);

/*list定义*/
typedef struct _OSList{

    /*头节点*/
    POSListNode osLsIm;

    /*添加节点时，如何处理数据，用户可以自定义*/
    _OSListAddNtData osListAddNtDataFn;
    /*删除节点时，如何处理数据，用户可以自定义*/
    _OSListDelNtData osListDelNtDataFn;
    /*查找时的比较函数*/
    _OSListFindCompare  osListFindCompareFn;

    /*长度*/
    uint32_t len;

    /*互斥锁*/
    spinlock_handler lock;
}*POSList,OSList;

/*List遍历器*/
typedef struct _OSListIterator{
    POSList pOSLs;
    POSListNode newLsNode;
}*POSListIterator,OSListIterator;

POSList OSListCreate(
        POSList pOSLs
        ,_OSListAddNtData addNtDataFun
        ,_OSListDelNtData delNetDataFun
        ,_OSListFindCompare findComFun
);

/*数据、添加与查找*/
int32_t OSListDelNodeByData(POSList pOSLs,void* data );
int32_t OSListDelNode(POSList pOSLs,POSListNode pOsLtDelNd);
int32_t OSListAddNode(POSList pOSLs,void *data);
void* OSListFind(POSList pOSLs,void *findVal);

/*下面三个用于栈的实现*/
int32_t OSListAddNode2Top(POSList pOSLs,void* data);
void* OSListGetTopData(POSList pOSLs);
int32_t OSListPopData(POSList pOSLs);

/*下面是list的遍历器*/
int32_t OSListIteratorStart(POSListIterator pOsLsItr,POSList pOSLs);
void* OSListIteratorGetNext(POSListIterator pOsLsItr);
int32_t OSListIteratorStop(POSListIterator pOsLsItr);

#endif
