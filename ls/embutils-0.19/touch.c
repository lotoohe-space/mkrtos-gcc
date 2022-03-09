#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <errno.h>
#include "write12.h"

#include "error.c"

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

/*
 * missing:
 *
 * -d DATASTRING			This is a parsing nightmare...
 */

static const char* errorill="touch: illegal timestamp";

void oops(const char *fn) {
  int err=errno;
  __write2(errorill);
  if (fn) { __write2(fn); __write2(": "); }
  switch (err) {
  case EPERM:		warn("permission denied"); break;
  case EEXIST:		warn("file exists"); break;
  case EFAULT:		warn("invalid pointer"); break;
  case ENAMETOOLONG:	warn("name too long"); break;
  case ENOENT:		warn("file not found"); break;
  case ENOTDIR:		warn("a component of the path is not a directory"); break;
  case ENOMEM:		warn("out of virtual memory"); break;
  case EROFS:		warn("read-only file system"); break;
  case EACCES:		warn("directory search permissions denied"); break;
  case ELOOP:		warn("too many symbolic links"); break;
  case ENOSPC:		warn("no space left on device/quota exceeded"); break;
  case EIO:		warn("I/O error"); break;
  default: warn("unknown error");
  }
}

void usage() {
  error("touch [-a|-m] [-c] [-r <file>|-t <[[CC]YY]MMDDhhmm[.ss]>] [--] FILE...");
}

time_t gen_timestamp(char *ts) {
  time_t ret;
  struct tm stm;
  char *s;
  int i;
  struct tm *ltm;
  for (s=ts;*s && *s!='.';++s);
  if ((s-ts)<8) {
err_out:
    error(errorill);
  }
  if (*s=='.') {
    if ((stm.tm_sec=atoi(s+1))>59) goto err_out;
    *s=0;
  }
  s-=2;
  if ((stm.tm_min=atoi(s))>59) goto err_out;
  *s=0;
  s-=2;
  if ((stm.tm_hour=atoi(s))>23) goto err_out;
  *s=0;
  s-=2;
  if (!(stm.tm_mday=atoi(s))) goto err_out;
  if (stm.tm_mday>31) goto err_out;
  *s=0;
  s-=2;
  if (!(stm.tm_mon=atoi(s))) goto err_out;
  if (stm.tm_mon>12) goto err_out;
  --stm.tm_mon;
  *s=0;
  ret=time(0);
  ltm=localtime(&ret);	/* now timezone is set */
  if (i=strlen(ts)) {
    if (i==4)
      stm.tm_year=atoi(ts)-1900;
    else if (i==2) {
      stm.tm_year=atoi(ts);
      if (stm.tm_year<70) stm.tm_year+=100;
    }
    else goto err_out;
  } else {
    stm.tm_year=ltm->tm_year;
  }
  if ((ret=mktime(&stm))==((time_t)-1)) goto err_out;
  ret-=timezone;	/* I don't know if this is only a cludge or a must... */
  return ret;
}

int main(int argc,char*argv[]) {
  int i,flag_create=1,flag_only_a=0,flag_only_m=0,timestamp=0;
  struct stat fst;
  char *fn;
  struct utimbuf utimestamp,ats;

  for(i=1;i<argc;++i) {
    if (argv[i][0]=='-') {
      if ((argv[i][1]=='-')&&(!argv[i][2])) {
	++i;
	break;
      }
      switch (argv[i][1]) {
      case 'c': flag_create=0; break;
      case 'a': ++flag_only_a; break;
      case 'm': ++flag_only_m; break;
      case 'r':
	if (timestamp) goto def;
	if (argv[i][2]) fn=&argv[i][2]; else fn=argv[++i];
	if (stat(fn,&fst)) {
	  oops(fn);
	  exit(1);
	}
	utimestamp.actime=fst.st_atime;
	utimestamp.modtime=fst.st_mtime;
	++timestamp;
	break;
      case 't':
	if (timestamp) goto def;
	if (argv[i][2]) fn=&argv[i][2]; else fn=argv[++i];
	utimestamp.actime=utimestamp.modtime=gen_timestamp(fn);
	++timestamp;
	break;
      default:
      def:
	usage();
	break;
      }
    }
    else break;
  }

  if ((i==argc)||(flag_only_a&&flag_only_m)) goto def;

  if (!timestamp && (flag_only_a||flag_only_m)) {
    utimestamp.actime=utimestamp.modtime=time(0);
    ++timestamp;
  }

  for(;i<argc;++i) {
    fn=argv[i];
    if (flag_create) {
      register int fd=open(fn,O_WRONLY|O_NONBLOCK|O_CREAT|O_NOCTTY|O_LARGEFILE,0666);
      if (fd!=-1) close(fd);
    }
    if (timestamp) {
      if (stat(fn,&fst)) goto err_next;
      if (flag_only_a) ats.modtime=fst.st_mtime;
      else ats.modtime=utimestamp.modtime;
      if (flag_only_m) ats.actime=fst.st_atime;
      else ats.actime =utimestamp.actime;
    }
    if (utime(fn,(timestamp)?&ats:0)) {
err_next:
      oops(fn);
    }
  }
}
