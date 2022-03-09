#ifndef NOGETPWNAM

#ifndef EMBEDDED
#define getgrnam libc_getgrnam
#define getgrgid libc_getgrgid
#include <grp.h>
#undef getgrnam
#undef getgrgid

extern int openreadclose(char *fn, char **buf, unsigned long *len);
extern char **split(char *buf,int c,int *len,int plus,int ofs);
#endif

static struct group *__getgr(int idx,const char *value,unsigned long value2) {
  static struct group p;
  char *buf=0;
  unsigned long len;
  char **tmp;
  int count;
  int i;
  if (openreadclose("/etc/group",&buf,&len))
    return 0;
  tmp=split(buf,'\n',&count,0,0);
  for (i=0; i<count; i++) {
    char **tmp2;
    int four;
    int tmplen=strlen(tmp[i]);
    tmp2=split(tmp[i],':',&four,0,0);
    if (four==4) {
      if ((idx==2 && atoi(tmp2[idx])==value2) ||
	  (idx!=2 && !strcmp(tmp2[idx],value))) {
	long diff;
	p.gr_name=(char*)malloc(tmplen+1);
	if (!p.gr_name) { free(tmp2); free(tmp); free(buf); return 0; }
	diff=tmp2[0]-p.gr_name;
	memcpy(p.gr_name,tmp[i],tmplen);
#ifndef DONT_NEED_PASSWD
	p.gr_passwd=tmp2[1]-diff;
#endif
#ifndef DONT_NEED_GID
	p.gr_gid=atoi(tmp2[2]);
#endif
#ifndef DONT_NEED_MEMBERS
	{	/* oh great, a split of a splittered splinter */
	  int n;
	  p.gr_mem=split(tmp2[3],',',&n,1,0);
	  if (p.gr_mem) p.gr_mem[n]=0;
	}
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
#define getgrnam(name) __getgr(0,name,0)
#define getgrgid(uid) __getgr(2,0,uid)
#else
struct group *getgrnam(char *name) {
  return __getgr(0,name,0);
}

struct group *getgrgid(long uid) {
  return __getgr(2,0,uid);
}
#endif

#endif
