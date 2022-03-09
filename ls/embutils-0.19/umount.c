#include <sys/mount.h>
#include "write12.h"
#include <errno.h>

int main(int argc,char* argv[]) {
  int i;
  int f;
  int res;
  f=0;
  for (i=1; i<argc; ++i) {
    char *v=argv[i];
    if (v[0]=='-') {
      int j;
      for (j=1; v[j]; ++j)
	switch (v[j]) {
	case 'f': f|=MNT_FORCE; break;
	case 'd': f|=MNT_DETACH; break;
	default: __write2("usage: umount [-fd] /mountpoint [...]"); return 1;
	}
    } else break;
  }
  res=0;
  for (; i<argc; ++i) {
    if (umount2(argv[i],f)==-1) {
      char* m="unknown error.";
      switch (errno) {
      case EPERM: m="permission denied (must be root)"; break;
      case EBUSY: m="file system busy"; break;
      case EINVAL: m="not a mount point"; break;
      case ENAMETOOLONG: m="name too long"; break;
      case ENOMEM: m="kernel is out of memory"; break;
      case ELOOP: m="symbolic link loop"; break;
      case ENOENT: m="no such file or directory"; break;
      }
      __write2("umount: ");
      __write2(argv[i]);
      __write2(": ");
      __write2(m);
      __write2("\n");
      res=-1;
    }
  }
  return res;
}
