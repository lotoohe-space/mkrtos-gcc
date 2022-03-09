#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "write12.h"

extern int __syscall_syslog(int type, char *bufp, int len);

int main(int argc,char *argv[]) {
  char buf[16*1024];
  int len=__syscall_syslog(3,buf,16*1024);
  int i;
  char *t;
  for (i=1; i<argc; i++) {
    if (!argv[i]) continue;
    if (argv[i][0]=='-') {
      int j,level;
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'c':
	  if (__syscall_syslog(5,0,0)) {
	    __write2("Operation not permitted\n");
	    return 1;
	  }
	  break;
	case 'n':
	  level=atoi(argv[i+1]);
	  argv[i+1]=0;
	  if (__syscall_syslog(8,0,level)) {
	    __write2("Operation not permitted\n");
	    return 1;
	  }
	  return 0;
	}
      }
    }
  }
  for (t=buf; t && (t<buf+len); ) {
    char *cur;
    if (*t=='<') {
      do ++t; while (*t && *t!='>'); ++t;
    }
    cur=t;
    t=strchr(t,'\n');
    if (t)
      write(1,cur,t-cur+1);
    else
      break;
    ++t;
  }
  return 0;
}
