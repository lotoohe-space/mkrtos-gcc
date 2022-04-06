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
    struct file* files;
    int new_fd;
    struct file * filp;
    int new_flag;

    files=CUR_TASK->files;
    if (fd >= NR_FILE || (filp = &(files[fd]))->used==0) {
        return -EBADF;
    }

    switch(cmd){
        case F_DUPFD:
            new_fd=dupfd(fd,0);
            files[new_fd].fo_flag&=~FD_CLOEXEC;
            return new_fd;
        case F_GETFD:
            return files[fd].fo_flag;
        case F_SETFD:
            files[fd].fo_flag=arg;
            return 0;
        case F_GETFL:
            return files[fd].f_flags;
        case F_SETFL:
            new_flag=(arg&O_APPEND)|(arg&O_NONBLOCK)|(arg&O_SYNC)|(arg&O_ASYNC);
            files[fd].f_flags=new_flag;
            return 0;
        case F_GETOWN:
            return -ENOSYS;
        case F_SETOWN:
            return -ENOSYS;
        case F_GETLK:
            return -ENOSYS;
        case F_SETLK:
            return -ENOSYS;
        case F_SETLKW:
            return -ENOSYS;
    }

    return 0;
}
