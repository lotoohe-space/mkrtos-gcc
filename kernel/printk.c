//
// Created by Administrator on 2022/1/19.
//

#include <xprintf.h>
#include <stdarg.h>
#include <type.h>

static uint8_t printk_cache[512];
void printk(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);
}
void fatalk(const char* fmt,...){
    va_list args;

    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);
    while(1);
}