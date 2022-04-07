//
// Created by Administrator on 2022/1/10.
//
#include <type.h>
#include <errno.h>
#include <mkrtos/stat.h>
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <fcntl.h>
#include <mkrtos/debug.h>
int sys_ustat(int dev, struct ustat * ubuf){
    return -ENOSYS;
}
/**
 * 获取文件系统状态
 * @param path
 * @param buf
 * @return
 */
int sys_statfs(const char * path, struct statfs * buf){
    struct inode * inode;
    int32_t res;
    res=namei(path,&inode);
    if(res<0){
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
    struct file *files;
    files=CUR_TASK->files;
    if(fd>=NR_FILE || files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(files[fd].f_inode)){
        return -ENOENT;
    }
    if((files[fd].f_inode->i_sb->s_ops->statfs)){
        files[fd].f_inode->i_sb->s_ops->statfs(files[fd].f_inode->i_sb,buf);
    }else{
        return -ENOSYS;
    }
    return 0;
}
int sys_truncate(const char * path, unsigned int length){
    struct inode * inode;
    int error;

    error = namei(path,&inode);
    if (error) {
        return error;
    }
    if (!S_ISREG(inode->i_type_mode)
  //  || !permission(inode,MAY_WRITE)
    ) {
        puti(inode);
        return -EACCES;
    }
//    if (IS_RDONLY(inode)) {
//        iput(inode);
//        return -EROFS;
//    }
    if (inode->i_ops && inode->i_ops->truncate) {
        inode->i_ops->truncate(inode,length);
    }
//    inode->i_ctime = inode->i_mtime = CURRENT_TIME;
    inode->i_dirt = 1;
//    error = notify_change(NOTIFY_SIZE, inode);
    puti(inode);

    DEBUG("fs",INFO,"截断文件：%s",path);

    return error;
}
int sys_ftruncate(unsigned int fd, unsigned int length){
    struct inode *inode;
    struct file *files;
    files=CUR_TASK->files;
    if(fd>=NR_FILE || files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(files[fd].f_inode)){
        return -ENOENT;
    }
    inode=&files[fd].f_inode;
    if (inode->i_ops && inode->i_ops->truncate) {
        inode->i_ops->truncate(inode,length);
    }
    inode->i_dirt = 1;
    puti(inode);
    return -ENOSYS;
}
int sys_utime(char * filename, struct utimbuf * times){
    return -ENOSYS;
}
int sys_access(const char * filename,int mode){
    return -ENOSYS;
}
/**
 * 进入某个目录
 * @param filename
 * @return
 */
int sys_chdir(const char * filename){
    struct inode *o_inode;
    const char* file_name;
    int32_t f_len;
    int32_t res;
    res=dir_namei(filename,&f_len,&file_name,NULL,&o_inode);
    if(res<0){
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
    puti(o_inode);
//
//    if(filename[0]=='.'&&filename[1]=='.'){
//        //相对路径
//    }else if(filename[0]=='.'){
//
//    }else{
        strncpy(CUR_TASK->pwd_path,filename,sizeof(CUR_TASK->pwd_path));
//    }

    return 0;
}
/**
 * 获取当前工作目录
 * @param source
 * @param cn
 * @return
 */
int sys_getcwd(char* source,int cn){
    strncpy(source,CUR_TASK->pwd_path,cn);
    return strlen(CUR_TASK->pwd_path);
}
/**
 * 进入某个目录
 * @param fd
 * @return
 */
int sys_fchdir(unsigned int fd){
    struct inode *o_inode;
    struct file *files;
    files=CUR_TASK->files;
    if(fd>=NR_FILE || files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(files[fd].f_inode)){
        return -ENOENT;
    }
    o_inode =files[fd].f_inode;
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
    char* file_name;
    int32_t f_len;
    int32_t res;
    res=dir_namei(filename,&f_len,&file_name,NULL,&o_inode);
    if(res<0){
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
    puti(o_inode);

    return 0;
}
//更改文件的权限
int sys_fchmod(unsigned int fd, mode_t mode){
    struct inode *o_inode;
    struct file *files;
    files=CUR_TASK->files;
    if(fd>=NR_FILE || files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(files[fd].f_inode)){
        return -ENOENT;
    }

    if(files[fd].f_flags&O_RDONLY){
        return -EROFS;
    }
    //需要检查权限，还没做

    o_inode=files[fd].f_inode;
    o_inode->i_type_mode&=0xffff0000;
    o_inode->i_type_mode|=mode;

    o_inode->i_dirt=1;
    return 0;
}
//改变指定文件的权限
int sys_chmod(const char * filename, mode_t mode){
    struct inode *o_inode;
    int32_t res;
    res=namei(filename,&o_inode);
    if(res<0){
        return -ENOENT;
    }

    //检查权限
    //特别是检查当前用户有没有权限编辑这个文件
    o_inode->i_type_mode&=0xffff0000;
    o_inode->i_type_mode|=mode;
    o_inode->i_dirt=1;
    puti(o_inode);
    return 0;
}
int sys_fchown(unsigned int fd, int user, int group){
    return -ENOSYS;
}
int sys_chown(const char * filename, int user, int group){
    return -ENOSYS;
}

int32_t do_open(struct file* files,const char *path,int32_t flags,int32_t mode){
    uint32_t i;
    int32_t res;
    struct inode *o_inode;
    //找到一个空的
    for(i=0;i<NR_FILE;i++){
        if(files[i].used==0){
            files[i].used=1;
            break;
        }
    }
    //是否没有多余的文件描述符了
    if(i>=NR_FILE){
        errno = EMFILE;
        return -1;
    }
    DEBUG("fs",INFO,"打开%s,flags:0x%x,mode:0x%x,fd:%d.",path,flags,mode,i);
    //打开文件
    res= open_namei(path,flags,mode,&o_inode,NULL);
    if(res<0){
        files[i].used=0;
        return res;
    }
    //设置文件操作标志
    files[i].f_flags=flags;
    //设置操作的函数集
    if((!files[i].f_op)
       && o_inode->i_ops
       && o_inode->i_ops->default_file_ops
            ) {
        files[i].f_op = o_inode->i_ops->default_file_ops;
    }
    //调用驱动或者文件open函数
    if(files[i].f_op&&files[i].f_op->open) {
        if ((res=files[i].f_op->open(o_inode, &(files[i]))) < 0) {
            files[i].used = 0;
            puti(o_inode);
            return res;
        }
    }
    //偏移默认为0
    files[i].f_ofs=0;
    files[i].f_inode=o_inode;
    //设置权限与模式
    files[i].f_mode= o_inode->i_type_mode;

//    static int count;
//    if(count++>15) {
//        printk("%s %s fp:%d\n", __FUNCTION__, path,i);
//    }

    return i;
}
/**
 * 打开文件
 * @param path
 * @param flags
 * @param mode
 * @return
 */
int32_t sys_open(const char* path,int32_t flags,int32_t mode){
    return do_open(CUR_TASK->files,path,flags,mode);
}

/**
 * 创建一个文件
 * @param pathname
 * @param mode
 * @return
 */
int sys_creat(const char * pathname, int mode){
    DEBUG("fs",ERR,"创建文件:%s,mode:0x%x",pathname,mode);
    return sys_open(pathname,O_CREAT|O_WRONLY,mode);
}
/**
 * 关闭一个文件
 * @param fp
 */
void file_close(int fp){
    struct file *files;
    struct inode *inode;
    if(fp<0||fp>=NR_FILE){
        DEBUG("fs",ERR,"文件fd：%d有误.",fp);
//        printk("%s fp.\n",__FUNCTION__ );
        return ;
    }
    files=CUR_TASK->files;
    if(files[fp].used==0){
        //文件已经关闭了
        return ;
    }
    inode=files[fp].f_inode;

    if(files[fp].f_op
    &&files[fp].f_op->release
    ){
        files[fp].f_op->release(inode,&files[fp]);
    }
    if(files[fp].f_op
       &&files[fp].f_op->fsync
            ) {
        files[fp].f_op->fsync(inode,&files[fp]);
    }

    files[fp].f_op=NULL;
    files[fp].used=0;
    files[fp].f_ofs=0;
    files[fp].f_inode=NULL;


    DEBUG("fs",INFO,"关闭文件：%d",fp);

    puti(inode);
}

int sys_fsync(int fp){
    int res=-ENOSYS;
    struct file *files;
    struct inode *inode;
    if(fp<0||fp>=NR_FILE){
        DEBUG("fs",ERR,"文件fd：%d有误.",fp);
        return -EBADF;
    }
    files=CUR_TASK->files;
    if(files[fp].used==0){
        //文件已经关闭了
        return -EINVAL;
    }
    inode=files[fp].f_inode;
    if(files[fp].f_op
       &&files[fp].f_op->fsync
            ) {
        res=files[fp].f_op->fsync(inode,&files[fp]);
    }
    DEBUG("fs",INFO,"文件同步：%d",fp);
    return res;
}
