#include "shell_cmd.h"
#include "list.h"
#include <string.h>
#if SHELL_USING_CMD_EXPORT == 1
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
        extern const unsigned int shellCmd$$Base;
        extern const unsigned int shellCmd$$Limit;
    #elif defined(__ICCARM__) || defined(__ICCRX__)
        #pragma section="shellCmd"
    #elif defined(__GNUC__)
        extern const uint64_t*  _shell_command_start;
        extern const uint64_t* _shell_command_end;
    #endif
#endif
/**
* @breif Insert按键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellInsert(PShell pShell);
/**
* @breif 右键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellRight(PShell pShell);
/**
* @breif 左键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellLeft(PShell pShell);

/**
* @breif 删除一个字符的实现
* @return 0成功 <0失败
*/
int32_t OSShell_BS(PShell pShell);

/**
* @breif 回车按钮
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellCR(PShell pShell);

/**
* @breif 回删键
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellDelete(PShell pShell);

/**
* @breif 上键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellUp(PShell pShell);

/**
* @breif 下键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellDown(PShell pShell);
/**
* @breif 上键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellTab(PShell pShell);
/**
* @brief ctrl+c
*/
int32_t OSShellCtrlC(PShell pShell);
/**
* @breif 清除当前输入的数据，包括终端的
* @param pShell shell指针
* @return 0成功 <0失败
*/
int32_t OSShellClearCurInput(PShell pShell){
    if( pShell == NULL){
        return -1;
    }
    /**右移动指针，然后在删除*/
    uint16_t i;
    uint16_t end=pShell->shellRecvCount-pShell->shellDataPosInx;
    for(i=0;i<end;i++){
        OSShellRight(pShell);
    }
    while(pShell->shellRecvCount){
        OSShell_BS(pShell);
    }
    return 0;
}
/**
* @breif 比较两个字符串，返回字符串不相同位置的指针,如果位置为第一个，则返回NULL
* @return NULL 两个字符串相同 不为NULL则表示不相同位置处的指针
*/
const char* OSShellCmp(const char *str1,const char *str2){
    char register* strn1=(char*)str1,*strn2=(char*)str2;
    while(*strn1 && *strn2 && *strn1==*strn2){
        strn1++;
        strn2++;
    }
    if(*strn1){
        if(strn1==str1){
            return NULL;
        }
        return strn1;
    }
    return NULL;
}
/**
* @breif 预设的key值
*/
Key keys[]={
        [0].keyFun=OSShellUp,
        [0].data="\x1b\x5b\x41",
        [0].keyLen=3,
        [0].matchInx=0,

        [1].keyFun=OSShellDown,
        [1].data="\x1b\x5b\x42",
        [1].keyLen=3,
        [1].matchInx=0,

        [2].keyFun=OSShellRight,
        [2].data="\x1b\x5b\x43",
        [2].keyLen=3,
        [2].matchInx=0,

        [3].keyFun=OSShellLeft,
        [3].data="\x1b\x5b\x44",
        [3].keyLen=3,
        [3].matchInx=0,

        [4].keyFun=OSShell_BS,
        [4].data="\x7F",
        [4].keyLen=1,
        [4].matchInx=0,

        [5].keyFun=OSShellCR,
        [5].data="\r",
        [5].keyLen=1,
        [5].matchInx=0,

        [6].keyFun=OSShellDelete,
        [6].data="\x1b\x5b\x33\x7e",
        [6].keyLen=4,
        [6].matchInx=0,

        [7].keyFun=OSShellTab,
        [7].data="\x09",
        [7].keyLen=1,
        [7].matchInx=0,

        [8].keyFun=OSShellInsert,
        [8].data="\x1b\x5b\x32\x7e",
        [8].keyLen=4,
        [8].matchInx=0,

        [9].keyFun=OSShellCtrlC,
        [9].data="\x03",
        [9].keyLen=1,
        [9].matchInx=0,

};
/**
* @breif Insert按键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellInsert(PShell pShell){
    if(pShell==NULL){
        return -1;
    }
    /**模式的切换*/
    pShell->inputMode=!pShell->inputMode;
    return 0;
}
/**
* @breif Tab处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellTab(PShell pShell){
    if(pShell == NULL){
        return -1;
    }
    if(pShell->ShellTick==NULL){
        return -2;
    }
    pShell->shellRecvCache[pShell->shellRecvCount]=0;
    if(pShell->ShellTick()-pShell->lastKeyPressTick<=200){
        /**
        * TAB键自动补全
        */
        OSListIterator osLI;
        PShellCmd pShellCmd;
        OSListIteratorStart(&osLI,&(pShell->ShellCmdList));
        while((pShellCmd = OSListIteratorGetNext(&osLI)) != NULL){
            const char *resChar;
            if((resChar=OSShellCmp(pShellCmd->funName,(char*)pShell->shellRecvCache))
               !=NULL
                    ){
                uint16_t i;
                for(i=0;resChar[i];i++){
                    OSShellPutInChar(pShell, resChar[i]);
                }
                break;
            }
        }
        OSListIteratorStop(&osLI);
    }

    pShell->lastKeyPressTick=pShell->ShellTick();
    return 0;
}
/**
* @breif 上键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellUp(PShell pShell){
    uint16_t i;
    if(pShell == NULL){
        return -1;
    }

    /**选择一个可用的输入*/
    pShell->cmdInx++;
    if(pShell->cmdInx >= pShell->cmdCount-1){
        pShell->cmdInx = pShell->cmdCount-1;
    }
    if(pShell->cmdInx==-1){
        return -2;
    }

    /**清除当前输入的数据*/
    OSShellClearCurInput(pShell);

    /**调用输入函数进行输入*/
    for(i=0;i<strlen(pShell->cmdList[pShell->cmdInx]);i++){
        OSShellPutInChar(pShell, pShell->cmdList[pShell->cmdInx][i]);
    }
    return 0;
}
/**
* @breif 下键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellDown(PShell pShell){
    uint16_t i;
    if(pShell == NULL){
        return -1;
    }
    /**选择一个可用的输入*/
    pShell->cmdInx--;
    if(pShell->cmdInx <0){
        pShell->cmdInx =0;
    }

    /**清除当前输入的数据*/
    OSShellClearCurInput(pShell);

    /**调用输入函数进行输入*/
    for(i=0;i<strlen(pShell->cmdList[pShell->cmdInx]);i++){
        OSShellPutInChar(pShell, pShell->cmdList[pShell->cmdInx][i]);
        //OSShellInputChar(pShell, pShell->cmdList[pShell->cmdInx][i]);
    }

    return 0;
}
/**
* @breif 右键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellRight(PShell pShell){
    if(pShell == NULL){
        return -1;
    }
    if(pShell->shellDataPosInx<pShell->shellRecvCount){
        OSShellOutputChar(pShell,pShell->shellRecvCache[pShell->shellDataPosInx++]);
    }
    return 0;
}
/**
* @breif 左键处理
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellLeft(PShell pShell){
    if(pShell == NULL){
        return -1;
    }
    if(pShell->shellDataPosInx<=0){
        return -2;
    }
    pShell->shellDataPosInx--;
    OSShellOutputChar(pShell,'\b');
    return 0;
}
/**
* @breif 删除一个字符的实现
* @return 0成功 <0失败
*/
int32_t OSShell_BS(PShell pShell){
    if( pShell == NULL){
        return -1;
    }
    if(pShell->shellDataPosInx<=0){
        return -2;
    }
    /**让控制台删除一个字符*/
    OSShellOutput(pShell,"\b");

    uint16_t i;
    for(i=pShell->shellDataPosInx-1 ;i<pShell->shellRecvCount-1;i++){
        pShell->shellRecvCache[i]=pShell->shellRecvCache[i+1];
        OSShellOutputChar(pShell,pShell->shellRecvCache[i]);
    }
    OSShellOutputChar(pShell,' ');
    for(i=0;i<pShell->shellRecvCount-pShell->shellDataPosInx;i++){
        OSShellOutput(pShell,"\b");
    }
    OSShellOutput(pShell,"\b");
    /**缓存指针移动一个*/
    pShell->shellDataPosInx--;
    pShell->shellRecvCount--;
    return 0;
}
/**
* @breif 回车按钮
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellCR(PShell pShell){
    int16_t i=0;
    if( pShell == NULL){
        return -1;
    }
    if(pShell->runTaskPid>=0){
        return -1;
    }
    pShell->shellRecvCache[pShell->shellRecvCount]=0;
    /**向后移动一个*/
    if(pShell->cmdCount == HISTORY_CMD_NUM){
        pShell->ShellFree((void*)(pShell->cmdList[pShell->cmdCount-1]));
        for(i=pShell->cmdCount-1;i>=0;i--){
            if(i!=0){
                pShell->cmdList[i]=pShell->cmdList[i-1];
            }
        }
    }else{
        for(i=pShell->cmdCount-1;i>=0;i--){
            pShell->cmdList[i+1]=pShell->cmdList[i];
        }
    }

    pShell->cmdList[0]=NULL;

    uint16_t len = strlen((const char*)(pShell->shellRecvCache)) + 1;
    pShell->cmdList[0] = pShell->ShellMalloc(len);
    if(pShell->cmdList[0] == NULL){
        /**系统没有可用内存*/
        return -2;
    }
    /**保存当前条到历史记录*/
    strcpy(
            (char*)(pShell->cmdList[0]),
            ((const char*)pShell->shellRecvCache)
    );
    if(pShell->cmdCount != HISTORY_CMD_NUM){
        pShell->cmdCount++;
    }
    /**执行一条命令*/
    OSShellExecCmd(pShell);
    /**清空数据标记*/
    pShell->shellDataPosInx=0;
    pShell->shellRecvCount=0;
    /**输出开始头*/
    OSShellOutput(pShell,OS_SHELL_NEW_LINE);
    if (pShell->ShellLRHook != NULL) {
        OSShellOutput(pShell, pShell->ShellLRHook());
    }
    OSShellOutput(pShell, ":");
    /**清除历史记录Inx*/
    pShell->cmdInx=-1;
    return 0;
}
/**
* @breif 回删键
* @param pShell shell
* @return 0成功 <0失败
*/
int32_t OSShellDelete(PShell pShell){
    if( pShell == NULL){
        return -1;
    }
    if(pShell->runTaskPid>=0){
        return -1;
    }
    if(pShell->shellDataPosInx==pShell->shellRecvCount){
        return -2;
    }
    OSShellRight(pShell);
    if(pShell->shellDataPosInx==pShell->shellRecvCount){
        goto next;
    }
    uint16_t i;
    for(i=pShell->shellDataPosInx ;i<pShell->shellRecvCount;i++){
        pShell->shellRecvCache[i]=pShell->shellRecvCache[i+1];

        if(i==pShell->shellRecvCount-1){
            OSShellOutputChar(pShell,' ');
        }else{
            OSShellOutputChar(pShell,pShell->shellRecvCache[i]);
        }
    }
    for(i=0;i<pShell->shellRecvCount-pShell->shellDataPosInx;i++){
        OSShellOutput(pShell,"\b");
    }

    /**缓存指针移动一个*/
    pShell->shellRecvCount--;

    next:
    pShell->shellDataPosInx--;
    OSShellOutput(pShell,"\b");

    return 0;
}
//#include "task.h"
/**
* @brief ctrl+c
*/
int32_t OSShellCtrlC(PShell pShell){

//    if(pShell->runTaskPid >= 0){
//        pShell->ctrlCSig=TRUE;
//        //发送停止信号
//        TaskSigSet(pShell->runTaskPid,SigStop);
//        pShell->runTaskPid=-1;
//    }

    return 0;
}
/**
* @brief 系统信息打印
*/
/**
* @brief 系统信息打印
*/
const char* sysInfo[]={
        " _____ ______   ___  __    ________  _________  ________  ________      \r\n",
        "|\\   _ \\  _   \\|\\  \\|\\  \\ |\\   __  \\|\\___   ___\\\\   __  \\|\\   ____\\     \r\n",
        "\\ \\  \\\\\\__\\ \\  \\ \\  \\/  /|\\ \\  \\|\\  \\|___ \\  \\_\\ \\  \\|\\  \\ \\  \\___|_    \r\n",
        " \\ \\  \\\\|__| \\  \\ \\   ___  \\ \\   _  _\\   \\ \\  \\ \\ \\  \\\\\\  \\ \\_____  \\   \r\n",
        "  \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\ \\  \\\\  \\|   \\ \\  \\ \\ \\  \\\\\\  \\|____|\\  \\  \r\n",
        "   \\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\ \\__\\\\ _\\    \\ \\__\\ \\ \\_______\\____\\_\\  \\ \r\n",
        "    \\|__|     \\|__|\\|__| \\|__|\\|__|\\|__|    \\|__|  \\|_______|\\_________\\\r\n",
        "                                                            \\|_________|\r\n",
        "\r\nShell version:"SHELL_VERSION"\r\n",
        "Complie Time:" __DATE__ " " __TIME__"\r\n",
};


/**
* @breif 打印系统信息
* @param argc 参数个数
* @param args 参数列表
*/
int32_t OSInfo(int32_t argc,const char** args){

    if(curShell!=NULL){
        uint16_t i;
        for(i=0;i<(sizeof(sysInfo))/sizeof(char*);i++){
            OSShellOutput(curShell,sysInfo[i]);
        }
    }
    return 0;
}
SHELL_EXPORT_CMD(OSInfo,OSInfo);
/**
* @breif 打印系统信息
* @param argc 参数个数
* @param args 参数列表
*/
int32_t OSClear(int32_t argc,const char** args){
    OSShellOutput(curShell,"\x0c");
    return 0;
}
SHELL_EXPORT_CMD(OSClear,clear);

/**
* @breif cmd命令与键值初始化
* @param pShell shell
* @return 0 成功 <0 失败
*/
int32_t OSShellCmdInit(PShell pShell){
    uint16_t i;
    int32_t res;
    for(i=0;i<sizeof(keys)/sizeof(Key);i++){
        OSShellAddKey(pShell,(PKey)&(keys[i]));
    }

//	for(i=0;i<sizeof(shellCmdList)/sizeof(ShellCmd);i++){
//		/**添加默认的命令函数*/
//		res=OSShellAddCmd2CurShell((PShellCmd)(&(shellCmdList[i])));
//		if(res!=0){
//			return res;
//		}
//	}

#if SHELL_USING_CMD_EXPORT == 1
    #if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
		PShellCmd pscList= (PShellCmd)(&shellCmd$$Base);
		uint32_t lsLen = ((unsigned int)(&shellCmd$$Limit)
												- (unsigned int)(&shellCmd$$Base))
												/ sizeof(ShellCmd);
		for(uint32_t i=0;i<lsLen;i++){
			res=OSShellAddCmd2CurShell(&(pscList[i]));
			if(res!=0){
				return res;
			}
		}
    #elif defined(__ICCARM__) || defined(__ICCRX__)
		#error not supported compiler, please use command table mode
    #elif defined(__GNUC__)
        PShellCmd pscList= (PShellCmd )(&_shell_command_start+1);
        uint32_t lsLen = ((uint64_t)(&_shell_command_end)- (uint64_t)(&_shell_command_start)) / sizeof(ShellCmd);
		for(uint32_t i=0;i<lsLen;i++){
			res=OSShellAddCmd2CurShell(&(pscList[i]));
			if(res!=0){
				return res;
			}
		}
    #else
        #error not supported compiler, please use command table mode
    #endif
#endif


    return 0;
}

