/* wc [-clLw] [file...]
   -c chars
   -l lines
   -w words
   -L longest line length
 */

#define _FILE_OFFSET_BITS 64

/* uncomment the following line to get long long precision */
#define WANT_LONG_LONG

#ifdef WANT_LONG_LONG
typedef unsigned long long accumulator;
#define _FILE_OFFSET_BITS 64
#else
typedef unsigned long accumulator;
#endif

#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include "write12.h"

static int c=0,l=0,w=0,L=0,res=0,called=0;

void warn(const char* filename, const char* message) {
  __write2("wc: ");
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
    case ENOENT:	warn(filename,"file not found"); break;
    case EACCES:	warn(filename,"directory search permissions denied"); break;
    case ELOOP:		warn(filename,"too many symbolic links"); break;
    case EISDIR:	warn(filename,"is a directory"); break;
    case EIO:		warn(filename,"I/O error"); break;
    default:		warn(filename,"unknown error");
  }
  res=1;
}

int usage() {
  warn(0,"usage: wc [-cwlL] [filename...]");
  return 1;
}

unsigned int fmt_accu(char *dest,accumulator i) {
  register unsigned long len,len2;
  register accumulator tmp;
  for (len=1, tmp=i; tmp>9; ++len) tmp/=10;
  if (dest)
    for (tmp=i, dest+=len, len2=len+1; --len2; tmp/=10)
      *--dest = (tmp%10)+'0';
  return len;
}

long long tlines=0,tchars=0,twords=0,tmaxlen=0;

void dowc(int fd) {
  char buf[4096];
  int i,len;
  int wasword=0;
  accumulator lines=0,chars=0,words=0,thislen=0,maxlen=0;
  for (;;) {
    if (!(len=read(fd,buf,4096))) break;
    chars+=len;
    for (i=0; i<len; ++i) {
      if (buf[i]=='\n') {
	if (thislen>maxlen) maxlen=thislen;
	thislen=0;
	++lines;
      } else
	++thislen;
      if (isspace(buf[i])) {
	if (wasword)
	  wasword=0;
      } else
	if (!wasword) {
	  ++words;
	  wasword=1;
	}
    }
  }
  i=0;
  if (l) {
    i=fmt_accu(buf,lines);
  }
  if (w) {
    if (i) { buf[i]=' '; ++i; }
    i+=fmt_accu(buf+i,words);
  }
  if (c) {
    if (i) { buf[i]=' '; ++i; }
    i+=fmt_accu(buf+i,chars);
  }
  if (L) {
    if (i) { buf[i]=' '; ++i; }
    i+=fmt_accu(buf+i,maxlen);
  }
  write(1,buf,i);
  twords+=words; tlines+=lines; tchars+=chars;
  if (tmaxlen<maxlen) tmaxlen=maxlen;
  ++called;
}

void wc(const char *filename) {
  char *buf;
  int len=strlen(filename);
  int fd=open(filename,O_RDONLY);
  if (fd>=0) {
    dowc(fd);
    buf=alloca(len+2);
    buf[0]=' ';
    strcpy(buf+1,filename);
    ++len;
    buf[len]='\n';
    write(1,buf,len+1);
    close(fd);
  } else
    error(filename);
}

int main(int argc,char *argv[]) {
  int i;
  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      int j;
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case 'c': c=1; break;
	case 'w': w=1; break;
	case 'l': l=1; break;
	case 'L': L=1; break;
	default: return usage();
	}
      }
      argv[i]=0;
    }
  }
  if (!c && !w && !l && !L) c=w=l=1;
  for (i=1; i<argc; ++i)
    if (argv[i])
      wc(argv[i]);
  if (!called && res==0) {
    dowc(0);
    __write1("\n");
  }
  if (called>1) {
    char buf[512];
    int i;
    i=0;
    if (l) {
      i=fmt_accu(buf,tlines);
    }
    if (w) {
      if (i) { buf[i]=' '; ++i; }
      i+=fmt_accu(buf+i,twords);
    }
    if (c) {
      if (i) { buf[i]=' '; ++i; }
      i+=fmt_accu(buf+i,tchars);
    }
    if (L) {
      if (i) { buf[i]=' '; ++i; }
      i+=fmt_accu(buf+i,tmaxlen);
    }
    strcpy(buf+i," total\n");
    __write1(buf);
  }
  return res;
}
