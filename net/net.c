//
// Created by Administrator on 2022/3/29.
//

#include "lwip/sockets.h"
#include <sys/types.h>
#include <mkrtos/task.h>

#define GET_FILE_INFO (CUR_TASK->files)

void sys_close(int s){
    struct file* files=GET_FILE_INFO;
    if(files[s].net_file){
        lwip_close(files[s].net_sock);
    }else{
        file_close(s);
    }
}
ssize_t sys_recv(int s,void* mem,size_t len,int flags){
    struct file* files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }

    if(files[s].net_file){
        return lwip_recv(GET_FILE_INFO[s].net_sock,mem,len,flags);
    }else{
        return -ENOSYS;
    }
}
int sys_send(int s,const void* data,size_t size,int flags){
    struct file* files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        return lwip_send(GET_FILE_INFO[s].net_sock,data,size,flags);
    }else{
        return -ENOSYS;
    }
}
int sys_accept(int s,struct sockaddr *addr,socklen_t *addrlen){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        int i;
        int new_sock= lwip_accept(GET_FILE_INFO->net_sock,addr,addrlen);
        for(i=0;i<NR_FILE;i++){
            if(!files[i].used){
                files[i].used=1;
                files[i].net_sock=new_sock;
                files[i].net_file=1;
                files[i].f_inode=NULL;
                files[i].f_ofs=NULL;
            }
        }
        return i;
    }else{
        return -ENOSYS;
    }
}
int sys_bind(int s,const struct sockaddr* name,socklen_t namelen){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        return lwip_bind(GET_FILE_INFO[s].net_sock,name,namelen);
    }else{
        return -ENOSYS;
    }
}
int sys_shutdown(int s,int how){
    struct file* files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        return lwip_shutdown(GET_FILE_INFO[s].net_sock,how);
    }else{
        return -ENOSYS;
    }
}
int sys_socket(int domain,int type,int protocol){
    int i;
    struct file * files=GET_FILE_INFO;
    for(i=0;i<NR_FILE;i++){
        if(!files[i].used) {
            files[i].used = 1;
        }
    }
    int32_t new_sock= lwip_socket(domain,type,protocol);
    if(new_sock<0){
        files[i].used=0;
        return errno;
    }
    files[i].net_sock=new_sock;
    files[i].net_file=1;
    return i;
}
int sys_conncet(int s,const struct sockaddr* name, socklen_t namelen){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        return lwip_connect(GET_FILE_INFO[s].net_sock,name,namelen);
    }else{
        return -ENOSYS;
    }
}
int sys_listen(int s,int backlog){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        return lwip_listen(GET_FILE_INFO[s].net_sock,backlog);
    }else{
        return -ENOSYS;
    }
}
int sys_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen) {
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_setsockopt(GET_FILE_INFO[s].net_sock, level, optname, optval, optlen);
    }else {
        return -ENOSYS;
    }
}
int sys_getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen) {
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_getsockopt(GET_FILE_INFO[s].net_sock, level, optname, optval, optlen);
    }else {
        return -ENOSYS;
    }
}
ssize_t sys_recvmsg(int s, struct msghdr *message, int flags){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_recvmsg(GET_FILE_INFO[s].net_sock, message, flags);
    }else {
        return -ENOSYS;
    }
}
ssize_t sys_recvfrom(int s, void *mem, size_t len, int flags,
              struct sockaddr *from, socklen_t *fromlen){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_recvfrom(GET_FILE_INFO[s].net_sock, mem, len, flags, from, fromlen);
    }else {
        return -ENOSYS;
    }
}
ssize_t sys_sendmsg(int s, const struct msghdr *msg, int flags){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_sendmsg(GET_FILE_INFO[s].net_sock, msg, flags);
    }else {
        return -ENOSYS;
    }
}
ssize_t sys_sendto(int s, const void *data, size_t size, int flags,
            const struct sockaddr *to, socklen_t tolen){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_sendto(GET_FILE_INFO[s].net_sock, data, size, flags, to, tolen);
    }else {
        return -ENOSYS;
    }
}
int sys_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
            struct timeval *timeout){
   return lwip_select(maxfdp1,readset,writeset,exceptset,timeout);
}
int sys_poll(struct pollfd *fds, nfds_t nfds, int timeout){
    return lwip_poll(fds,nfds,timeout);
}
int sys_ioctl(int s, long cmd, void *argp){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_ioctl(files[s].net_sock,cmd,argp);
    }else{
        return file_ioctl(s,cmd,argp);
    }
}
const char * sys_inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    return lwip_inet_ntop(af,src,dst,size);
}
int sys_inet_pton(int af, const char *src, void *dst)
{
    return lwip_inet_pton(af,src,dst);
}
int sys_getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_getpeername(GET_FILE_INFO[s].net_sock, name, namelen);
    }else {
        return -ENOSYS;
    }
}
int sys_getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        return lwip_getsockname(GET_FILE_INFO[s].net_sock, name, namelen);
    }else {
        return -ENOSYS;
    }
}
