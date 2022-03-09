#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <errno.h>
#include "write12.h"

int main(int argc,char *argv[]) {
  if (argc>1) {
    if (sethostname(argv[1],strlen(argv[1]))) {
      switch (errno) {
	case EINVAL: __write2("name too long\n"); return 1;
	case EPERM: __write2("must be superuser\n"); return 1;
	case 0: break;
	default: return 1;
      }
    }
  } else {
    struct utsname un;
    uname(&un);
    __write1(un.nodename);
    __write1("\n");
  }
  return 0;
}
