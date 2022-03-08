
#include "list.h"
#include <stdlib.h>
#include <ipc/spin_lock.h>
/*默认的数据处理函数*/
static void OSListAddNtDataDefault(uint8_t**data,void *newData){
    *data=newData;
}
/*默认的数据处理函数*/
static void OSListDelNtDataDefault(uint8_t**data){
    return ;
}
/*默认的数据处理函数*/
static int32_t OSListFindCompareDefault(POSListNode pOSListNode,void* toCompareVal){
    return pOSListNode==toCompareVal;
}
POSList OSListCreate(
        POSList pOSLs
        ,_OSListAddNtData addNtDataFun
        ,_OSListDelNtData delNetDataFun
        ,_OSListFindCompare findComFun
){
    if(pOSLs==NULL){return NULL;}

    spin_lock_init(&(pOSLs->lock));

    pOSLs->osListAddNtDataFn=addNtDataFun;
    pOSLs->osListDelNtDataFn=delNetDataFun;
    pOSLs->osListFindCompareFn=findComFun;
    /*设置默认的数据处理函数*/
    if(pOSLs->osListAddNtDataFn==NULL){
        pOSLs->osListAddNtDataFn=OSListAddNtDataDefault;
    }
    if(pOSLs->osListDelNtDataFn==NULL){
        pOSLs->osListDelNtDataFn=OSListDelNtDataDefault;
    }
    if(pOSLs->osListFindCompareFn==NULL){
        pOSLs->osListFindCompareFn=OSListFindCompareDefault;
    }
    pOSLs->len=0;
    pOSLs->osLsIm=NULL;
    return pOSLs;
}
int32_t OSListDelNodeByData(POSList pOSLs,void* data ){
    if(pOSLs==NULL){return -1;}

    POSListNode pOsLastLtNd=NULL;
    POSListNode pOsLtNd;
    spin_lock(&(pOSLs->lock));
    pOsLtNd=pOSLs->osLsIm;
    /*上一个节点*/
    pOsLastLtNd=pOsLtNd;

    while(pOsLtNd){

        /*找到节点了*/
        if(pOSLs->osListFindCompareFn(pOsLtNd,data)==0){/*找到数据相同的*/
            if(pOsLastLtNd==pOSLs->osLsIm){/*上一个节点就是第一个节点*/

                /*用户函数删除节点数据*/
                pOSLs->osListDelNtDataFn(&(pOsLtNd->data));
                /*节点相连*/
                pOsLastLtNd=pOsLtNd->next;

                pOSLs->len--;
                spin_unlock(&(pOSLs->lock));
                /*释放节点内存*/
                free(pOsLtNd);
                return 0;
            }else{/*不是第一个节点*/

                /*用户函数删除节点数据*/
                pOSLs->osListDelNtDataFn(&(pOsLtNd->data));
                /*节点相连*/
                pOsLastLtNd->next=pOsLtNd->next;

                pOSLs->len--;
                spin_unlock(&(pOSLs->lock));
                /*释放节点内存*/
                free(pOsLtNd);
                return 0;
            }
        }

        pOsLastLtNd=pOsLtNd;
        pOsLtNd=pOsLtNd->next;
    }
    spin_unlock(&(pOSLs->lock));
    return -1;
}
/*list删除数据*/
int32_t OSListDelNode(POSList pOSLs,POSListNode pOsLtDelNd){
    if(pOSLs==NULL || pOsLtDelNd==NULL){return -1;}

    POSListNode pOsLastLtNd=NULL;
    POSListNode pOsLtNd;
    spin_lock(&(pOSLs->lock));
    pOsLtNd=pOSLs->osLsIm;
    /*上一个节点*/
    pOsLastLtNd=pOsLtNd;

    while(pOsLtNd){

        /*找到节点了*/
        if(pOsLtDelNd==pOsLtNd){
            if(pOsLastLtNd==pOSLs->osLsIm){/*上一个节点就是第一个节点*/

                /*用户函数删除节点数据*/
                pOSLs->osListDelNtDataFn(&(pOsLtNd->data));
                /*节点相连*/
                pOsLastLtNd=pOsLtNd->next;

                pOSLs->len--;
                spin_unlock(&(pOSLs->lock));
                /*释放节点内存*/
                free(pOsLtNd);
                return 0;
            }else{/*不是第一个节点*/

                /*用户函数删除节点数据*/
                pOSLs->osListDelNtDataFn(&(pOsLtNd->data));
                /*节点相连*/
                pOsLastLtNd->next=pOsLtNd->next;

                pOSLs->len--;
                spin_unlock(&(pOSLs->lock));
                /*释放节点内存*/
                free(pOsLtNd);
                return 0;
            }
        }

        pOsLastLtNd=pOsLtNd;
        pOsLtNd=pOsLtNd->next;
    }
    spin_unlock(&(pOSLs->lock));
    return -1;
}

/**
* @brief 获取list的顶部数据
* @return 获取到的数据，等于NULL代表没有数据
*/
void* OSListGetTopData(POSList pOSLs){
    if(pOSLs==NULL){return NULL;}
    POSListNode pOsLtNd;
    pOsLtNd=pOSLs->osLsIm;

    if(pOsLtNd==NULL){
        return NULL;
    }
    return pOsLtNd->data;
}
/**
* @brief 删除顶部的节点
*	@return 0成功 -1失败
*/
int32_t OSListPopData(POSList pOSLs){
    if(pOSLs==NULL){
        return -1;
    }
    POSListNode pOsLtNd;
    pOsLtNd=pOSLs->osLsIm;

    return OSListDelNode(pOSLs,pOsLtNd);
}
/**
* @brief 添加数据到顶部
* @param 需要添加的数据
*/
int32_t OSListAddNode2Top(POSList pOSLs,void* data){
    if(pOSLs==NULL){return -1;}

    POSListNode pOSltNtNew;
    POSListNode pOsLtNd;

    pOSltNtNew=malloc(sizeof(OSListNode));
    if(pOSltNtNew==NULL){
        return -1;
    }
    pOSltNtNew->next=NULL;

    spin_lock(&(pOSLs->lock));
    pOsLtNd=pOSLs->osLsIm;
    /*用户进行数据处理*/
    pOSLs->osListAddNtDataFn(&(pOSltNtNew->data),data);
    if(pOsLtNd==NULL){
        pOsLtNd=pOSltNtNew;
    }else{
        /*插入一个*/
        POSListNode pTemp;
        pTemp=pOsLtNd->next;
        pOsLtNd=pOSltNtNew;
        pOSltNtNew->next=pTemp;
    }
    spin_unlock(&(pOSLs->lock));
    return 0;
}
/**
* @brief List添加数据到尾部
* @param pOSLs list指针
* @param data 需要添加的数据
*/
int32_t OSListAddNode(POSList pOSLs,void *data){
    if(pOSLs==NULL){return -1;}
    POSListNode pOSltNtNew;
    POSListNode pOsLtNd;

    pOSltNtNew=malloc(sizeof(OSListNode));
    if(pOSltNtNew==NULL){
        return -1;
    }
    pOSltNtNew->next=NULL;

    spin_lock(&(pOSLs->lock));
    pOsLtNd	=pOSLs->osLsIm;
    if(pOsLtNd==NULL){
        /*用户进行数据处理*/
        pOSLs->osListAddNtDataFn(&(pOSltNtNew->data),data);
        /*赋值第一个节点*/
        pOSLs->osLsIm=pOSltNtNew;
        /*增加长度*/
        pOSLs->len++;
    }else{
        while(pOsLtNd->next){
            pOsLtNd=pOsLtNd->next;
        }
        /*用户进行数据处理*/
        pOSLs->osListAddNtDataFn(&(pOSltNtNew->data),data);
        /*赋值第一个节点*/
        pOsLtNd->next=pOSltNtNew;
        /*增加长度*/
        pOSLs->len++;
    }
    spin_unlock(&(pOSLs->lock));
    return 0;
}
/*查找指定node*/
void* OSListFind(POSList pOSLs,void *findVal){
    if(pOSLs==NULL){return NULL;}
    spin_lock(&(pOSLs->lock));
    POSListNode pOsLtNd=pOSLs->osLsIm;

    while(pOsLtNd){
        if(pOSLs->osListFindCompareFn(pOsLtNd,findVal)==0){
            spin_unlock(&(pOSLs->lock));
            return pOsLtNd->data;
        }
        pOsLtNd=pOsLtNd->next;
    }
    spin_unlock(&(pOSLs->lock));
    return NULL;
}

/*遍历器初始化*/
int32_t OSListIteratorStart(POSListIterator pOsLsItr,POSList pOSLs){
    if(pOsLsItr==NULL || pOSLs==NULL){
        return -1;
    }
    spin_lock(&(pOSLs->lock));
    pOsLsItr->pOSLs = pOSLs;
    pOsLsItr->newLsNode=pOSLs->osLsIm;

    return 0;
}
/*获取data*/
void* OSListIteratorGetNext(POSListIterator pOsLsItr){
    POSListNode tempNode;
    if(pOsLsItr==NULL){
        return NULL;
    }
    tempNode=pOsLsItr->newLsNode;
    if(pOsLsItr->newLsNode!=NULL){
        pOsLsItr->newLsNode=pOsLsItr->newLsNode->next;

        return tempNode->data;
    }
    return NULL;
}
int32_t OSListIteratorStop(POSListIterator pOsLsItr){
    if(pOsLsItr==NULL){
        return -1;
    }

    pOsLsItr->newLsNode=NULL;
    spin_unlock(&(pOsLsItr->pOSLs->lock));
    return 0;
}



