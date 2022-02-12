//
// Created by Administrator on 2022/2/11/011.
//
#include <sys/types.h>
#include "bsp/CH432T.h"
#include <mkrtos/tty.h>
#include <mkrtos/dev.h>
#include <mkrtos/fs.h>
#include <arch/isr.h>
static int initFlag;
struct tty_struct *my_tty[2]={0};

static void ch432t_read_cb(uint8_t port,uint8_t * data,uint16_t len){
    struct tty_queue *w_tmp;
    struct tty_struct *w_tty;
    switch(port){
        case 0:
            w_tmp=&my_tty[1]->r_queue;
            w_tty=my_tty[1];
            break;
        case 1:
            w_tmp=&my_tty[0]->r_queue;
            w_tty=my_tty[0];
            break;
    }
    for(int i=0;i<len;i++){
        q_add(w_tmp, data[i]);
    }
    wake_up(w_tty->r_wait);
}


static void uart_close(struct tty_struct * tty, struct file * filp){
    my_tty[MINOR(filp->f_rdev)]=0;
    initFlag=0;
}
static int32_t uart_write(struct tty_struct * tty){
    int res;
    uint8_t r;
    int w_len=0;
    while((res=q_get(&tty->w_queue,&r))>=0){
        if(tty->line_no==0){
            CH432Seril1Send((uint8_t *) &r, 1);
        }else {
            CH432Seril0Send((uint8_t *) &r, 1);
        }
        w_len++;
    }
    return w_len;
}

static int32_t  uart_ioctl(struct tty_struct *tty, struct file * file,uint32_t cmd, uint32_t arg){

    return -ENOSYS;
}
extern void EXTI9_5_IRQHandler(void);
int32_t uart_open(struct tty_struct * tty, struct file * filp){
    if (initFlag) { return 0; }
    initFlag = 1;
    Ch432_SPI_Init();
    CH432T_recv_1_data_fun = ch432t_read_cb;
    RegIsrFunc(EXTI9_5_IRQHandler,24,0);
    my_tty[tty->line_no]=tty;
    tty->write=uart_write;
    tty->close=uart_close;
    tty->ioctl=uart_ioctl;
    return 0;
}
