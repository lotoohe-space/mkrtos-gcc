/* rm [-fir] [-] [file...]
   "-" the rest of argv[] is files, even if they start with "-"
   -v verbose
   -f don't prompt for confirmation, don't check if the file is write protected
   -i ask for confirmation before removing files
   -r recursively removes files and directories
 */

#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include "write12.h"
#include <stdlib.h>

static int f=0,I=0,r=0,v=0,res=0,called=0;
uid_t u;
gid_t g;

void warn(const char* filename, const char* message) {
  __write2("rm: ");
  if (filename) {
    __write2(filename);
    __write2(": ");
  }
  __write2(message);
  __write2("\n");
}

void error(const char* filename) {
  switch (errno) {
    case EPERM:		warn(filename,"permission denied"); break;
    case ENAMETOOLONG:	warn(filename,"name too long"); break;
    case ENOENT:	if (!f) warn(filename,"file not found"); break;
    case EACCES:	warn(filename,"permission denied"); break;
    case ELOOP:		warn(filename,"too many symbolic links"); break;
    case EROFS:		warn(filename,"read-only file system"); break;
    case EISDIR:	warn(filename,"is a directory"); break;
    case EIO:		warn(filename,"I/O error"); break;
    case ENOTEMPTY:	warn(filename,"directory is not empty"); break;
    default:		warn(filename,"unknown error");
  }
  res=1;
}

unsigned int str_copy(char *out,const char *in) {
  register char* s=out;
  register const char* t=in;
  for (;;) {
    if (!(*s=*t)) break; ++s; ++t;
    if (!(*s=*t)) break; ++s; ++t;
    if (!(*s=*t)) break; ++s; ++t;
    if (!(*s=*t)) break; ++s; ++t;
  }
  return s-out;
}

void rm(const char* filename) {
  struct stat s;
  called=1;
  if (v) {
    __write2("deleting \"");
    __write2(filename);
    __write2("\"\n");
  }
  if (lstat(filename,&s)) { if (!f) error(filename); return; }
  {
    const char* c=strrchr(filename,'/');
    if (c) ++c; else c=filename;
    if (c[0]=='.' && (c[1]==0 || c[1]=='.' && c[2]==0)) {
      __write2("rm: cannot remove `.' or `..'\n");
      return;
    }
  }
  if (!f) {
    int writable=
	(s.st_uid==u && (s.st_mode&S_IWUSR)) ||
	(s.st_gid==g && (s.st_mode&S_IWGRP)) ||
	(s.st_mode&S_IWOTH);
    if (!u) writable=1;
    if (I || !writable) {
      char buf[10];
      if (!writable)
	__write2("rm: remove write-protected file \"");
      else
	__write2("rm: remove \"");
      __write2(filename);
      __write2("\"? ");
      if (read(0,buf,9)<1 || buf[0]!='y')
	return;
    }
  }
  if (unlink(filename)) {
    if (r && errno==EISDIR) {
      DIR* d=opendir(filename);
      struct dirent *de;
      if (!d) goto kaputt;
      while ((de=readdir(d))) {
	int len;
	char *buf;
	if (de->d_name[0]=='.')
	  if ((de->d_name[1]=='.' && de->d_name[2]==0) ||
	      de->d_name[1]==0)
	    continue;
	buf=alloca(strlen(filename)+strlen(de->d_name)+2);
	len=str_copy(buf,filename);
	buf[len]='/'; ++len;
	len+=str_copy(buf+len,de->d_name);
	rm(buf);
      }
      closedir(d);
      if (rmdir(filename)) goto kaputt;
    } else
kaputt:
      error(filename);
  }
}

int usage() {
  warn(0,"usage: rm [-vfir] [-] filename [filename...]");
  return 1;
}

int main(int argc,char *argv[]) {
  int i;

  u=geteuid();
  g=getegid();
  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      int j;
      if (!strcmp(argv[i],"--force")) f=1; else
      if (!strcmp(argv[i],"--interactive")) I=1; else
      if (!strcmp(argv[i],"--recursive")) r=1; else
      if ((argv[i][1]=='-') && !argv[i][2]) { argv[i]=0; break; } else
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case 'f': f=1; break;
	case 'i': I=1; break;
	case 'R':
	case 'r': r=1; break;
	case 'v': v=1; break;
	default: return usage();
	}
      }
      argv[i]=0;
      if (j==1) break;
    }
  }
  for (i=1; i<argc; ++i)
    if (argv[i])
      rm(argv[i]);
  if (!called && !f) usage();
  return (res);
}
