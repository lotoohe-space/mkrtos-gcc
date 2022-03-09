#define _FILE_OFFSET_BITS 64
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "write12.h"
#include <stdlib.h>

int main(int argc,char *argv[]) {
  int append=0;
  int i,k;
  int *fds=alloca((argc)*sizeof(int));
  fds[0]=1;
  for (i=1,k=0; i<argc; i++) {
    int j;
    if (argv[i][0]=='-') {
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'i': signal(SIGINT,SIG_IGN); break;
	case 'a': append=1; break;
	}
      }
    } else
      if ((fds[++k]=open(argv[i],append?O_WRONLY|O_APPEND|O_CREAT:O_WRONLY|O_CREAT|O_TRUNC,0644))<0) {
	__write2("could not open "); __write2(argv[i]); __write2("\n");
      }
  }
  do {
    char buf[1024];
    int j;
    i=read(0,buf,1024);
    if (i==0) break;
    if (i<0) {
      __write2("read error.\n");
      return 1;
    }
    for (j=0; j<=k; j++) {
      if (fds[j]>=0)
	if (write(fds[j],buf,i)<i)
	  __write2("short write (disk full?)\n");
    }
  } while (1);
  return 0;
}
