//
// Created by Administrator on 2022/2/6/006.
//


//tty实现
//首先，一个系统只有一个console设备，但是有多个tty设备，这些tty设备可以链接到console设备，这些tty设备又对应了串口设备
//所以顺序是这样的console<->tty<->serise

//系统启动后，我们首先创建一个console设备

#include <sys/types.h>
#include <mkrtos/fs.h>
#include <termios.h>
#include <sys/arm-ioctl.h>
#include <string.h>
#include <mkrtos/tty.h>
#include <mkrtos/dev.h>
#include <ctype.h>
#include <mkrtos/task.h>


int32_t tty_def_line_read(struct tty_struct * tty,struct file* fp,uint8_t *buf,int32_t count);
int32_t tty_def_line_write(struct tty_struct * tty,struct file* fp,uint8_t *buf,int32_t count);


#define TTY_MAX_NUM 3
struct tty_struct ttys[TTY_MAX_NUM]={0};
struct tty_line tty_lines[TTY_MAX_NUM]={0};

//当前的终端号码，默认-1
static struct tty_struct * get_tty(dev_t dev_no){
    struct tty_struct *cur_tty;
    int tty_dev_no;

    tty_dev_no=MINOR(dev_no);
    if(tty_dev_no>=TTY_MAX_NUM){
        return NULL;
    }
    cur_tty=&ttys[tty_dev_no];
    return cur_tty;
}
//初始化termios
static void init_termios(int line, struct termios * tp){
    memset(tp, 0, sizeof(struct termios));
    memcpy(tp->c_cc, C_CC_INIT, NCCS);

//    if (IS_A_CONSOLE(line) || IS_A_PTY_SLAVE(line)) {
//        tp->c_iflag = ICRNL | IXON;
//        tp->c_oflag = OPOST | ONLCR;
//        tp->c_cflag = B38400 | CS8 | CREAD;
//        tp->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
//                      ECHOCTL | ECHOKE | IEXTEN;
//    } else if (IS_A_SERIAL(line)) {
        tp->c_iflag = ICRNL | IXOFF;
        tp->c_oflag = OPOST | ONLCR | XTABS;
        tp->c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;
        tp->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
                      ECHOCTL | ECHOKE | IEXTEN;
//    } else if (IS_A_PTY_MASTER(line))
//        tp->c_cflag = B9600 | CS8 | CREAD;
}
//tty打开
static int tty_open(struct inode * ino, struct file * fp){
	//打开
    int tty_dev_no;
    struct tty_struct *cur_tty;
    //检查设备是否打开啊
    if(MAJOR(ino->i_rdev_no)!=TTY_MAJOR&&MAJOR(ino->i_rdev_no)!=TTYMAUX_MAJOR){
        return -ENODEV;
    }
    if(MAJOR(ino->i_rdev_no)==TTYMAUX_MAJOR){
        fp->f_rdev = MKDEV(TTY_MAJOR,0);
    }
    tty_dev_no = MINOR(ino->i_rdev_no);
    if (tty_dev_no >= TTY_MAX_NUM) {
        return -ENODEV;
    }
    cur_tty=&ttys[tty_dev_no];
    //暂时只支持串口
    //设置open函数
    cur_tty->open=uart_open;

    if(!cur_tty->used_cn) {
        //初始化termio
        init_termios(0,&cur_tty->termios);
        cur_tty->open(cur_tty, fp);
    }
    cur_tty->used_cn++;
    //参数
    cur_tty->line_no=tty_dev_no;
    cur_tty->termios.c_line=tty_dev_no;

//    fp->f_rdev=ino->i_rdev_no;
    return 0;
}
//这里是给vfs的读函数，读取流程是：vfs_read->tty_read->line_read（从buf里面读取）
static int tty_read(struct inode *ino, struct file *fp, char * buf, int count){
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //检查设备是否打开啊
    if(MAJOR(fp->f_rdev)!=TTY_MAJOR){
        return -ENODEV;
    }
    tty_dev_no=MINOR(fp->f_rdev);
    if(tty_dev_no>=TTY_MAX_NUM){
        return -ENODEV;
    }
    cur_tty=&ttys[tty_dev_no];
    if(!cur_tty->used_cn){
        return -ENODEV;
    }

    cur_line=&tty_lines[tty_dev_no];
    if(cur_line->read){
        ret = cur_line->read(cur_tty,fp,buf,count);
    }else{
        return -ENODEV;
    }

    return ret;
}

//写入流程vfs_write->tty_write->line_write->dirver_write
static int tty_write(struct inode *ino, struct file * fp, char * buf, int count){
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //检查设备是否打开啊
    if(MAJOR(fp->f_rdev)!=TTY_MAJOR){
        return -ENODEV;
    }
    tty_dev_no=MINOR(fp->f_rdev);
    if(tty_dev_no>=TTY_MAX_NUM){
        return -ENODEV;
    }
    cur_tty=&ttys[tty_dev_no];
    if(!cur_tty->used_cn){
        return -ENODEV;
    }

    cur_line=&tty_lines[tty_dev_no];
    if(cur_line->write){
        ret = cur_line->write(cur_tty,fp,buf,count);
    }else{
        return -ENODEV;
    }

    return ret;
}
static int tty_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg){
    void *res_term;
    dev_t  dev_no;
    struct tty_struct *cur_tty;
    struct winsize *pw_size;
    struct serial_struct seri;
    dev_no=file->f_rdev;
    res_term=(void*)arg;
    if(!res_term){
        return -EINVAL;
    }
    //查找设备对应的tty
    cur_tty=get_tty(dev_no);

    if(!cur_tty){
        return -ENOTTY;
    }

    switch(cmd){
        case TCGETS:
            if(cur_tty->ioctl){
                if(cur_tty->ioctl(cur_tty,file,TTY_IO_GET,&seri)>=0){
                    cur_tty->termios.c_cflag=seri.c_cflag;
                }
            }
            //获取参数
            memcpy(res_term,&cur_tty->termios,sizeof(struct termio));
            break;
        case TCSETS:
            //设置参数
            memcpy(&cur_tty->termios,res_term,sizeof(struct termio));
            seri.c_cflag=cur_tty->termios.c_cflag;
            if(cur_tty->ioctl){
                cur_tty->ioctl(cur_tty,file,TTY_IO_SET,&seri);
            }
            break;
        case TIOCGWINSZ:
            //TODO:
            cur_tty->w_size.ws_col=80;
            cur_tty->w_size.ws_row=80;
            //获取窗口大小
            pw_size =(struct winszie* )arg;
            *pw_size=cur_tty->w_size;
            break;
        case TIOCSWINSZ:
            //改变窗体大小
            pw_size=(struct winszie* )arg;
            if(pw_size->ws_col!=cur_tty->w_size.ws_col
            ||pw_size->ws_row!=cur_tty->w_size.ws_row
            ){
                inner_set_sig(SIGWINCH);
            }
            cur_tty->w_size=*pw_size;
            break;
        default:
            //暂时不支持的CMD
            break;
    }
    return 0;
}
static void tty_release(struct inode * ino, struct file * fp){
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //检查设备是否打开啊
    if(MAJOR(fp->f_rdev)!=TTY_MAJOR){
        return ;
    }
    tty_dev_no=MINOR(fp->f_rdev);
    if(tty_dev_no>=TTY_MAX_NUM){
        return ;
    }
    cur_tty=&ttys[tty_dev_no];
    if(cur_tty->used_cn==0){
        return ;
    }
    if(cur_tty->used_cn==1){

        cur_tty->close(cur_tty,fp);
        cur_tty->open=NULL;
        memset(&cur_tty->termios,0,sizeof(cur_tty->termios));
    }
    cur_tty->used_cn--;
}


int tty_reg_line(int disc, struct tty_line *new_line)
{
    if (disc < N_TTY || disc >= TTY_MAX_NUM)
        return -EINVAL;

    if (new_line) {
        tty_lines[disc] = *new_line;
//        tty_lines[disc].flags |= LDISC_FLAG_DEFINED;
    } else
        memset(&tty_lines[disc], 0, sizeof(struct tty_line));

    return 0;
}
/**
 * tty默认的read处理函数
 * @param tty
 * @param bug
 * @param count
 * @return
 */
int32_t tty_def_line_read(struct tty_struct * tty,struct file* fp,uint8_t *buf,int32_t count){
    uint8_t r;
    int i;

    again:
    if(!q_length(&tty->r_queue)){//读取数据的长度为零，则休眠等待
        struct wait_queue wait = {CUR_TASK , NULL };
        add_wait_queue(&tty->r_wait, &wait);
        task_suspend();
//        CUR_TASK->status = TASK_SUSPEND;
        task_sche();
        remove_wait_queue(&tty->r_wait, &wait);
        task_run();
//        CUR_TASK->status = TASK_RUNNING;
        goto again;
    }
    if(tty_lines[tty->line_no].handler){
        tty_lines[tty->line_no].handler(tty);
    }
    if(!tty->is_nl){
        ttys[tty->line_no].write(tty);
        //规范模式下没有成一行
        goto again;
    }
    tty->is_nl=0;
//    if(q_length(&tty->pre_queue)<=0){
//        goto again;
//    }
    for(i=0;i<count && q_get(&tty->pre_queue,&r)>=0; i++){
        buf[i]=r;
    }
    //如果有回显的字符，直接在这里写
    ttys[tty->line_no].write(tty);
    return i;
}
static void tty_add_w_queue(struct tty_struct * tty,char r){
    if(O_OPOST(tty)){
        if(O_OLCUC(tty) && islower(r)){
            //小写转大写
            r= toupper(r);
        }
        if(O_ONLCR(tty) && r=='\n'){
            q_add(&tty->w_queue,'\r');
        }
        if(O_OCRNL(tty) && r=='\r'){
            r='\n';
            if(O_ONLRET(tty)){
                tty->col=0;
            }
        }

        if(O_XTABS(tty) && r=='\t'){
            //制表符会被转换成空格符
            r=' ';
        }
    }

    q_add(&tty->w_queue,r);
}
/**
 * tty默认的read处理函数
 * @param tty
 * @param bug
 * @param count
 * @return
 */
int32_t tty_def_line_write(struct tty_struct * tty,struct file* fp,uint8_t *buf,int32_t count){
    int32_t ret;
    int32_t i;
    uint8_t r;
    uint32_t t;
    spin_lock(&tty->w_lock);
    for(i=0;i<count;i++){
        r=buf[i];
        tty_add_w_queue(tty,r);
    }
    spin_unlock(&tty->w_lock);
    //调用写函数
    ret=tty->write(tty);

    return i;
}
/**
 * 删除一个字符
 * @param tty
 * @return
 */
static int erase_c(struct tty_struct *tty){
    char r_tmp;
    //如果最后一个字符是换行符号，则不能在进行擦除了。
    if(q_check_f(&tty->pre_queue,'\n')==0){
        return 0;
    }
    if(q_get_f(&tty->pre_queue,&r_tmp)>=0) {
        //刪除上次写的两个字符
        //如果在标准模式下设定了ECHOE标志，则当收到一个ERASE控制符时将删除前一个显示字符。
        tty_add_w_queue(tty, '\b');
        tty_add_w_queue(tty, ' ');
        tty_add_w_queue(tty, '\b');
        if(tty->col>0) {
            tty->col--;
        }
    }
    return 0;
}
/**
 * 对读取的数据进行处理
 * @param tty
 */
void tty_def_line_handler(struct tty_struct *tty){
    uint8_t r;

    if(L_ICANON(tty)) {
        //能够读到数据
        while (q_get(&tty->r_queue, &r) != -1) {

            if (tty->is_error) {
                if (!I_IGNPAR(tty)) {
                    if (I_PARMRK(tty)) {
                        q_add(&tty->pre_queue, '\377');
                        q_add(&tty->pre_queue, '\0');
                        q_add(&tty->pre_queue, r);
                    } else {
                        q_add(&tty->pre_queue, '\0');
                    }
                }
            } else {
                if (I_ISTRIP(tty)) {//去掉最高位
                    r &= 0x7f;
                }
                if (I_IGNCR(tty)) {//去掉输入的cr
                    if (r == '\r') {
                        continue;
                    }
                }else if (I_INLCR(tty)) {\
                    if (r == '\n') {
                        r = '\r';
                    }
                }
                if (I_ICRNL(tty)&&!I_IGNCR(tty)) {\
                    if (r == '\r') {
                        r = '\n';
                    }
                }

                if (I_IUCLC(tty) && isupper(r)) {//大写转小写
                    r = tolower(r);
                }

                //本地模式的处理
                if (L_ECHO(tty)) {
                    if (L_ECHOCTL(tty) && iscntrl(r)) {
                        if (r != START_C(tty)//显示指定的控制字符
                            && r != STOP_C(tty)
                            && r != '\t'
                            && r != '\n'
                            && (r >= 0 && r <= 37)
                                ) {
                            tty_add_w_queue(tty, '^');
                            tty_add_w_queue(tty, r + 0x40);
                            goto next;
                        }
                    }
                    if (L_ICANON(tty)) {//标准模式
                        if (L_ECHOE(tty) && r == ERASE_C(tty)) {//删除一个字符
                            erase_c(tty);
                            continue;
                        }
                        if (!L_ECHOKE(tty)) {
                            if (L_ECHOK(tty)) {
                                //如果ICANON同时设置，KILL将删除当前行
                                if (r == KILL_C(tty)) {
                                    //删除当前行，还没写
                                    tty_add_w_queue(tty, '\n');
                                    continue;
                                }
                            }
                        } else if (r == KILL_C(tty)) {
                            //刪除这行的每一个字符
                            for (int i = 0; i < tty->col; i++) {
                                erase_c(tty);
                            }
                            continue;
                        }
                    }
                    tty_add_w_queue(tty, r);
                } else {
                    if (L_ECHONL(tty) && L_ICANON(tty)) {
                        //如果在标准模式下设置了该标志，即使没有设置ECHO标志，换行符还是会被显示出来。
                        if (r == '\n') {
                            tty_add_w_queue(tty, r);
                        }
                    }
                }
                next:
//                if(!I_IGNBRK(tty) && I_BRKINT(tty) && r==){
//                    CUR_TASK->signalBMap|=(1<<(SIGINT-1));
//                }

                if (L_ISIG(tty)) {
                    //发送响应的信号
                    if (r == INTR_C(tty)) {
                        //发送给前台进程组的所有进程
                        inner_set_sig(SIGINT);
                        if (!L_NOFLSH(tty)) {
                            q_clear(&tty->w_queue);
                            q_clear(&tty->r_queue);
                        }
                        continue;
                    } else if (r == QUIT_C(tty)) {
                        inner_set_sig(SIGQUIT);
                        if (!L_NOFLSH(tty)) {
                            q_clear(&tty->w_queue);
                            q_clear(&tty->r_queue);
                        }
                        continue;
                    } else if (r == SUSP_C(tty)) {
                        inner_set_sig(SIGTSTP);
                        if (!L_NOFLSH(tty)) {
                            q_clear(&tty->r_queue);
                        }
                        continue;
                    }
                }
                if (L_TOSTOP(tty)) {
                    //个非前台进程组的进程试图向它的控制终端写入数据时，
                    // 信号SIGTTOU会被被发送到这个进程所在的进程组。
                    // 默认情况下，这个信号会使进程停止
                    // ，就像收到SUSP控制符一样。
                }

                if (!iscntrl(r)) {
                    tty->col++;
                } else {
                    if (r == '\n') {
                        tty->col = 0;
                    }
                }
                if (L_ICANON(tty)) {
                    //规范工作方式
                    if (
//                            r == EOF_C(tty)
//                        || r == EOL_C(tty)
//                        || r == EOL2_C(tty)
//                        || r == ERASE_C(tty)
//                        || r == KILL_C(tty)
//                        || r == REPRINT_C(tty)
//                        || r == START_C(tty)
//                        || r == WERASE_C(tty)
                        r=='\n'
                                ||r== EOL_C(tty)
                                ||r== EOL2_C(tty)
                                ||r== EOF_C(tty)
                            ) {
                        if(r!= EOF_C(tty)){
                            if (q_add(&tty->pre_queue, r) < 0) {
                                //添加失败了
                                //这里应该加上等待机制
                            }
                        }
                        tty->is_nl=1;
//                        q_clear(&tty->r_queue);
                        return ;
                    }else{
                        tty->is_nl=0;
                    }
                }
                if (q_add(&tty->pre_queue, r) < 0) {
                    //添加失败了
                    //这里应该加上等待机制
                }
            }
        }
    }else{
        //非规范工作方式
    }
    end:
    //唤醒所有等待的
  //  wake_up(tty->r_wait);
    return ;
}

static struct file_operations tty_ops={
	.open=tty_open,
	.read=tty_read,
    .write=tty_write,
	.ioctl=tty_ioctl,
    .release=tty_release
};

static struct tty_line def_tty_line={
        .write=tty_def_line_write,
        .read=tty_def_line_read,
        .handler=tty_def_line_handler
};
int tty_init(void){
    tty_reg_line(0,&def_tty_line);
    if(reg_ch_dev(TTY_MAJOR,
                  "tty",
                  &tty_ops
    )<0){
        return -1;
    }
    if(reg_ch_dev(TTYMAUX_MAJOR,
                  "tty",
                  &tty_ops
    )<0){
        return -1;
    }

    return 0;
}
int tty_close(void){
    tty_reg_line(0,0);
    unreg_ch_dev(TTY_MAJOR,"tty");
    unreg_ch_dev(TTYMAUX_MAJOR,"tty");
	return 0;
}

DEV_BK_EXPORT(tty_init,tty_close,tty);
