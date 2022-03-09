#include <unistd.h>
#include <sys/utsname.h>
#include <errno.h>
#include "write12.h"

int main(int argc,char *argv[]) {
  if (argc>1) {
    if (setdomainname(argv[1],strlen(argv[1]))) {
      switch (errno) {
	case EINVAL: __write2("name too long\n"); return 1;
	case EPERM: __write2("must be superuser\n"); return 1;
	case 0: break;
	default: return 1;
      }
    }
  } else {
#ifdef __linux__
    struct utsname un;
    int len;
    uname(&un);
    len=strlen(un.domainname);
    un.domainname[len]='\n';
    write(1,un.domainname,len+1);
#else
    char buf[1024];
    int l;
    getdomainname(buf,sizeof(buf));
    buf[sizeof(buf)-1]=0;
    l=strlen(buf);
    buf[l]='\n';
    write(1,buf,l+1);
#endif
  }
  return 0;
}
