/*
 * install [options] [-s] [--strip] source dest
 * install [options] [-s] [--strip] source... directory
 * install [options] [-d,--directory] directory...
 *   [-c] [-D] [-g group] [-m mode] [-o owner] [--]
 *     -c     ignored
 *     -D     implicit mkdir -d for missing path components
 *     -g x   chgrp to x
 *     -o x   chown to x
 *     -m x   chmod to x
 *     -s     strip installed binaries
 */

#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <utime.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "write12.h"

int d=0,D=0,g=-1,o=-1,s=0,v=0,p=0,res=0,b=0;
int mode=0755;

static void myname() {
  __write2("install: ");
}

void warn(const char* filename, const char* message) {
  myname();
  if (filename) {
    __write2(filename);
    __write2(": ");
  }
  __write2(message);
  __write2("\n");
}

void error(const char* message) {
  warn(0,message);
  exit(1);
}

void errormsg(const char* filename) {
  switch (errno) {
    case EPERM:		warn(filename,"permission denied"); break;
    case ENAMETOOLONG:	warn(filename,"name too long"); break;
    case ENOENT:	warn(filename,"file not found"); break;
    case EACCES:	warn(filename,"permission denied"); break;
    case ELOOP:		warn(filename,"too many symbolic links"); break;
    case EROFS:		warn(filename,"read-only file system"); break;
    case EISDIR:	warn(filename,"is a directory"); break;
    case EIO:		warn(filename,"I/O error"); break;
    case EEXIST:	warn(filename,"file exists"); break;
    default:		warn(filename,"unknown error");
  }
  res=1;
}

void oops2(const char* syscall,const char* filename) {
  char* buf=alloca(strlen(syscall)+strlen(filename)+5);
  strcpy(buf,syscall);
  strcat(buf,": ");
  strcpy(buf,filename);
  errormsg(buf);
}

#define EMBEDDED
#include "parsemode.c"
#include "openreadclose.c"
#include "split.c"
#define DONT_NEED_PASSWD
#define DONT_NEED_GECOS
#define DONT_NEED_DIR
#define DONT_NEED_SHELL
#include "mygetpwnam.c"
#define DONT_NEED_MEMBERS
#include "mygetgrnam.c"

int usage() {
  __write2("usage: install [options] [-s] source dest\n"
	 "       install [options] [-s] source... directory\n"
	 "       install [options] [-d] directory...\n"
	 "options == [-pcD] [-g group] [-m mode] [-o owner] [--]\n");
  exit(1);
}

uid_t parseuid(const char*c) {
  if (isdigit(*c))
    return atoi(c);
  else {
    struct passwd* p=getpwnam(c);
    return p?p->pw_uid:0;
  }
}

gid_t parsegid(const char*c) {
  if (isdigit(*c))
    return atoi(c);
  else {
    struct group* g=getgrnam(c);
    return g?g->gr_gid:0;
  }
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

void domkdir(char *s,int m,int iexist,char *full) {
  errno=0;
  if (*s!=0 && mkdir(s,m))
    if (!iexist || errno!=EEXIST) { errormsg(s); return; }
  if (errno==0) {
    if (v) {
      myname();
      __write2("created directory `");
      __write2(full);
      __write2("'\n");
    }
  }
}

void minusp(char *s,int m) {
  int fd=open(".",O_RDONLY);
  char *t;
  char *full=s;
  if (fd<0) { errormsg("."); return; }
  if (*s=='/') {
    chdir("/");
    ++s;
  }
  while ((t=strchr(s,'/'))) {
    *t=0;
    domkdir(s,m,1,full);
    chdir(s);
    s=t+1;
    *t='/';
  }
  fchdir(fd);
  close(fd);
}

int copymove(const char *src,const char *dest) {
  int sfd=-1,dfd=-1;
  struct stat ss;
  int res=0;
  if ((sfd=open(src,O_RDONLY))<0) { errormsg(src); return -1; }
  if (fstat(sfd,&ss)) oops2("fstat",src);
  if ((dfd=open(dest,O_WRONLY|O_CREAT|O_TRUNC|O_EXCL,0600))<0) goto errorclose;
  {
    if (ss.st_size>(1ul<<31) || !S_ISREG(ss.st_mode)) {	/* large file */
normalcopy:
      {
	char buf[16*1024];
	int len;
	for (;;) {
	  len=read(sfd,buf,16*1024);
	  if (write(dfd,buf,len)!=len) { res=1; warn(dest,"short write"); break; }
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
  if (s) {
#ifdef __linux__
    const char *argv[]={"strip","-R",".comment","-R",".note",dest,0};
#else
    const char *argv[]={"strip",dest,0};
#endif
    switch (fork()) {
    case -1: warn(0,"could not fork"); exit(1);
    case 0: /* child */
      execvp(argv[0],(char**)argv);
      warn(0,"execvp failed!"); exit(1);
    }
    wait(0);
  }
  if (o>=0 || g>=0) if (chown(dest,o,g)) { res=1; oops2("chown",dest); }
  if (chmod(dest,mode)) { res=1; oops2("chmod",dest); }
  if (p) {
    struct utimbuf ub;
    ub.actime=ss.st_atime;
    ub.modtime=ss.st_mtime;
    if (utime(dest,&ub)) { res=1; oops2("utime",dest); }
  }
  return 0;
errorclose:
  {
    errormsg(dest);
    if (sfd>=0) close(sfd);
    if (dfd>=0) close(dfd);
  }
  return -1;
}

/* mustbedir=0 means "may be dir"
 * mustbedir=1 means "must be dir"
 * mustbedir=-1 means "must not be dir" */
int doinstall(char *src,const char *dest,int mustbedir) {
  char *d=(char*)dest;
  int idx=str_rchr(src,'/');
  const char *F;
  struct stat ss,ss2;
  if (src[idx]=='/' && src[idx+1]==0) {
    src[idx]=0;
    while (idx>0 && src[idx]!='/') --idx;
  }
  F=src[idx]?src+idx+1:src;
  if (stat(dest,&ss)) {
    if (mustbedir>0) {
      oops2(dest,"last argument must be a directory");
      return 0;
    }
  } else {
    int ok=0;
    if (S_ISDIR(ss.st_mode)) {
      if (mustbedir<0) {
	int len;
	d=alloca(strlen(dest)+100);
	myname();
	len=str_copy(d+len,"cannot overwrite directory '");
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
	return doinstall(src,d,-1);
      }
    }
    if (!stat(src,&ss2)) {
      if (ss2.st_dev==ss.st_dev &&
	  ss2.st_ino==ss.st_ino &&
	  ss2.st_size==ss.st_size) {
	char *tmp=alloca(strlen(src)+strlen(dest)+50);
	int len;
	myname();
	len=str_copy(tmp+len,src);
	len+=str_copy(tmp+len," and ");
	len+=str_copy(tmp+len,dest);
	len+=str_copy(tmp+len," are the same file.\n");
	__write2(tmp);
	return 0;
      } else {
	if (b) {
	  size_t l=strlen(dest);
	  char* bakfilename=alloca(l+2);
	  memcpy(bakfilename,dest,l);
	  bakfilename[l]='~';
	  bakfilename[l+1]=0;
	  if (rename(dest,bakfilename)==-1) {
	    if (errno!=ENOENT) {
	      char* x=alloca(l+l+15);
	      strcpy(x,"backup of ");
	      strcat(x,dest);
	      errormsg(x);
	      return -1;
	    }
	  }
	}
	unlink(dest);
      }
    }
  }
  if (v) {
    char *tmp=alloca(strlen(src)+strlen(dest)+50);
    int len;
    len=str_copy(tmp,"\"");
    len+=str_copy(tmp+len,src);
    len+=str_copy(tmp+len,"\" -> \"");
    len+=str_copy(tmp+len,dest);
    len=str_copy(tmp+len,"\"\n");
    __write2(tmp);
  }
  return copymove(src,dest);
}


int main(int argc,char *argv[]) {
  int k,first,num,last;
  mode_t and,or;

  if (argc<2) usage();
  first=num=last=0;
  for (k=1; k<argc; ++k) {
    if (argv[k][0]=='-') {
      unsigned int j;
      for (j=1; j<strlen(argv[k]); j++) {
	switch (argv[k][j]) {
	case 'd': d=1;
	case 'D': D=1;
	case 'c': break;
	case 's': s=1; break;
	case 'v': v=1; break;
	case 'p': p=1; break;
	case 'b': b=1; break; /* backup */
	case 'm':
		  if (argv[k][j+1]) {
		    parsemode(argv[k]+j+1,&and,&or);
		    mode=(mode&and)|or;
		    goto fini;
		  } else {
minusm:
		    if (!argv[k+1]) usage();
		    parsemode(argv[k+1],&and,&or);
		    mode=(mode&and)|or;
parsed:		    argv[++k]=0;
fini:		    argv[k]=0;
		    goto done;
		  }
		  break;
	case 'o':
		  if (argv[k][j+1]) {
		    o=parseuid(argv[k]+j+1);
		    goto fini;
		  } else {
minuso:
		    if (!argv[k+1]) usage();
		    o=parseuid(argv[k+1]);
		    goto parsed;
		  }
		  break;
	case 'g':
		  if (argv[k][j+1]) {
		    g=parseuid(argv[k]+j+1);
		    goto fini;
		  } else {
minusg:
		    if (!argv[k+1]) usage();
		    g=parseuid(argv[k+1]);
		    goto parsed;
		  }
		  break;
	case '-': /* double dash.  May those GNU idiots rot in hell */
		  {
		    char *x=argv[k]+2;
		    j=strlen(argv[k]);
		    if (!strcmp(x,"strip")) {
		      s=1; break;
		    } else if (!strcmp(x,"mode"))
		      goto minusm;
		    else if (!strcmp(x,"owner"))
		      goto minuso;
		    else if (!strcmp(x,"group"))
		      goto minusg;
		    else if (!strcmp(x,"directory")) {
		      d=D=1; break;
		    }
		  }
	default: usage();
	}
      }
      argv[k]=0;
    } else {
      if (first==0) first=k;
      ++num;
      last=k;
    }
done: ;
  }
  if (d) {
    /* directory mode. */
    int i;
    for (i=1; i<argc; ++i)
      if (argv[i]) {
	minusp(argv[i],0755);
	domkdir(argv[i],mode,1,argv[i]);
      }
    return 0;
  }
  if (num<2) usage();
  switch (num) {
  case 1:
    doinstall(argv[first],".",0);
    break;
  case 2:
    if (D) minusp(argv[last],0755);
    doinstall(argv[first],argv[last],0);
    break;
  default:
    if (D) minusp(argv[last],0755);
    while (num>1) {
      if (argv[first] && argv[first][0]!='-') {
	doinstall(argv[first],argv[last],1);
	--num;
      }
      ++first;
    }
  }
  return res;
}

