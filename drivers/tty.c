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
struct tty_struct{
    struct termio termio;    //��ǰʹ�õ��ն���Ϣ
    dev_t dev_no;//��ʹ�õ��ַ��豸���豸��
    //�ַ��豸�Ķ�д���ƺ���
    struct file_operations* f_ops;
    uint8_t is_used;//�Ƿ�ʹ����
};


#define TTY_MAX_NUM 6
struct tty_struct ttys[TTY_MAX_NUM]={0};
//��ǰ���ն˺��룬Ĭ��-1
int cur_tty_no=-1;

static struct termio * get_tty(dev_t dev_no){
    int i;
    //�����豸��Ӧ��tty
    for( i=0;i<TTY_MAX_NUM;i++){
        if( ttys[i].dev_no ==dev_no && ttys[i].is_used ){
            break;
        }
    }
    if(i==TTY_MAX_NUM){
        return NULL;
    }
    return &ttys[i];
}
//��ʼ��tty����ָ����tty���豸
int tty_connect(dev_t dev_no,struct file_operations* f_ops){
    if(!f_ops){
        return -1;
    }

    int i;
    for(i=0;i<TTY_MAX_NUM;i++){
        if(ttys[i].is_used==0){
            ttys[i].is_used=1;
            ttys[i].dev_no=dev_no;
            ttys[i].f_ops=f_ops;
        }
    }

    return 0;
}


static int tty_open(struct inode * inode, struct file * fp){
	//��

    return 0;
}
static int tty_read(struct inode *ino, struct file *fp, char * buf, int count){
    return 0;
}
static int tty_write(struct inode *ino, struct file * fp, char * buf, int count){
    return 0;
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
            memcpy(res_term,&cur_tty->termio,sizeof(struct termio));
            break;
        case TCSETS:
            //���ò���
            memcpy(&cur_tty->termio,res_term,sizeof(struct termio));
            break;
    }
    return 0;
}
static void tty_release(struct inode * ino, struct file * f){

}
static struct file_operations tty_ops={
	.open=tty_open,
	.read=tty_read,
    .write=tty_write,
	.ioctl=tty_ioctl,
    .release=tty_release
};

dev_t used_dev_no=-1;
#define TTY_DEV_NO 100
int tty_init(void){
    //ע���豸������
    if(request_char_no(TTY_DEV_NO)<0){
        if((used_dev_no=alloc_bk_no())<0){
            return -1;
        }
    }else{
        used_dev_no=TTY_DEV_NO;
    }

    if(reg_ch_dev(TTY_DEV_NO,
                  "tty0",
                  &tty_ops
    )<0){
        return -1;
    }

    extern int sys_mknod(const char * filename, int mode, dev_t dev);
    if(sys_mknod("/dev/tty0",0777|(2<<16),used_dev_no)<0){

    }

    return 0;
}
int tty_close(void){

	return 0;
}