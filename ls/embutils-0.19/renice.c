#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>

#include "write12.h"

#define ps(s) write(2,s,sizeof(s)-1)
#define die(s) do { ps(s); exit(1); } while(0)

int main(int argc,char*argv[]) {
  int prio,ret=0;
  if (argc<3) {
    ps("renice <nice_value> <pid> [<pid> ...]\n");
    return 2;
  }
  prio=atoi(*(++argv));
  if (prio>PRIO_MAX) prio=PRIO_MAX;
  if (prio<PRIO_MIN) prio=PRIO_MIN;
  while (*(++argv)) {
    int ps,oldp;
    char buf[20];
    ps=atoi(*argv);
    oldp=20-getpriority(PRIO_PROCESS,ps);
    buf[fmt_long(buf,ps)]=0;
    if (setpriority(PRIO_PROCESS,ps,prio)==0) {
      __write1(buf);
      __write1(": old priority ");
      buf[fmt_long(buf,oldp)]=0;
      __write1(buf);
      __write1(", new priority ");
      buf[fmt_long(buf,prio)]=0;
      __write1(buf);
    }
    else {
      __write1("ERROR: setpriority failed for: ");
      __write1(buf);
      ret=1;
    }
    __write1("\n");
  }
  return ret;
}
