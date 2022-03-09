#include "write12.h"
#include <string.h>
#include <stdlib.h>

int main(int argc,char* argv[],char* envp[]) {
  int i;
  if (argc>1) {
    for (i=1; i<argc; ++i) {
      char* c=getenv(argv[i]);
      if (c) __write1(c);
      __write1("\n");
    }
  } else {
    for (i=0; envp[i]; ++i) {
      __write1(envp[i]);
      __write1("\n");
    }
  }
  return 0;
}
