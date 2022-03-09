/* head [-vq] [-c bytes] [-n lines] [files...]
   -v verbose, print "==> id.c <==" headers for each file
   -f quiet, never print headers
   -c n  print n bytes
   -n n  print n lines
 */

#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "write12.h"

static int q=0,v=0,dostdin=0,res=0;
static unsigned long l=10;

enum { LINES, BYTES } mode=LINES;

void warn(const char* filename, const char* message) {
  __write2("head: ");
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
    case EROFS:		warn(filename,"read-only file system"); break;
    case EISDIR:	warn(filename,"is a directory"); break;
    case EIO:		warn(filename,"I/O error"); break;
    default:		warn(filename,"unknown error");
  }
  res=1;
}

void dohead(const char* filename,int fd) {
  if (v) {
    __write2("==> ");
    __write2(filename);
    __write2(" <==\n");
  }
  if (mode==BYTES) {
    unsigned long todo=l;
    char buf[32768];
    while (todo>32767) {
      int len=read(fd,buf,32768);
      if (len) write(1,buf,len);
      todo-=len;
      if (len<32768) { todo=0; break; }
    }
    if (todo) {
      int len=read(fd,buf,todo);
      if (len) write(1,buf,len);
    }
  } else {
    char buf[4096];
    unsigned int len,i;
    char cur=0;
    for (;;) {
      len=read(fd,buf,4096);
      if (len<=0) {
	if (len<0) error(filename);
	break;
      }
      for (i=0; i<len; ++i) {
	if (buf[i]=='\n') { ++cur; if (cur>=l) break; }
      }
      if (cur>=l) {
	write(1,buf,i+1);
	break;
      }
      write(1,buf,len);
    }
  }
}

void head(const char* filename) {
  int fd;
  if ((fd=open(filename,O_RDONLY))<0)
    error(filename);
  else {
    dohead(filename,fd);
    close(fd);
  }
}

int usage() {
  warn(0,"usage: head [-vq] [-c bytes] [-n lines] [filename...]");
  exit(1);
}

void n2a(const char*c) {
  l=0;
  while (isdigit(*c)) {
    l=l*10+*c-'0';
    ++c;
  }
  if (*c) mode=BYTES;
  switch (*c) {
  case 'b': l*=512; break;
  case 'k': l*=1024; break;
  case 'm': l*=1024*1024; break;
  case 0: break;
  default: usage();
  }
}

int main(int argc,char *argv[]) {
  int i;

  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      int j;
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  n2a(argv[i]+j);
	  argv[i][j+1]=0;
	  break;
	case 'n': mode=LINES; goto eatn;
	case 'c': mode=BYTES;
eatn:
		  if (argv[i][j+1])
		    n2a(argv[i]+j+1);
		  else {
		    if (!argv[i+1] || !isdigit(argv[i+1][0])) usage();
		    n2a(argv[i+1]);
		    argv[i]=0;
		    argv[++i]=0;
		    goto done;
		  }
		  break;
	case 'v': v=1; break;
	case 'q': q=1; break;
	default: return usage();
	}
      }
      argv[i]=0;
      if (j==1) ++dostdin;
    }
done: ;
  }
  {
    int count=0;
    for (i=1; i<argc; ++i)
      if (argv[i])
	++count;
    if (!count) { ++count; ++dostdin; }
    if (count>1) v=1;
    if (q) v=0;
  }
  for (i=1; i<argc; ++i)
    if (argv[i])
      head(argv[i]);
  while (dostdin) { dohead("standard input",0); --dostdin; }
  return (res);
}
