/* This is mostly mingetty without printf */

#define _GNU_SOURCE
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/param.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>

#include "fmt.h"

static struct utsname uts;
static char hn[MAXHOSTNAMELEN + 6]="HOST=";
static int hn_len=5;
static time_t cur_time;
static char *tty;

static int noclear=0;

void whine(const char* message) {
  write(2,message,strlen(message));
}

void error(char *message,int exitcode) {
  whine(message);
  exit(exitcode);
}

static void echo_off() {
  struct termios foo;
  if (!tcgetattr(0,&foo)) {
    foo.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    tcsetattr(0, TCSANOW, &foo);
  }
}

int doutmp() {
  off_t curpos;
  struct utmp ut;
  pid_t mypid=getpid();
  int fd=open(_PATH_UTMP,O_RDWR);
  if (fd) {
    for (;;) {
      int len;
      curpos=lseek(fd,0,SEEK_CUR);
      len=read(fd,&ut,sizeof(ut));
      if (len!=sizeof(ut)) break;
      if (ut.ut_pid==mypid || !strcmp(ut.ut_line,tty+5)) {
/*	write(1,"found my utmp record\n",21); */
	break;
      }
    }
    if (ut.ut_pid!=mypid) {
      memset(&ut,0,sizeof(ut));
      ut.ut_pid=mypid;
      memcpy(ut.ut_id,tty+3,sizeof(ut.ut_id));
    }
    memcpy(ut.ut_user,"LOGIN",6);
    memcpy(ut.ut_line,tty+5,sizeof(ut.ut_line));
    ut.ut_tv.tv_sec=cur_time;
    ut.ut_type=LOGIN_PROCESS;
    lseek(fd,curpos,SEEK_SET);
    write(fd,&ut,sizeof(ut));
    close(fd);
  }
  if ((fd=open(_PATH_WTMP,O_APPEND|O_WRONLY))>=0) {
    write(fd,&ut,sizeof(ut));
    close(fd);
  }
}

void sigquit_handler(int signum) {
  error("SIGQUIT received\n",23);
}

void open_tty() {
  struct sigaction sa;
  int fd;
  if (chown(tty,0,0) || chmod(tty,0600))
    error("fgetty: could not chown/chmod tty device\n",1);
  sa.sa_handler=SIG_IGN;
  sa.sa_flags=0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGHUP,&sa,NULL);
  sa.sa_handler=sigquit_handler;
  sigaction(SIGQUIT,&sa,NULL);
  setsid();
  if ((fd=open(tty, O_RDWR, 0))<0)
    error("fgetty: could not open tty device\n",3);
  if (!isatty(fd))
    error("fgetty: \"not a typewriter\" ;-)\n",4);
  if (ioctl (fd, TIOCSCTTY, (void *)1)==0) {
    if (vhangup())	/* linux specific */
      error("fgetty: vhangup failed\n",5);
  } else
    whine("fgetty: warning: could not set controlling tty!\n");
  close(2); close(1); close(0); close(fd);
  if (open(tty,O_RDWR,0) != 0)
    error("fgetty: could not open tty\n",6);
  if (dup(0) != 1 || dup(0) != 2)
    error("could not dup stdout and stderr\n",7);
  if (!noclear)
    write(0,"\033c",2);		/* linux specific */
  sa.sa_handler=SIG_DFL;
  sa.sa_flags=0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGHUP,&sa,NULL);
}

void output_special_char(char c) {
  switch (c) {
  case 's': write(1,uts.sysname,strlen(uts.sysname)); break;
  case 'n': write(1,uts.nodename,strlen(uts.nodename)); break;
  case 'r': write(1,uts.release,strlen(uts.release)); break;
  case 'v': write(1,uts.version,strlen(uts.version)); break;
  case 'm': write(1,uts.machine,strlen(uts.machine)); break;
  case 'o': write(1,uts.domainname,strlen(uts.domainname)); break;
  case 't':
  case 'd':
	    {
	      time_t now;
	      struct tm *tm;
	      char buf[30];
	      char *tmp;

	      time (&now);
	      tm = localtime (&now);
	      if (c == 'd') {
		tmp=buf+fmt_ulong(buf,tm->tm_year+1900);
		*tmp++='-';
		tm->tm_mon++;
		*tmp++=tm->tm_mon/10+'0';
		*tmp++=tm->tm_mon%10+'0';
		*tmp++='-';
		*tmp++=tm->tm_mday/10+'0';
		*tmp++=tm->tm_mday%10+'0';
		*tmp++=0;
		write(1,buf,strlen(buf));
#if 0
		/* ISO 8601 */
		printf ("%d-%02d-%02d", tm->tm_year,
			tm->tm_mon+1, tm->tm_mday);
#endif
	      } else {
		buf[0]=tm->tm_hour/10+'0';
		buf[1]=tm->tm_hour%10+'0';
		buf[2]=':';
		buf[3]=tm->tm_min/10+'0';
		buf[4]=tm->tm_min%10+'0';
		buf[5]=':';
		buf[6]=tm->tm_sec/10+'0';
		buf[7]=tm->tm_sec%10+'0';
		write(1,buf,8);
	      }
#if 0
		tmp=buf;
		printf ("%02d:%02d:%02d",
			tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif
	      break;
	    }
  case 'l': write(1,tty+5,strlen(tty)-5); break;
  case 'u':
  case 'U':
	    {
	      int users=0;
	      struct utmp ut;
	      int fd=open(_PATH_UTMP,O_RDWR);
	      char buf[20];
	      if (fd) {
		for (;;) {
		  int len;
		  len=read(fd,&ut,sizeof(ut));
		  if (len!=sizeof(ut)) break;
		  if (ut.ut_type == USER_PROCESS) users++;
		}
		close(fd);
	      }
	      write(1,buf,fmt_ulong(buf,users));
	      if (c=='U') {
		if (users==1)
		  write(1," user",5);
		else
		  write(1," users",6);
	      }
	    }
	    break;
  default:
	    write(1,&c,1);
  }
}

void do_prompt() {
  int fd=open("/etc/issue",O_RDONLY);
  char *buf;
  off_t length;
  write(1,"\n",1);
  if (fd) {
    char *c,*last;
    length=lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    buf=alloca(length+1);
    read(fd,buf,length);
    close(fd);
    last=buf+length;
    for (c=buf; c<last; c++) {
      if (*c=='\\')
	output_special_char(*++c);
      else
	write(1,c,1);
    }
  }

  write(1,hn+5,hn_len-5);
  write(1," login: ",8);
}

static inline int _isprint(char c) {
  return ((c>='A' && c<='Z') ||
          (c>='a' && c<='z') ||
          (c>='0' && c<='9') ||
          (c=='_' || c=='.' || c==',' || c=='-'));
}

char *get_logname() {
  static char logname[40];
  char *c;
  ioctl(0, TCFLSH, 0); /* flush pending input */
  for (*logname=0; *logname==0; ) {
    do_prompt();
    for (c=logname;;) {
      if (read(0,c,1)<1) {
	if (errno==EINTR || errno==EIO || errno==ENOENT)
	  exit(0);
	error("received strange error\n",9);
      }
      if (*c == '\n' || *c == '\r') {
	*c=0;
	break;
      } else if (!_isprint(*c))
	error("unprintable character in login name\n",10);
      else if (c-logname >= sizeof(logname)-1)
	error("login name too long\n",11);
      else
	c++;
    }
  }
#if 0
  write(1,"\n\ngot name ",11);
  write(1,logname,strlen(logname));
  write(1,"\n\n",2);
#endif
  return logname;
}

extern char ** environ;

char ttybuf[20]="/dev/";
char ttybuf2[25]="TTY=";

int main(int argc,char *argv[]) {
  char *loginargv[]={"/bin/login", "--", 0, 0};
  char *logname;
  int  i;
  char hostname_end='.';
  tty=argv[1];
  if (!tty)
    error("usage: fgetty 1\n"
	  "       fgetty vc/1\n"
	  "       fgetty /dev/tty1\n",111);
  if (tty[0]=='/')
    strncpy(ttybuf,tty,15);
  else if (isdigit(tty[0])) {
    struct stat ss;
    /* try prepending /dev/vc/1 and /dev/tty1 */
    strcpy(ttybuf,"/dev/vc/"); strncpy(ttybuf+8,tty,3);
    if (stat(ttybuf,&ss) && errno==ENOENT) {
      ttybuf[5]=ttybuf[6]='t'; ttybuf[7]='y';
    }
  } else
    strncpy(ttybuf+5,tty,10);
  tty=ttybuf;
  strcpy(ttybuf2+4,ttybuf);

  uname(&uts);
  if (gethostname(hn+5, MAXHOSTNAMELEN)!=0) hn[5]=0;
  hn[5+MAXHOSTNAMELEN]=0;
  putenv("TERM=linux");
  putenv(ttybuf2);
  putenv(hn);
  time(&cur_time);
  for (i=2; i<argc; ++i) {
    if (!strcmp(argv[i],"--noclear"))
      noclear=1;
    else if (!strcmp(argv[i],"--long-hostname"))
      hostname_end=0;
  }
  while (hn[hn_len]!=0 && hn[hn_len]!=hostname_end) ++hn_len;
#ifndef DEBUG
  doutmp();
  open_tty();
#endif
  ioctl(0,TCFLSH,2);	/* mingetty says this is important for modem users */
  while ((logname=get_logname()) == 0);
  if (logname[0]=='-') error("username may not start with a dash\n",13);
  loginargv[2]=logname;
  echo_off();
#ifdef TEST
  execve("/bin/login1", loginargv, environ);
#else
  execve("/bin/login", loginargv, environ);
#endif
  exit(8);
}
