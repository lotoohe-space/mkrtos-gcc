/* diet login without all the bloat but with checkpassword for pluggable
 * authentication support. */

/* algorithm:
   1. print motd unless (-f .hushlogin)
   2. check for mail (maybe)
   3. set TERM according to /etc/ttytypes
   4. edit utmp and wtmp (passed as fd 4 and fd 5)
   5. exec $SHELL
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utmp.h>
#include <grp.h>
#include <write12.h>

#include <sys/stat.h>

void die(const char *message) {
  __write2(message); __write2("\n");
  exit(1);
}

extern char **environ;

int doutmp(const char *login) {
  off_t curpos;
  struct utmp ut;
  pid_t mypid=getpid();
  int fd=4;
  for (;;) {
    int len;
    curpos=lseek(fd,0,SEEK_CUR);
    len=read(fd,&ut,sizeof(ut));
    if (len!=sizeof(ut)) break;
    if (ut.ut_pid==mypid) {
/*	write(1,"found my utmp record\n",21); */
      break;
    }
  }
  if (ut.ut_pid!=mypid) {
    memset(&ut,0,sizeof(ut));
    ut.ut_pid=mypid;
    memcpy(ut.ut_id,getenv("TTY")+4+3,sizeof(ut.ut_id));
  }
  strncpy(ut.ut_user,login,sizeof(ut.ut_user));
  ut.ut_tv.tv_sec=time(0);
  ut.ut_type=USER_PROCESS;
  lseek(fd,curpos,SEEK_SET);
  write(fd,&ut,sizeof(ut));
  close(4);
  write(5,&ut,sizeof(ut));
  close(5);
}

main(int argc,char *argv[]) {
  int fd;
  char *shell=getenv("SHELL");
  char *Argv[]={"-sh",0};
  char *login=getenv("USER");
  if (getuid()==0) {	/* checkpassword honored "nosetuid" */
    char *tmp=getenv("UID");
    char *tty=getenv("TTY");
    if (tmp) {
      uid_t u=strtoul(tmp,&tmp,10);
      struct group *g=getgrnam("console");
      gid_t gid=getgid();
      if (*tmp==0)
	chown(tty,u,gid);
      initgroups(login,g?g->gr_gid:gid);
        /* if it fails, too bad.  checkpassword should already have made
	 * sure no additional groups are there */
      setuid(u);
      if (u && getuid()!=u) { die("getuid() != u!\n"); return 2; }
    } else return 2;
  }
  close(11);
  if ((fd=open(".hushlogin",O_RDONLY))>=0)
    close(fd);
  else {
    if ((fd=open("/etc/motd",O_RDONLY))>=0) {
      char buf[1024];
      int len;
      while ((len=read(fd,buf,1024))>0)
	write(0,buf,len);
      close(fd);
    }
  }
  /* login passes open utmp and wtmp on fd #4 and #5 */
  doutmp(login);
  {
    char *buf=alloca(strlen(login)+20);
    strcpy(buf,"LOGNAME=");
    strcat(buf,login);
    putenv(buf);
    if (shell) {
      char *tmp=strrchr(shell,'/');
      char *argv0=alloca(strlen(shell)+2);
      if (tmp) ++tmp; else tmp=shell;
      strcpy(argv0+1,tmp);
      *argv0='-';
      Argv[0]=argv0;
      execve(shell,Argv,environ);
    }
    execve("/bin/sh",Argv,environ);
  }
}
