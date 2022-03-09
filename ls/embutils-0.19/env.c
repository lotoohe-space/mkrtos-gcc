#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "write12.h"

#define EMBEDDED

#include "__puts.c"

#ifdef __OpenBSD__
extern char** environ;
#endif

int main(int argc,char *argv[]) {
  int i;
  for (i=1; i<argc; ++i) {
    char *v=argv[i];
    if (v[0]=='-') {
      if (v[1]==0)
	environ[0]=0;
      else {
	int j;
	for (j=1; v[j]; ++j)
	  switch (v[j]) {
	  case 'i': environ[0]=0; break;
	  case 'u': if (argv[i][j+1])
		      argv[i]+=j+1;
		    else
		      ++i;
		    if (!argv[i]) {
		      __write2("env: option requires argument: -u\n");
		      return 1;
		    }
		    putenv(argv[i]);
		    break;
	  }
      }
    } else {
      if (strchr(argv[i],'='))
	putenv(argv[i]);
      else {
	execvp(argv[i],argv+i);
	return 0;
      }
    }
  }
  for (; *environ; ++environ) {
    __puts(*environ);
  }
  return 0;
}
