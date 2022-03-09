/* compile with:
 * diet gcc -Os mesg.c -o mesg ; strip -R .note -R .comment mesg
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "write12.h"

int main(int argc, char * argv[]) {
  struct stat sbuf;
  if (!isatty(0)) {
    __write2("stdin: not a tty\n");
    return EXIT_FAILURE;
  }
  fstat(0,&sbuf);
  if (argc>1) {
    if (argv[1][0]!='y' && argv[1][0]!='n' || /* if neither y nor n */
        argv[1][0]!='\0' && argv[1][1]!='\0') { /* or string longer than 1 char */
      __write2("Usage: mesg [y|n]\n");
      return EXIT_FAILURE;
    }
    fchmod(0,(argv[1][0]=='y')?sbuf.st_mode|S_IWGRP:sbuf.st_mode&~S_IWGRP);
  } else
    __write1((sbuf.st_mode&S_IWGRP)?"is y\n":"is n\n");
  return EXIT_SUCCESS;
}
