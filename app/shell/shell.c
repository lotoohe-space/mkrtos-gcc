//
// Created by Administrator on 2022/3/6.
//

#include "shell.h"
#include "list.h"
//#include "OSShellCmd.h"
#include <stdio.h>
//#include "SysCall.h"
//#include "OSShellNetPort.h"

#include "string.h"

/**
* @breif 当前正在使用的ShellCmd
*/
PShell curShell=NULL;

/**
* @breif 这是list的添加数据时数据的处理函数
* @param data 数存放在节点中的位置
* @param newData 需要添加的数据
*/
static void OSListAddNtData(uint8_t** data,void * newData){
    *data=newData;
}
/**
* @breif list的数据删除函数
* @param 被删除的数据存放的位置
*/
static void OSListDelNtData(uint8_t** data){
    return;
}

/**
* @breif list查找时的数据比较函数
* @param pOSListNode List中数据的节点
* @param toCompareVal 比较的数据
*/
static int32_t OSListFindCompareDefault(POSListNode pOSListNode,void* toCompareVal){
    PShellCmd pShellCmd=(PShellCmd)pOSListNode->data;
    /**查找指定函数名称*/
    if(strcmp((char*)(pShellCmd->funName),(const char*)toCompareVal)==0){
        return 0;
    }
    return -1;
}

/**
 * @brief shell初始化
 * @param PShell shell
 * @return 0 成功 <0失败
 */
int32_t OSShellInit(PShell pShell){
//	int32_t res;
//	int32_t i;
    if(pShell == NULL){
        return -1;
    }

    curShell = pShell;

    OSListCreate(&(pShell->ShellCmdList),OSListAddNtData,
                 OSListDelNtData,OSListFindCompareDefault);

    OSListCreate(&(pShell->KeyCmdList),NULL,
                 NULL,NULL);

    OSShellCmdInit(pShell);


    return 0;
}

/**
*	@breif 控制台输出字符串
* @param pShell shell
* @param str 输出的字符串
* @return 0成功 <0失败
*/
int32_t OSShellOutput(PShell pShell,const char* str){
    if( pShell == NULL){
        return -1;
    }
    /**参数超过限制*/
    pShell->ShellWrite((uint8_t*)str,strlen(str));
    return 0;
}

/**
*	@breif 控制台输出一个字符
* @param pShell shell指针
* @param ch 输出的字符
* @return 0成功 <0失败
*/
int32_t OSShellOutputChar(PShell pShell,char ch){
    if( pShell == NULL){
        return -1;
    }
    /**参数超过限制*/
    pShell->ShellWrite((uint8_t*)&ch,1);
    return 0;
}

/**
* @breif Shell执行一条cmd命令
* @param pShell shell指针
* @param cmdStr 命令字符串，以\n结尾
* @return 0成功 <0失败
*/
int32_t OSShellExecCmd(PShell pShell){
    uint32_t cmdArgCount=0;
    uint16_t i;
    uint8_t flag=0;
    PInputShellCmd inShellCmd;
    PShellCmd pShellCmd;

    if( pShell == NULL){
        return -1;
    }
    if(pShell->shellRecvCount==0){
        return -2;
    }
    char *cmdStr=(char*)pShell->shellRecvCache;
    cmdStr[pShell->shellRecvCount]=0;
    /**进行单词分割*/
    inShellCmd=&(pShell->inputShellCmd);

    for(i=0;i<pShell->shellRecvCount;i++){

        /**一直为空格则跳过*/
        if(flag==1||(cmdStr[i]==' '||cmdStr[i]=='\t')){
            if((cmdStr[i]==' '||cmdStr[i]=='\t')){
                cmdStr[i]=0;
                flag=0;
                continue;
            }
        }
        /**不等于空格并且不等于\t，则为一个单词*/
        if((flag==0||i==0)&&cmdStr[i]!=' '
           &&cmdStr[i]!='\t'
                ){
            flag=1;
            if(cmdArgCount==0){
                inShellCmd->funName=cmdStr;
            }else{
                inShellCmd->strArg[cmdArgCount-1]=cmdStr+i;
            }
            cmdArgCount++;
            if(cmdArgCount >= SHELL_CMD_ARG_NUM-1){
                /**参数超过限制*/
                OSShellOutput(pShell,"\r\nThe command parameter exceeds the system limit.\r\n");
                break;
            }
        }
    }
    cmdArgCount--;
#if OS_SHELL_DEBUG
    OSShellOutput(pShell,"\r\nfunction name\r\n");
	OSShellOutput(pShell,inShellCmd->funName);
	for(i=0;i<cmdArgCount;i++){
		OSShellOutput(pShell,"\r\n");
		OSShellOutput(pShell,inShellCmd->strArg[i]);
		OSShellOutput(pShell,"\r\n");
	}
#endif

    OSShellOutput(pShell, "\r\n");

    pShellCmd = OSShellFindCmdByFunName(pShell,inShellCmd->funName);
    if(pShellCmd==NULL){
//		if((inShellCmd->funName[0]=='.'&&inShellCmd->funName[1]=='/')
//			||(inShellCmd->funName[0]=='.'&&inShellCmd->funName[1]=='\\')){
//
//				return 0;
//			}
//
        OSShellOutput(pShell,"\r\nThe command could not be found.\r\n");
        return -1;
    }

    _MainFun mainFun=(_MainFun)(pShellCmd->fun);
    if(mainFun==NULL){
        return -1;
    }

    int32_t res;
    res=mainFun(cmdArgCount,inShellCmd->strArg);
    if(res<0){
        printf("\n%s\nreturn is%d\n",inShellCmd->funName,res);

    }
    /**打印执行的结果*/
    /*OSShellOutput(pShell,"\r\n");
    OSShellOutput(pShell,inShellCmd->funName);
    OSShellOutput(pShell," command exec succuess.\r\n");*/

    return 0;
}

/**
* @breif 通过函数名称找到ShellCmd
* @param funName 需要找到的函数名称
* @return 查找到的对象指针
*/
PShellCmd OSShellFindCmdByFunName(PShell pShell,const char* funName){
    if(funName==NULL){
        return NULL;
    }

    return OSListFind(&(pShell->ShellCmdList),(void*)funName);
}
/**
* @breif 添加命令到当前的shell
* @param pShellCmd 需要添加的命令
* @return 0成功 <0失败
*/
int32_t OSShellAddCmd2CurShell(PShellCmd pShellCmd){
    if(curShell==NULL||pShellCmd==NULL){
        return -1;
    }
    return OSShellAddCmd(curShell,pShellCmd);
}
/**
 * @brief 添加命令
 * @param pShellCmd 添加的命令
 * @return 0 成功 <0失败
 */
int32_t OSShellAddCmd(PShell pShell,PShellCmd pShellCmd){
    int32_t res;
    if(pShellCmd==NULL){
        return -1;
    }

    /*添加一条*/
    res=OSListAddNode(&(pShell->ShellCmdList),pShellCmd);
    if(res!=0){
        return res;
    }

    return 0;
}

/**
* @breif shell添加键值
* @return 0成功 <0失败
*/
int32_t OSShellAddKey(PShell pShell,PKey pKey){
    int32_t res;
    if( pShell == NULL || pKey==NULL){
        return -1;
    }

    /*添加一条*/
    res=OSListAddNode(&(pShell->KeyCmdList),pKey);
    if(res!=0){
        return res;
    }

    return 0;
}
/**
* @breif 对输入的字符进行key匹配，返回匹配成功的Key
* @return 0成功 <0失败
*/
int32_t OSShellKeyMatch(PShell pShell, char inchar){
    int32_t res=-1;
    if( pShell == NULL){
        goto end;
    }
    OSListIterator osLI;

    OSListIteratorStart(&osLI,&(pShell->KeyCmdList));
    uint8_t matchFlag=0;
    PKey pkey;
    while((pkey = OSListIteratorGetNext(&osLI)) != NULL){
        if(
                pkey->data[pkey->matchInx] == inchar /**字符匹配*/
                ){
            pkey->matchInx++;
            if(pkey->matchInx >= pkey->keyLen){
                PKey tKey;
                OSListIteratorStop(&osLI);

                OSListIteratorStart(&osLI,&(pShell->KeyCmdList));
                while((tKey = OSListIteratorGetNext(&osLI)) != NULL){
                    /**所有匹配清零*/
                    tKey->matchInx=0;
                }
                OSListIteratorStop(&osLI);

                /**匹配成功*/
                if(pkey->keyFun!=NULL){
                    pkey->keyFun(pShell);
                }
                pkey->matchInx=0;
                res = 0;
                goto end;
            }

            /**部分匹配*/
            matchFlag=1;
        }
    }
    if(matchFlag){
        res=1;
    }
    /**没有匹配到*/
    OSListIteratorStop(&osLI);
    end:
    return res;
}
/**
* @breif 缓存后移动一个，溢出的直接扔掉
* @param data 需要移动的指针
* @param inx 从那个位置开始移动
* @param len 数据长度
* @return 0成功 <0失败
*/
int32_t OSShellMoveCache(uint8_t *data,int16_t inx,uint16_t dataLen,uint16_t chLen){
    if( data == NULL ){
        return -1;
    }
    if(dataLen==0){
        return -2;
    }
    int16_t i;
    for(i=dataLen-1;i>=inx;i--){
        if( dataLen == chLen && i == dataLen-1){
            continue;
        }
        data[i+1]=data[i];
    }
    return 0;
}

/**
* @breif 存入一个新字符到shell缓存区的尾部，并进行回显
* @param pShell shell的指针
* @param inChar 输入的字符
* @return 0成功 <0失败
*/
int32_t OSShellPutInChar(PShell pShell, uint8_t inChar){
    if( pShell == NULL){
        return -1;
    }
    if(pShell->inputMode==0){
        if(pShell->shellDataPosInx==pShell->shellRecvCount){
            goto next;
        }
        /**数据向后移动n个*/
        OSShellMoveCache(pShell->shellRecvCache
                ,pShell->shellDataPosInx-1
                ,pShell->shellRecvCount
                ,pShell->shellRecvCacheSize
        );
        pShell->shellRecvCache[pShell->shellDataPosInx]=inChar;
        pShell->shellRecvCount++;
        uint16_t i;
        for(i=pShell->shellDataPosInx;i<pShell->shellRecvCount;i++){
            OSShellOutputChar(pShell,pShell->shellRecvCache[i]);
        }
        for(i=pShell->shellDataPosInx;i<pShell->shellRecvCount;i++){
            OSShellOutputChar(pShell,'\b');
        }

    }else{
        next:
        pShell->shellRecvCache[pShell->shellRecvCount]=inChar;
        pShell->shellRecvCount++;
        if(pShell->shellRecvCount >= pShell->shellRecvCacheSize-1){
            return -2;
        }
        pShell->shellDataPosInx++;
        OSShellOutputChar(pShell,inChar);
    }
    return 0;
}
/**
* @breif 输入一个字符到shell
* @param pShell shell的指针
* @param inChar 输入的字符
* @return 0正确 <0错误
*/
int32_t OSShellInputChar(PShell pShell, uint8_t inChar){

    if( pShell == NULL){
        return -1;
    }

    if(OSShellKeyMatch(pShell,inChar)>=0){
        /**说明有匹配*/
        return -2;
    }

    if(inChar>=32 && inChar<=126){
        /**输入的字符*/
        OSShellPutInChar(pShell,inChar);
    }

    return 0;
}
/**
* @breif OSShell的主让你无
* @param arg0 传入的参数
*/
void OSShellTask(void* arg0,void* arg1){
    PShell pShell=arg0;

//    if(arg1){
//
//        int32_t netFd;
//        netFd=ShellNetPortInit();
//        if(netFd>0){
//            int32_t save_fd = dup(0);
//            dup2(netFd, 0);
//        }
//    }
    OSInfo(NULL,NULL);
    while(1){
        int32_t rLen;
        uint8_t rData[6];
        if( pShell->ShellRead!=NULL &&
            (rLen=pShell->ShellRead(rData,sizeof(rData)))>0
                ){
            for(int32_t i=0;i<rLen;i++){
                //if(rData[i]!='\n'){
                if(OSShellInputChar(pShell, rData[i])!=0){
                }
                //}
            }
        }

//        if(pShell->runTaskPid>0){
//            if(waitpid(pShell->runTaskPid,NULL,NULL)<0){
//                SVCOSTaskDelay(-1,50);
//            }else{
//                pShell->runTaskPid=-1;
//            }
//        }
//        SVCOSTaskDelay(-1,10);
    }

}

