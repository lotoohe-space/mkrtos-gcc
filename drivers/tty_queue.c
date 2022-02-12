//
// Created by Administrator on 2022/2/11/011.
//

#include <mkrtos/tty.h>
#include <sys/types.h>
#include <arch/arch.h>
int32_t q_add(struct tty_queue *t_queue,uint8_t d){
    uint32_t t;
    t=DisCpuInter();
    if((t_queue->rear+1)%TTY_READ_BUF_LEN==t_queue->front){
        RestoreCpuInter(t);
        return -1;//ย๚มห
    }
    t_queue->read_buf[t_queue->rear]=d;
    t_queue->rear=(t_queue->rear+1)%TTY_READ_BUF_LEN;
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
    *d=t_queue->read_buf[t_queue->front];
    t_queue->front=(t_queue->front+1)%TTY_READ_BUF_LEN;
    RestoreCpuInter(t);
    return 0;
}