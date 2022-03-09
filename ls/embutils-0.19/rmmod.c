#define _LINUX_SOURCE
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "write12.h"

struct {
  int e;
  const char* m;
} x[] = {
  { ENOENT, "No such module found" },
  { EPERM, "Permission denied" },
  { EAGAIN, "Module is in use" },
  { 0, "Success" } };

static const char* strerr() {
  int i;
  for (i=0; x[i].e; ++i)
    if (x[i].e==errno) return x[i].m;
  return "Unknown error";
}

int main(int argc,char* argv[]) {
  int flags = O_EXCL | O_NONBLOCK;
  char* buf;
  int i,r=0;
  if (argc<2) {
usage:
    __write2("usage: rmmod [-fw] module [module...]\n");
    return 0;
  }
  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      int j;
      if (!strcmp(argv[i],"--force")) flags|=O_TRUNC; else
      if (!strcmp(argv[i],"--wait")) flags&= ~O_NONBLOCK; else
      if ((argv[i][1]=='-') && !argv[i][2]) { argv[i]=0; break; } else
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case 'f': flags|=O_TRUNC; break;
	case 'w': flags&= ~O_NONBLOCK; break;
	default: goto usage;
	}
      }
      argv[i]=0;
      if (j==1) break;
    }
  }
  for (i=1; i<argc; ++i) {
    if (argv[i])
      if (delete_module(argv[i],flags)==-1) {
	__write2("rmmod: ");
	__write2(strerr());
	__write2("\n");
	r=1;
      }
  }
  return r;
}
