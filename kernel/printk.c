//
// Created by Administrator on 2022/1/19.
//

#include <xprintf.h>
#include <stdarg.h>
#include <type.h>
#include <mkrtos/fs.h>

static uint32_t flag=0;
static uint8_t printk_cache[512];
void trace(const char* fmt, ...){
    va_list args;
    while(flag);
    flag=1;
    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);

    console_write(printk_cache);
    flag=0;
}
void printk(const char *fmt, ...)
{
    va_list args;
    while(flag);
    flag=1;
    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);

    console_write(printk_cache);
    flag=0;
}
void fatalk(const char* fmt,...){
    va_list args;
    while(flag);
    flag=1;
    va_start(args, fmt);
    vsprintf(printk_cache,fmt,args);
    va_end(args);
    console_write(printk_cache);
    flag=0;
    while(1);
}