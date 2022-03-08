#ifndef _OS_SHELL_CMD_H__
#define _OS_SHELL_CMD_H__

#include "shell.h"
/**
* @breif cmd命令与键值初始化
* @param pShell shell
* @return 0 成功 <0 失败
*/
int32_t OSShellCmdInit(PShell pShell);
int32_t OSShell_BS(PShell pShell);

#endif
