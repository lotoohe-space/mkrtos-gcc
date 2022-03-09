#include <unistd.h>
#include <string.h>
#include <pwd.h>

int main(void) {
  struct passwd* p;
  if ((p=getpwuid(geteuid()))) {
    int len=strlen(p->pw_name);
    p->pw_name[len]='\n';
    write(1,p->pw_name,len+1);
    return 0;
  }
  return 1;
}
