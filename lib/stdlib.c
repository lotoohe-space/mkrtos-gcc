//
// Created by Administrator on 2022/1/16.
//
#include <type.h>
static uint32_t next=1;//��̬ȫ�ֱ�������Ϊ����

void srand(uint32_t seed)
{
    next=seed;
}

int32_t rand(void)
{
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}
