#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "write12.h"

#ifndef __GNUC__
#undef __attribute__
#define __attribute__(xyz)
#endif

void error() __attribute__((noreturn));
void panic() __attribute__((noreturn));

void error(char *message) {
  __write2(message);
  __write2("\n");
  exit(1);
}

void panic() {
  switch (errno) {
  case EPERM:
    error("permission denied");
  case EFAULT:
    error("invalid pointer");
  case ENAMETOOLONG:
    error("name too long");
  case ENOENT:
    error("file not found");
  case ENOMEM:
    error("out of virtual memory");
  case ENOTDIR:
    error("not a directory");
  case EACCES:
    error("directory search permissions denied");
  case ELOOP:
    error("too many symbolic links");
  case EIO:
    error("I/O error");
  }
  error("unknown error");
}

int main(int argc,char *argv[]) {
  if (chroot(argv[1]) || chdir("/"))
    panic();
  execvp(argv[2],argv+2);
  panic();
}
