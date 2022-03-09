/* cat [-suvet] [file...]
   "-" == stdin
   -s no error messages for nonexistant files
   -u (ignore)
   -v print nonprinting characters in a visible form
   -e print $ at new-line
   -t print ^I instead of tab
 */

#define _FILE_OFFSET_BITS 32
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include "write12.h"

static int s=0,v=0,e=0,t=0,res=0,called=0;

void warn(const char* filename, const char* message) {
  __write2("cat: ");
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
    case EACCES:	if (s==0) warn(filename,"directory search permissions denied"); break;
    case ELOOP:		warn(filename,"too many symbolic links"); break;
    case ENOSPC:	warn(filename,"no space left on device/quota exceeded"); break;
    case EIO:		warn(filename,"I/O error"); break;
    case EISDIR:	warn(filename,"is a directory"); break;
    case ENXIO:		warn(filename,"no such device or address"); break;
    default:		warn(filename,"unknown error");
  }
  res=-1;
}

void cat(const char* filename) {
  int len,len2,i;
  static char buf[256];
  static char buf2[1024];
  int fd;
  ++called;
  if (*filename=='-') fd=0; else fd=open(filename,O_RDONLY);
  if (fd<0) {
    error(filename);
    return;
  }
  for (;;) {
    len=read(fd,buf,512);
    if (len==0) break;
    if (len<0) {
      error(filename);
      break;
    }
    for (len2=i=0; i<len; ++i) {
      unsigned char c=buf[i];
      if (c=='\n') {
	if (e) { buf2[len2]='$'; ++len2; }
      } else if (c=='\t') {
	if (t) {
	  buf2[len2]='^'; ++len2;
	  c='I';
	}
      } else if (v && !isprint(c)) {
	if (c>127) {
	  buf2[len2]='M'; ++len2;
	  buf2[len2]='-'; ++len2;
	  c-=128;
	}
	if (c<32) {
	  buf2[len2]='^'; ++len2;
	  c+='@';
	}
      }
      buf2[len2]=c; ++len2;
    }
    if (write(1,buf2,len2)<len2) {
      warn(filename,"short write");
      break;
    }
  }
  if (fd) close(fd);
}

int main(int argc,char *argv[]) {
  int i;

  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      int j;
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case 'v': v=1; break;
	case 's': s=1; break;
	case 'e': e=1; break;
	case 't': t=1;
	case 'u': break;
	default: warn(0,"usage: cat [-vsetu] [filename...]"); return 1;
	}
      }
    }
  }
  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      if (argv[i][1]==0)
	cat("-");
    } else
      cat(argv[i]);
  }
  if (!called) cat("-");
  return (res);
}
