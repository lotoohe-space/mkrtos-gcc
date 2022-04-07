//
// Created by Administrator on 2022/1/19.
//

#include <stdio.h>
#include <stdarg.h>
#include <type.h>
#include <mkrtos/fs.h>
#define DEBUG 0
static uint8_t printk_cache[512];
void trace(const char* fmt, ...){
#if DEBUG
    va_list args;
    while(flag);
    flag=1;
    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);

    console_write(printk_cache);
    flag=0;
#endif
}
void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);

    console_write(printk_cache);
}

void fatalk(const char* fmt,...){
    va_list args;
    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);
    console_write(printk_cache);
    while(1);
}