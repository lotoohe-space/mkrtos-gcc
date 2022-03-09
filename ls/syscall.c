//
// Created by Administrator on 2022/3/6.
//
#include "arm/syscalls.h"
#include "include/stddef.h"
#include "include/sys/types.h"

int errno=0;
int call4(int call_num,int a,int b,int c,int d){
    int out=0;
    __asm__ __volatile__(
    "mov r0,%1\n"
    "mov r1,%2\n"
    "mov r2,%3\n"
    "mov r3,%4\n"
    "push {r7}\n"
    "mov r7, %5\n"
    "svc 128\n"
    "pop {r7}\n"
    "mov %0,r0\n"
    :"=&r"(out)
    :"r"(a),"r"(b),"r"(c),"r"(d),"r"(call_num)
    :
    );

    if(out<0){
        errno=-out;
    }

    return out;
}
int call7(int call_num,int a,int b,int c,int d,int e,int f,int g){
    int out=0;
    __asm__ __volatile__(
    "mov r0,%1\n"
    "mov r1,%2\n"
    "mov r2,%3\n"
    "mov r3,%4\n"
    "mov r4,%5\n"
    "mov r5,%6\n"
    "mov r6,%7\n"
    "push {r7}\n"
    "mov r7, %8\n"
    "svc 128\n"
    "pop {r7}\n"
    "mov %0,r0\n"
    :"=&r"(out)
    :"r"(a),"r"(b),"r"(c),"r"(d),"r"(e),"r"(f),"r"(g),"r"(call_num)
    :
    );
    if(out<0){
        errno=-out;
    }
    return out;
}
#define syscall0(name,res_type)\
res_type name(void){ \
    return (res_type)call4(__NR_##name,0,0,0,0); \
}
#define syscall1(name,res_type,type0)\
res_type name(type0 a){ \
    return (res_type)call4(__NR_##name,(int)a,0,0,0); \
}
#define syscall2(name,res_type,type0,type1)\
res_type name(type0 a,type1 b){ \
    return (res_type)call4(__NR_##name,(int)a,(int)b,0,0); \
}
#define syscall3(name,res_type,type0,type1,type2)\
res_type name(type0 a,type1 b,type2 c){ \
    return (res_type)call4(__NR_##name,(int)a,(int)b,(int)c,0); \
}
#define syscall4(name,res_type,type0,type1,type2,typ3)\
res_type name(type0 a,type1 b,type2 c,typ3 d){ \
    return (res_type)call4(__NR_##name,(int)a,(int)b,(int)c,(int)d); \
}

#define syscall6(name,res_type,type0,type1,type2,typ3,type4,type5)\
res_type name(type0 a,type1 b,type2 c,typ3 d,type4 e,type5 f){ \
    return (res_type)call7(__NR_##name,(int)a,(int)b,(int)c,(int)d,(int)e,(int)f,0); \
}

syscall0(setup,int)
syscall1(_exit,int,int)
syscall1(fchdir,int,int)
syscall1(chdir,int,char*)
syscall2(mkdir,int ,const char*, int)
syscall1(umask,int,int)
syscall1(time,int,void*)
syscall2(stat,int ,const char*, void*)
syscall2(lstat,int ,const char*, void*)
syscall3(readlink,int,const char*,char*const,int)
syscall3(execve,int,char*,char*const,char*const)
syscall3(waitpid,int,int,int*,int)

syscall1(uname,int,void*);
//int _exit(int exit_code){
//    return exit(exit_code);
//}
//int __libc_exit(int exit_code){
//    return exit(exit_code);
//}
syscall0(fork,int)
syscall3(read,int,int,void*,int)
int __libc_read(int fd,void* buf,int cn){
    return read(fd,buf,cn);
}
syscall3(write,int,int,void*,int)
syscall3(lseek,int,int,int,int);
int __libc_write(int fd,void* buf,int cn){
    return write(fd,buf,cn);
}

syscall4(mremap,void*,void*,int,int,unsigned long);


syscall3(open,int,int,int,int)
syscall1(close,int,int)

syscall6(mmap,void*,void*,size_t,int,int,int,off_t)
syscall2(munmap,int,void*,size_t)

#include <dirent.h>
syscall3(getdents,int,int,struct dirent*,unsigned int)

syscall3(fcntl,int,int,int,int)
syscall0(getpid,int)
syscall0(getppid,int)

syscall3(ioctl,int,int,long int,int)
