//
// Created by Administrator on 2022/2/11/011.
//

#include <mkrtos/tty.h>
#include <sys/types.h>
#include <arch/arch.h>

void q_clear(struct tty_queue *t_queue){
    uint32_t t;
    t=DisCpuInter();
    t_queue->front=t_queue->rear=0;
    RestoreCpuInter(t);
}

int32_t q_add(struct tty_queue *t_queue,uint8_t d){
    uint32_t t;
    t=DisCpuInter();
    if((t_queue->rear+1)%TTY_READ_BUF_LEN==t_queue->front){
        RestoreCpuInter(t);
        return -1;//满了
    }
    t_queue->read_buf[t_queue->rear]=d;
    t_queue->rear=(t_queue->rear+1)%TTY_READ_BUF_LEN;
    RestoreCpuInter(t);
    return 0;
}
//检测尾部的字符是否是指定的字符
int32_t q_check_f(struct tty_queue *t_queue,uint8_t c){
    uint32_t t;
    t=DisCpuInter();
    if(t_queue->rear==t_queue->front){//空的
        RestoreCpuInter(t);
        return -1;
    }
    uint32_t tmp_rear = (t_queue->rear - 1 + TTY_READ_BUF_LEN) % TTY_READ_BUF_LEN;
    if(t_queue->read_buf[tmp_rear]==c){
        RestoreCpuInter(t);
        return 0;
    }
    RestoreCpuInter(t);
    return -1;
}
//尾部部出队
int32_t q_get_f(struct tty_queue *t_queue,uint8_t *d){
    uint32_t t;
    t=DisCpuInter();
    if(t_queue->rear==t_queue->front){//空的
        RestoreCpuInter(t);
        return -1;
    }
    t_queue->rear = (t_queue->rear - 1 + TTY_READ_BUF_LEN) % TTY_READ_BUF_LEN;
    if(d) {
        *d = t_queue->read_buf[t_queue->rear];
    }
    RestoreCpuInter(t);
    return 0;
}
int32_t q_get(struct  tty_queue *t_queue,uint8_t *d){
    uint32_t t;
    t=DisCpuInter();
    if(t_queue->rear==t_queue->front){
        RestoreCpuInter(t);
        return -1;
    }
    if(d) {
        *d = t_queue->read_buf[t_queue->front];
    }
    t_queue->front=(t_queue->front+1)%TTY_READ_BUF_LEN;
    RestoreCpuInter(t);
    return 0;
}