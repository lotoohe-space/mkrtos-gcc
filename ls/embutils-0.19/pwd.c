#include <string.h>
#include <limits.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int main() {
  char buf[PATH_MAX+1];
  char *tmp=getcwd(buf,PATH_MAX);
  if (tmp) {
    int len=strlen(buf);
    buf[len]='\n';
    write(1,buf,len+1);
  } else
    write(1,".\n",2);
  return 0;
}
