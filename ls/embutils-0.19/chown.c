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

static char* name="chown: ";

static enum {
  CHOWN,
  CHGRP
} mode;

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

#include "openreadclose.c"
#include "split.c"

// #include "ftw.c"

#define DONT_NEED_PASSWD
#define DONT_NEED_GECOS
#define DONT_NEED_DIR
#define DONT_NEED_SHELL
#include "mygetpwnam.c"

#define DONT_NEED_MEMBERS
#include "mygetgrnam.c"

static int co=0,cg=0;
static uid_t owner=0;
static gid_t group=(gid_t)-1;

static int c=0,v=0,h=0,f=0;

char *chg;

int dochown(const char* file,const struct stat *sb, int flag) {
  int d=0;
  uid_t o=sb->st_uid;
  uid_t g=sb->st_gid;
  if (co) o=owner;
  if (cg) g=group;
  if (flag==FTW_NS || o!=sb->st_uid || g!=sb->st_gid) {
    if (h?lchown(file,o,g):chown(file,o,g))
      if (!f)
	oops(file);
    if (c) d=1;
  }
  if (d || v) {
    if (mode==CHOWN)
      __write2("owner of ");
    else
      __write2("group of ");
    __write2(file);
    __write2(" changed to ");
    __write2(chg);
    __write2("\n");
  }
  return 0;
}

void parsegid(char *tmp) {
  struct group *g;
  if (tmp[0]>='0' && tmp[0]<='9')
    group=atoi(tmp);
  else if ((g=getgrnam(tmp)))
    group=g->gr_gid;
  else
    error2("group not found");
  cg=1;
}

void parseuid(char *tmp) {
  struct passwd *p;
  if (*chg>='0' && *chg<='9')
    owner=atoi(chg);
  else if ((p=getpwnam(chg))) {
    owner=p->pw_uid;
    group=p->pw_gid;
  } else
    error2("user not found");
  co=1;
}

int main(int argc,char *argv[]) {
  int i;
  int R=0;
  int count=0;
  char *me=strrchr(argv[0],'/');
  if (me) ++me; else me=argv[0];
  if (!strcmp(me,"chown"))
    mode=CHOWN;
  else if (!strcmp(me,"chgrp")) {
    mode=CHGRP; name="chgrp: ";
  } else
    error("argv[0] has to be chown or chgrp.");
  for (i=1; i<argc; i++) {
    if (!argv[i]) continue;
    if (argv[i][0]=='-') {
      int j;
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case 'c': c=1; break;
	case 'h': h=1; break;
	case 'f': f=1; break;
	case 'R': R=1; break;
	case 'v': v=1; break;
	}
      }
    } else {
      if (!count++) {
	char *tmp;
	chg=argv[i];
	if (mode==CHOWN) {
	  if ((tmp=strchr(chg,'.')) || (tmp=strchr(chg,':'))) {
	    *tmp=0;
	    if (tmp>chg)
	      /* owner given */
	      parseuid(chg);
	    if (!tmp[1]) {
	      if (co) {
		if (group==(gid_t)-1) {
		  struct passwd* p=getpwuid(owner);
		  if (p)
		    group=p->pw_gid;
		  else
		    error2("could not find login group for uid");
		}
	      } else
		error2("must specify user or owner");
	    } else
	      parsegid(tmp+1);
	  } else
	    parseuid(chg);
	  cg=1;
	} else
	  parsegid(chg);
      } else {
	struct stat sb;
	int flag;
	if (R) {
	  ftw(argv[i],dochown,65535);
	}
	if (!lstat(argv[i],&sb)) {
	  if (S_ISLNK(sb.st_mode)) flag=FTW_SL; else
	  if (S_ISDIR(sb.st_mode)) flag=FTW_D; else
	  flag=FTW_F;
	} else
	  flag=FTW_NS;
	dochown(argv[i],&sb,flag);
      }
    }
  }
  return res;
}

