#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[]) {
  if (argc>=2) {
    char *tmp;
    int secs=strtol(argv[1],&tmp,10);
    switch (*tmp) {
    case 'd': secs *= 24;
    case 'h': secs *= 60;
    case 'm': secs *= 60;
      break;
    }
    sleep(secs);
    return 0;
  }
  return 1;
}

