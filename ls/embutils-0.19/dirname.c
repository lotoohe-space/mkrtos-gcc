#include <string.h>
#include <unistd.h>
#include "write12.h"

int main(int argc, char* argv[]) {
  if (argc>=2) {
    char *tmp=strrchr(argv[1],'/');
    if (tmp) {
      *tmp=0; tmp=argv[1];
    } else
      tmp=".";
    __write1(tmp);
    __write1("\n");
  }
  return 0;
}
