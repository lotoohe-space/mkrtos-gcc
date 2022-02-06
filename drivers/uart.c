#include <arch/isr.h>
#include "bsp/CH432T.h"
#include <string.h>
#include <mkrtos/fs.h>
static uint8_t initFlag=0;


uint8_t fifo[128]={0};

//这里是中断的回调
static void CH432TRecvCB(uint8_t port,uint8_t * data,uint16_t len){
    if(port==1){
//        for(int32_t i=0;i<len;i++){
//            QueueInISR(&ttyFifo, &(data[i]));
//        }
    }
}
/*打开设备*/
static int32_t open(struct inode * inode, struct file * fp) {

    if (initFlag) { return 0; }
    initFlag = 1;
//    QueueCfgPrmtDef def;
//    def.elemSize=1;
//    def.paddr=fifoData;
//    def.queueCnt=sizeof(fifoData);
//    QueueInit(&ttyFifo, &def);

    Ch432_SPI_Init();
    CH432T_recv_1_data_fun = CH432TRecvCB;
    return 0;
}
static int read(struct inode *ino, struct file *fp, char * buf, int count){



    return 0;
}
static int write(struct inode *ino, struct file * fp, char * buf, int count){
    CH432Seril1Send((uint8_t*)buf,count);
    return count;
}
/*关闭设备*/
static void release (struct inode * ino, struct file * f){
    //printf("设备关闭 INodeNum:%d \r\n", pInode->iNodeNum);
}
#include "sys/arm-ioctl.h"
//ioctl
static  int ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg){
//    struct tty_struct * tty;
//    struct tty_struct * other_tty;
//    struct tty_struct * termios_tty;
    pid_t pgrp;
    int dev;
    int termios_dev;
    int retval;

//    if (MAJOR(file->f_rdev) != TTY_MAJOR) {
//        printk("tty_ioctl: tty pseudo-major != TTY_MAJOR\n");
//        return -EINVAL;
//    }
//    dev = MINOR(file->f_rdev);
//    tty = TTY_TABLE(dev);
//    if (!tty)
//        return -EINVAL;
//    if (IS_A_PTY(dev))
//        other_tty = tty_table[PTY_OTHER(dev)];
//    else
//        other_tty = NULL;
//    if (IS_A_PTY_MASTER(dev)) {
//        termios_tty = other_tty;
//        termios_dev = PTY_OTHER(dev);
//    } else {
//        termios_tty = tty;
//        termios_dev = dev;
//    }
    switch (cmd) {
        case TCGETS:
//            retval = verify_area(VERIFY_WRITE, (void *) arg,
//                                 sizeof (struct termios));
//            if (retval)
//                return retval;
            memcpy_tofs((struct termios *) arg,
                        termios_tty->termios,
                        sizeof (struct termios));
            return 0;
        case TCSETSF:
        case TCSETSW:
        case TCSETS:
            retval = check_change(termios_tty, termios_dev);
            if (retval)
                return retval;
            if (cmd == TCSETSF || cmd == TCSETSW) {
                if (cmd == TCSETSF)
                    flush_input(termios_tty);
                wait_until_sent(termios_tty, 0);
            }
            return set_termios(termios_tty, (struct termios *) arg,
                               termios_dev);
        case TCGETA:
            return get_termio(termios_tty,(struct termio *) arg);
        case TCSETAF:
        case TCSETAW:
        case TCSETA:
            retval = check_change(termios_tty, termios_dev);
            if (retval)
                return retval;
            if (cmd == TCSETAF || cmd == TCSETAW) {
                if (cmd == TCSETAF)
                    flush_input(termios_tty);
                wait_until_sent(termios_tty, 0);
            }
            return set_termio(termios_tty, (struct termio *) arg,
                              termios_dev);
        case TCXONC:
            retval = check_change(tty, dev);
            if (retval)
                return retval;
            switch (arg) {
                case TCOOFF:
                    stop_tty(tty);
                    break;
                case TCOON:
                    start_tty(tty);
                    break;
                case TCIOFF:
                    if (STOP_CHAR(tty) != __DISABLED_CHAR)
                        put_tty_queue(STOP_CHAR(tty),
                                      &tty->write_q);
                    break;
                case TCION:
                    if (START_CHAR(tty) != __DISABLED_CHAR)
                        put_tty_queue(START_CHAR(tty),
                                      &tty->write_q);
                    break;
                default:
                    return -EINVAL;
            }
            return 0;
        case TCFLSH:
            retval = check_change(tty, dev);
            if (retval)
                return retval;
            switch (arg) {
                case TCIFLUSH:
                    flush_input(tty);
                    break;
                case TCIOFLUSH:
                    flush_input(tty);
                    /* fall through */
                case TCOFLUSH:
                    flush_output(tty);
                    break;
                default:
                    return -EINVAL;
            }
            return 0;
        case TIOCEXCL:
            set_bit(TTY_EXCLUSIVE, &tty->flags);
            return 0;
        case TIOCNXCL:
            clear_bit(TTY_EXCLUSIVE, &tty->flags);
            return 0;
        case TIOCSCTTY:
            if (current->leader &&
                (current->session == tty->session))
                return 0;
            /*
             * The process must be a session leader and
             * not have a controlling tty already.
             */
            if (!current->leader || (current->tty >= 0))
                return -EPERM;
            if (tty->session > 0) {
                /*
                 * This tty is already the controlling
                 * tty for another session group!
                 */
                if ((arg == 1) && suser()) {
                    /*
                     * Steal it away
                     */
                    struct task_struct *p;

                    for_each_task(p)
                    if (p->tty == dev)
                        p->tty = -1;
                } else
                    return -EPERM;
            }
            current->tty = dev;
            tty->session = current->session;
            tty->pgrp = current->pgrp;
            return 0;
        case TIOCGPGRP:
            retval = verify_area(VERIFY_WRITE, (void *) arg,
                                 sizeof (pid_t));
            if (retval)
                return retval;
            if (current->tty != termios_dev)
                return -ENOTTY;
            put_fs_long(termios_tty->pgrp, (pid_t *) arg);
            return 0;
        case TIOCSPGRP:
            retval = check_change(termios_tty, termios_dev);
            if (retval)
                return retval;
            if ((current->tty < 0) ||
                (current->tty != termios_dev) ||
                (termios_tty->session != current->session))
                return -ENOTTY;
            pgrp = get_fs_long((pid_t *) arg);
            if (pgrp < 0)
                return -EINVAL;
            if (session_of_pgrp(pgrp) != current->session)
                return -EPERM;
            termios_tty->pgrp = pgrp;
            return 0;
        case TIOCOUTQ:

            put_fs_long(CHARS(&tty->write_q),
                        (unsigned long *) arg);
            return 0;
        case TIOCINQ:

            if (L_ICANON(tty))
                put_fs_long(inq_canon(tty),
                            (unsigned long *) arg);
            else
                put_fs_long(CHARS(&tty->secondary),
                            (unsigned long *) arg);
            return 0;
        case TIOCSTI:
            if ((current->tty != dev) && !suser())
                return -EPERM;

            put_tty_queue(get_fs_byte((char *) arg), &tty->read_q);
            TTY_READ_FLUSH(tty);
            return 0;
        case TIOCGWINSZ:

            memcpy_tofs((struct winsize *) arg, &tty->winsize,
                        sizeof (struct winsize));
            return 0;
        case TIOCSWINSZ:
            if (IS_A_PTY_MASTER(dev))
                set_window_size(other_tty,(struct winsize *) arg);
            return set_window_size(tty,(struct winsize *) arg);
        case TIOCLINUX:
            switch (get_fs_byte((char *)arg))
            {
                case 0:
                    return do_screendump(arg);
                case 1:
                    return do_get_ps_info(arg);
#ifdef CONFIG_SELECTION
                    case 2:
					return set_selection(arg);
				case 3:
					return paste_selection(tty);
				case 4:
					unblank_screen();
					return 0;
#endif /* CONFIG_SELECTION */
                default:
                    return -EINVAL;
            }
        case TIOCCONS:
            if (IS_A_CONSOLE(dev)) {
                if (!suser())
                    return -EPERM;
                redirect = NULL;
                return 0;
            }
            if (redirect)
                return -EBUSY;
            if (!suser())
                return -EPERM;
            if (IS_A_PTY_MASTER(dev))
                redirect = other_tty;
            else if (IS_A_PTY_SLAVE(dev))
                redirect = tty;
            else
                return -ENOTTY;
            return 0;
        case FIONBIO:
            arg = get_fs_long((unsigned long *) arg);
            if (arg)
                file->f_flags |= O_NONBLOCK;
            else
                file->f_flags &= ~O_NONBLOCK;
            return 0;
        case TIOCNOTTY:
            if (current->tty != dev)
                return -ENOTTY;
            if (current->leader)
                disassociate_ctty(0);
            current->tty = -1;
            return 0;
        case TIOCGETD:

            put_fs_long(tty->disc, (unsigned long *) arg);
            return 0;
        case TIOCSETD:

            arg = get_fs_long((unsigned long *) arg);
            return tty_set_ldisc(tty, arg);
        case TIOCGLCKTRMIOS:
            arg = get_fs_long((unsigned long *) arg);

            memcpy_tofs((struct termios *) arg,
                        &termios_locked[termios_dev],
                        sizeof (struct termios));
            return 0;
        case TIOCSLCKTRMIOS:
            if (!suser())
                return -EPERM;
            arg = get_fs_long((unsigned long *) arg);
            memcpy_fromfs(&termios_locked[termios_dev],
                          (struct termios *) arg,
                          sizeof (struct termios));
            return 0;
        case TIOCPKT:
            if (!IS_A_PTY_MASTER(dev))
                return -ENOTTY;

            if (get_fs_long(arg)) {
                if (!tty->packet) {
                    tty->packet = 1;
                    tty->link->ctrl_status = 0;
                }
            } else
                tty->packet = 0;
            return 0;
        case TCSBRK: case TCSBRKP:
            retval = check_change(tty, dev);
            if (retval)
                return retval;
            wait_until_sent(tty, 0);
            if (!tty->ioctl)
                return 0;
            tty->ioctl(tty, file, cmd, arg);
            return 0;
        default:
            if (tty->ioctl) {
                retval = (tty->ioctl)(tty, file, cmd, arg);
                if (retval != -EINVAL)
                    return retval;
            }
            if (ldiscs[tty->disc].ioctl) {
                retval = (ldiscs[tty->disc].ioctl)
                        (tty, file, cmd, arg);
                return retval;
            }
            return -EINVAL;
    }

    return 0;
}
void console_write(const char* str){
    if(initFlag==0){
        extern void EXTI9_5_IRQHandler(void);

        RegIsrFunc(EXTI9_5_IRQHandler,23,0);
        Ch432_SPI_Init();
        CH432T_recv_1_data_fun = CH432TRecvCB;
        initFlag=1;
//        return ;
    }
    CH432Seril1Send((uint8_t*)str,strlen(str));
}
//设备注册结构体
static struct file_operations uart_fops={
        .write=write,
        .open=open,
        .release=release,
        .ioctl=ioctl
};
#define TTY_DEV_NO 0
static int32_t used_dev_no=-1;
//初始化驱动
static int32_t uart_init(void) {
    //注册设备到链表
    if(request_char_no(TTY_DEV_NO)<0){
        if((used_dev_no=alloc_bk_no())<0){
            return -1;
        }
    }else{
        used_dev_no=TTY_DEV_NO;
    }
    extern void EXTI9_5_IRQHandler(void);

    RegIsrFunc(EXTI9_5_IRQHandler,23,0);

    if(reg_ch_dev(TTY_DEV_NO,
                  "tty0",
                  &uart_fops
    )<0){
        return -1;
    }

    extern int sys_mknod(const char * filename, int mode, dev_t dev);
    if(sys_mknod("/dev/tty0",0777|(2<<16),used_dev_no)<0){

    }

    return 0;
}
//删除驱动所执行的操作
static int32_t uart_exit(void) {
    unreg_ch_dev(used_dev_no,"tty0");
    return 0;
}
DEV_BK_EXPORT(uart_init,uart_exit,tty0);



