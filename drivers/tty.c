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
struct tty_struct{
    struct termio termio;    //当前使用的终端信息
    dev_t dev_no;//所使用的字符设备的设备号
    //字符设备的读写控制函数
    struct file_operations* f_ops;
    uint8_t is_used;//是否使用了
};


#define TTY_MAX_NUM 6
struct tty_struct ttys[TTY_MAX_NUM]={0};
//当前的终端号码，默认-1
int cur_tty_no=-1;

static struct termio * get_tty(dev_t dev_no){
    int i;
    //查找设备对应的tty
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
//初始化tty，绑定指定的tty与设备
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
	//打开

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
    //查找设备对应的tty
    cur_tty=get_tty(dev_no);

    if(!cur_tty){
        return -ENOTTY;
    }

    switch(cmd){
        case TCGETS:
            //获取参数
            memcpy(res_term,&cur_tty->termio,sizeof(struct termio));
            break;
        case TCSETS:
            //设置参数
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
    //注册设备到链表
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