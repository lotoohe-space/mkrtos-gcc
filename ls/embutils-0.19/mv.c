#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#include "write12.h"

#include "error.c"

#ifdef __GLIBC__
#define __NO_STAT64
#endif

enum { CP, MV } mode;

static const char* name="mv: ";

mode_t mask;

static int res=0;

void oops(const char *fn) {
  __write2(name);
  if (fn) { __write2(fn); __write2(": "); }
  switch (errno) {
  case EPERM: case EACCES:	warn("permission denied"); break;
  case EEXIST:		warn("file exists"); break;
  case EFAULT:		warn("invalid pointer"); break;
  case ENAMETOOLONG:	warn("name too long"); break;
  case ENOENT:		warn("file not found"); break;
  case ENOMEM:		warn("out of virtual memory"); break;
  case EROFS:		warn("read-only file system"); break;
  case ELOOP:		warn("too many symbolic links"); break;
  case ENOSPC:		warn("no space left on device/quota exceeded"); break;
  case EIO:		warn("I/O error"); break;
  case EINVAL:		warn("can't make directory a subdirectory of itself"); break;
  case ENXIO:		warn("no such device or address"); break;
  case EBUSY:		warn("device or resource busy"); break;
  default: warn("unknown error");
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

void oops2(const char *func,const char *fn) {
  char *a=alloca(strlen(func)+strlen(fn)+10);
  int len;
  len=str_copy(a,func);
  len+=str_copy(a+len," '");
  len+=str_copy(a+len,fn);
  len+=str_copy(a+len,"'");
  a[len]=0;
  oops(a);
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

int p=0,r=0,f=0,v=0,I=0;
uid_t euid;

int perms(const char *file,struct stat *s) {
  int res=0;
  mode_t m=s->st_mode;
  if (mode==CP && !p) m&=mask;
  if (chmod(file,m)) { res=1; oops2("chmod",file); }
  if (!p) return 0;
  {
    struct utimbuf buf;
    buf.actime=s->st_atime;
    buf.modtime=s->st_mtime;
    if (utime(file,&buf)) { res=1; oops2("utime",file); }
  }
  if (chown(file,s->st_uid,s->st_gid))
    if (!euid) { res=1; oops2("chown",file); }
  return res;
}

void verbose(const char* src,const char* dest) {
  if (v) {
    char *tmp=alloca(strlen(src)+strlen(dest)+50);
    int len;
    len=str_copy(tmp,"\"");
    len+=str_copy(tmp+len,src);
    len+=str_copy(tmp+len,mode==MV?"\" -> \"":"\" => \"");
    len+=str_copy(tmp+len,dest);
    len=str_copy(tmp+len,"\"\n");
    __write2(tmp);
  }
}

int copymove(const char *src,const char *dest) {
  int sfd=-1,dfd=-1;
  struct stat ss;
  int res=0;
/*  printf("inside copymove(\"%s\",\"%s\");\n",src,dest); */
  if (stat(src,&ss)) {
    char buf[1024];
    if ((res=readlink(src,buf,1024))>=0) {
      buf[res]=0;
      if (symlink(buf,dest)) {
	oops2("symlink",dest);
	return -1;
      }
      if (mode==MV) { if (unlink(src)) oops2("unlink",src); }
      return 0;
    }
    goto errorclose;
  }
  if (S_ISDIR(ss.st_mode)) {
    struct dirent *d;
    DIR *D;
    if (!r) {
      int len;
      char *d=alloca(strlen(dest)+100);
      len=str_copy(d,name);
      len+=str_copy(d+len,dest);
      str_copy(d+len," is a directory.\n");
      __write2(d);
      return 0;
    }
/*    printf("mkdir(\"%s\");\n",dest); */
    if (mkdir(dest,0700)) return -1;
    if (!(D=opendir(src))) return -1;
    while ((d=readdir(D))) {
      if (d->d_name[0]=='.')
	if ((d->d_name[1]=='.' && d->d_name[2]==0) || d->d_name[1]==0)
	  continue;
      {
	char *a=alloca(strlen(src)+strlen(d->d_name)+4);
	char *b=alloca(strlen(dest)+strlen(d->d_name)+4);
	int len;
	len=str_copy(a,src); a[len]='/'; ++len;
	len+=str_copy(a+len,d->d_name); a[len]=0;
	len=str_copy(b,dest); b[len]='/'; ++len;
	len+=str_copy(b+len,d->d_name); b[len]=0;
/*	printf("copymove(\"%s\",\"%s\");\n",a,b); */
	verbose(a,b);
	copymove(a,b);
      }
    }
    closedir(D);
    perms(dest,&ss);
    if (mode==MV && rmdir(src)) oops2("rmdir",src);
    return 0;
  }
  if ((sfd=open(src,O_RDONLY))<0) return -1;
  if ((dfd=open(dest,O_WRONLY|O_CREAT|O_TRUNC,0600))<0) goto errorclose;
  {
    if (ss.st_size>(1ul<<31) || !S_ISREG(ss.st_mode)) {	/* large file */
normalcopy:
      {
	char buf[16*1024];
	int len;
	for (;;) {
	  len=read(sfd,buf,16*1024);
	  if (write(dfd,buf,len)!=len) { res=1; oops("short write"); break; }
	  if (len<16*1024) break;
	}
      }
    } else {
#ifdef __linux__
#undef off_t
      off_t ofs;
      ofs=0;
      if (sendfile(dfd,sfd,&ofs,ss.st_size)!=ss.st_size)
#endif
	goto normalcopy;
    }
  }
  close(sfd);
  if (close(dfd)) { res=1; oops2("close",dest); }
  if (perms(dest,&ss)) res=1;
  if (res==0 && mode==MV) { if (unlink(src)) oops2("unlink",src); }
  return 0;
errorclose:
  {
    int tmp=errno;
    close(sfd);
    close(dfd);
    errno=tmp;
  }
  return -1;
}

/* mustbedir=0 means "may be dir"
 * mustbedir=1 means "must be dir"
 * mustbedir=-1 means "must not be dir" */
int domove(char *src,const char *dest,int mustbedir) {
  char *d=(char*)dest;
  int idx=str_rchr(src,'/');
  const char *F;
  struct stat ss,ss2;
  if (src[idx]=='/' && src[idx+1]==0) {
    src[idx]=0;
    while (idx>0 && src[idx]!='/') --idx;
  }
  F=src[idx]?src+idx:src;
  if (stat(dest,&ss)) {
    if (mustbedir>0) {
      __write2(name);
      error("when moving several files, last argument must be a directory");
    }
  } else {
    int ok=0;
    if (S_ISDIR(ss.st_mode)) {
      if (mustbedir<0) {
	int len;
	d=alloca(strlen(dest)+100);
	len=str_copy(d,name);
	len+=str_copy(d+len,"cannot overwrite directory '");
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
	return domove(src,d,-1);
      }
    }
    if (I && !f) {
      char buf[10];
      write(1,dest,strlen(dest));
      write(1," exists.  Overwrite? ",22);
      if (read(0,buf,9)>=1)
	if (buf[0]=='y' || buf[0]=='Y')
	  ok=1;
      if (!ok)
	return 0;
    }
    if (f || I || mode==CP) {
      if (!stat(src,&ss2)) {
	if (ss2.st_dev==ss.st_dev &&
	    ss2.st_ino==ss.st_ino &&
	    ss2.st_size==ss.st_size) {
	  char *tmp=alloca(strlen(src)+strlen(dest)+50);
	  int len;
	  len=str_copy(tmp,name);
	  len+=str_copy(tmp+len,src);
	  len+=str_copy(tmp+len," and ");
	  len+=str_copy(tmp+len,dest);
	  len+=str_copy(tmp+len," are the same file.\n");
	  __write2(tmp);
	  return 0;
	} else
	  unlink(dest);
      } else
	if (mode==CP)
	  unlink(dest);
    }
  }
  verbose(src,dest);
  if (mode==CP)
    return copymove(src,dest);
  {
    register int res;
    if ((res=rename(src,dest))<0) {
      if (errno==EXDEV) {
	return copymove(src,dest);
      }
    }
    return res;
  }
}

void usage() {
  if (mode==MV)
    warn("usage: mv [-fvi] source... destination");
  else
    warn("usage: cp [-prfvi] source... destination");
  exit(0);
}

int main(int argc,char *argv[]) {
  int k,first,num,last;

  char *me=strrchr(argv[0],'/');
  euid=geteuid();
  mask=0777-umask(0);
  if (me) ++me; else me=argv[0];
  if (!strcmp(me,"mv"))
    mode=MV;
  else if (!strcmp(me,"cp")) {
    mode=CP; name="cp: ";
  } else
    error("argv[0] has to be cp or mv.");

  if (argc<2) usage();
  first=num=last=0;
  for (k=1; k<argc; ++k) {
    if (argv[k][0]=='-') {
      unsigned int j;
      for (j=1; j<strlen(argv[k]); j++) {
	switch (argv[k][j]) {
	case 'f': f=1; break;
	case 'v': v=1; break;
	case 'i': I=1; break;
	case 'p': p=1; break;
	case 'r': r=1; break;
	}
      }
    } else {
      if (first==0) first=k;
      ++num;
      last=k;
    }
  }
  if (num<1) usage();
  if (mode==MV) r=p=1;
  switch (num) {
  case 1:
    if (domove(argv[first],".",0))
      oops(errno!=EEXIST?argv[first]:".");
    break;
  case 2:
    if (domove(argv[first],argv[last],0))
      oops(errno!=EEXIST?argv[first]:argv[last]);
    break;
  default:
    while (num>1) {
      if (argv[first][0]!='-') {
	if (domove(argv[first],argv[last],1))
	  oops(errno!=EEXIST?argv[first]:argv[last]);
	--num;
      }
      ++first;
    }
  }
  return res;
}

