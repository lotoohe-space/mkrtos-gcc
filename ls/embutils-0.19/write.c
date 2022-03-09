#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <utmp.h>
#include <fcntl.h>
#include "write12.h"

static int write_fd(int fd,const char* str) {
  return write(fd,str,strlen(str));
}

/* gets a tty that is owned by user */
char * get_tty(char * user) {
  struct utmp * u;
  int found_one = 0;
  int fd;
  chdir("/dev");
  setutent();
  while ((u = getutent()) != NULL) {
    if (strncmp(u->ut_name,user,strlen(user))==0) {
      found_one = 1;
      if ((fd=open(u->ut_line,O_WRONLY))<0) {
        /* must be disabled in some way */
        continue;
      }
      close(fd);
      return u->ut_line;
    }
  }
  if (found_one) {
    /* user's logged in, but no writable ttys */
    __write2("write: ");
    __write2(user);
    __write2(" has messages disabled\n");
    exit(EXIT_FAILURE);
  }
  return NULL;
}

int main(int argc, char * argv[]) {
  char * user, * terminal;
  char * my_name, * my_term, * newterm;
  struct passwd * p;
  char ch;
  int rc;
  int tty_fd;

  if (argc<2) {
    /* too few args */
    __write1("usage: write user [tty]\n");
    return EXIT_FAILURE;
  }

  /* get args and other important information */
  p = getpwuid(geteuid());
  my_name = p->pw_name;
  my_term = ttyname(0);
  my_term = my_term ? my_term : "(no tty)";
  user = argv[1];

  /* if no tty argument given, get a tty for user */
  terminal = argv[2] ? argv[2] : get_tty(user);
  if (terminal==NULL) {
    __write2("write: ");
    __write2(user);
    __write2(" is not logged in\n");
    return EXIT_FAILURE;
  }
  /* we both accept /dev/ttyX and ttyX */
  if (strstr(terminal,"/dev")==NULL) {
    newterm = alloca(6+strlen(terminal));
    strcpy(newterm,"/dev/");
    strcat(newterm,terminal);
    terminal = newterm;
  }

  /* open other user's tty */
  tty_fd = open(terminal,O_WRONLY);
  if (tty_fd < 0) {
    __write2("write: "); __write2(user);
    __write2(" has messages disabled on ");
    __write2(terminal); __write2("\n");
    return EXIT_FAILURE;
  }
  /* announce message */
  write_fd(tty_fd,"\nMessage from "); write_fd(tty_fd,my_name);
  write_fd(tty_fd," on "); write_fd(tty_fd,my_term);
  write_fd(tty_fd,"...\007\007\007\n");

  /* read from stdin and write to other tty */
  rc = read(0,&ch,sizeof(ch));
  while (rc>0) {
    write(tty_fd,&ch,sizeof(ch));
    rc = read(0,&ch,sizeof(ch));
  }

  /* end message */
  write_fd(tty_fd,"EOF\n");

  close(tty_fd);
  return EXIT_SUCCESS;
}
