#include <string.h>
#include <unistd.h>
#include "write12.h"

int main(int argc, char* argv[]) {
  if (argc>=2) {
    char *tmp=strrchr(argv[1],'/');
    if (tmp)
      ++tmp;
    else
      tmp=argv[1];
    if (argv[2]) {
      char *t=tmp+strlen(tmp)-strlen(argv[2]);
      if (!strcmp(t,argv[2]))
	*t=0;
    }
    __write1(tmp);
    __write1("\n");
  }
  return 0;
}
