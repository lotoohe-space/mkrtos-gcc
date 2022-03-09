#include "embfeatures.h"
#ifdef SUPPORT_64BIT_FILESIZE
#define _FILE_OFFSET_BITS 64
#endif
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "write12.h"

main(int argc,char* argv[]) {
  off_t o;
  int i;
  unsigned char c;
  if (argc<3) {
usage:
    __write2("usage: truncate size filename [filename...]\n");
    return 0;
  }
  o=0;
  for (i=0; c=argv[1][i]; ++i) {
    if ((c-='0')>10) goto usage;
    o=o*10+c;
  }
  for (i=2; i<argc; ++i) {
    if (truncate(argv[i],o)) {
      __write2("truncate: ");
      __write2(argv[i]); __write2(": ");
      switch (errno) {
      case ENOTDIR: __write2("not a directory\n"); break;
      case ENAMETOOLONG: __write2("file name too long\n"); break;
      case ENOENT: __write2("no such file or directory\n"); break;
      case EACCES: __write2("permission denied\n"); break;
      case ELOOP: __write2("too many symbolic links\n"); break;
      case EISDIR: __write2("is a directory\n"); break;
      case EROFS: __write2("read-only file system\n"); break;
      case EIO: __write2("I/O error\n"); break;
      default: __write2("strange error\n"); break;
      }
    }
  }
}
