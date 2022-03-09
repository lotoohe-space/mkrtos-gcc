#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define EMBEDDED

#include "__puts.c"

int main(int argc,char *argv[]) {
  char *res=ttyname(0);
  int silent=0;
  if (argc>=2) {
    if (argv[1][0]=='-') {
      char *tmp=argv[1]+1;
      switch (*tmp) {
      case '-':
	if (!strcmp(tmp,"-quiet") || !strcmp(tmp,"-silent"))
      case 's':
	silent=1;
	break;
      default:
	break;
      }
    }
  }
  if (res) {
    if (!silent) {
      __puts(res);
    }
    return 0;
  }
  if (!silent)
    write(2,"not a tty\n",10);
  return 1;
}

