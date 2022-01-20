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
//获取目录的名字，并返回截断的位置
uint32_t get_path_name(const char* file_path, char* path, uint32_t cache_len) {
    uint32_t len;
    uint32_t end_inx;
    if (file_path == NULL) {
        return 0;
    }
    len = strlen(file_path);
    end_inx= len;
    for (int32_t i = len - 1; i >= 0; i--) {
        if (file_path[i] == '\\' || file_path[i] == '/') {
            end_inx = i;
            break;
        }
    }
    if (path != NULL) {
        uint32_t i = 0;
        for (i = 0; i < end_inx && i < (cache_len - 1); i++) {
            path[i] = file_path[i];
        }
        path[i] = '\0';
    }
    return end_inx;
}

int32_t dir_namei(const char*file_path,struct inode** p_inode){
    int32_t res=0;
    //目录深度
    uint32_t dir_deep = 0;
    uint32_t word_st_inx = 0;
    uint32_t word_end_inx = 0;
    if (file_path == NULL) {
        return -EINVAL;
    }
    struct inode* res_inode = NULL;

    if (file_path[0] == '\0') {
        //目录长度为0，则标志为当前工作目录
        res_inode = PWD_INODE;
        atomic_inc(&res_inode->i_used_count);
        goto end;
    }
    for (uint32_t i = 0; file_path[i]; i++) {
        if (file_path[i] == '\\' || file_path[i] == '/' || file_path[i + 1] == '\0') {
            word_end_inx = i;
            if (dir_deep == 0 && (word_end_inx - word_st_inx == 0)) {
                //设置根节点
                res_inode = ROOT_INODE;
                atomic_inc(&res_inode->i_used_count);
            }
            else {
                char *fnTemp;
                if (res_inode == NULL) {
                    //等于NULL说明没有,则设置为当前工作目录
                    res_inode=PWD_INODE;
                    atomic_inc(&res_inode->i_used_count);
                }
                fnTemp=OSMalloc(512);
                if(fnTemp==NULL){
//                    while(1);
                }
                uint32_t j = 0;
                for (j = word_st_inx; j <= word_end_inx; j++) {
                    fnTemp[j - word_st_inx] = file_path[j];
                }
                if (file_path[i] == '\\' || file_path[i] == '/') {
                    fnTemp[j - word_st_inx - 1] = '\0';
                }
                else {
                    fnTemp[j - word_st_inx] = '\0';
                }
                atomic_inc(&res_inode->i_used_count);
                if((res=res_inode->i_ops->lookup(res_inode,fnTemp,sizeof(struct inode),&res_inode))<0){
                    OSFree(fnTemp);
                    goto end;
                }
                OSFree(fnTemp);
            }
            word_st_inx = i + 1;
            dir_deep++;
        }
    }
    end:
    *p_inode=res_inode;
    puti(res_inode);
    return res;
}

struct inode* open_namei(const char* file_path,int32_t flags,int32_t mode){
    struct inode* ino;
    int32_t res;
    res= dir_namei(file_path,&ino);
    if(res<0){
        if(res==-ENOENT) {
            if ( (flags & O_CREAT)&& (flags & O_WRONLY) ) {//是否需要创建文件
                char cache[128];
                struct inode *path_inode;
                uint32_t c_ofs = get_path_name(file_path, cache, sizeof(cache));
                //获得所在路径的inode
                if ((dir_namei(cache,&path_inode)<0)) {
                    errno = ENOENT;
                    return NULL;
                }
                atomic_inc(&path_inode->i_used_count);
                //创建失败
                if ((res = ino->i_ops->create(path_inode, file_path + c_ofs + 1, mode, sizeof(struct inode), &ino)) <
                    0) {
                    errno = res;
                    return NULL;
                }
                goto next;
            }
        }
        return NULL;
    }else{
        if(//是否需要截断文件
            (flags & O_TRUNC)
            &&(flags& O_WRONLY)
            ){
            if(ino->i_ops->truncate) {
                atomic_inc(&ino->i_used_count);
                ino->i_ops->truncate(ino);
            }else{
                puti(ino);
                errno=-ENOSYS;
                return NULL;
            }
        }
    }
    next:
    atomic_inc(&ino->i_used_count);
    return ino;
}

//新建目录
int sys_mkdir(const char * pathname, int mode){
    char cache[128];
    struct  inode* path_inode;
    uint32_t c_ofs;
    int32_t res;
    c_ofs = get_path_name(pathname,cache,sizeof(cache));
    //获得所在路径的inode
    if((dir_namei(cache,&path_inode))<0){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(path_inode->i_type_mode)){
        puti(path_inode);
        return -ENOENT;
    }

    //这里需要检查权限

    if(path_inode->i_ops
    &&path_inode->i_ops->mkdir
    ) {
        atomic_inc(&path_inode->i_used_count);
        res = path_inode->i_ops->mkdir(path_inode, pathname+c_ofs+1, strlen(pathname) - c_ofs-1, mode);
        if (res < 0) {
            puti(path_inode);
            return res;
        }
    }else{
        puti(path_inode);
        return -ENOSYS;
    }
    return 0;
}
/**
 * 删除目录
 * @param path_name
 * @param dir_name
 * @param name_len
 * @return
 */
int sys_rmdir (const char *pathname){
    char cache[128];
    struct  inode* path_inode;
    uint32_t c_ofs;
    int32_t res;
    c_ofs = get_path_name(pathname,cache,sizeof(cache));
    //获得所在路径的inode
    if((dir_namei(cache,&path_inode))<0){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(path_inode->i_type_mode)){
        puti(path_inode);
        return -ENOENT;
    }

    if(path_inode->i_ops
       &&path_inode->i_ops->rmdir
            ) {
        atomic_inc(&path_inode->i_used_count);
        res = path_inode->i_ops->rmdir(path_inode, cache, strlen(pathname) - c_ofs);
        if (res < 0) {
            puti(path_inode);
            return res;
        }
    }else{
        puti(path_inode);
        return -ENOSYS;
    }

    return 0;
}
int sys_link(const char * oldname, const char * newname){
    return -ENOSYS;
}
//删除文件
int sys_unlink(const char * pathname){
    char cache[128];
    struct  inode* path_inode;
    uint32_t c_ofs;
    int32_t res;
    c_ofs = get_path_name(pathname,cache,sizeof(cache));
    //获得所在路径的inode
    if((dir_namei(cache,&path_inode))<0){
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
        res = path_inode->i_ops->unlink(path_inode, cache, strlen(pathname) - c_ofs);
        if (res < 0) {
            puti(path_inode);
            return res;
        }
    }else{
        puti(path_inode);
        return -ENOSYS;
    }

    return 0;
}
int sys_symlink(const char * oldname, const char * newname){
    return -ENOSYS;
}
//int sys_link(const char * oldname, const char * newname){
//    return -ENOSYS;
//}
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
    char cache[128];
    struct  inode* path_inode;
    uint32_t c_ofs;
    int32_t res;
    c_ofs = get_path_name(filename,cache,sizeof(cache));
    //获得所在路径的inode
    if((dir_namei(cache,&path_inode))<0){
        return -ENOENT;
    }
    if(!IS_DIR_FILE(path_inode->i_type_mode)){
        puti(path_inode);
        return -ENOENT;
    }
    //调用文件系统的新建驱动文件函数
    if(path_inode->i_ops
    &&path_inode->i_ops->mknod
    ) {
        atomic_inc(&path_inode->i_used_count);
        int32_t res=path_inode->i_ops->mknod(path_inode, cache + c_ofs + 1, strlen(filename) - c_ofs, mode, dev);
        if(res<0){
            return res;
        }
    }else{
        return -ENOSYS;
    }
    return 0;
}





