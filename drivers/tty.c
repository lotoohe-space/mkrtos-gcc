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

//读buf长度
#define TTY_READ_BUF_LEN 256
struct tty_line;
struct tty_struct{
    struct termio termio;                   //当前使用的终端信息
    dev_t dev_no;                           //所使用的字符设备的设备号
    //这里是底层的处理函数
    int32_t (*open)(struct tty_struct * tty, struct file * filp);
    void (*close)(struct tty_struct * tty, struct file * filp);
    int32_t (*write)(struct tty_struct * tty,uint8_t *buf,int len);
    /////

    //读取缓存利用环形队列
    uint8_t read_buf[TTY_READ_BUF_LEN];     //读buf长度
    int32_t rear;
    int32_t front;

    int32_t  (*ioctl)(struct tty_struct *tty, struct file * file,uint32_t cmd, uint32_t arg);
    uint8_t is_used;                        //是否使用了
};

//line处理结构体
struct tty_line{
    int32_t (*read)(struct tty_struct * tty,uint8_t * buf,int32_t count);
    int32_t (*write)(struct tty_struct * tty,uint8_t * buf,int32_t count);
};

int32_t tty_def_line_read(struct tty_struct * tty,uint8_t *buf,int32_t count);
int32_t tty_def_line_write(struct tty_struct * tty,uint8_t *buf,int32_t count);


#define TTY_MAX_NUM 6
struct tty_struct ttys[TTY_MAX_NUM]={0};
struct tty_line tty_lines[TTY_MAX_NUM]={0};
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


#define TTY_MAJOR 4
//主次设备号码获取
#define MAJOR(a) (a>>16)
#define MINOR(a) (a&0xffff)


static int tty_open(struct inode * inode, struct file * fp){
	//打开

    return 0;
}
//这里是给vfs的读函数，读取流程是：vfs_read->tty_read->line_read（从buf里面读取）
static int tty_read(struct inode *ino, struct file *fp, char * buf, int count){

    return 0;
}

//写入流程vfs_write->tty_write->line_write->dirver_write
static int tty_write(struct inode *ino, struct file * fp, char * buf, int count){
    int tty_dev_no;
    int ret;
    struct tty_struct *cur_tty;
    struct tty_line *cur_line;
    //检查设备是否打开啊
    if(MAJOR(ino->i_rdev_no)!=TTY_MAJOR){
        return -ENODEV;
    }
    tty_dev_no=MINOR(ino->i_rdev_no);
    if(tty_dev_no>=TTY_MAX_NUM){
        return -ENODEV;
    }
    cur_tty=&ttys[tty_dev_no];
    if(!cur_tty->is_used){
        return -ENODEV;
    }

    cur_line=&tty_lines[tty_dev_no];
    if(cur_line->write){
        ret = cur_line->write(cur_tty,buf,count);
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
/**
 * tty默认的read处理函数
 * @param tty
 * @param bug
 * @param count
 * @return
 */
int32_t tty_def_line_read(struct tty_struct * tty,uint8_t *buf,int32_t count){





    return 0;
}
/**
 * tty默认的read处理函数
 * @param tty
 * @param bug
 * @param count
 * @return
 */
int32_t tty_def_line_write(struct tty_struct * tty,uint8_t *buf,int32_t count){
    int32_t ret;
    ret=tty->write(tty,buf, count);
    return ret;
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