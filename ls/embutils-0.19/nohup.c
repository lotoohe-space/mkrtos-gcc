#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "write12.h"

int main(int argc, char * argv[]) {
  int outfile; 
  int old_fd;
  int yes_it_is;
  if (argc<2) {
    __write2("usage: nohup <command>\n");
    return 1;
  }
  outfile = (yes_it_is=isatty(1))?open("nohup.out",O_WRONLY|O_CREAT|O_APPEND,0600):1;
  if (outfile < 0) {
    __write2("Error: couldn't open file nohup.out.\n");
    return 1;
  }
  if (yes_it_is) {
    __write1("Appending output to `nohup.out'.\n");
    old_fd = dup(2);
    close(1);
    close(2);
    dup(outfile);
    dup(outfile);
  }
  signal(SIGHUP,SIG_IGN);
  execvp(argv[1],argv+1);
  if (yes_it_is) {
    close(2);
    dup(old_fd);
  }
  __write2("Error: couldn't execute ");
  __write2(argv[1]);
  __write2("\n");
  if (yes_it_is)
    unlink("nohup.out");
  return 1;
}
