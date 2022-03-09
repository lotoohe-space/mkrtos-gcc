#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "write12.h"

static void error(char *message) {
  __write2("rmdir: ");
  __write2(message);
  __write2("\n");
}

static int res=0;

void panic() {
  switch (errno) {
  case EPERM:
    error("permission denied"); break;
  case EEXIST:
    error("file exists"); break;
  case EFAULT:
    error("invalid pointer"); break;
  case ENAMETOOLONG:
    error("name too long"); break;
  case ENOENT:
    error("no such file or directory"); break;
  case ENOMEM:
    error("out of virtual memory"); break;
  case EROFS:
    error("read-only file system"); break;
  case EACCES:
    error("directory search permissions denied"); break;
  case ELOOP:
    error("too many symbolic links"); break;
  case ENOSPC:
    error("no space left on device/quota exceeded"); break;
  case EIO:
    error("I/O error"); break;
  case ENOTEMPTY:
    error("directory is not empty"); break;
  case ENOTDIR:
    error("not a directory"); break;
  case EINVAL:
    error("can't remove (parent of) current directory"); break;
  default:
    error("unknown error"); break;
  }
  res=1;
}

void dormdir(char *s,int v,int iexist) {
  if (rmdir(s))
    if (!iexist || errno!=EEXIST) { panic(); return; }
  if (errno==0) {
    if (v) {
      __write2("rmdir: removing directory, ");
      __write2(s);
      __write2("\n");
    }
  }
}

void minusp(char *s,int v) {
  char *t=strrchr(s,'/');
  do {
    dormdir(s,v,1);
    if (t==0) return;
    *t=0;
    t=strrchr(s,'/');
  } while (1);
}

int main(int argc,char *argv[]) {
  int i;
  int p=0,v=0;
  if (argc<2) {
    __write1("usage: rmdir [-vp] directory...\n");
    return 0;
  }
  for (i=1; i<argc; i++) {
    if (!argv[i]) continue;
    if (argv[i][0]=='-') {
      int j;
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'p': p=1; break;
	case 'v': v=1; break;
	}
      }
    } else {
      if (p) {
	minusp(argv[i],v);
      } else
	dormdir(argv[i],v,0);
    }
  }
  return res;
}

