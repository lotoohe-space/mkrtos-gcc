#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "write12.h"

int q=0;
int u=0;

int main(int argc,char *argv[]) {
  int i,j;
  char* prefix=0;
  char* tmpdir;
  if (argc<2) {
    __write1("usage: mktemp [-q] [-u] template\n");
    return 0;
  }
  tmpdir=getenv("TMPDIR");
  for (i=1; i<argc; ++i) {
    if (argv[i][0]=='-') {
      for (j=1; argv[i][j]; ++j) {
	switch (argv[i][j]) {
	case 't': break;
	case 'p': tmpdir=argv[i]+j+1;
		  if (!*tmpdir) {
		    ++i;
		    tmpdir=argv[i];
		  }
		  goto nextword;
	case 'u': u=1; break;
	case 'q': q=1; break;
	default:
usage:
	  __write1("usage: mktemp [-q] [-u] template\n");
	  return 1;
	}
      }
nextword: ;
    } else break;
  }
  if (!prefix) prefix=tmpdir;
  if (!prefix) prefix="/tmp";
  if (!argv[i]) goto usage;
  {
    char* p;
    if (!strchr(p=argv[i],'/'))
      strcat(strcat(strcpy(p=alloca(strlen(prefix)+2+strlen(argv[i])),prefix),"/"),argv[i]);
    if ((j=mkstemp(p))==-1) {
      if (!q) __write2("mktemp: mkstemp failed!\n");
      return 1;
    }
    if (u) unlink(p);
    __write1(p);
  }
  return 0;
}
