/*
 * du [-a] [-b] [-c] [-D] [-h] [-k] [-L] [-l] [-m] [-S] [-s] [-x] [--] FILE...
 * -a	write counts for all files, not just directories
 * -b	print size in bytes
 * -c	produce a grand total
 * -D	dereference PATHs when symbolic link (symlink in argument)
 * -h	print sizes in human readable format (e.g., 4.0K 23.4M 2.3G 1.5T)
 * -k	print size in kilobytes (1024 byte blocks)
 * -L	dereference ALL symbolic links
 * -l	count sizes many times if hard linked
 * -m	print size in megabytes (1048576 byte blocks)
 * -S	do not include size of subdirectories
 * -s	display only a total for each argument
 * -x	skip directories on different filesystems
 *
 * missing:
 *
 * -X FILE	Exclude files that match any pattern in FILE.
 */

/*
 * NOTE: "Human readable form" in "GNU du" compared to this implementation:
 * 1k==1024, 1M==1048576 1G==1073741824 1T==1099511627776
 *
 * 3439075328 == 3358472.0k ~= 3279.7M ~= 3.2G	(error -17.02M)
j*   18898944 ==   18456.0k ~=   18.0M		(error -24k)
 *
 * GNU du uses a MIX of 1024 and 1000; this is rubbish.
 * HOW can 18898944 be 19M ?!? even 18.5M would be better and more realistic
 * if you round up for every started 0.1M then IT HAS TO BE 18.1M.
 *
 * And then the additional option -H (use power of 1000 not 1024) it still 
 * yield 19M, now its the correct value....
 *
 * This implementation of 'du' rounds up for every startet tenth-unit.
 * e.g.:
 * 3358472.0k would yield  3.3G
 *   18456.0k would yield 18.1M
 */

#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "write12.h"

#include "error.c"

static const char* errorill="du: ";

void oops(const char *fn) {
  int err=errno;
  __write2(errorill);
  if (fn) { __write2(fn); __write2(": "); }
  switch (err) {
  case EPERM:		warn("permission denied"); break;
  case EEXIST:		warn("file exists"); break;
  case EFAULT:		warn("invalid pointer"); break;
  case ENAMETOOLONG:	warn("name too long"); break;
  case ENOENT:		warn("file not found"); break;
  case ENOTDIR:		warn("a component of the path is not a directory"); break;
  case ENOMEM:		warn("out of virtual memory"); break;
  case EROFS:		warn("read-only file system"); break;
  case EACCES:		warn("directory search permissions denied"); break;
  case ELOOP:		warn("too many symbolic links"); break;
  case ENOSPC:		warn("no space left on device/quota exceeded"); break;
  case EIO:		warn("I/O error"); break;
  default: warn("unknown error");
  }
}

void usage() {
  error("du [-a] [-b] [-c] [-D] [-h] [-k] [-L] [-l] [-m] [-S] [-s] [-x] [--] FILE...");
}

unsigned long long total_sum=0;

int flag_all=0;			/* -a */
int flag_total=0;		/* -c */
int flag_dsym=0;		/* -D */
int flag_asym=0;		/* -L */
int flag_count_hardlinks=0;	/* -l */
int flag_sub=0;			/* -S */
int flag_sum=0;			/* -s */
int flag_xref=0;		/* -x */

int flag_size=0;		/* -h -b -k -m */

unsigned int fmt_ull(char *dest,unsigned long long i) {
  register unsigned long len,len2;
  unsigned long long tmp;
  for (len=1,tmp=i;tmp>9;++len) tmp/=10;
  if (dest)
    for (tmp=i,dest+=len,len2=len+1;--len2;tmp/=10)
      *--dest=(tmp%10)+'0';
  return len;
}

void output(unsigned long long sum,const char*fn,int fnl) {
  char buf[32];
  unsigned long i;
  if (flag_size==-1) sum<<=10;
  if (flag_size==1) {
    i=sum&0x3ff;
    sum>>=10;
    if (i) ++sum;
  }
  if (flag_size==3) {
    register unsigned int j;
    i=0;
    buf[30]='0';
    if (sum>0x3fffffff) {
      buf[31]='T';
      i=sum&0x3fffffff;
      j=i&0xfffff;
      i=(i>>20)+(j?1:0);
      sum>>=30;
    }
    else if (sum>0xfffff) {
      buf[31]='G';
      i=sum&0xfffff;
      j=i&0x3ff;
      i=(i>>10)+(j?1:0);
      sum>>=20;
    }
    else if (sum>0x3ff) {
      buf[31]='M';
      i=sum&0x3ff;
      sum>>=10;
    }
    else {
      buf[31]='k';
    }
    if (i) {
      for (j=1;i>102;++j) i-=103;
      if (j>=10) j=0,++sum;
      buf[30]+=j;
    }
  }
  i=fmt_ull(buf,sum);
  if (flag_size==3) {
    buf[i]='.';
    buf[++i]=buf[30];
    buf[++i]=buf[31];
    ++i;
  }
  buf[i]='\t';
  write(1,buf,++i);
  write(1,fn,fnl);
  write(1,"\n",1);
}

#define HASH_SIZE 127
#define hash_a_node(n) ((n)%HASH_SIZE)
struct ino_hash_bucket {
  struct ino_hash_bucket*next;
  unsigned long dev;
  unsigned long long ino;
} *ino_hashtable[HASH_SIZE];

int is_in_hash(const struct stat*st) {
  struct ino_hash_bucket*bucket;
  bucket=ino_hashtable[hash_a_node(st->st_ino)];
  while (bucket) {
    if ((bucket->ino==st->st_ino)&&(bucket->dev==st->st_dev)) return 1;
    bucket=bucket->next;
  }
  return 0;
}
void add_to_hash(const struct stat*st) {
  unsigned int i;
  struct ino_hash_bucket*bucket,*tmp;
  i=hash_a_node(st->st_ino);
  bucket=malloc(sizeof(struct ino_hash_bucket));
  bucket->next=0;
  bucket->ino=st->st_ino;
  bucket->dev=st->st_dev;
  tmp=ino_hashtable[i];
  if (tmp) {
    while(tmp->next) tmp=tmp->next;
    tmp->next=bucket;
  }
  else ino_hashtable[i]=bucket;
}
void clear_hash() {
  int i;
  for (i=0;i<HASH_SIZE;++i) {
    struct ino_hash_bucket*bucket=ino_hashtable[i];
    while (bucket) {
      struct ino_hash_bucket*tmp=bucket;
      bucket=bucket->next;
      free(tmp);
    }
    ino_hashtable[i]=0;
  }
}

unsigned int du_depth=0;
unsigned long du_xref;
unsigned long long du(char *fn) {
  struct stat st;
  unsigned long long sum=0;
  int fnl;
  fnl=(flag_asym)?stat(fn,&st):lstat(fn,&st);
  if (fnl==-1) {
    oops(fn);
    goto du_stat_out;
  }

  ++du_depth;

  if ((flag_dsym) && (du_depth==1) && (S_ISLNK(st.st_mode))) {
    stat(fn,&st);
  }

  if (flag_xref) {
    if (du_depth==1) du_xref=st.st_dev;
    else if (du_xref!=st.st_dev) {
      sum=0;
      goto du_out;
    }
  }

  fnl=strlen(fn);
  sum=(st.st_blocks>>1);

  if (S_ISDIR(st.st_mode)) {
    DIR*dir;
    struct dirent*de;
    if (!(dir=opendir(fn))) {
      sum=0;
      goto du_out;
    }
    if (fn[fnl-1]=='/') fn[--fnl]=0;
    while (de=readdir(dir)) {
      char *buf;
      char *name=de->d_name;
      int nl;
      if ((!strcmp(name,"..")) || (!strcmp(name,"."))) continue;
      nl=strlen(name);
      buf=alloca(fnl+nl+2);
      memcpy(buf,fn,fnl);
      buf[fnl]='/';
      memcpy(buf+fnl+1,name,nl);
      buf[fnl+nl+1]=0;
      sum+=du(buf);
    }
    closedir(dir);
    if ((!flag_sum)||(du_depth==1)) output(sum,fn,fnl);
    if (flag_sub) {
      total_sum+=sum;
      sum=0;
    }
  }
  else {
    if ((st.st_nlink>1)&&(!flag_count_hardlinks)) {
      if (is_in_hash(&st)) sum=0;
      else add_to_hash(&st);
    }
    if ((flag_all)||(du_depth==1)) output(sum,fn,fnl);
  }
du_out:
  --du_depth;
du_stat_out:
  return sum;
}

int main(int argc,char*argv[]) {
  int i,ret=0;
  unsigned long long sum;
  for(i=1;i<argc;++i) {
    if (argv[i][0]=='-') {
      int j;
      if ((argv[i][1]=='-')&&(!argv[i][2])) {
	++i;
	break;
      }
      for (j=1; argv[i][j]; ++j)
	switch (argv[i][j]) {
	case 'a': ++flag_all; break;
	case 'b': flag_size=-1; break;
	case 'c': ++flag_total; break;
	case 'D': ++flag_dsym; break;
	case 'h': flag_size=3; break;
	case 'k': flag_size=2; break;
	case 'L': ++flag_asym; break;
	case 'l': ++flag_count_hardlinks; break;
	case 'm': flag_size=1; break;
	case 'S': ++flag_sub; break;
	case 's': ++flag_sum; break;
	case 'x': ++flag_xref; break;
	default:
	  usage();
	  break;
	}
    }
    else break;
  }
  if (flag_size==2) flag_size=0;
  if (i>=argc) {
    if ((total_sum=du("."))==0) ret=1;
  }
  else {
    for (;i<argc;++i) {
      if (sum=du(argv[i])) total_sum+=sum;
      else ret=1;
      clear_hash();
    }
  }
  if (flag_total) output(total_sum,"total",5);
  return ret;
}
