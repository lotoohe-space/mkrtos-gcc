//
// Created by Administrator on 2022/2/11/011.
//

#ifndef UNTITLED1_TTY_H
#define UNTITLED1_TTY_H
#include <sys/types.h>
#include <termios.h>
#include <mkrtos/fs.h>
#include "mkrtos/task.h"
//buf长度
#define TTY_READ_BUF_LEN 512

struct tty_queue{
    //读取缓存利用环形队列
    uint8_t read_buf[TTY_READ_BUF_LEN];     //读buf长度
    int32_t rear;
    int32_t front;
};

struct tty_struct{
    struct termios termios;//当前使用的终端信息
    struct winsize w_size;
    int line_no;//所使用的line号码

    //这里是底层的处理函数
    int32_t (*open)(struct tty_struct * tty, struct file * filp);
    void (*close)(struct tty_struct * tty, struct file * filp);
    int32_t (*write)(struct tty_struct * tty);
    int32_t  (*ioctl)(struct tty_struct *tty, struct file * file,uint32_t cmd, uint32_t arg);
    /////
    //读等待
    struct wait_queue *r_wait;
    //最底层的数据首先读取到这里
    struct tty_queue r_queue;
    //写数据的缓存
    struct tty_queue w_queue;
    //然后通过handler处理机制存放到per_queue中，pre_queue中的数据直接可以给用户，或者进行回显
    struct tty_queue pre_queue;

    spinlock_handler w_lock;

    //有多少列
    int col;
    //有多少行
    int row;
    //字符错误
    uint8_t is_error;

    //是否成行了
    uint8_t is_nl;
    //
//    uint8_t print_ctl;
    //是否使用了
    uint8_t used_cn;
};

/*	intr=^C		quit=^|		erase=del	kill=^U
	eof=^D		vtime=\0	vmin=\1		sxtc=\0
	start=^Q	stop=^S		susp=^Z		eol=\0
	reprint=^R	discard=^U	werase=^W	lnext=^V
	eol2=\0
*/
#define C_CC_INIT "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"

//line处理结构体
struct tty_line{
    //读取函数
    int32_t (*read)(struct tty_struct * tty,struct file* fp,uint8_t * buf,int32_t count);
    //写函数
    int32_t (*write)(struct tty_struct * tty,struct file* fp,uint8_t * buf,int32_t count);
    //数据处理函数
    void (*handler)(struct tty_struct *tty);
};

#define INTR_C(tty) ((tty)->termios.c_cc[VINTR])
#define QUIT_C(tty) ((tty)->termios.c_cc[VQUIT])
#define ERASE_C(tty) ((tty)->termios.c_cc[VERASE])
#define KILL_C(tty) ((tty)->termios.c_cc[VKILL])
#define EOF_C(tty) ((tty)->termios.c_cc[VEOF])
#define TIME_C(tty) ((tty)->termios.c_cc[VTIME])
#define MIN_C(tty) ((tty)->termios.c_cc[VMIN])
#define SWTC_C(tty) ((tty)->termios.c_cc[VSWTC])
#define START_C(tty) ((tty)->termios.c_cc[VSTART])
#define STOP_C(tty) ((tty)->termios.c_cc[VSTOP])
#define SUSP_C(tty) ((tty)->termios.c_cc[VSUSP])
#define EOL_C(tty) ((tty)->termios.c_cc[VEOL])
#define REPRINT_C(tty) ((tty)->termios.c_cc[VREPRINT])
#define DISCARD_C(tty) ((tty)->termios.c_cc[VDISCARD])
#define WERASE_C(tty) ((tty)->termios.c_cc[VWERASE])
#define LNEXT_C(tty)	((tty)->termios.c_cc[VLNEXT])
#define EOL2_C(tty) ((tty)->termios.c_cc[VEOL2])

#define _I_FLAG(tty,f)	((tty)->termios.c_iflag & (f))
#define _O_FLAG(tty,f)	((tty)->termios.c_oflag & (f))
#define _C_FLAG(tty,f)	((tty)->termios.c_cflag & (f))
#define _L_FLAG(tty,f)	((tty)->termios.c_lflag & (f))

#define I_IGNBRK(tty)	_I_FLAG((tty),IGNBRK)
#define I_BRKINT(tty)	_I_FLAG((tty),BRKINT)
#define I_IGNPAR(tty)	_I_FLAG((tty),IGNPAR)
#define I_PARMRK(tty)	_I_FLAG((tty),PARMRK)
#define I_INPCK(tty)	_I_FLAG((tty),INPCK)
#define I_ISTRIP(tty)	_I_FLAG((tty),ISTRIP)
#define I_INLCR(tty)	_I_FLAG((tty),INLCR)
#define I_IGNCR(tty)	_I_FLAG((tty),IGNCR)
#define I_ICRNL(tty)	_I_FLAG((tty),ICRNL)
#define I_IUCLC(tty)	_I_FLAG((tty),IUCLC)
#define I_IXON(tty)	_I_FLAG((tty),IXON)
#define I_IXANY(tty)	_I_FLAG((tty),IXANY)
#define I_IXOFF(tty)	_I_FLAG((tty),IXOFF)
#define I_IMAXBEL(tty)	_I_FLAG((tty),IMAXBEL)

#define O_OPOST(tty)	_O_FLAG((tty),OPOST)
#define O_OLCUC(tty)	_O_FLAG((tty),OLCUC)
#define O_ONLCR(tty)	_O_FLAG((tty),ONLCR)
#define O_OCRNL(tty)	_O_FLAG((tty),OCRNL)
#define O_ONOCR(tty)	_O_FLAG((tty),ONOCR)
#define O_ONLRET(tty)	_O_FLAG((tty),ONLRET)
#define O_OFILL(tty)	_O_FLAG((tty),OFILL)
#define O_OFDEL(tty)	_O_FLAG((tty),OFDEL)
#define O_NLDLY(tty)	_O_FLAG((tty),NLDLY)
#define O_CRDLY(tty)	_O_FLAG((tty),CRDLY)
#define O_TABDLY(tty)	_O_FLAG((tty),TABDLY)
#define O_BSDLY(tty)	_O_FLAG((tty),BSDLY)
#define O_VTDLY(tty)	_O_FLAG((tty),VTDLY)
#define O_FFDLY(tty)	_O_FLAG((tty),FFDLY)
#define O_XTABS(tty)    _O_FLAG((tty),XTABS)


#define __C_FLAG(flag,f)	((flag) & (f))

#define C_BAUD(tty)	    _C_FLAG((tty),CBAUD)
#define C_CSIZE(tty)	_C_FLAG((tty),CSIZE)
#define C_CSTOPB(tty)	_C_FLAG((tty),CSTOPB)
#define C_CREAD(tty)	_C_FLAG((tty),CREAD)
#define C_PARENB(tty)	_C_FLAG((tty),PARENB)
#define C_PARODD(tty)	_C_FLAG((tty),PARODD)
#define C_HUPCL(tty)	_C_FLAG((tty),HUPCL)
#define C_CLOCAL(tty)	_C_FLAG((tty),CLOCAL)
#define C_CIBAUD(tty)	_C_FLAG((tty),CIBAUD)
#define C_CRTSCTS(tty)	_C_FLAG((tty),CRTSCTS)
#define C_BAUDEX(tty)   _C_FLAG((tty),CBAUDEX)

#define F_C_BAUD(tty)	    __C_FLAG((tty),CBAUD)
#define F_C_CSIZE(tty)	__C_FLAG((tty),CSIZE)
#define F_C_CSTOPB(tty)	__C_FLAG((tty),CSTOPB)
#define F_C_CREAD(tty)	__C_FLAG((tty),CREAD)
#define F_C_PARENB(tty)	__C_FLAG((tty),PARENB)
#define F_C_PARODD(tty)	__C_FLAG((tty),PARODD)
#define F_C_HUPCL(tty)	__C_FLAG((tty),HUPCL)
#define F_C_CLOCAL(tty)	__C_FLAG((tty),CLOCAL)
#define F_C_CIBAUD(tty)	__C_FLAG((tty),CIBAUD)
#define F_C_CRTSCTS(tty)	__C_FLAG((tty),CRTSCTS)
#define F_C_BAUDEX(tty)   __C_FLAG((tty),CBAUDEX)

#define L_ISIG(tty)	    _L_FLAG((tty),ISIG)
#define L_ICANON(tty)	_L_FLAG((tty),ICANON)
#define L_XCASE(tty)	_L_FLAG((tty),XCASE)
#define L_ECHO(tty)	    _L_FLAG((tty),ECHO)
#define L_ECHOE(tty)	_L_FLAG((tty),ECHOE)
#define L_ECHOK(tty)	_L_FLAG((tty),ECHOK)
#define L_ECHONL(tty)	_L_FLAG((tty),ECHONL)
#define L_NOFLSH(tty)	_L_FLAG((tty),NOFLSH)
#define L_TOSTOP(tty)	_L_FLAG((tty),TOSTOP)
#define L_ECHOCTL(tty)	_L_FLAG((tty),ECHOCTL)
#define L_ECHOPRT(tty)	_L_FLAG((tty),ECHOPRT)
#define L_ECHOKE(tty)	_L_FLAG((tty),ECHOKE)
#define L_FLUSHO(tty)	_L_FLAG((tty),FLUSHO)
#define L_PENDIN(tty)	_L_FLAG((tty),PENDIN)
#define L_IEXTEN(tty)	_L_FLAG((tty),IEXTEN)

void q_clear(struct tty_queue *t_queue);
inline int32_t q_length(struct tty_queue* t_queue){
    return (t_queue->rear-t_queue->front+TTY_READ_BUF_LEN)%TTY_READ_BUF_LEN;
}
int32_t q_add(struct tty_queue *t_queue,uint8_t d);
int32_t q_get(struct  tty_queue *t_queue,uint8_t *d);
int32_t q_get_f(struct tty_queue *t_queue,uint8_t *d);
int32_t q_check_f(struct tty_queue *t_queue,uint8_t c);
int32_t uart_open(struct tty_struct * tty, struct file * filp);

//serail.c
#define TTY_IO_SET 0x0001
#define TTY_IO_GET 0x0002
struct serial_struct{
    //串口的控制，与c_cflag一致
    uint32_t c_cflag;
};

#endif //UNTITLED1_TTY_H
