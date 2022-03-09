#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "write12.h"

void disp(char *s) {
  static int notfirst=0;
  if (notfirst) __write1(" ");
  __write1(s);
  notfirst=1;
}

#define M 0x01
#define N 0x02
#define R 0x04
#define S 0x08
#define P 0x10
#define V 0x20

int main(int argc,char *argv[]) {
  struct utsname un;
  int flags=0;
  int i;
  for (i=1; i<argc; i++) {
    int j;
    if (argv[i][0]=='-') {
      for (j=1; j<strlen(argv[i]); j++) {
	switch (argv[i][j]) {
	case 'm': flags|=M; break;
	case 'n': flags|=N; break;
	case 'r': flags|=R; break;
	case 's': flags|=S; break;
	case 'p': flags|=P; break;
	case 'v': flags|=V; break;
	case 'a': flags=0x2f; break;
	}
      }
    }
  }
  uname(&un);
  if (!flags) flags=S;
  if (flags&S) disp(un.sysname);
  if (flags&N) disp(un.nodename);
  if (flags&R) disp(un.release);
  if (flags&V) disp(un.version);
  if (flags&M) disp(un.machine);
  if (flags&P) disp("unknown");
  __write1("\n");
  return 0;
}
