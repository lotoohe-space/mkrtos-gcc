#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "write12.h"

int die(const char* error) {
  const char* x="unknown error";
  __write2("time: ");
  __write2(error);
  __write2(": ");
  switch(errno) {
  case ENOMEM: x="Out of memory"; break;
  case EACCES: x="Permission denied"; break;
  case ENOTDIR: x="Not a directory"; break;
  case ENOENT: x="No such file or directory"; break;
  case ELOOP: x="Too many symbolic links"; break;
  case EFAULT: x="Broken executable"; break;
  case EIO: x="I/O error"; break;
  case ENFILE: x="Too many open files"; break;
  }
  __write2(x);
  __write2("\n");
  exit(1);
}

#include "fmt_ulong.c"

int fmt_time(char* buf,struct timeval* t,int prec) {
  unsigned long l;
  int i=fmt_ulong(buf,t->tv_sec);
  buf[i]='.'; ++i;
  if (prec==3) {
    l=t->tv_usec/1000;
    if (l<100) {
      buf[i]='0'; ++i;
    }
  } else
    l=t->tv_usec/10000;
  if (l<10) {
    buf[i]='0'; ++i;
  }
  i+=fmt_ulong(buf+i,l);
  return i;
}

int main(int argc,char* argv[]) {
  char buf[100];
  struct rusage r;
  int i,status;
  struct timeval before,after;
  gettimeofday(&before,0);
  if (argc<2) {
    __write2("time: usage: time command [args...]\n");
    return 0;
  }
  switch (fork()) {
  case -1: die("fork");
  case 0: execvp(argv[1],argv+1);
	  die("execvp");
  }
  wait(&status);
  gettimeofday(&after,0);
  getrusage(RUSAGE_CHILDREN,&r);
  __write1(argv[1]);
  __write1(" ");
  write(1,buf,fmt_time(buf,&r.ru_utime,2));
  __write1("s user ");
  write(1,buf,fmt_time(buf,&r.ru_stime,2));
  __write1("s system ");
  after.tv_sec-=before.tv_sec;
  if (after.tv_usec<before.tv_usec) {
    --after.tv_sec;
    after.tv_usec+=1000000;
  }
  after.tv_usec-=before.tv_usec;
  write(1,buf,fmt_time(buf,&after,3));
  __write1("s total\n");
}
