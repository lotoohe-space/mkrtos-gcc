//
// Created by Administrator on 2022/1/10.
//
#include <type.h>
#include <errno.h>
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <fcntl.h>
int sys_ustat(int dev, struct ustat * ubuf){
    return -ENOSYS;
}
int sys_statfs(const char * path, struct statfs * buf){
    struct inode * inode;
    inode=_open_namei(path);
    if(inode==NULL){
        return -1;
    }
    if(!inode->i_sb->s_ops->statfs){
        //释放这个inode
        puti(inode);
        return -ENOSYS;
    }

    inode->i_sb->s_ops->statfs(inode->i_sb,buf);
    puti(inode);
    return 0;
}
int sys_fstatfs(unsigned int fd, struct statfs * buf){
    if(fd>=NR_FILE || CUR_TASK->files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(CUR_TASK->files[fd].f_inode)){
        return -ENOENT;
    }
    if((CUR_TASK->files[fd].f_inode->i_sb->s_ops->statfs)){
        CUR_TASK->files[fd].f_inode->i_sb->s_ops->statfs(CUR_TASK->files[fd].f_inode,buf);
    }else{
        return -ENOSYS;
    }
    return 0;
}
int sys_truncate(const char * path, unsigned int length){
    return -ENOSYS;
}
int sys_ftruncate(unsigned int fd, unsigned int length){
    return -ENOSYS;
}
int sys_utime(char * filename, struct utimbuf * times){
    return -ENOSYS;
}
int sys_access(const char * filename,int mode){
    return -ENOSYS;
}
//进入某个目录
int sys_chdir(const char * filename){
    struct inode *o_inode;
    o_inode=_open_namei(filename);
    if(o_inode==NULL){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(o_inode->i_type_mode)){
        puti(o_inode);
        return -ENOENT;
    }
    //释放掉之前的
    puti(CUR_TASK->pwd_inode);
    //设置新的
    CUR_TASK->pwd_inode=o_inode;
    return 0;
}
//感觉这个没有必要存在，根据fd进入某个目录
int sys_fchdir(unsigned int fd){
    struct inode *o_inode;
    if(fd>=NR_FILE || CUR_TASK->files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(CUR_TASK->files[fd].f_inode)){
        return -ENOENT;
    }
    o_inode =CUR_TASK->files[fd].f_inode;
    if(o_inode==NULL){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(o_inode->i_type_mode)){
        //这里不要释放f_inode，因为可能还会使用
        return -ENOENT;
    }
    //释放掉之前的
    puti(CUR_TASK->pwd_inode);
    //设置新的
    CUR_TASK->pwd_inode=o_inode;
    return 0;
}
//更改根目录
int sys_chroot(const char * filename){
    struct inode *o_inode;
    o_inode=_open_namei(filename);
    if(o_inode==NULL){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(o_inode->i_type_mode)){
        puti(o_inode);
        return -ENOENT;
    }

    //释放掉之前的
    puti(CUR_TASK->root_inode);
    //设置新的
    CUR_TASK->root_inode=o_inode;
    return 0;
}
//更改文件的权限
int sys_fchmod(unsigned int fd, mode_t mode){
    struct inode *o_inode;
    if(fd>=NR_FILE || CUR_TASK->files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(CUR_TASK->files[fd].f_inode)){
        return -ENOENT;
    }

    if(CUR_TASK->files[fd].f_flags&O_RDONLY){
        return -EROFS;
    }
    //需要检查权限，还没做

    o_inode=CUR_TASK->files[fd].f_inode;
    o_inode->i_type_mode&=0xffff0000;
    o_inode->i_type_mode|=mode;

    o_inode->i_dirt=1;
    return 0;
}
//改变指定文件的权限
int sys_chmod(const char * filename, mode_t mode){
    struct inode *o_inode;
    o_inode=_open_namei(filename);
    if(o_inode==NULL){
        return -ENOENT;
    }

    //检查权限
    //特别是检查当前用户有没有权限编辑这个文件

    o_inode->i_type_mode&=0xffff0000;
    o_inode->i_type_mode|=mode;
    o_inode->i_dirt=1;
    return 0;
}
int sys_fchown(unsigned int fd, int user, int group){
    return -ENOSYS;
}
int sys_chown(const char * filename, int user, int group){
    return -ENOSYS;
}
/**NR_FILE
 * 打开文件
 * @param path
 * @param flags
 * @param mode
 * @return
 */
int32_t sys_open(const char* path,int32_t flags,int32_t mode){
    uint32_t i;
    struct inode *o_inode;
    for(i=0;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used==0){
            CUR_TASK->files[i].used=1;
            break;
        }
    }
    if(i>=NR_FILE){
        errno = EMFILE;
        return -1;
    }
    //打开文件
    o_inode = open_namei(path,flags,mode);
    if(o_inode==NULL){
        CUR_TASK->files[i].used=0;
        return -1;
    }
    CUR_TASK->files[i].f_flags=flags;
    //设置文件操作符号
    if((!CUR_TASK->files[i].f_op)
        &&  o_inode->i_ops->default_file_ops
    ) {
        CUR_TASK->files[i].f_op = o_inode->i_ops->default_file_ops;
    }

    //调用打开函数
    if(
        CUR_TASK->files[i].f_op
        &&CUR_TASK->files[i].f_op->open
        ) {
        if (CUR_TASK->files[i].f_op->open(o_inode, CUR_TASK->files[i].f_op) < 0) {
            CUR_TASK->files[i].used = 0;
            puti(o_inode);
            return -1;
        }

    }
    CUR_TASK->files[i].f_inode=o_inode;
    CUR_TASK->files[i].f_mode= FILE_MODE((o_inode->i_type_mode));

    return i;
}

//创建文件
int sys_creat(const char * pathname, int mode){
    return sys_open(pathname,O_CREAT|O_WRONLY,mode);
}
//关闭文件
void sys_close(int fp){
    struct inode *inode;

    if(CUR_TASK->files[fp].used==0){
        //文件已经关闭了
        return ;
    }
    inode=CUR_TASK->files[fp].f_inode;

    if(CUR_TASK->files[fp].f_op
    &&CUR_TASK->files[fp].f_op->release
    ){
        CUR_TASK->files[fp].f_op->release(inode,&CUR_TASK->files[fp]);
    }
    CUR_TASK->files[fp].used=0;

    CUR_TASK->files[fp].f_inode=NULL;
    puti(inode);
}
int sys_vhangup(void){
    return -ENOSYS;
}
