#define _GNU_SOURCE
#include <unistd.h>
#include "write12.h"
#include <errno.h>

main(int argc,char* argv[]) {
  if (argc!=3) {
    __write2("usage: pivot_root newroot putold\n");
    return 0;
  }
  if (pivot_root(argv[1],argv[2])==-1) {
    __write2("pivot_root: ");
    switch (errno) {
    case EBUSY: __write2("Device or resource busy\n"); break;
    case EINVAL: __write2("Invalid argument\n"); break;
    case ENOTDIR: __write2("Not a directory\n"); break;
    case EPERM: __write2("Permission denied\n"); break;
    default: __write2("unknown error\n"); break;
    }
    return 1;
  }
  return 0;
}
