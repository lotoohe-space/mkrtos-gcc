//
// Created by Administrator on 2022/2/11/011.
//

#ifndef UNTITLED1_DEV_H
#define UNTITLED1_DEV_H

//字符设备
#define NULL_MAJOR 6
#define TTY_MAJOR 4
#define TTYMAUX_MAJOR 5
#define FB_DEV_MAJOR 7
//块设备
#define BK_FLASH 0
#define MMC_MAJOR 1

//主次设备号码获取
#define MAJOR(a) (a>>16)
#define MINOR(a) (a&0xffff)

//创建一个设备
#define MKDEV(a,b) ((a<<16)|(b&0xffff))


#endif //UNTITLED1_DEV_H
