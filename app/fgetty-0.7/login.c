/* diet login without all the bloat but with checkpassword for pluggable
 * authentication support. */

/* algorithm:
   1. argv[1] is the user name
      (optional: argv[2-argc] are environment variables)
   2. print "user's password: " to stdout
   3. set terminal to "don't echo"
   4. read password
   5. set terminal to "echo"
   6. fork and run checkpassword login2
   7. wait() for checkpassword
   8. if checkpassword returns 1, the password was wrong.  sleep(1) and ++counter
   9. if counter reaches 5, sleep(5) and exit

  login2 is expected to:

   1. kill ppid (i.e. this process) if it is run.
   2. print motd unless (-f .hushlogin)
   3. check for mail (maybe)
   4. set TERM according to /etc/ttytypes
   5. edit utmp and wtmp (uh-oh, how should we do this?  We are not root any more!)
   6. exec $SHELL
 */

#define CHECKPASSWORD "/bin/checkpassword.login"
#define FALLBACKCHECKPASSWORD "/bin/checkpassword"
#define LOGIN2 "/bin/login2"

#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <utmp.h>
#include <fcntl.h>
#include <signal.h>
#include <write12.h>

static void die(const char *message) {
  __write2(message); __write2("\n");
  exit(1);
}

static struct termios oldtermios;

static void echo_off() {
  struct termios foo;
  if (tcgetattr(0,&oldtermios))
    die("tcgetattr failed");
  foo=oldtermios;
  foo.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
  tcsetattr(0, TCSANOW, &foo);
}

static void echo_on() {
  oldtermios.c_lflag |= ECHO | ECHOE | ECHOK;
  tcsetattr(0, TCSANOW, &oldtermios);
}

int login_main(int argc,char *argv[]) {
  int filedes[2];
  char *username=argv[1];
  char *buf;
  char __username[32];
  char password[32];
  int pwlen;
  char *Argv[]={"checkpassword",LOGIN2,0};
  pid_t child;
  int utmpfd,wtmpfd;
  {
    int i=1;
    while (username && username[0]=='-')
      username=argv[++i];
  }

  if (!username) {
    char *host=getenv("HOST");
    if (host) {
      int len=strlen(host);
      host[len]=' ';
      write(0,host,len+1);
      host[len]=0;
    }
    __write1("login: ");
    pwlen=read(0,__username,9);
    if (pwlen<0) die("read failed");
    __username[pwlen-1]=0; /* skip newline */
    username=__username;
  }
  {
    buf=malloc(strlen(username)+20);
    strcpy(buf,username);
    strcat(buf,"'s password: ");
    __write1(buf);
  }
  echo_off();
  pwlen=read(0,password,99);
  if (pwlen<0) die("read failed");
  password[pwlen-1]=0;
  echo_on();
  __write1("\n");

  if (pipe(filedes))
    die("pipe failed");
  if (filedes[0]!=3)
    die("pipe did not return fd 3");
  switch (child=fork()) {
  case -1:
    die("login: could not fork");
  case 0:
    /* child */
    close(3);
    {
      char buf[32];
      int len;
      len=strlen(username)+1;
      strcpy(buf,username);
      strlcpy(buf+len,password,32-len);
      len+=strlen(password)+1;
/*	buf[len++]='Y'; */
      len+=__ltostr(buf+len,32-len,time(0),10,0);
      buf[len]=0;
      if (len<400) {
	strcpy(buf+len+1,"nosetuid");
	len+=9;
      }
      write(4,buf,len+1);
      close(4);
    }
    break;
  default:
    close(4);
    utmpfd=open(_PATH_UTMP,O_RDWR);
    if (utmpfd==-1)  utmpfd=open("/dev/null",O_RDWR);
    if (utmpfd>0 && utmpfd!=4) { dup2(utmpfd,4); close(utmpfd); utmpfd=4; };
    wtmpfd=open(_PATH_WTMP,O_APPEND|O_WRONLY);
    if (wtmpfd==-1)  wtmpfd=open("/dev/null",O_WRONLY);
    if (wtmpfd>0 && wtmpfd!=5) { dup2(wtmpfd,5); close(wtmpfd); wtmpfd=5; };
    if (utmpfd!=4 || wtmpfd!=5) {
      close(utmpfd); close(wtmpfd);
      __write2("utmpfd!=4 || wtmpfd!=5\n");
    }
      extern int check_pwd_main(int argc,char* argv[]) ;
        int i=0;
        while(Argv[i]){
            i++;
        }
      check_pwd_main(i,Argv);
//    execve(CHECKPASSWORD,Argv,environ);
//    if (errno==ENOENT)
//      execve(FALLBACKCHECKPASSWORD,Argv,environ);
    die("login: could not exec checkpassword");
  }
}
