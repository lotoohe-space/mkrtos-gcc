//
// Created by Administrator on 2022/1/10.
//

#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <errno.h>
#include <mkrtos/smem_knl.h>
#include <fcntl.h>
#include <string.h>
#include <mkrtos/mem.h>
#include "mkrtos/stat.h"


int follow_link(struct inode * dir, struct inode * inode,
                int flag, int mode, struct inode ** res_inode)
{
    //根目录并且需要跟踪的inode为NULL
    if (!dir || !inode) {
        puti(dir);
        puti(inode);
        *res_inode = NULL;
        return -ENOENT;
    }
    /* 如果follow_link为空，则释放dir，
     * 同时将res_inode指向inode,有一种情况可能就是目录的符号连接
     * 当读取到目录符号连接时，则回继续转到真正的目录下面
     */
    if (!inode->i_ops || !inode->i_ops->follow_link) {
        puti(dir);
        *res_inode = inode;
        return 0;
    }
    return inode->i_ops->follow_link(dir,inode,flag,mode,res_inode);
}

/* 在dir对应的目录当中查找文件名为name,且长度为len的文件或目录的inode
 */
int lookup(struct inode * dir,const char * name, int len,
           struct inode ** result)
{
    struct super_block * sb;
    int perm;

    *result = NULL;
    if (!dir)
        return -ENOENT;
//    perm = permission(dir,MAY_EXEC);
    if (len==2 && name[0] == '.' && name[1] == '.') {
        //如果查看的是..目录
        if (dir == CUR_TASK->root_inode) {
            //如果当前是跟节点，则直接返回根节点就行了，因为根节点的上一个目录就是自己
            *result = dir;
            return 0;
        }
        else if ((sb = dir->i_sb) && (dir == sb->root_inode)) {
            //如果等于super_block的根目录
            sb = dir->i_sb;
            puti(dir);
            //重新设置为sb的挂载的inode
            dir = sb->s_covered;
            if (!dir) {
                return -ENOENT;
            }
            atomic_inc(&dir->i_used_count);
        }
    }
    /* 判断不是一个目录，目录也是一种特殊的文件
     */
    if (!dir->i_ops || !dir->i_ops->lookup) {
        puti(dir);
        return -ENOTDIR;
    }
//    if (!perm) {
//        puti(dir);
//        return -EACCES;
//    }
    if (!len) {
        *result = dir;
        return 0;
    }
    return dir->i_ops->lookup(dir,name,len,result);
}
static int _namei(const char * pathname, struct inode * base,
                  int follow_links, struct inode ** res_inode)
{
    const char * basename;
    int namelen,error;
    struct inode * inode;

    *res_inode = NULL;
    error = dir_namei(pathname,&namelen,&basename,base,&base);
    if (error) {
        return error;
    }
    atomic_inc(&(base->i_used_count));
    error = lookup(base,basename,namelen,&inode);
    if (error) {
        puti(base);
        return error;
    }
    /* 如果跟踪连接 */
    if (follow_links) {
        error = follow_link(base,inode,0,0,&inode);
        if (error) {
            return error;
        }
    } else {
        puti(base);
    }
    *res_inode = inode;
    return 0;
}

/* 不跟踪连接 */
int lnamei(const char * pathname, struct inode ** res_inode)
{
    int error;
    error = _namei(pathname,NULL,0,res_inode);
    return error;
}
int namei(const char * pathname, struct inode ** res_inode)
{
    int error;
    error = _namei(pathname,NULL,1,res_inode);
    return error;
}

/**
 * 获取路径所在的inode
 * @param file_path 路径
 * @param p_inode 文件的inode
 * @param dir 目录的inode
 * @param file_name 文件名字
 * @return
 */
int32_t dir_namei(const char * pathname, int32_t * namelen, const char ** name,
                  struct inode * base, struct inode ** res_inode){
    char c;
    const char * thisname;
    int len,error;
    struct inode * inode;

    *res_inode = NULL;
    if (!base) {
        //从那个目录开始扫描
        base=PWD_INODE;
        if(base) {
            atomic_inc(&base->i_used_count);
        }
    }
    if ((c = *pathname) == '/' || (c = *pathname) == '\\') {
        // 以 / 或者 \开始的路径，说明是从根目录开始的
        puti(base);
        base = ROOT_INODE;
        pathname++;
        if(base) {
            atomic_inc(&base->i_used_count);
        }
    }
    while (1) {
        thisname = pathname;
        //找到一段路径或者到字符串结尾
        for(len=0;(c = *(pathname++))&&(c != '/') && c!='\0';len++);
        if (!c) {
            //到最后直接退出
            break;
        }
        //增加引用计数，如果不增加，则可能在当前进程被切换出去之后，把该inode给释放掉了
        atomic_inc(&base->i_used_count);
        error = lookup(base,thisname,len,&inode);
        if (error) {
            puti(base);
            return error;
        }
        //目标可能是一个符号文件，所以要找到符号文件对应的inode
        error = follow_link(base,inode,0,0,&base);
        if (error) {
            return error;
        }
    }
    if (!base->i_ops || !base->i_ops->lookup) {
        puti(base);
        return -ENOTDIR;
    }
    *name = thisname;
    *namelen = len;
    *res_inode = base;
    return 0;
}

int32_t open_namei(const char* file_path,int32_t flags,int32_t mode,struct inode **res_inode,struct inode *base_dir){
    struct inode* ino;
    struct inode* dir;
    int32_t res=0;
    int32_t f_len;
    char *file_name ;
    //打开目录
    res= dir_namei(file_path,&f_len,&file_name,base_dir,&dir);
    if(res<0){
        return res;
    }
    //打开一个目录
    if( f_len == 0 ){
        if ((flags & O_WRONLY) || (flags & O_RDWR)) {
            //目录不能在写模式或者在读写模式
            puti(dir);
            return -EISDIR;
        }
        *res_inode = dir;
        return 0;
    }
    atomic_inc(&dir->i_used_count);
    if(flags & O_CREAT){
        res=lookup(dir,file_name,f_len,&ino);
        if(res>=0) {
            if (flags & O_EXCL) {
                puti(ino);
                res = -EEXIST;
            }
        }else if(!dir->i_ops || !dir->i_ops->create){
            res=-EACCES;
        }else {
            atomic_inc(&dir->i_used_count);
            res=dir->i_ops->create(dir, file_name, mode, sizeof(struct inode), &ino);
            *res_inode = ino;
            puti(dir);
            return res;
        }
    }else {
        res = lookup(dir, file_name, f_len, &ino);
        if(res<0){
            puti(dir);
            return -ENOENT;
        }
    }
    if (flags & O_TRUNC) {
        if (ino->i_ops && ino->i_ops->truncate) {
            ino->i_ops->truncate(ino,0);
        }
//        if ((error = notify_change(NOTIFY_SIZE, inode))) {
//            iput(inode);
//            return error;
//        }
        ino->i_dirt = 1;
    }
    *res_inode = ino;

    return res;
}

//新建目录
int sys_mkdir(const char * pathname, int mode){
    struct  inode* path_inode;
    struct inode* dir;
    const char *file_name;
    int32_t f_len;
    int32_t res;
    //获得所在路径的inode
    if((dir_namei(pathname,&f_len,&file_name,NULL,&path_inode))<0){
        //目录存在
        return -ENOENT;
    }

    //这里需要检查权限
    if(path_inode->i_ops
    &&path_inode->i_ops->mkdir
    ) {
        atomic_inc(&path_inode->i_used_count);
        res = path_inode->i_ops->mkdir(path_inode,file_name, strlen(file_name), mode);

        puti(path_inode);
        return res;
    }
    puti(path_inode);
    return -ENOSYS;
}
/**
 * 删除目录
 * @param path_name
 * @param dir_name
 * @param name_len
 * @return
 */
int sys_rmdir (const char *pathname){
    struct  inode* path_inode;
    struct inode* dir;
    char *file_name;
    int32_t f_len;
    int32_t res;
    //获得所在路径的inode
    if((dir_namei(pathname,&f_len,&file_name,NULL,&path_inode))<0){
        //目录存在
        return -ENOENT;
    }
    if(!IS_DIR_FILE(path_inode->i_type_mode)){
        puti(path_inode);
        return -ENOENT;
    }

    //这里需要检查权限
    if(path_inode->i_ops
       &&path_inode->i_ops->rmdir
            ) {
        atomic_inc(&path_inode->i_used_count);
        res = path_inode->i_ops->rmdir(path_inode,file_name, strlen(file_name));
        if (res < 0) {
            puti(path_inode);
            return res;
        }
    }
    puti(path_inode);
    return -ENOSYS;
}
int do_link(struct inode * oldinode, const char * newname)
{
    struct inode * dir;
    struct inode * p_dir;
    const char * basename;
    int error;
    int32_t f_len;
    error = dir_namei(newname,&f_len,&basename,NULL,&dir);
    if (error<0) {
        //文件已经存在了
        puti(oldinode);
        return error;
    }
    if (!strlen(newname)) {
        puti(oldinode);
        puti(dir);
        return -EPERM;
    }
//    if (IS_RDONLY(dir)) {
//        iput(oldinode);
//        iput(dir);
//        return -EROFS;
//    }
    if (dir->i_sb->s_dev_no != oldinode->i_sb->s_dev_no) {
        puti(dir);
        puti(oldinode);
        return -EXDEV;
    }
//    if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
//        iput(dir);
//        iput(oldinode);
//        return -EACCES;
//    }
    if (!dir->i_ops || !dir->i_ops->link) {
        puti(dir);
        puti(oldinode);
        return -EPERM;
    }
//    down(&dir->i_sem);
    error = dir->i_ops->link(oldinode, dir, basename, strlen(basename));
//    up(&dir->i_sem);
    return error;
}

int sys_link(const char * oldname, const char * newname){
    int error;
    struct inode * oldinode;
    error = namei(oldname,&oldinode);
    if (error<0) {
        return error;
    }
    error = do_link(oldinode,newname);
    return error;
}
//删除文件
int sys_unlink(const char * pathname){
    struct  inode* path_inode;
    int32_t res;
    int32_t f_len;
    char *file_name;

    //获得所在路径的inode
    if((dir_namei(pathname,&f_len,&file_name,NULL,&path_inode))<0){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(path_inode->i_type_mode)){
        puti(path_inode);
        return -ENOENT;
    }

    if(path_inode->i_ops
       &&path_inode->i_ops->unlink
            ) {
        atomic_inc(&path_inode->i_used_count);
        res = path_inode->i_ops->unlink(path_inode, file_name, strlen(file_name));
        puti(path_inode);
        return res;
    }
    puti(path_inode);

    return -ENOSYS;
}
static int do_symlink(const char * oldname, const char * newname)
{
    struct inode * dir;
    struct inode * p_dir;
    char * basename;
    int32_t f_len;
    int error;

    error = dir_namei(newname,&f_len,&basename,NULL,&dir);
    if (error<0) {
        return error;
    }
    if (!strlen(basename)) {
        puti(dir);
        return -ENOENT;
    }
//    if (IS_RDONLY(dir)) {
//        puti(dir);
//        return -EROFS;
//    }
//    if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
//        iput(dir);
//        return -EACCES;
//    }
    if (!dir->i_ops || !dir->i_ops->symlink) {
        puti(dir);
        return -EPERM;
    }
//    down(&dir->i_sem);
    error = dir->i_ops->symlink(dir,basename,strlen(basename),oldname);
//    up(&dir->i_sem);
    return error;
}
int sys_symlink(const char * oldname, const char * newname){
    int error;
    char * from, * to;

    error = getname(oldname,&from);
    if (!error) {
        error = getname(newname,&to);
        if (!error) {
            error = do_symlink(from,to);
            putname(to);
        }
        putname(from);
    }
    return error;
}
int sys_rename(const char * oldname, const char * newname){
    return -ENOSYS;
}
/**
 * 新建一个驱动文件
 * @param filename
 * @param mode
 * @param dev
 * @return
 */
int sys_mknod(const char * filename, int mode, dev_t dev){
    struct  inode* path_inode;
    struct  inode* dir;
    char * base_name;
    int32_t res;
    int32_t f_len;
    //获得所在路径的inode
    if((dir_namei(filename,&f_len,&base_name,NULL,&path_inode))<0){
        return -ENOENT;
    }
    //调用文件系统的新建驱动文件函数
    if(path_inode->i_ops
    &&path_inode->i_ops->mknod
    ) {
        atomic_inc(&path_inode->i_used_count);
        int32_t res=path_inode->i_ops->mknod(path_inode, base_name, strlen(base_name), mode, dev);
        return res;
    }
    puti(path_inode);
    return -ENOSYS;
}





