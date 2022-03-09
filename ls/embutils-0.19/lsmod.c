#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "write12.h"

main() {
  int fd=open("/proc/modules",O_RDONLY);
  char buf[8192];
  char obuf[1024];
  int i,j;
  if (fd==-1) {
    __write2("/proc not mounted or kernel with no module support\n");
    return 1;
  }
  __write1("Module                  Size  Used by\n");
  j=read(fd,buf,sizeof(buf));
  for (i=0; i<j; ) {
    char* name,* size,* used,* deps;
    char* x,* y,* z;
    int nl,sl,ul,dl;

    name=buf+i;

    x=strchr(buf+i,' ');
    y=strchr(buf+i,'\n');
    if (!x || !y) return 1;
    nl=x-name;
    z=strchr(size=++x,' ');
    if (!z || z>y) return 1;
    sl=z-size;
    z=strchr(used=++z,' ');
    if (!z || z>y) return 1;
    ul=z-used;
    z=strchr(deps=++z,' ');
    if (!z || z>y) return 1;
    dl=z-deps;
    if (dl==1 && *deps=='-') dl=0;
    if (dl && deps[dl-1]==',') --dl;

    memset(obuf,' ',1024);
    memcpy(obuf,name,nl);
    memcpy(obuf+28-sl,size,sl);
    memcpy(obuf+30,used,ul);
    memcpy(obuf+ul+31,deps,dl);
    obuf[ul+dl+31]='\n';
    write(1,obuf,ul+dl+32);
    i=y-buf+1;
  }
}
