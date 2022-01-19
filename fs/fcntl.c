//
// Created by Administrator on 2022/1/18.
//

#include <mkrtos/task.h>
#include <mkrtos/fs.h>
static int dupfd(unsigned int fd, unsigned int arg)
{
    if (fd >= NR_FILE || CUR_TASK->files[fd].used==0)
        return -EBADF;
    if (arg >= NR_FILE)
        return -EINVAL;
    /*找到一个可用的fd*/
    for (;arg < NR_FILE;) {
        if (CUR_TASK->files[arg].used == 0)
            arg++;
        else
            break;
    }
    /*如果超过进程可以打开的最大文件数，则失败*/
    if (arg >= NR_FILE) {
        return -EMFILE;
    }
    CUR_TASK->files[arg] = CUR_TASK->files[fd];
    return arg;
}
int sys_dup2(unsigned int oldfd, unsigned int newfd)
{
    if (oldfd >= NR_FILE || CUR_TASK->files[oldfd].used==0)
        return -EBADF;
    if (newfd == oldfd)
        return newfd;
    if (newfd > NR_FILE)
        return -EBADF;
    if (newfd == NR_FILE)
        return -EBADF;
    sys_close(newfd);
    return dupfd(oldfd,newfd);
}
int sys_dup(unsigned int fildes)
{
    return dupfd(fildes,0);
}
int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    struct file * filp;

    if (fd >= NR_FILE || (filp = &(CUR_TASK->files[fd]))->used==0)
        return -EBADF;
    return -ENOSYS;
}
