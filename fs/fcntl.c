//
// Created by Administrator on 2022/1/18.
//

#include <mkrtos/task.h>
#include <mkrtos/fs.h>

/**
 * dup
 * @param fd
 * @param arg
 * @return
 */
static int dupfd(unsigned int fd, unsigned int arg)
{
    struct file* files;
    files= CUR_TASK->files;
    if (fd >= NR_FILE || files[fd].used==0)
        return -EBADF;
    if (arg >= NR_FILE)
        return -EINVAL;
    /*找到一个可用的fd*/
    for (;arg < NR_FILE;) {
        if (files[arg].used == 0)
            arg++;
        else
            break;
    }
    /*如果超过进程可以打开的最大文件数，则失败*/
    if (arg >= NR_FILE) {
        return -EMFILE;
    }
    files[arg] = files[fd];
    return arg;
}
/**
 *
 * @param oldfd
 * @param newfd
 * @return
 */
int sys_dup2(unsigned int oldfd, unsigned int newfd)
{
    if (oldfd >= NR_FILE || CUR_TASK->files[oldfd].used==0) {
        return -EBADF;
    }
    if (newfd == oldfd) {
        return newfd;
    }
    if (newfd > NR_FILE) {
        return -EBADF;
    }
    if (newfd == NR_FILE) {
        return -EBADF;
    }
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
    if (fd >= NR_FILE || (filp = &(CUR_TASK->files[fd]))->used==0) {
        return -EBADF;
    }

    switch(cmd){
        case F_DUPFD:
            break;
        case F_GETFD:
            break;
        case F_SETFD:
            break;
        case F_GETFL:
            break;
        case F_SETFL:
            break;
        case F_GETOWN:
            break;
        case F_SETOWN:
            break;
        case F_GETLK
    }

    return 0;
}
