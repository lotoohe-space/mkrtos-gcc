//
// Created by Administrator on 2022/1/10.
//
#include <type.h>
#include <errno.h>
#include <mkrtos/stat.h>
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <fcntl.h>
int sys_ustat(int dev, struct ustat * ubuf){
    return -ENOSYS;
}
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
    if(fd>=NR_FILE || CUR_TASK->files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(CUR_TASK->files[fd].f_inode)){
        return -ENOENT;
    }
    if((CUR_TASK->files[fd].f_inode->i_sb->s_ops->statfs)){
        CUR_TASK->files[fd].f_inode->i_sb->s_ops->statfs(CUR_TASK->files[fd].f_inode->i_sb,buf);
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
    return error;
}
int sys_ftruncate(unsigned int fd, unsigned int length){
    struct inode *inode;
    if(fd>=NR_FILE || CUR_TASK->files[fd].used==0){
        //文件已经关闭了
        return -EBADF;
    }
    if(!(CUR_TASK->files[fd].f_inode)){
        return -ENOENT;
    }
    inode=&CUR_TASK->files[fd].f_inode;
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
//进入某个目录
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
int sys_getcwd(char* source,int cn){
    strncpy(source,CUR_TASK->pwd_path,cn);
    return strlen(CUR_TASK->pwd_path);
}
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
    for(i=0;i<NR_FILE;i++){
        if(files[i].used==0){
            files[i].used=1;
            break;
        }
    }
    if(i>=NR_FILE){
        errno = EMFILE;
        return -1;
    }

    //打开文件
    res= open_namei(path,flags,mode,&o_inode,NULL);
    if(res<0){
        files[i].used=0;
        return -1;
    }
    files[i].f_flags=flags;
    //设置文件操作符号
    if((!files[i].f_op)
       &&  o_inode->i_ops->default_file_ops
            ) {
        files[i].f_op = o_inode->i_ops->default_file_ops;
    }
    //调用打开函数
    if(
            files[i].f_op
            &&files[i].f_op->open
            ) {
        if (files[i].f_op->open(o_inode, &(files[i])) < 0) {
            files[i].used = 0;
            puti(o_inode);
            return -1;
        }

    }
    files[i].f_ofs=0;
    files[i].f_inode=o_inode;
    files[i].f_mode= o_inode->i_type_mode;

//    static int count;
//    if(count++>15) {
//        printk("%s %s fp:%d\n", __FUNCTION__, path,i);
//    }

    return i;
}
/**NR_FILE
 * 打开文件
 * @param path
 * @param flags
 * @param mode
 * @return
 */
int32_t sys_open(const char* path,int32_t flags,int32_t mode){
    return do_open(CUR_TASK->files,path,flags,mode);
}

//创建文件
int sys_creat(const char * pathname, int mode){
    return sys_open(pathname,O_CREAT|O_WRONLY,mode);
}
//关闭文件
void sys_close(int fp){
    struct inode *inode;
    if(fp<0||fp>=NR_FILE){
        printk("%s fp.\n",__FUNCTION__ );
        return ;
    }
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
    CUR_TASK->files[fp].f_op=NULL;
    CUR_TASK->files[fp].used=0;
    CUR_TASK->files[fp].f_ofs=0;
    CUR_TASK->files[fp].f_inode=NULL;
    puti(inode);
}
int sys_vhangup(void){
    return -ENOSYS;
}
