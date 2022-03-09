#define _LINUX_SOURCE
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include "write12.h"

struct {
  int e;
  const char* m;
} x[] = {
  { EISDIR, "Is a directory" },
  { EACCES, "Permission denied" },
  { ENAMETOOLONG, "Name too long" },
  { ENOENT, "No such file or directory" },
  { ENOTDIR, "Not a directory" },
  { ENXIO, "No such device or address" },
  { ENODEV, "No such device" },
  { ELOOP, "Too many symbolic links" },
  { ENOMEM, "Out of memory" },
  { EMFILE, "File table overflow" },
  { ENFILE, "Too many open files" },

  { ENOEXEC, "Invalid module format" },
  { ESRCH, "Module has wrong symbol version" },
  { EINVAL, "Invalid parameters" },
  { EEXIST, "Module already loaded" },
  { 0, "Success" } };

static const char* strerr() {
  int i;
  for (i=0; x[i].e; ++i)
    if (x[i].e==errno) return x[i].m;
  return "Unknown error";
}

int main(int argc,char* argv[]) {
  int fd;
  off_t o;
  char* buf;
  char* options;
  long olen;
  if (argc<2) {
    __write2("usage: insmod module.ko [arg1...]\n");
    return 0;
  }
  {
    int i;
    olen=0;
    for (i=2; argv[i]; ++i) {
      olen+=strlen(argv[i])+1;
      if (strchr(argv[i],' ')) olen+=2;
    }
    options=alloca(olen+2);
    olen=0;
    for (i=2; argv[i]; ++i) {
      char* q;
      if ((q=strchr(argv[i],' '))) options[olen++]='"';
      strcpy(options+olen,argv[i]);
      olen+=strlen(argv[i]);
      if (q) options[olen++]='"';
      options[olen]=' '; ++olen;
    }
    options[olen]=0;
  }
  if ((fd=open(argv[1],O_RDONLY))==-1 ||
      (o=lseek(fd,0,SEEK_END))==-1 ||
      (buf=mmap(0,o,PROT_READ,MAP_SHARED,fd,0))==MAP_FAILED) {
    __write2("insmod: ");
    __write2(strerr());
    __write2("\n");
    return 1;
  }
  if (init_module(buf,o,options)==-1) {
    __write2("insmod: ");
    __write2(errno==ENOENT?"Unknown symbol in module":strerr());
    __write2("\n");
    return 1;
  }
  return 0;
}
