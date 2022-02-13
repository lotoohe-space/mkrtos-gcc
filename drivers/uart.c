#include <arch/isr.h>
#include "bsp/CH432T.h"
#include <string.h>
#include <mkrtos/fs.h>
static uint8_t initFlag=0;


uint8_t fifo[128]={0};


//只要设备的主设备号一样，那么设备的驱动就一样，可以使用同样的驱动去驱动设备，但是子设备号就区分了不同的设备
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
    return -ENOSYS;
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
                  "ttyS0",
                  &uart_fops
    )<0){
        return -1;
    }

    extern int sys_mknod(const char * filename, int mode, dev_t dev);
    if(sys_mknod("/dev/ttyS0",0777|(2<<16),used_dev_no)<0){

    }

    return 0;
}
//删除驱动所执行的操作
static int32_t uart_exit(void) {
    unreg_ch_dev(used_dev_no,"tty0");
    return 0;
}
//DEV_BK_EXPORT(uart_init,uart_exit,tty0);



