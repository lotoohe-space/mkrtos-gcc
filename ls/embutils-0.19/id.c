/* printf: -rwxr-xr-x    1 leitner  users        8148 Feb  3 21:46 id */
/* buffer: -rwxr-xr-x    1 leitner  users        5140 Feb  4 03:48 id */

#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>

#define EMBEDDED
#include "openreadclose.c"
#include "split.c"
#define DONT_NEED_PASSWD
#define DONT_NEED_MEMBERS
#include "mygetgrnam.c"
#define DONT_NEED_GECOS
#define DONT_NEED_DIR
#define DONT_NEED_SHELL
#include "mygetpwnam.c"

#include "fmt_ulong.c"
#include "buffer.c"
#include "write12.h"

int main(int argc,char *argv[]) {
  int i;
  enum { ALL, GROUP, USER, GROUPS } todo=ALL;
  int name=0;
  uid_t u=geteuid();
  gid_t g=getegid();
  uid_t ru=getuid();
  uid_t rg=getgid();
  struct passwd *pw=0;
  gid_t supplementary[128];
  int size;
  char *gname=0,*uname=0;
  for (i=1; i<argc; i++) {
    int j;
    if (argv[i][0]=='-') {
      int k=strlen(argv[i]);
      for (j=1; j<k; j++) {
	switch (argv[i][j]) {
	case 'g': todo=GROUP; break;
	case 'u': todo=USER; break;
	case 'a': todo=ALL; break;
	case 'G': todo=GROUPS; break;
	case 'r': u=ru; g=rg; break;
	case 'n': name=1; break;
	}
      }
    } else {
      pw=getpwnam(argv[i]);
      if (!pw) {
	__write2("id: no such user\n");
	return 1;
      }
      uname=argv[i];
      u=ru=pw->pw_uid;
      g=rg=pw->pw_gid;
      pw=0;
      break;
    }
  }
  switch (todo) {
  case ALL:
    if (!uname) {
      pw=getpwuid(ru);
      if (!pw) {
	__write2("getpwuid failed\n");
	return 1;
      }
      uname=pw->pw_name;
    }
    if (!uname) uname="?";
    {
      struct group *gr=getgrgid(rg);
      if (gr) gname=gr->gr_name; else gname="?";
      {
	buffer_puts(buffer_1,"uid=");
	buffer_putulong(buffer_1,ru);
	buffer_putc(buffer_1,'(');
	buffer_puts(buffer_1,uname);
	buffer_puts(buffer_1,") gid=");
	buffer_putulong(buffer_1,rg);
	buffer_putc(buffer_1,'(');
	buffer_puts(buffer_1,gname);
	buffer_putc(buffer_1,')');
      }
      if (u!=ru) {
	uid_t ouid=pw->pw_uid;
	struct passwd *pw2=getpwuid(u);
	buffer_puts(buffer_1," euid=");
	buffer_putulong(buffer_1,u);
	buffer_putc(buffer_1,'(');
	buffer_puts(buffer_1,pw2?pw2->pw_name:"?");
	buffer_putc(buffer_1,')');
	pw->pw_uid=ouid;
      }
      if (g!=rg) {
	uid_t ogid=gr->gr_gid;
	struct group *gr2=getgrgid(g);
	buffer_puts(buffer_1," egid=");
	buffer_putulong(buffer_1,g);
	buffer_putc(buffer_1,'(');
	buffer_puts(buffer_1,gr2?gr2->gr_name:"?");
	buffer_putc(buffer_1,')');
	gr->gr_gid=ogid;
      }
      if (pw && ru==pw->pw_uid)
	if ((size=getgroups(128,supplementary))>0) {
	  buffer_puts(buffer_1," groups=");
	  for (i=0; i<size; i++) {
	    gr=getgrgid(supplementary[i]);
	    if (i) buffer_putc(buffer_1,' ');
	    buffer_putulong(buffer_1,supplementary[i]);
	    buffer_putc(buffer_1,'(');
	    buffer_puts(buffer_1,gr?gr->gr_name:"?");
	    buffer_putc(buffer_1,')');
	  }
	}
      buffer_putc(buffer_1,'\n');
    }
    break;
  case GROUP:
    buffer_putulong(buffer_1,g);
    buffer_putc(buffer_1,'\n');
    break;
  case USER:
    buffer_putulong(buffer_1,u);
    buffer_putc(buffer_1,'\n');
    break;
  case GROUPS:
    if (!pw || (pw && ru==pw->pw_uid)) {
      if ((size=getgroups(128,supplementary))>0) {
	for (i=0; i<size; i++) {
	  if (i) buffer_putc(buffer_1,' ');
	  buffer_putulong(buffer_1,supplementary[i]);
	}
      }
    } else
      buffer_putulong(buffer_1,g);
    buffer_putc(buffer_1,'\n');
  }
  buffer_flush(buffer_1);
/* "uid=100(leitner) gid=100(users) groups=100(users)" */
  return 0;
}
