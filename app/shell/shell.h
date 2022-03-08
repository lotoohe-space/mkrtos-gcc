#ifndef _OS_SHELL_H__
#define _OS_SHELL_H__

#include "list.h"
#include "type.h"
#include "mkrtos/fs.h"
/**
* @breif shell的版本
*/
#define SHELL_VERSION "1"
/**
 * @breif 是否支持命令导出
 */
#define SHELL_USING_CMD_EXPORT 1
/**
* @breif 是否调试
*/
#define OS_SHELL_DEBUG 0

/**
* @breif 历史记录数量
*/
#define HISTORY_CMD_NUM 10

/**
* @breif shell 命令行参数数量
*/
#define SHELL_CMD_ARG_NUM 8

/**
* @breif 新行输入的字符
*/
#define OS_SHELL_NEW_LINE "\r\nOS Shell"

/**
* @breif shell接收缓存区大小
*/
#define SHELL_RECV_FIFO_CACHE_SIZE 512

/**
* @breif 命令的入口函数，入口函数定义为统一的入口函数
*/
typedef int32_t (*_MainFun)(int32_t argc,const char* args[]);

/**
* @breif 输入的参数
*/
typedef struct _InputShellCmd{

    /**
    * @breif 函数名称
    */
    const char *funName;

    /**
    * @breif 参数列表
    */
    const char *strArg[SHELL_CMD_ARG_NUM];

}*PInputShellCmd,InputShellCmd;

/**
* @breif 命令
*/
typedef struct _ShellCmd{

    /**
    * @breif shell命令函数
    */
    void* fun;
    /**
    * @breif 函数名字
    */
    const char* funName;

}*PShellCmd,ShellCmd;

/**
*	@breif shell的写数据
* @param data 需要发送的数据
* @param len 需要发送的长度
* @return 需要返回的数据
*/
typedef int32_t (*_ShellWrite)(uint8_t* data, uint16_t len);
/**
* @breif shell读取数据
* @param data 需要读取的数据长度
* @parma len 读取缓存区的长度
* @return 读取到的数据长度
*/
typedef int32_t (*_ShellRead)(uint8_t* data, uint16_t len);
/**
* @breif 显示新行后需用显示的其它东西的钩子函数
*/
typedef char* (*_ShellLRHook)(void);
/**
* @breif 获取tick时间，单位ms
* @return 返回获取到的时间
*/
typedef uint32_t (*_ShellTick)(void);

/**
* @breif 分配内存
* @param num 分配的内存
* @return 申请到的内存
*/
typedef void* (*_ShellMalloc)(size_t num);

/**
* @breif 释放内存
* @param mem 释放的内存
*/
typedef void 	(*_ShellFree)(void* mem);

/**
* @breif 控制台的struct
*/
typedef struct _Shell{

    /**
    * @breif 申请内存的函数
    */
    _ShellMalloc ShellMalloc;

    /**
    * @breif 释放内存的函数
    */
    _ShellFree ShellFree;

    /**
    * @breif shell写字符
    */
    _ShellWrite ShellWrite;

    /**
    *	@breif shell读字符
    */
    _ShellRead ShellRead;
    /**
    *	@breif 回车后显示的钩子函数
    */
    _ShellLRHook ShellLRHook;


    /**
    *	@breif shell tick
    */
    _ShellTick ShellTick;



    /**
    * @breif shell接受缓存区
    */
    uint8_t *shellRecvCache;
    /**
    * @breif shell接收数据的缓存区大小
    */
    uint16_t shellRecvCacheSize;
    /**
    * @breif shell已经接收的数据大小
    */
    uint16_t shellRecvCount;
    /**
    * @breif shell指针控制的位置
    */
    int16_t shellDataPosInx;
    /**
    * @breif 上次按键按下的时间
    */
    uint32_t lastKeyPressTick;


    /**
    * @breif 命令列表
    */
    OSList ShellCmdList;

    /**
    * @breif 键值列表
    */
    OSList KeyCmdList;

    /**
    * @breif 存储历史记录
    */
    const char* cmdList[HISTORY_CMD_NUM];
    /**
    * @breif 历史记录个数
    */
    int16_t cmdCount;
    /**
    * @breif 使用的那一条inx，当为-1时则使用的当前输入缓存区
    */
    int16_t cmdInx;

    /**
    * @breif 输入的模式，当按下insert键的输入模式，一个时覆盖模式，一个是插入模式，默认为插入模式
    */
    uint8_t inputMode;

    /**
    * @breif 当前命令输入的参数存储
    */
    InputShellCmd inputShellCmd;

    /**
    *@brief ctrl c
    */
    uint8_t ctrlCSig;

    int16_t runTaskPid;

}*PShell, Shell;

/**
* @breif 键值结构体
*/
typedef struct _Key{
    /**
    * @breif 按键回调函数
    * @param pShell shell的指针
    */
    int32_t (*keyFun)(PShell pShell);
    /**
    * @breif 键值数据
    */
    const char *data;
    /**
    * @breif 键值数据长度
    */
    uint8_t keyLen;
    /**
    * @breif 匹配的inx
    */
    uint8_t matchInx;
}*PKey, Key;

/**
 * @brief shell初始化
 * @param PShell shell
 * @return 0 成功 <0失败
 */
int32_t OSShellInit(PShell pShell);
/**
* @breif 添加命令到当前的shell
* @param pShellCmd 需要添加的命令
*/
int32_t OSShellAddCmd2CurShell(PShellCmd pShellCmd);

/**
 * @brief 添加命令
 * @param pShellCmd 添加的命令
 * @return 0 成功 <0失败
 */
int32_t OSShellAddCmd(PShell pShell,PShellCmd pShellCmd);

/**
* @breif 通过函数名称找到ShellCmd
* @param funName 需要找到的函数名称
* @return 查找到的对象指针
*/
PShellCmd OSShellFindCmdByFunName(PShell pShell,const char* funName);

/**
* @breif Shell执行一条cmd命令
* @param pShell shell指针
* @param cmdStr 命令字符串，以\n结尾
*/
int32_t OSShellExecCmd(PShell pShell);

/**
* @breif OSShell的主让你无
* @param arg0 传入的参数
*/
void OSShellTask(void* arg0,void* arg1);
/**
* @breif 对输入的字符进行key匹配，返回匹配成功的Key
*/
int32_t OSShellKeyMatch(PShell pShell,char inchar);
/**
* @breif shell添加键值
*/
int32_t OSShellAddKey(PShell pShell,PKey pKey);
/**
* @breif 存入一个新字符到shell缓存区的尾部，并进行回显
* @param pShell shell的指针
* @param inChar 输入的字符
*/
int32_t OSShellPutInChar(PShell pShell, uint8_t inChar);
/**
* @breif 输入一个字符到shell
* @param pShell shell的指针
* @param inChar 输入的字符
* @return 0正确 <0错误
*/
int32_t OSShellInputChar(PShell pShell, uint8_t inChar);
/**
*	@breif 控制台输出一个字符
*/
int32_t OSShellOutputChar(PShell pShell,char ch);
/**
*	@breif 控制台输出字符串
*/
int32_t OSShellOutput(PShell pShell,const char* str);
/**
* @breif 清除当前输入的数据
* @param pShell shell指针
*/
int32_t OSShellClearCurInput(PShell pShell);

/**
 * @breif shell的命令导出可以快速导出命令
 */
#define SHELL_EXPORT_CMD(_fun, _name) \
        const char shellCmd##_##_name[] = #_name; \
        const ShellCmd \
        shellCommand##_name SECTION("shellCmd") =  \
        { \
        	.fun = _fun,\
			.funName = shellCmd##_##_name,\
        }


/**
* @breif 当前使用中的shell
*/
extern PShell curShell;

#endif
