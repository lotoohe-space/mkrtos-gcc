//
// Created by Administrator on 2022/2/6/006.
//


//ttyʵ��
//���ȣ�һ��ϵͳֻ��һ��console�豸�������ж��tty�豸����Щtty�豸�������ӵ�console�豸����Щtty�豸�ֶ�Ӧ�˴����豸
//����˳����������console<->tty<->serise

//ϵͳ�������������ȴ���һ��console�豸

#include <sys/types.h>
#include <mkrtos/fs.h>
#include <termios.h>
#include <sys/arm-ioctl.h>
#include <string.h>
#include <mkrtos/tty.h>
#include <mkrtos/dev.h>

int32_t tty_def_line_read(struct tty_struct * tty,uint8_t *buf,int32_t count);
int32_t tty_def_line_write(struct tty_struct * tty,uint8_t *buf,int32_t count);

#define TTY_MAX_NUM 6
struct tty_struct ttys[TTY_MAX_NUM]={0};
struct tty_line tty_lines[TTY_MAX_NUM]={0};
//��ǰ���ն˺��룬Ĭ��-1
int cur_tty_no=-1;

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

static void init_termios(int line, struct termios * tp)
{
    memset(tp, 0, sizeof(struct termios));
    memcpy(tp->c_cc, C_CC_INIT, NCCS);
//    if (IS_A_CONSOLE(line) || IS_A_PTY_SLAVE(line)) {
//        tp->c_iflag = ICRNL | IXON;
//        tp->c_oflag = OPOST | ONLCR;
//        tp->c_cflag = B38400 | CS8 | CREAD;
//        tp->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
//                      ECHOCTL | ECHOKE | IEXTEN;
//    } else if (IS_A_SERIAL(line)) {
        tp->c_iflag = ICRNL | IXON;
        tp->c_oflag = OPOST | ONLCR | XTABS;
        tp->c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
        tp->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
                      ECHOCTL | ECHOKE | IEXTEN;
//    } else if (IS_A_PTY_MASTER(line))
//        tp->c_cflag = B9600 | CS8 | CREAD;
}



static int tty_open(struct inode * ino, struct file * fp){
	//��
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //����豸�Ƿ�򿪰�
    if(MAJOR(ino->i_rdev_no)!=TTY_MAJOR&&MAJOR(ino->i_rdev_no)!=TTYMAUX_MAJOR){
        return -ENODEV;
    }
    tty_dev_no=MINOR(ino->i_rdev_no);
    if(tty_dev_no>=TTY_MAX_NUM){
        return -ENODEV;
    }
    cur_tty=&ttys[tty_dev_no];
    //��ʱֻ֧�ִ���
    //����open����
    cur_tty->open=uart_open;
    //��ʼ��termio
    init_termios(0,&cur_tty->termios);
    cur_tty->used_cn++;
    cur_tty->open(cur_tty,fp);
    return 0;
}
//�����Ǹ�vfs�Ķ���������ȡ�����ǣ�vfs_read->tty_read->line_read����buf�����ȡ��
static int tty_read(struct inode *ino, struct file *fp, char * buf, int count){

    return 0;
}

//д������vfs_write->tty_write->line_write->dirver_write
static int tty_write(struct inode *ino, struct file * fp, char * buf, int count){
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //����豸�Ƿ�򿪰�
    if(MAJOR(ino->i_rdev_no)!=TTY_MAJOR&&MAJOR(ino->i_rdev_no)!=TTYMAUX_MAJOR){
        return -ENODEV;
    }
    tty_dev_no=MINOR(ino->i_rdev_no);
    if(tty_dev_no>=TTY_MAX_NUM){
        return -ENODEV;
    }
    cur_tty=&ttys[tty_dev_no];
    if(!cur_tty->used_cn){
        return -ENODEV;
    }

    cur_line=&tty_lines[tty_dev_no];
    if(cur_line->write){
        ret = cur_line->write(cur_tty,buf,count);
    }else{
        return -ENODEV;
    }

    return ret;
}
static int tty_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg){
    void *res_term;
    dev_t  dev_no;
    struct tty_struct *cur_tty;
    int i;
    dev_no=file->f_rdev;
    res_term=(void*)arg;
    if(!res_term){
        return -EINVAL;
    }
    //�����豸��Ӧ��tty
    cur_tty=get_tty(dev_no);

    if(!cur_tty){
        return -ENOTTY;
    }

    switch(cmd){
        case TCGETS:
            //��ȡ����
            memcpy(res_term,&cur_tty->termios,sizeof(struct termio));
            break;
        case TCSETS:
            //���ò���
            memcpy(&cur_tty->termios,res_term,sizeof(struct termio));
            break;
    }
    return 0;
}
static void tty_release(struct inode * ino, struct file * fp){
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //����豸�Ƿ�򿪰�
    if(MAJOR(ino->i_rdev_no)!=TTY_MAJOR){
        return ;
    }
    tty_dev_no=MINOR(ino->i_rdev_no);
    if(tty_dev_no>=TTY_MAX_NUM){
        return ;
    }
    cur_tty=&ttys[tty_dev_no];
    if(cur_tty->used_cn==0){
        return ;
    }
    if(cur_tty->used_cn==1){

    }
    cur_tty->used_cn--;
    cur_tty->close(cur_tty,fp);
    cur_tty->open=NULL;
    memset(&cur_tty->termios,0,sizeof(cur_tty->termios));
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
 * ttyĬ�ϵ�read������
 * @param tty
 * @param bug
 * @param count
 * @return
 */
int32_t tty_def_line_read(struct tty_struct * tty,uint8_t *buf,int32_t count){
    uint8_t r;
    int i;
    for(i=0;i<count && q_get(&tty->pre_queue,&r)>=0; i++){
        buf[i]=r;
    }
    return i;
}
/**
 * ttyĬ�ϵ�read������
 * @param tty
 * @param bug
 * @param count
 * @return
 */
int32_t tty_def_line_write(struct tty_struct * tty,uint8_t *buf,int32_t count){
    int32_t ret;
    //����д����
    ret=tty->write(tty,buf, count);
    return ret;
}
/**
 * �Զ�ȡ�����ݽ��д���
 * @param tty
 */
void tty_def_line_handler(struct tty_struct *tty){
    uint8_t r;

    //�ܹ���������
    while(q_get(&tty->r_queue,&r)!=-1) {
        if (L_ECHO(tty)) {
            //����
            tty->write(tty,&r,1);
        }
        if(q_add(&tty->pre_queue,r)<0){
            //��ȡʧ����
            //����Ӧ�ü��ϵȴ�����
        }
    }
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
