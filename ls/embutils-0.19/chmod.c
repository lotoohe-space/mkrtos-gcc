#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/stat.h>
#include "write12.h"

#include "error.c"

static char* name="chmod: ";

static int res=0;

void oops(const char *fn) {
  __write2(name);
  if (fn) { __write2(fn); __write2(": "); }
  switch (errno) {
  case EPERM:		warn("permission denied"); break;
  case EEXIST:		warn("file exists"); break;
  case EFAULT:		warn("invalid pointer"); break;
  case ENAMETOOLONG:	warn("name too long"); break;
  case ENOENT:		warn("file not found"); break;
  case ENOMEM:		warn("out of virtual memory"); break;
  case EROFS:		warn("read-only file system"); break;
  case EACCES:		warn("directory search permissions denied"); break;
  case ELOOP:		warn("too many symbolic links"); break;
  case ENOSPC:		warn("no space left on device/quota exceeded"); break;
  case EIO:		warn("I/O error"); break;
  default: warn("unknown error");
  }
  res=1;
}

void panic(const char *fn) {
  oops(fn);
  exit(1);
}

void error2(const char *msg) {
  __write2(name);
  error(msg);
}

#define EMBEDDED

// #include "ftw.c"

#include "parsemode.c"

static int c=0,v=0,h=0,f=0;

static mode_t and,or;

char *chg;

int dochmod(const char* file,const struct stat *sb, int flag) {
  int d=0;
  mode_t m=((sb->st_mode&and)|or);
  if (flag==FTW_NS || sb->st_mode != m) {
    if (chmod(file,m))
      if (!f)
	oops(file);
    if (c) d=1;
  }
  if (d || v) {
    __write2("mode of ");
    __write2(file);
    __write2(" changed to ");
    __write2(chg);
    __write2("\n");
  }
  return 0;
}

int main(int argc,char *argv[]) {
  int i;
  int R=0;
  int count=0;
  for (i=1; i<argc; i++) {
    if (!argv[i]) continue;
    if (argv[i][0]=='-') {
      int j;
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'c': c=1; break;
	case 'h': h=1; break;
	case 'f': f=1; break;
	case 'R': R=1; break;
	case 'v': v=1; break;
	case 'r': case 'w': case 'x': case 's': case 't':
	  chg=argv[i];
	  parsemode(argv[i],&and,&or);
	  ++count;
	}
      }
    } else {
      if (!count++) {
	chg=argv[i];
	parsemode(argv[i],&and,&or);
      } else {
	struct stat sb;
	int flag;
	if (R)
	  ftw(argv[i],dochmod,65535);
	if (!lstat(argv[i],&sb)) {
	  if (S_ISLNK(sb.st_mode)) flag=FTW_SL; else
	  if (S_ISDIR(sb.st_mode)) flag=FTW_D; else
	  flag=FTW_F;
	} else
	  flag=FTW_NS;
	dochmod(argv[i],&sb,flag);
      }
    }
  }
  return res;
}

