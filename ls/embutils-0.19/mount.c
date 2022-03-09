/* a small 'util-linux'-like mount(8) implementation
 *
 * TODO: add NFS and loop support. (and more options)
 *
 * (C) 2004 Ruediger Sonderfeld <kingruedi@c-plusplus.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*config :)*/
#define LINUX
#define HAVE_STDBOOL

#ifdef LINUX
#include <sys/mount.h>
#include <paths.h>
#ifdef _PATH_MOUNTED
const char *const mtab=_PATH_MOUNTED;
#else
const char *const mtab="/etc/mtab";
#endif
#ifdef _PATH_MNTTAB
const char *const fstab=_PATH_MNTTAB;
#else
const char *const fstab="/etc/fstab";
#endif
const char *const filesystems="/proc/filesystems";

/*#elif 43BSD
#include <sys/mount.h>
#define MS_RDONLY M_RDONLY
...
#elif FREEBSD
#include <sys/mount.h>
#define MS_RDONLY MNT_RDONLY
...
#elif MINIX
#include <unistd.h>
#include <sys/mount.h>*/
#else
#error "mount.c doesn't support your system"
#endif

#ifndef HAVE_STDBOOL
typedef enum { false,true } bool;
#else
#include <stdbool.h>
#endif

#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
//#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "write12.h"

/* I don't use stdio because it adds a 7k> bloat */

struct io_file {
  int fh;
  size_t length;
  size_t read;
  char *buf;
  bool workarround;
};

/* opens file */
static struct io_file *io_open(const char *file,unsigned long flags) {
  struct io_file *ret=malloc(sizeof(struct io_file));
  if(!(ret->fh=open(file,flags)))
    return NULL; /*free(ret);*/
  ret->read=0;
  if((off_t)(ret->length=lseek(ret->fh,0,SEEK_END))==(off_t)-1) {
    /* workarround for /proc */
    char buffer[100];
    ssize_t n;
    ret->workarround=true;
    while((n=read(ret->fh,buffer,sizeof(buffer))))
      ret->length+=n;
    close(ret->fh);
    if(!(ret->fh=open(file,flags)))
      return NULL;
    ret->buf=malloc(ret->length+1);
    read(ret->fh,ret->buf,ret->length);
    ret->buf[ret->length]=0;
  }
  else {
    ret->workarround=false;
    if(!(ret->buf=mmap(0,ret->length,PROT_READ,MAP_PRIVATE,ret->fh,0)))
      return NULL; /*free(ret);*/
  }
  return ret;
}

/* closes file */
static void io_close(struct io_file *fh) {
  if(fh->workarround)
    free(fh->buf);
  else
    munmap(fh->buf,fh->length);
  close(fh->fh);
  free(fh);
}

/* reads next byte (or returns 0 on EOF) */
static char io_get_byte(struct io_file *fh) {
  return fh->read++==fh->length ? 0 : *fh->buf++;
}

/* reads until newline, EOS or EOF */
static bool io_readline(struct io_file *fh,char *line,size_t n) {
  size_t i=0;
  for(;i<n;++i) {
    line[i]=io_get_byte(fh);
    if(!line[i])
      return i==0 ? false : true;
    if(line[i]=='\n') {
      line[i]=0;
      return true;
    }
  }
  return true;
}

/*
 * I don't use getmntent(3) and so on because it uses stdio (7k bloat!) and it
 * isn't really portable
 */

/* tokenizes the string when a space appears (not thread safe) */
static char *spacetok(char *s) {
  static char *buffer=NULL;
  char* tmp;
  if(s)
    buffer=s;
  tmp=buffer;
  for(;*buffer;++buffer) {
    if(isspace(*buffer)) {
      *buffer++=0;
      return tmp;
    }
  }
  return NULL;
}

struct mntentry {
  const char *device;
  const char *dir;
  const char *fs_type;
  const char *opts;
  /*  not used:
    int freq;
    int passno;
   */
};

/* reads next entry and returns it (not thread safe!) */
static struct mntentry *mnt_entry(struct io_file *fh) {
  static char buffer[100];
  static char *entries[4];
  static struct mntentry entry;

  for(;;) {
    size_t i;
    if(!io_readline(fh,buffer,sizeof(buffer)))
      return NULL;
    if(buffer[0]=='#')
      continue;
    i=0;
    entries[i]=spacetok(buffer);
    while((entries[++i]=spacetok(NULL)) &&
	  i<sizeof(entries));
    break;
  }

  entry.device=entries[0];
  entry.dir=entries[1];
  entry.fs_type=entries[2];
  entry.opts=entries[3];

  return &entry;
}

/* The mount_options specification is copyied from busybox-mount
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 * Copyright (C) 1999-2003 by Erik Andersen <andersen@codepoet.org>
 * http://www.busybox.org
 *
 * COPYING START
 */
struct mount_options {
	const char *name;
	unsigned long and;
	unsigned long or;
};

static const struct mount_options options[] = {
	{"async", ~MS_SYNCHRONOUS, 0},
	{"atime", ~0, ~MS_NOATIME},
	{"defaults", ~0, 0},
	{"noauto", ~0, 0},
	{"dev", ~MS_NODEV, 0},
	{"diratime", ~0, ~MS_NODIRATIME},
	{"exec", ~MS_NOEXEC, 0},
	{"noatime", ~0, MS_NOATIME},
	{"nodev", ~0, MS_NODEV},
	{"nodiratime", ~0, MS_NODIRATIME},
	{"noexec", ~0, MS_NOEXEC},
	{"nosuid", ~0, MS_NOSUID},
	{"remount", ~0, MS_REMOUNT},
	{"ro", ~0, MS_RDONLY},
	{"rw", ~MS_RDONLY, 0},
	{"suid", ~MS_NOSUID, 0},
	{"sync", ~0, MS_SYNCHRONOUS},
	{"bind", ~0, MS_BIND},
	{0, 0, 0}
};
/*
 * COPYING STOP
 */

/* parses options (data must be of length DATA_BUFFER_SIZE) */
static void parse_options(const char *str,unsigned long *flags,char *data,
			  size_t data_size)
{
  size_t data_set=0;
  for(;;) {
    const struct mount_options *i;
    char *ptr=strchr(str,',');
    if(!ptr)
      break;
    *ptr=0;
    for(i=options; i->name; ++i)
      if(!strcmp(str,i->name)) {
	*flags&=i->and;
	*flags|=i->or;
	break;
      }
    if(!i->name) { /*no options was found*/
      size_t option_length=strlen(str)+1;
      if(data_set) {
	*(data+data_set)=',';
	++data_set;
      }
      memcpy(data+data_set,str,option_length);
      data_set+=option_length;
      if(data_set>=data_size)
	return; /*too many options*/
    }
    str=ptr+1;
  }
}

/*#define PUTSTRING(fh,str) write(fh,str,sizeof(str)-1);*/

/* mounts `device` on `dir` with given flags */
static int do_mount(const char *device,const char *dir,const char *fs_type,
		     unsigned long flags,const char *data,bool write_mtab)
{
  int result=mount(device,dir,fs_type,flags,data);
  if(result==-1 && errno==EROFS) {
    __write2(device);
    write(2," is write-proteced, mounting read-only\n",40);
    flags|=MS_RDONLY;
    result=mount(device,dir,fs_type,flags,data);
  }
  if(result==-1 && errno==EBUSY) {
    __write2(device);
    write(2," is already mounted\n",20);
    return 0;
  }
  else if(result==-1)
    return result;
  if(write_mtab) {
    /*TODO add locks*/
    int fh=open(mtab,O_WRONLY|O_APPEND);
    if(!fh)
      return -1;
    write(fh,device,strlen(device));
    write(fh," ",1);
    write(fh,dir,strlen(dir));
    write(fh," ",1);
    write(fh,fs_type,strlen(fs_type));
    /*TODO reparse options ...*/
    if((flags | MS_RDONLY)==flags)
      write(fh," ro,",4);
    else
      write(fh," rw,",4);
    write(fh,data,strlen(data));
    write(fh," 0 0\n",5);
    close(fh);
  }
  return 0;
}

/* wrapper arround do_mount that handles "auto" filesystem */
static int mount_fs(const char *device,const char *dir,const char *fs_type,
		     unsigned long flags,const char *data,bool write_mtab)
{
  if(!strcmp(fs_type,"auto")) {
    struct io_file *fh=io_open(filesystems,O_RDONLY);
    char buffer[100];
    if(fh) {
      while(io_readline(fh,buffer,sizeof(buffer))) {
	char *ptr;
	if(buffer[0]=='#'||buffer[0]==0)
	  continue;
	if((ptr=strchr(buffer,'\t')))
	  ++ptr;
	else
	  ptr=buffer;
	if(do_mount(device,dir,ptr,flags,data,write_mtab)==0) {
	  io_close(fh);
	  return 0;
	}
      }
      io_close(fh);
    }
  }
  else
    if(do_mount(device,dir,fs_type,flags,data,write_mtab)==0)
      return 0;

  write(2,"mounting ",9);
  __write2(device);
  write(2," failed: ",9);
  __write2(strerror(errno));
  write(2,"\n",1);
  return 1;
}

/* mount all filesystems specified in /etc/fstab */
static inline int mount_all(const char *fs_type,unsigned long flags,
			     char *data,size_t data_size,bool write_mtab)
{
  struct io_file *fh=io_open(fstab,O_RDONLY);
  size_t data_length=data_size-strlen(data);
  if(fh) {
    struct mntentry *mnt;
    while((mnt=mnt_entry(fh))) {
      unsigned long flag=flags;
      char *data_tmp=data;

      if(fs_type)
	if(strcmp(mnt->fs_type,fs_type))
	  continue;
      
      parse_options(mnt->opts,&flag,data_tmp,data_length);
      
      return mount_fs(mnt->device,mnt->dir,mnt->fs_type,flag,data_tmp,
		      write_mtab);
    }
  }
#ifdef CLEANUP
  io_close(fh);
#endif
}

/* show all mounted file systems (with the given filesystem-type) */
static inline void show_mounts(const char *fs_type) {
  struct io_file *fh=io_open(mtab,O_RDONLY);
  if(fh) {
    struct mntentry *mnt;
    while((mnt=mnt_entry(fh))) {
      if(fs_type)
	if(strcmp(mnt->fs_type,fs_type))
	  continue;
      
      __write1(mnt->device);
      write(1," on ",4);
      __write1(mnt->dir);
      write(1," type ",6);
      __write1(mnt->fs_type);
      write(1," (",2);
      __write1(mnt->opts);
      write(1,")\n",2);
    }
  }
#ifdef CLEANUP
  io_close(fh);
#endif
}

int main(int argc, char **argv) {
  unsigned long flags=0;
  const char *fs_type=NULL;
  const char *device=NULL;
  const char *dir=NULL;
  enum { DATA_BUFFER_SIZE=100 };
  char data[DATA_BUFFER_SIZE];
  size_t data_size=0;
  bool write_mtab=true;
  bool all=false;
  int i=1;
  if(argc>1) {
    for(;i<argc;++i) {
      if(argv[i][0]=='-') {
	switch(argv[i][1]) {
	case 'a': /*mount 'all'-filesystems (specified in /etc/fstab)*/
	  all=true;
	  break;
	case 'n': /*mount without writing to /etc/mtab*/
	  write_mtab=false;
	  break;
	case 'r': /*mount read-only*/
	  flags|=MS_RDONLY;
	  break;
	case 'w': /*mount read-write*/
	  flags&=~MS_RDONLY;
	  break;
	case 't': /*specifiy filesysten-type*/
	  ++i;
	  /*if(i==argc) return 1;*/
	  fs_type=argv[i];
	  break;
	case 'o': /*mount(2) options*/
	  ++i;
	  /*if(i==argc) return 1;*/
	  parse_options(argv[i],&flags,data,DATA_BUFFER_SIZE);
	  data_size=strlen(data);
	  break;
	case 'h':
	case 'V':
	default:
	  write(2,"emb-mount 0.1\nman 8 mount\n",26);
	  return 1;
	}
      }
      else {
	/* only takes absolute-path! */
	if(!device)
	  device=argv[i];
	else
	  dir=argv[i];
      }
    }
  }

  if(!data_size)
    memset(data,0,DATA_BUFFER_SIZE);

  if(!all && !device)
    show_mounts(fs_type);
  else if(all)
    return mount_all(fs_type,flags,data,DATA_BUFFER_SIZE,write_mtab);
  else {
    if(!dir) { /*device not given (e.g. mount /floppy)! read from fstab*/
      struct io_file *fh=io_open(fstab,O_RDONLY);
      if(fh) {
	struct mntentry *mnt;
	while((mnt=mnt_entry(fh)))
	  if(!strcmp(mnt->dir,device)) {
	    device=mnt->device;
	    dir=mnt->dir;
	    fs_type=mnt->fs_type;
	    parse_options(mnt->opts,&flags,data+data_size,DATA_BUFFER_SIZE-data_size);
	    break;
	  }
      }
#ifdef CLEANUP
      io_close(fh);
#endif
    }
    return mount_fs(device,dir,fs_type,flags,data,write_mtab);
  }
  return 0;
}
