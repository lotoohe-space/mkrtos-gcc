//
// Created by Administrator on 2022/3/26.
//

#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <mkrtos/mem.h>

/**
 * 文件系统的等待队列
 */
struct sp_wait_queue{
    struct wait_queue wait;
    uint32_t flag;
    uint32_t mask;
};

//POLLIN           有数据可读
//POLLRDNORM 有普通数据可读，等效与POLLIN
//POLLPRI         有紧迫数据可读
//POLLOUT        写数据不会导致阻塞
//POLLER          指定的文件描述符发生错误
//POLLHUP        指定的文件描述符挂起事件
//POLLNVAL      无效的请求，打不开指定的文件描述符
/**
 * 文件系统的poll
 */
static int sp_poll(struct inode * inode,struct file *fp, uint32_t flag,struct timeval *timeout){
//    int ret=0;

//    if(!timeout) {
        struct sp_wait_queue *wait;
        wait=OSMalloc(sizeof(struct sp_wait_queue));
        if(!wait){
            return -ENOMEM;
        }
        wait->wait.task=CUR_TASK;
        wait->wait.next=NULL;
        wait->mask=0;
        wait->flag=flag;

        add_wait_queue(&inode->poll_wait, &wait);
//        task_suspend();
//        remove_wait_queue(&inode->poll_wait, &wait);
//    }else{
//        extern int sys_nanosleep(const struct timespec *req, struct timespec *rem);
//        struct timespec times;
//        times.tv_sec=timeout->tv_sec;
//        times.tv_nsec=timeout->tv_usec*1000;
//        //休眠一段时间
//        ret=sys_nanosleep(&times,NULL);
//
//    }
    return 0;
}
//int sp_select(struct inode * inode, struct file * fp, int, uint32_t *){
//
//}

