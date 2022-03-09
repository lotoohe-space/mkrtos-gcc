#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "write12.h"

int main(int argc,char *argv[]) {
  struct utsname un;
  uname(&un);
  __write1(un.machine);
  __write1("\n");
  return 0;
}
