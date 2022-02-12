//
// Created by Administrator on 2022/2/11/011.
//
#include <sys/types.h>
#include "bsp/CH432T.h"
#include <mkrtos/tty.h>
#include <mkrtos/dev.h>
#include <mkrtos/fs.h>

static int initFlag;
struct tty_struct *my_tty[2]={0};

static void ch432t_read_cb(uint8_t port,uint8_t * data,uint16_t len){
    if(port==1){
        for(int i=0;i<len;i++){
            if(my_tty[port]) {
                q_add(&my_tty[port]->r_queue, data[i]);
            }
        }
    }
}


static void uart_close(struct tty_struct * tty, struct file * filp){
    my_tty[0]=0;
    initFlag=0;
}
static int32_t uart_write(struct tty_struct * tty,uint8_t *buf,int len){
    CH432Seril1Send((uint8_t*)buf,len);
    return len;
}

static int32_t  uart_ioctl(struct tty_struct *tty, struct file * file,uint32_t cmd, uint32_t arg){

    return -ENOSYS;
}

int32_t uart_open(struct tty_struct * tty, struct file * filp){
    if (initFlag) { return 0; }
    initFlag = 1;
    Ch432_SPI_Init();
    CH432T_recv_1_data_fun = ch432t_read_cb;
    my_tty[0]=tty;
    tty->write=uart_write;
    tty->close=uart_close;
    tty->ioctl=uart_ioctl;
    return 0;
}
