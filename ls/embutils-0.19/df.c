#include <unistd.h>
#include <fcntl.h>
#ifdef __linux__
#include <sys/vfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif
#include <stdlib.h>

/* TODO: use /etc/mtab, too, to find the real device for /dev/root, */
/* TODO: allow "df ." */

#include "fmt_ulong.c"
#include "fmt_str.c"
#include "fmt_ulongpadright.c"

void dostatfs(char *device,char *mountpoint) {
  struct statfs sfs;
  int factor;
  if (statfs(mountpoint,&sfs))
    return;
  factor=sfs.f_bsize/512;
  if (sfs.f_blocks) {
    char *buf;
    int len;
    buf=alloca(fmt_str(0,device)+fmt_str(0,mountpoint)+200);
    if (fmt_str(0,device)>19) {
      len=fmt_str(buf,device);
      len+=fmt_str(buf+len,"\n                    ");
    } else {
      len=fmt_str(buf,device);
      while (len<20) {
	buf[len]=' '; ++len;
      }
    }
    len+=fmt_ulongpadright(buf+len,sfs.f_blocks*factor/2,10);
    buf[len]=' '; ++len;
    len+=fmt_ulongpadright(buf+len,(sfs.f_blocks-sfs.f_bfree)*factor/2,9);
    buf[len]=' '; ++len;
    len+=fmt_ulongpadright(buf+len,sfs.f_bavail*factor/2,9);
    buf[len]=' '; ++len;
    {
      register long tmp=sfs.f_blocks-sfs.f_bfree;
      tmp=sfs.f_bavail/((tmp+sfs.f_bavail)/100);
      if (tmp<0) tmp=0;
      if (tmp>100) tmp=100;
      len+=fmt_ulongpadright(buf+len,sfs.f_blocks?100-tmp:0,3);
    }
    buf[len]='%'; ++len;
    buf[len]=' '; ++len;
    len+=fmt_str(buf+len,mountpoint);
    buf[len]='\n'; ++len;
    write(1,buf,len);
  }
}

int main(int argc, char* argv[]) {
  int fd=open("/proc/mounts",O_RDONLY);
  char buf[4096];
  int len=read(fd,buf,4096);
  int i;
  char *tmp,*max;
  write(1,"Filesystem           1k-blocks      Used Available Use% Mounted on\n",67);
  if (argv[1]) {
    dostatfs("-",argv[1]);
    return 0;
  }
  for (i=0,tmp=buf,max=buf+len; tmp<max; ++tmp) {
/* parse "/dev/root / reiserfs rw 0 0" */
    char *device,*mp;
#if 0
    char *fs;
#endif
    device=tmp;
    while (tmp<max && *tmp!=' ') ++tmp;
    *tmp=0;
    ++tmp;
    mp=tmp;
    while (tmp<max && *tmp!=' ') ++tmp;
    *tmp=0;
#if 0
    ++tmp;
    fs=tmp;
    while (tmp<max && *tmp!=' ') ++tmp;
    *tmp=0; ++tmp;
#endif
    while (tmp<max && *tmp!='\n') ++tmp;
    dostatfs(device,mp);
  }
  return 0;
}
