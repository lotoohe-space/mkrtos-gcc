#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "write12.h"

char* Name="mknod: ";

static void oops(char *message) {
  __write2(Name);
  __write2(message);
  __write2("\n");
}

static void error(char *message) {
  oops(message);
  exit(1);
}

#include "parsemode.c"

void panic() {
  switch (errno) {
  case EPERM:
    oops("permission denied"); break;
  case EEXIST:
    oops("file exists"); break;
  case EFAULT:
    oops("invalid pointer"); break;
  case ENAMETOOLONG:
    oops("name too long"); break;
  case ENOENT:
    oops("file not found"); break;
  case ENOMEM:
    oops("out of virtual memory"); break;
  case EROFS:
    oops("read-only file system"); break;
  case EACCES:
    oops("directory search permissions denied"); break;
  case ELOOP:
    oops("too many symbolic links"); break;
  case ENOSPC:
    oops("no space left on device/quota exceeded"); break;
  case EIO:
    oops("I/O error"); break;
  default:
    oops("unknown error"); break;
  }
}

void usage() {
  error("usage: mknod [-m mode] filename type major minor\n"
	"       mkfifo [-m mode] filename\n"
	"type == {b|c|u|p}");
}

int main(int argc,char *argv[]) {
  int i;
  enum { MKNOD, MKFIFO } me;
  int mode=0666-umask(0);
  mode_t and,or;
  unsigned int count=0;
  char *name=0;
  char type=0;
  unsigned short major=0,minor=0;
  dev_t dev;

  {
    char *Me=strrchr(argv[0],'/');
    if (Me) ++Me; else Me=argv[0];
    if (!strcmp(Me,"mknod"))
      me=MKNOD;
    else if (!strcmp(Me,"mkfifo")) {
      me=MKFIFO; name="mkfifo: ";
    } else
      error("argv[0] has to be mknod or mkfifo.");
  }
  for (i=1; i<argc; i++) {
    if (!argv[i]) continue;
    if (argv[i][0]=='-') {
      int j;
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'm': parsemode(argv[i+1],&and,&or); mode=(mode&and)|or; argv[i+1]=0; break;
	}
      }
    } else {
      if (me==MKFIFO) break;
      switch (++count) {
      case 1: name=argv[i]; break;
      case 2: if (strlen(argv[i])!=1) error("[bcup] expected"); type=argv[i][0]; break;
      case 3: major=atoi(argv[i]); break;
      case 4: minor=atoi(argv[i]); break;
      case 5: usage();
      }
    }
  }
  if (me==MKFIFO) ++count, type='p';
  if (type=='p') count+=3;
  if (count<4) usage();
  dev=makedev(major,minor);
  switch (type) {
  case 'b': mode|=S_IFBLK; break;
  case 'c': case 'u': mode|=S_IFCHR; break;
  case 'p': mode|=S_IFIFO; break;
  }
  if (me==MKFIFO) {
    for (; i<argc; ++i) {
      if (argv[i][0]=='-')
	++i;
      else
	if (mknod(argv[i],mode,0)) panic();
    }
    return 0;
  }
  if (mknod(name,mode,dev)) panic();
  return 0;
}

