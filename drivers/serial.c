//
// Created by Administrator on 2022/2/11/011.
//
#include <sys/types.h>
#include "bsp/CH432T.h"
#include <mkrtos/tty.h>
#include <mkrtos/dev.h>
#include <mkrtos/fs.h>
#include <arch/isr.h>
#include <errno.h>

static int initFlag;
static struct tty_struct *my_tty[2]={0};
static struct serial_struct serial[2]={0};

static void ch432t_read_cb(uint8_t port,uint8_t * data,uint16_t len){
    struct tty_queue *w_tmp;
    struct tty_struct *w_tty;
    if(port==1) {
        w_tmp = &(my_tty[1]->r_queue);
        w_tty = my_tty[1];
    }else if(port==0){
        w_tmp=&(my_tty[0]->r_queue);
        w_tty=my_tty[0];
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
    struct serial_struct * s_s=(void*)arg;
    uint32_t cflag=s_s->c_cflag;
    int line_no;
    line_no=tty->line_no;
    if(!s_s){
        return -EINVAL;
    }
    if(line_no==0){
        line_no=1;
    }else {
        line_no=0;
    }
    //Ch432_hw_init();
    //ch432t_reset(line_no);
    switch(cmd){
        case TTY_IO_SET:
            if(F_C_BAUD(cflag)) {
                switch (F_C_BAUD(cflag)) {
                    case B0:
                    case B50:
                    case B75:
                    case B110:
                    case B134:
                    case B150:
                    case B200:
                    case B300:
                    case B600:
                    case B1200:
                    case B1800:
                        return -ENOSYS;
                    case B2400:
                        ch432t_set_baud(line_no, 2400);
                        break;
                    case B4800:
                        ch432t_set_baud(line_no, 4800);
                        break;
                    case B9600:
                        ch432t_set_baud(line_no, 9600);
                        break;
                    case B19200:
                        ch432t_set_baud(line_no, 19200);
                        break;
                    case B38400:
                        ch432t_set_baud(line_no, 38400);
                        break;
                    case B57600:
                        ch432t_set_baud(line_no, 57600);
                        break;
//                    case B115200:
                        ch432t_set_baud(line_no, 115200);
                        break;
                    case B230400:
                        ch432t_set_baud(line_no, 230400);
                        break;
                    case B460800:
                        ch432t_set_baud(line_no, 460800);
                        break;
                    case B500000:
                        ch432t_set_baud(line_no, 500000);
                        break;
                    case B576000:
                        ch432t_set_baud(line_no, 576000);
                        break;
                    case B921600:
                        ch432t_set_baud(line_no, 921600);
                        break;
                    case B1000000:
                        ch432t_set_baud(line_no, 1000000);
                        break;
                    case B1152000:
                        ch432t_set_baud(line_no, 1152000);
                        break;
                    case B1500000:
                        ch432t_set_baud(line_no, 1500000);
                        break;
                    case B2000000:
                        ch432t_set_baud(line_no, 2000000);
                        break;
                    case B2500000:
                        ch432t_set_baud(line_no, 2500000);
                        break;
                    case B3000000:
                        ch432t_set_baud(line_no, 3000000);
                        break;
                    case B3500000:
                        ch432t_set_baud(line_no, 3500000);
                        break;
                    case B4000000:
                        ch432t_set_baud(line_no, 4000000);
                        break;
                }
            }
            ch4324_set_par(line_no,F_C_CSIZE(cflag), F_C_CSTOPB(cflag)?1:0, F_C_PARENB(cflag)?1:0);
            ch432t_flow_ctrl(line_no, F_C_CRTSCTS(cflag)?1:0);
//            ch432t_init_intr(line_no);
            serial[line_no].c_cflag=cflag;
            break;
        case TTY_IO_GET:
            s_s->c_cflag=serial[line_no].c_cflag;
            break;

    }
    return -ENOSYS;
}
extern void EXTI9_5_IRQHandler(void);
void console_write(const char* str){
    if (!initFlag) {
        Ch432_SPI_Init();
        RegIsrFunc(EXTI9_5_IRQHandler,24,0);
        CH432T_recv_1_data_fun = ch432t_read_cb;
    }
    CH432Seril1Send(str, strlen(str));
}

int32_t uart_open(struct tty_struct * tty, struct file * filp){
    if (initFlag) { return 0; }
    initFlag = 1;
    Ch432_SPI_Init();
    CH432T_recv_1_data_fun = ch432t_read_cb;
    RegIsrFunc(EXTI9_5_IRQHandler,24,0);
    my_tty[tty->line_no==1?0:1]=tty;
    serial[tty->line_no==1?0:1].c_cflag=tty->termios.c_cflag;

    tty->write=uart_write;
    tty->close=uart_close;
    tty->ioctl=uart_ioctl;
    //ÉèÖÃ²ÎÊý
    uart_ioctl(tty,filp,TTY_IO_SET,&serial[tty->line_no==1?0:1]);
    return 0;
}
