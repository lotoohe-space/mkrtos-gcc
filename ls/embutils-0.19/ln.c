#include "embfeatures.h"
#ifdef SUPPORT_64BIT_FILESIZE
#define _FILE_OFFSET_BITS 64
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "write12.h"

#include "error.c"

#ifndef __dietlibc__
#define __NO_STAT64
#endif

static char* name="ln: ";

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
  case EACCES:		warn("permission denied"); break;
  case ELOOP:		warn("too many symbolic links"); break;
  case ENOSPC:		warn("no space left on device/quota exceeded"); break;
  case EXDEV:		warn("invalid cross-device link"); break;
  case EIO:		warn("I/O error"); break;
  default: warn("unknown error");
  }
  res=1;
}

void panic(const char *fn) {
  oops(fn);
  exit(1);
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

unsigned int str_rchr(const char *in, char needle) {
  register const char* t=in;
  register const char c=needle;
  register const char* found=0;
  for (;;) {
    if (!*t) break; if (*t==c) found=t; ++t;
    if (!*t) break; if (*t==c) found=t; ++t;
    if (!*t) break; if (*t==c) found=t; ++t;
    if (!*t) break; if (*t==c) found=t; ++t;
  }
  return (found?found:t)-in;
}

int f=0,s=0,v=0;

/* mustbedir=0 means "may be dir"
 * mustbedir=1 means "must be dir"
 * mustbedir=-1 means "must not be dir" */
int dolink(const char *src,const char *dest,int mustbedir) {
  char *d=(char*)dest;
  int idx=str_rchr(src,'/');
  const char *F=src[idx]?src+idx+1:src;
#ifdef __NO_STAT64
  struct stat ss,ss2;
  if (lstat(dest,&ss))
#else
  struct stat64 ss,ss2;
  if (lstat64(dest,&ss))
#endif
  {
    if (mustbedir>0)
      error("ln: when making multiple links, last argument must be a directory");
  } else {
    if (S_ISDIR(ss.st_mode)) {
      if (mustbedir<0) {
	int len;
	d=alloca(strlen(dest)+100);
	len=str_copy(d,"ln: cannot overwrite directory '");
	len+=str_copy(d+len,dest);
	str_copy(d+len,"'!\n");
	__write2(d);
	return 0;
      } else {
	char *e;
	d=alloca(strlen(dest)+strlen(F)+2);
	e=d+str_copy(d,dest);
	*e='/'; ++e;
	e+=str_copy(e,F);
	*e=0;
/*      printf("%slink %s to %s\n",s?"sym":"",src,d); */
	return dolink(src,d,-2);
      }
    }
    if (f) {
#ifdef __NO_STAT64
      if (s || !stat(src,&ss2)) {
#else
      if (s || !stat64(src,&ss2)) {
#endif
	if (!s &&
	    ss2.st_dev==ss.st_dev &&
	    ss2.st_ino==ss.st_ino &&
	    ss2.st_size==ss.st_size) {
	  char *tmp=alloca(strlen(src)+strlen(dest)+50);
	  int len;
	  len=str_copy(tmp,"ln: ");
	  len+=str_copy(tmp+len,src);
	  len+=str_copy(tmp+len," and ");
	  len+=str_copy(tmp+len,dest);
	  len+=str_copy(tmp+len," are the same file.\n");
	  __write2(tmp);
	  return 0;
	} else
	  unlink(dest);
      }
    }
  }
  if (v) {
    char *tmp=alloca(strlen(src)+strlen(dest)+50);
    int len;
    len=str_copy(tmp,"\"");
    len+=str_copy(tmp+len,src);
    len+=str_copy(tmp+len,s?"\" -> \"":"\" => \"");
    len+=str_copy(tmp+len,dest);
    len=str_copy(tmp+len,"\"\n");
    __write2(tmp);
  }
  {
    int tmp;
    if (s)
      tmp=symlink(src,dest);
    else
      tmp=link(src,dest);
    if (f) tmp=0;
    return tmp;
  }
}

main(int argc,char *argv[]) {
  int i,args,last;
  char *tmp;
  args=last=0;
  if (argc<2) {
    write(1,"usage: ln [-fs] source... destination\n",38);
    exit(0);
  }
  for (i=1; i<argc; ++i)
    if (argv[i][0]!='-') {
      last=i;
      ++args;
    }
  {
    for (i=1; argv[i][0]=='-'; ++i) {
      int j;
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'f': f=1; break;
	case 's': s=1; break;
	case 'v': v=1; break;
	}
      }
    }
  }
  switch (args) {
  case 1:
    tmp=strrchr(argv[i],'/');
    if (tmp) ++tmp; else tmp=argv[i];
    if (dolink(argv[i],tmp,0))
      oops(argv[i]);
    break;
  case 2:
    if (dolink(argv[i],argv[last],0))
      oops(argv[i]);
    break;
  default:
    while (i<argc-1) {
      tmp=strrchr(argv[i],'/');
      if (tmp && tmp[1]==0) *tmp=0;
      if (dolink(argv[i],argv[last],1))
	oops(argv[i]);
      ++i;
    }
  }
  return res;
}

