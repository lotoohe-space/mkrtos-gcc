#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <alloca.h>
#ifdef __dietlibc__
#include <write12.h>
#include "malloc.h"

#else
inline void __write2(const char* message) {
  write(2,message,strlen(message));
}
#endif

extern char** environ;

unsigned int fmt_ulong(char *dest,unsigned long i) {
  register unsigned long len,tmp,len2;
  /* first count the number of bytes needed */
  for (len=1, tmp=i; tmp>9; ++len) tmp/=10;
  if (dest)
    for (tmp=i, dest+=len, len2=len+1; --len2; tmp/=10)
      *--dest = (tmp%10)+'0';
  return len;
}

int check_pwd_main(int argc,char* argv[]) {
  char buf[129];
  char* last;
  int len;
  char *login,*passwd,*ptr;
  struct passwd *pw;
  struct spwd *spw;

  if (!argv[1]) return 2;
  for (len=0; len<128; ) {
    int tmp;
    tmp=read(3,buf+len,128-len);
    if (tmp==-1) return 111;
    if (tmp==0) break;
    len+=tmp;
  }
  close(3);
  buf[len]=0; last=buf+len;
  login=buf;
  if ((pw=getpwnam(login))) {
    passwd=pw->pw_passwd;
    if ((spw=getspnam(login)))
      passwd=spw->sp_pwdp;
    ptr=login+strlen(login)+1;
    if (!*passwd || !strcmp(ptr,passwd)) {//去掉了加密
//    if (!*passwd || !strcmp(crypt(ptr,passwd),passwd)) {
      char **env,**ep;
      char buf[100];
      for (len=0; environ[len]; ++len) ;
      env=malloc((len+4)*sizeof(char*));
      ep=env;
      for (len=0; environ[len]; ++len) {
        if (!strncmp(environ[len],"USER=",5)) continue;
        if (!strncmp(environ[len],"HOME=",5)) continue;
        if (!strncmp(environ[len],"SHELL=",6)) continue;
        if (!strncmp(environ[len],"UID=",4)) continue;
        *ep=environ[len]; ++ep;
      }
      *ep=malloc(strlen(pw->pw_shell)+7); strcat(strcpy(*ep,"SHELL="),pw->pw_shell); ++ep;
      *ep=malloc(strlen(login)+6); strcat(strcpy(*ep,"USER="),login); ++ep;
      *ep=malloc(strlen(pw->pw_dir)+7); strcat(strcpy(*ep,"HOME="),pw->pw_dir); ++ep;
      strcpy(buf,"UID=");
      buf[4+fmt_ulong(buf+4,pw->pw_uid)]=0;
      *ep=buf; ++ep;
      *ep=0;

      ptr+=strlen(ptr)+1;	/* skip password */

//      if (initgroups(pw->pw_name,pw->pw_gid)==-1) return 1;
        initgroups(pw->pw_name,pw->pw_gid);
/*      if (setgroups(1,&pw->pw_gid)==-1) return 1; */
      if (setgid(pw->pw_gid)==-1) return 1;
      if (ptr) {
	ptr+=strlen(ptr)+1;		/* skip timestamp */
	if (ptr>=last) ptr=0;
      }
      if (!ptr || strcmp(ptr,"nosetuid")) if (setuid(pw->pw_uid)==-1) return -1;
      if (chdir(pw->pw_dir)==-1) return 111;
      execve(argv[1],argv+1,env);
      return 111;
    }
  }
  if (isatty(0) && isatty(1)) {
    __write2("checkpassword: wrong password.\n");
    sleep(5);
  }
  return 1;
}
