#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

unsigned int str_copy(char *out,const char *in) {
  register char* s=out;
  register const char* t=in;
  for (;;) {
    if (!(*s=*t)) break; ++s; ++t;
  }
  return s-out;
}

int a=0;

int main(int argc,char *argv[]) {
  int i,found=0;
  char *path=getenv("PATH");
  for (i=1; i<argc; ++i) {
    char *tmp=path;
    int done=0;
    if (!strcmp(argv[i],"-a")) {
      a=1;
      continue;
    }
    while (tmp) {
      char *foo=strchr(tmp,':');
      if (foo) { *foo=0; ++foo; }
      {
	struct stat ss;
	char *bar=(char*)alloca(strlen(tmp)+strlen(argv[i])+5);
	int len=str_copy(bar,tmp);
	bar[len]='/'; ++len;
	len+=str_copy(bar+len,argv[i]);
	bar[len]=0;
	if (!stat(bar,&ss)) {
	  bar[len]='\n';
	  write(1,bar,len+1);
	  if (!a) done=1;
	  ++found;
	}
      }
      if (foo) foo[-1]=':';
      tmp=foo;
      if (done) break;
    }
  }
  return found==0;
}
