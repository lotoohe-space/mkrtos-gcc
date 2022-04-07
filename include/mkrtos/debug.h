//
// Created by Administrator on 2022/4/7.
//

#ifndef UNTITLED1_DEBUG_H
#define UNTITLED1_DEBUG_H


#define DEBUG_ENABLE  1
#define LOG_LEVEL     DEBUG_ALL

#define MODULE "BIGGG"
#define ERR    "ERR"
#define WARN   "WARN"
#define INFO   "INFO"

#define DEBUG_COLOR_ERR       "\033[1;31m"				//高亮红色
#define DEBUG_COLOR_WARN      "\033[1;34m"				//高亮黄色
#define DEBUG_COLOR_INFO      "\033[1;33m"              //高亮黄色
#define DEBUG_COLOR_END       "\033[1;37m"				//白色

enum DEBUG_LEVEL
{
    DEBUG_OFF = 0,
    DEBUG_ERR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_ALL
};

#define DEBUG(MODULE, LEVEL, FMT, ARGS...) \
do \
{                                          \
uint32_t t;                                \
t=DisCpuInter();                           \
    if(DEBUG_ENABLE && (DEBUG_##LEVEL <= LOG_LEVEL)) \
    { \
      printk(DEBUG_COLOR_##LEVEL); \
      printk("[%s]:[%s]:[%s]:%d "FMT"\r\n", MODULE, __FILE__, __FUNCTION__, __LINE__, ##ARGS); \
      printk(DEBUG_COLOR_END); \
    }                                      \
    RestoreCpuInter(t);\
} while(0)\


#endif //UNTITLED1_DEBUG_H
