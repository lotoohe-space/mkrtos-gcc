//
// Created by Administrator on 2022/2/11/011.
//

#ifndef UNTITLED1_DEV_H
#define UNTITLED1_DEV_H

#define TTY_MAJOR 4
#define TTYMAUX_MAJOR 5
//�����豸�����ȡ
#define MAJOR(a) (a>>16)
#define MINOR(a) (a&0xffff)

//����һ���豸
#define MKDEV(a,b) ((a<<16)|(b&0xffff))


#endif //UNTITLED1_DEV_H
