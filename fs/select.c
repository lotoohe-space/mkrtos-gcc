//
// Created by Administrator on 2022/3/26.
//
#include <sys/select.h>
#include <errno.h>
#include <mkrtos/task.h>
#include <sys/poll.h>

static int get_bit(unsigned long bits[],int inx){
    return (bits[inx/NFDBITS]>>(inx%NFDBITS))&0x1L;
}
//具体解释select的参数：
//（1）intmaxfdp是一个整数值，是指集合中所有文件描述符的范围，即所有文件描述符的最大值加1，不能错。
//说明：对于这个原理的解释可以看上边fd_set的详细解释，fd_set是以位图的形式来存储这些文件描述符。maxfdp也就是定义了位图中有效的位的个数。
//（2）fd_set*readfds是指向fd_set结构的指针，这个集合中应该包括文件描述符，我们是要监视这些文件描述符的读变化的，即我们关心是否可以从这些文件中读取数据了
// ，如果这个集合中有一个文件可读，select就会返回一个大于0的值，表示有文件可读；如果没有可读的文件，则根据timeout参数再判断是否超时，若超出timeout的时间
// ，select返回0，若发生错误返回负值。可以传入NULL值，表示不关心任何文件的读变化。
//（3）fd_set*writefds是指向fd_set结构的指针，这个集合中应该包括文件描述符，我们是要监视这些文件描述符的写变化的，即我们关心是否可以向这些文件中写入数据了
// ，如果这个集合中有一个文件可写，select就会返回一个大于0的值，表示有文件可写，如果没有可写的文件，则根据timeout参数再判断是否超时
// ，若超出timeout的时间，select返回0，若发生错误返回负值。可以传入NULL值，表示不关心任何文件的写变化。
//（4）fd_set*errorfds同上面两个参数的意图，用来监视文件错误异常文件。
//（5）structtimeval* timeout是select的超时时间，这个参数至关重要，它可以使select处于三种状态
// ，第一，若将NULL以形参传入，即不传入时间结构，就是将select置于阻塞状态，一定等到监视文件描述符集合中某个文件描述符发生变化为止；
//  第二，若将时间值设为0秒0毫秒，就变成一个纯粹的非阻塞函数，不管文件描述符是否有变化，都立刻返回继续执行，文件无变化返回0，有变化返回一个正值；第三，timeout的值大于0，这就是等待的超时时间，即 select在timeout时间内阻塞，超时时间之内有事件到来就返回了，否则在超时后不管怎样一定返回，返回值同上述。
//说明：
//函数返回：
//（1）当监视的相应的文件描述符集中满足条件时，比如说读文件描述符集中有数据到来时，内核(I/O)根据状态修改文件描述符集，并返回一个大于0的数。
//（2）当没有满足条件的文件描述符，且设置的timeval监控时间超时时，select函数会返回一个为0的值。
//（3）当select返回负值时，发生错误。
int file_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,struct timeval *timeout){
    int ret;
    //flag表
    uint32_t  flag_tb[FD_SETSIZE];
    if(!readset && !writeset){
        return -EINVAL;
    }
    struct file *tk;
    tk=CUR_TASK->files;
    int i;
    if(readset) {
        for (i = 0; i < maxfdp1; i++) {
            int bit;
            bit=get_bit(readset,i);
            if(!bit){
                continue;
            }
            if(!tk[i].used){
                continue;
            }
            if(!(tk[i].f_op
            &&tk[i].f_inode
            &&tk[i].f_op->poll
            )){
                continue;
            }

            //3个标志同时监听
            flag_tb[i]|=POLLIN;
            flag_tb[i]|=POLLRDNORM;
            flag_tb[i]|=POLLPRI;
            flag_tb[i]|=POLLERR;
        }
    }
    if(writeset){
        for(i=0;i<maxfdp1;i++){
            int bit;
            bit=get_bit(writeset,i);
            if(!bit){
                continue;
            }
            if(!tk[i].used){
                continue;
            }
            if(!(tk[i].f_op
                 &&tk[i].f_inode
                 &&tk[i].f_op->poll
            )){
                continue;
            }
            flag_tb[i]|=POLLOUT;
            flag_tb[i]|=POLLERR;
        }
    }
    //检查有没有事件发生
    //TODO:

    for(int i=0;i<FD_SETSIZE;i++) {
        if (timeout||(timeout->tv_usec == 0 && timeout->tv_sec == 0)) {
            //休眠
            ret = tk[i].f_op->poll(tk[i].f_inode, tk + i, flag_tb[i], timeout);
            if (ret < 0) {
                return ret;
            }
        } else{
            //不休眠，则直接检查
        }
    }
    return 0;
}

