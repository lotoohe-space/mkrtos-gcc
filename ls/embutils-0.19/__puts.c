#ifndef EMBEDDED
#include <unistd.h>
#include <string.h>
#endif

void __puts(char*s) {
  int len;
  s[len=strlen(s)]='\n';
  write(1,s,len+1);
}
