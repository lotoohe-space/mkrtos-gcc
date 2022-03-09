#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "write12.h"

static inline void usage(void) {
  __write2("usage: nice -increment utility [argument...]\n");
  exit(1);
}

int parse_priority(char * p) {
  int is_neg = 0;
  int prio = 0;
  if (p[0]=='-') {
    is_neg = 1;
    p++;
  }
  if (p[0]=='+') {
    p++;
  }
  while (*p && isdigit(*p)) {
    prio = prio * 10 + *p - '0';
    p++;
  }
  if (*p) {
    __write2("error: invalid priority.\n");
    exit(1);
  }
  return is_neg?-prio:prio;
}

int main(int argc, char * argv[]) {
  int priority;
  int rc;
  if (argc<3 || argv[1][0]!='-')
    usage();
  if (argv[1][1]=='n' && argv[1][2]=='\0') {
    priority = parse_priority(argv[2]);
    argc-=3;
    argv+=3;
  } else {
    priority = parse_priority(argv[1]+1);
    argc-=2;
    argv+=2;
  }
  if (!argv[0]) {
    usage();
  }
  errno=0;
  rc = nice(priority);
  if (rc==-1 && errno==-1) {
    __write2("warning: nice failed.\n");
  }
  rc = execvp(argv[0],argv);
  if (rc) {
    __write2("error: couldn't execute ");
    __write2(argv[0]);
    __write2("\n");
  }
  return 1;
}
