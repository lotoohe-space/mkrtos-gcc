#include <unistd.h>
#include "write12.h"

int main(int argc,char *argv[]) {
  int i;
  char *tmp[]={0,"y",0};
  if (argc<2) { argc=2; argv=tmp; }
  for (;;) {
    for (i=1; i<argc; ++i) {
      __write1(argv[i]);
      if (i<argc) __write1(" ");
    }
    __write1("\n");
  }
}
