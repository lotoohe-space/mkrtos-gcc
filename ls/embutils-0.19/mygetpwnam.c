#ifndef NOGETPWNAM

#ifndef EMBEDDED
#define getpwnam libc_getpwnam
#define getpwuid libc_getpwuid
#include <pwd.h>
#undef getpwnam
#undef getpwuid

extern int openreadclose(char *fn, char **buf, unsigned long *len);
extern char **split(char *buf,int c,int *len,int plus,int ofs);
#endif

static struct passwd *__getpw(int idx,const char *value,unsigned long value2) {
  static struct passwd p;
  char *buf=0;
  unsigned long len;
  char **tmp;
  int count;
  int i;
  if (openreadclose("/etc/passwd",&buf,&len))
    return 0;
  tmp=split(buf,'\n',&count,0,0);
  for (i=0; i<count; i++) {
    char **tmp2;
    int seven;
    int tmplen=strlen(tmp[i]);
    tmp2=split(tmp[i],':',&seven,0,0);
    if (seven==7) {
      if (((idx==2 || idx==3) && atoi(tmp2[idx])==value2) ||
	  (idx!=2 && idx!=3 && !strcmp(tmp2[idx],value))) {
	long diff;
	p.pw_name=(char*)malloc(tmplen+1);
	if (!p.pw_name) { free(tmp2); free(tmp); free(buf); return 0; }
	diff=tmp2[0]-p.pw_name;
	memcpy(p.pw_name,tmp[i],tmplen);
#ifndef DONT_NEED_PASSWD
	p.pw_passwd=tmp2[1]-diff;
#endif
#ifndef DONT_NEED_UID
	p.pw_uid=atoi(tmp2[2]);
#endif
#ifndef DONT_NEED_GID
	p.pw_gid=atoi(tmp2[3]);
#endif
#ifndef DONT_NEED_GECOS
	p.pw_gecos=tmp2[4]-diff;
#endif
#ifndef DONT_NEED_DIR
	p.pw_dir=tmp2[5]-diff;
#endif
#ifndef DONT_NEED_SHELL
	p.pw_shell=tmp2[6]-diff;
#endif
	free(tmp2); free(tmp); free(buf);
	return &p;
      }
    }
  }
  free(tmp);
  free(buf);
  return 0;
}

#ifdef EMBEDDED
#define getpwnam(name) __getpw(0,name,0)
#define getpwuid(uid) __getpw(2,0,uid)
#else
struct passwd *getpwnam(char *name) {
  return __getpw(0,name,0);
}

struct passwd *getpwuid(long uid) {
  return __getpw(2,0,uid);
}
#endif

#endif
