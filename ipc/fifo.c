//
// Created by Administrator on 2022/1/23.
//

#include <type.h>
#include <errno.h>
#include <arch/atomic.h>

//fifo µœ÷
struct fifo{
    uint32_t fifo_len;
    uint32_t data_size;
    uint32_t front;
    uint32_t rear;
    uint8_t *data;
};

#define FIFO_NUM 16
struct fifo fifo_list[FIFO_NUM]={0};

int32_t is_full(int32_t f_i){
    struct fifo *f;

    if(f_i<0||f_i>=FIFO_NUM){
        return -EINVAL;
    }
    f=&(fifo_list[f_i]);
//    if(atomic_test())
}
