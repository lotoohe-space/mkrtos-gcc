#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "write12.h"

int main(int argc,char* argv[]) {
  struct passwd* p;
  uid_t u;
  /* chrootuid /path uid [program] */
  if (argc<3) {
    __write2("usage: chrootuid /path uid program\n");
    return 0;
  }
  p=getpwnam(argv[2]);
  if (!p) {
    __write2("invalid uid \"");
    __write2(argv[2]);
    __write2("\"!\n");
    return 1;
  }
  if (chroot(argv[1])==-1 || chdir("/")==-1) {
    __write2("chroot \"");
    __write2(argv[1]);
    __write2("\" failed!\n");
    return 1;
  }
  setresgid(p->pw_gid,p->pw_gid,p->pw_gid);
  setresuid(p->pw_uid,p->pw_uid,p->pw_uid);
  execvp(argv[3],argv+3);
  __write2("execvp \"");
  __write2(argv[3]);
  __write2("\" failed!\n");
  return 1;
}
