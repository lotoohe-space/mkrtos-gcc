//
// Created by Administrator on 2022/3/29.
//

#include "lwip/sockets.h"
#include "linuxnet.h"
#include <sys/types.h>
#include <mkrtos/task.h>

//open.c
extern int file_read (int fd,uint8_t *buf,uint32_t len);
extern int file_write (int fd,uint8_t *buf,uint32_t len);
extern void file_close(int fd);


#define GET_FILE_INFO (CUR_TASK->files)
int errno_lwip=0;

int sys_read(int s,uint8_t * buf,uint32_t len){
    struct file* files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        int ret= lwip_read(files[s].net_sock,buf,len);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
    }else{
        return file_read(s,buf,len);
    }
}
int sys_write (int s,uint8_t *buf,uint32_t len){
    struct file* files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        printk("EBADF Error.\r\n");
        return -EBADF;
    }
    int ret;
    if(files[s].net_file){
        ret = lwip_write(files[s].net_sock,buf,len);
        if(ret<0) {
            return -errno_lwip;
//            printk("write err is %d.\r\n", ret);
        }
        return ret;
    }else{
        return file_write(s,buf,len);
    }
}
void sys_close(int s){
    struct file* files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return ;
    }
    if(files[s].net_file){
        lwip_close(files[s].net_sock);
        files[s].used=0;
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
        int ret= lwip_recv(files[s].net_sock,mem,len,flags);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_send(files[s].net_sock,data,size,flags);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        for(i=0;i<NR_FILE;i++){
            if(!files[i].used){
                files[i].used=1;

                files[i].net_file=1;
                files[i].f_inode=NULL;
                files[i].f_op=NULL;
//                files[i].sock_used_cn=1;
                break;
            }
        }
        if(i==NR_FILE){
            return -ENOSYS;
        }
        int new_sock= lwip_accept(files[s].net_sock,addr,addrlen);
        if(new_sock<0){
            files[i].used=0;
//            printk("accept error is %d %d.\r\n",new_sock,errno_lwip);
            return -errno_lwip;
        }
        files[i].net_sock=new_sock;
        printk("new fd is %d.\r\n");
        printk("sock is %d.\r\n");
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
        int ret= lwip_bind(files[s].net_sock,name,namelen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_shutdown(files[s].net_sock,how);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
            break;
        }
    }
    int32_t new_sock= lwip_socket(domain,type,protocol);
    if(new_sock<0){
        files[i].used=0;
        return errno_lwip;
    }
//    files[i].sock_used_cn=1;
    files[i].net_sock=new_sock;
    files[i].net_file=1;
    return i;
}

int sys_connect(int s,const struct sockaddr* name, socklen_t namelen){
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file){
        int ret= lwip_connect(files[s].net_sock,name,namelen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_listen(files[s].net_sock,backlog);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_setsockopt(files[s].net_sock, level, optname, optval, optlen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_getsockopt(files[s].net_sock, level, optname, optval, optlen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_recvmsg(files[s].net_sock, message, flags);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_recvfrom(files[s].net_sock, mem, len, flags, from, fromlen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_sendmsg(files[s].net_sock, msg, flags);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_sendto(files[s].net_sock, data, size, flags, to, tolen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
    }else {
        return -ENOSYS;
    }
}
int sys_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
            struct timeval *timeout){
   int ret= lwip_select(maxfdp1,readset,writeset,exceptset,timeout);
    if(ret<0){
        return -errno_lwip;
    }
    return ret;
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
        int ret= lwip_ioctl(files[s].net_sock,cmd,argp);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
    int ret= lwip_inet_pton(af,src,dst);
    if(ret<0){
        return -errno_lwip;
    }
    return ret;
}
int sys_getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
    struct file * files=GET_FILE_INFO;
    if((s<0&&s>=NR_FILE)||!files[s].used) {
        return -EBADF;
    }
    if(files[s].net_file) {
        int ret= lwip_getpeername(files[s].net_sock, name, namelen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
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
        int ret= lwip_getsockname(files[s].net_sock, name, namelen);
        if(ret<0){
            return -errno_lwip;
        }
        return ret;
    }else {
        return -ENOSYS;
    }
}

int sys_socketpair(int d, int type, int protocol, int sv[2]){
    return -ENOSYS;
}

int sys_socketcall(int type,int *args){
    switch(type){
        case SYS_SOCKET:
            return sys_socket(args[0],args[1],args[2]);
        case SYS_BIND:
            return sys_bind(args[0],args[1],args[2]);
        case SYS_CONNECT:
            return sys_connect(args[0],args[1],args[2]);
        case SYS_LISTEN:
            return sys_listen(args[0],args[1]);
        case SYS_ACCEPT:
            return sys_accept(args[0],args[1],args[2]);
        case SYS_GETSOCKNAME:
            return sys_getsockname(args[0],args[1],args[2]);
        case SYS_GETPEERNAME:
            return sys_getpeername(args[0],args[1],args[2]);
        case SYS_SOCKETPAIR:
            return sys_socketpair(args[0],args[1],args[2],args[3]);
        case SYS_SEND:
            return sys_send(args[0],args[1],args[2],args[3]);
        case SYS_RECV:
            return sys_recv(args[0],args[1],args[2],args[3]);
        case SYS_SENDTO:
            return sys_sendto(args[0],args[1],args[2],args[3],args[4],args[5]);
        case SYS_RECVFROM:
            return sys_recvfrom(args[0],args[1],args[2],args[3],args[4],args[5]);
        case SYS_SHUTDOWN:
            return sys_shutdown(args[0],args[1]);
        case SYS_SETSOCKOPT:
            return sys_setsockopt(args[0],args[1],args[2],args[3],args[4]);
        case SYS_GETSOCKOPT:
            return sys_getsockopt(args[0],args[1],args[2],args[3],args[4]);
        case SYS_SENDMSG:
            return sys_sendmsg(args[0],args[1],args[2]);
        case SYS_RECVMSG:
            return sys_recvmsg(args[0],args[1],args[2]);
        default:
            return -ENOSYS;
    }
//#define SYS_SOCKET	1		/* sys_socket(2)		*/
//#define SYS_BIND	2		/* sys_bind(2)			*/
//#define SYS_CONNECT	3		/* sys_connect(2)		*/
//#define SYS_LISTEN	4		/* sys_listen(2)		*/
//#define SYS_ACCEPT	5		/* sys_accept(2)		*/
//#define SYS_GETSOCKNAME	6		/* sys_getsockname(2)		*/
//#define SYS_GETPEERNAME	7		/* sys_getpeername(2)		*/
//#define SYS_SOCKETPAIR	8		/* sys_socketpair(2)		*/
//#define SYS_SEND	9		/* sys_send(2)			*/
//#define SYS_RECV	10		/* sys_recv(2)			*/
//#define SYS_SENDTO	11		/* sys_sendto(2)		*/
//#define SYS_RECVFROM	12		/* sys_recvfrom(2)		*/
//#define SYS_SHUTDOWN	13		/* sys_shutdown(2)		*/
//#define SYS_SETSOCKOPT	14		/* sys_setsockopt(2)		*/
//#define SYS_GETSOCKOPT	15		/* sys_getsockopt(2)		*/
//#define SYS_SENDMSG	16		/* sys_sendmsg(2)		*/
//#define SYS_RECVMSG	17		/* sys_recvmsg(2)		*/
}