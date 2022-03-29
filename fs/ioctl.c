
#include <mkrtos/fs.h>
#include <mkrtos/task.h>

//io¿ØÖÆ
int file_ioctl(int fd, uint32_t cmd, uint32_t arg){
    if(fd>=NR_FILE
       || CUR_TASK->files[fd].used==0
    ){
        return  -EBADF;
    }
    struct file *_file=&(CUR_TASK->files[fd]);

    if(!(_file->f_inode)
    ||!(_file->f_op)
    ||!(_file->f_op->ioctl)
    ){
        return -EINVAL;
    }else{
        int err =CUR_TASK->files[fd].f_op->ioctl(_file->f_inode,_file,cmd,arg);
        if(err<0){
            return err;
        }
    }
    return 0;
}






