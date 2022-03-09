/*
 * tail [-c N] [-f] [-n N] [-p PID] [-q] [-s SEC] [-v] FILE...
 * -c N  output the last N bytes
 * -f    follow
 * -n N  output the last N lines
 * -p P  with -f, terminate after process ID, P dies
 * -q    never output headers giving file names
 * -s S  with -f, each iteration lasts approximately S (default 1) seconds
 * -v    always output headers giving file names
 */

#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include "write12.h"

#include "error.c"

/* adds ~400 byte BUT speeds up a lot for LARGE files on slow devices or NFS */
#define WANT_REGFILE_SPEEDUP

void oops(const char *fn) {
  int err=errno;
  __write2("tail: ");
  if (fn) { __write2(fn); __write2(": "); }
  switch (err) {
  case 666:		warn("file truncated"); break;
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
  error("tail [-c N] [-f] [-n N] [-p PID] [-q] [-s SEC] [-v] [--] FILE...");
}

enum { LINES, BYTES } mode=LINES;
unsigned long buffersize=32768;
unsigned long number=10;
unsigned int sec=1;
unsigned int pid=0;
int flag_follow=0;
int flag_silent=0;
int flag_verbose=0;
int flag_plusmode=0;

int flag_skip=0;
int flag_header=1;
int nr_stdin=0;

int check_pid() {
  if (kill(pid,0)&&(errno==ESRCH)) return 1;
  return 0;
}

void waitsometime() {
  struct timespec ts;
  ts.tv_sec=sec;
  ts.tv_nsec=0;
  nanosleep(&ts,0);
}
void output_header(const char*fn,int nl) {
  __write1("\n==> "+nl);
  __write1(fn);
  __write1(" <==\n");
}

unsigned long n_files=0;
struct check_file {
  struct check_file*p,*n;
  int fd;
  const char*name;
  unsigned long long size;
} check_file_list = {
  &check_file_list,
  &check_file_list,
  0, 0
};

void add_file(int fd,const char*fn) {
  struct stat st;
  struct check_file*tmp;
  if (fd<0) {
    oops(fn);
    return;
  }
  fstat(fd,&st);
  if (S_ISDIR(st.st_mode)) return;
  tmp=malloc(sizeof(struct check_file));
  if (!tmp) return;
  tmp->n=&check_file_list;
  tmp->p=check_file_list.p;
  check_file_list.p->n=tmp;
  check_file_list.p=tmp;
  tmp->fd=fd;
  tmp->name=fn;
  tmp->size=(S_ISFIFO(st.st_mode))?0:st.st_size;
  ++n_files;
}
void del_file(struct check_file*tmp) {
  if (tmp!=&check_file_list) return;
  tmp->p->n=tmp->n;
  tmp->n->p=tmp->p;
  close(tmp->fd);
  free(tmp);
}

void check_files() {
  char buf[8192];
  int n;
  struct stat st;
  struct check_file*tmp;
  for (tmp=check_file_list.n;tmp!=&check_file_list;tmp=tmp->n) {
    if (fstat(tmp->fd,&st)) {
      oops(tmp->name);
      del_file(tmp);
    }
    else if (tmp->size!=st.st_size) {
      if (flag_header) output_header(tmp->name,n_files?0:1);
      if (tmp->size<st.st_size) {
	tmp->size=st.st_size;
	while ((n=read(tmp->fd,buf,sizeof(buf)))>0) write(1,buf,n);
      }
      else {
	tmp->size=st.st_size;
	errno=666;
	oops(tmp->name);
      }
    }
  }
}

void do_tail() {
  unsigned long long startpos;
  struct check_file*tmp;
  int i,n;
  for (tmp=check_file_list.n;tmp!=&check_file_list;tmp=tmp->n) {
    if (flag_header) output_header(tmp->name,(tmp==check_file_list.n)?1:0);
#ifdef WANT_REGFILE_SPEEDUP
    if (tmp->size) {	/* regular file */
      char buf[32768];
      if (mode==BYTES) {
	if (flag_plusmode)
	  startpos=number;
	else
	  if (tmp->size<number) startpos=0; else startpos=tmp->size-number;
	lseek(tmp->fd,startpos,SEEK_SET);
      } else {
	int len,buflen=sizeof(buf),count=0,found=0;
	if (flag_plusmode) ++count;
	if (tmp->size<sizeof(buf)) startpos=0; else startpos=tmp->size-sizeof(buf);
	while ((!found)) {
	  if (!flag_plusmode) lseek(tmp->fd,startpos,SEEK_SET);
	  if (buflen==0) goto out_all;
	  if ((len=read(tmp->fd,buf,buflen))==-1) { len=i=0; break; }
	  if (flag_plusmode) {
	    for (i=0; i<len; ++i)
	      if (buf[i]=='\n' && ++count>=number) {
		++found;
		write(1,buf+i+1,len-i-1);
		goto out_all;
	      }
	  } else {
	    for(i=len;(!found)&&(i>0);) {
	      if (buf[--i]=='\n') {
		++count;
		if (count>=(number+1)) ++found,++i;
	      }
	    }
	    if (startpos<sizeof(buf)) {
	      buflen=startpos;
	      startpos=0;
	    }
	    else startpos-=sizeof(buf);
	  }
	}
	if (len-i) write(1,buf+i,len-i);
      }
out_all:
      while((n=read(tmp->fd,buf,sizeof(buf)))>0)
	if (write(1,buf,n)!=n) {
	  __write2("tail: write error!\n");
	  break;
	}
    }
    else {		/* special cases: FIFO, PIPE, BLOCK/CHAR-DEVS */
#else
    {
#endif
      if (flag_plusmode) {
	if (mode==BYTES) {
	  char buf[4096];
	  while (number) {
	    long want=number>sizeof(buf)?sizeof(buf):number;
	    long len=read(tmp->fd,buf,want);
	    if (len==-1) oops(tmp->name);
	    if (len==0) return;
	    number-=len;
	  }
write_rest:
	  for (;;) {
	    long len=read(tmp->fd,buf,sizeof(buf));
	    if (len==-1) oops(tmp->name);
	    if (len==0) return;
	    if (write(1,buf,len)!=len) oops(tmp->name);
	  }
	} else {
	  char buf[4096];
	  unsigned long count;
	  count=1;
	  for (;;) {
	    long i,len=read(tmp->fd,buf,sizeof(buf));
	    if (len==-1) oops(tmp->name);
	    if (len==0) return;
	    for (i=0; i<len; ++i)
	      if (buf[i]=='\n' && ++count>=number) {
		write(1,buf+i+1,len-i-1);
		goto write_rest;
	      }
	  }
	}
      } else {
	if (mode==BYTES) {
	  unsigned long buflen=(number+16383)&(~8191);
	  char*buf;
	  if ((buf=mmap(0,buflen,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0))!=MAP_FAILED) {
	    char*e,*m,*p=buf;
	    e=p+(buflen-2048);
	    m=0;
	    while ((n=read(tmp->fd,p,2048))>0) {
	      p+=n;
	      if (p>e) {
		m=p;
		p=buf;
	      }
	    }
	    n=p-buf;
	    if (m && ((i=number-n)>0)) {
	      if (write(1,m-i,i) != i) goto writeerror;
	    }
	    if (n>number) n=number;
	    if (write(1,p-n,n) != n)
writeerror:   __write2("tail: write error!\n");
	    munmap(buf,buflen);
	  }
	} else {
	  struct ring_buf {
	    char*buf;
	    unsigned long len;
	    unsigned long a;
	  }*ring;

	  char buf[32768];
	  char*t=0,*p=buf;
	  int j=0,r=0;
	  unsigned long tl=0;

	  if (number>(4*1024*1024)) exit(2);
	  n=(number<<((sizeof(long)==8)?5:4));
	  ring=alloca(n);
	  memset(ring,0,n);

	  while ((n=read(tmp->fd,buf,sizeof(buf)))>0) {
	    for (i=0;i<n;++i) {
	      if (j>=tl) {
		char*nt;
		tl+=120;
		nt=realloc(t,tl);
		if (nt) t=nt;
		else goto err_out;
	      }
	      if ((t[j++]=buf[i])=='\n') {
		ring[r].buf=t;
		ring[r].len=j;
		ring[r].a=tl;
		r=(r+1)%number;
		t=ring[r].buf;
		tl=ring[r].a;
		j=0;
	      }
	    }
	  }
	  if (j) {
	    ring[r].buf=t;
	    ring[r].len=j;
	    ++r;
	  }
	  for (i=0;i<number;++i) {
	    if (ring[r].buf) write(1,ring[r].buf,ring[r].len);
	    r=(r+1)%number;
	  }
  err_out:
	  for (i=0;i<number;++i) {
	    if (ring[i].buf) free(ring[i].buf);
	  }
	}
      }
    }
  }
}

void do_skip() {
  char buf[32768];
  struct check_file*tmp;
  int n,i;
  for (tmp=check_file_list.n;tmp!=&check_file_list;tmp=tmp->n) {
    if (flag_header) output_header(tmp->name,n_files?0:1);
    if (mode==BYTES) {
      lseek(tmp->fd,number,SEEK_SET);
    }
    else {
      int count=1;
      i=n=0;
      while((count<number)&&((n=read(tmp->fd,buf,sizeof(buf)))>0)) {
	for (i=0;(count<number)&&(i<n);++i) {
	  if (buf[i]=='\n') {
	    ++count;
	  }
	}
      }
      if (n-i>0) write(1,buf+i,n-i);
    }
    while ((n=read(tmp->fd,buf,sizeof(buf)))>0) write(1,buf,n);
  }
}

unsigned long n2a(const char*c) {
  unsigned long l=0;
  if (*c=='+') {
    flag_plusmode=1; ++c;
  }
  while (isdigit(*c)) {
    l=l*10+*c-'0';
    ++c;
  }
  if (*c) mode=BYTES;
  switch (*c) {
  case 'c': if (mode==LINES) mode=BYTES; break;
  case 'b': l*=512; break;
  case 'k': l*=1024; break;
  case 'm': l*=1024*1024; break;
  case 0: break;
  default: usage();
  }
  return l;
}
int main(int argc,char*argv[]) {
  int i,ret=0;
  signal(SIGPIPE,SIG_IGN);
  for(i=1;i<argc;++i) {
    if (argv[i][0]=='+') {
      ++flag_skip;
      mode=LINES;
      number=n2a(&argv[i][1]);
    }
    else if (argv[i][0]=='-') {
      if (!argv[i][1]) break;	/* Oh oh... stdin ... */
      if ((argv[i][1]=='-')&&(!argv[i][2])) {	/* -- */
	++i;
	break;
      }
      switch (argv[i][1]) {
      case '0' ... '9':
	mode=LINES;
	number=n2a(&argv[i][1]);
	break;
      case 'c':
	mode=BYTES;
	if (argv[i][2]) number=n2a(&argv[i][2]); else number=n2a(argv[++i]);
	break;
      case 'n':
	mode=LINES;
	if (argv[i][2]) number=n2a(&argv[i][2]); else number=n2a(argv[++i]);
	if (mode!=LINES) goto def;
	break;
      case 'p':
	if (argv[i][2]) pid=atoi(&argv[i][2]); else pid=atoi(argv[++i]);
	break;
      case 's':
	if (argv[i][2]) sec=atoi(&argv[i][2]); else sec=atoi(argv[++i]);
	break;
      case 'f': ++flag_follow; break;
      case 'q': ++flag_silent; break;
      case 'v': ++flag_verbose; break;
      default:
      def:
	usage();
	break;
      }
    }
    else break;
  }
  if (i>=argc) {
    add_file(0,"standard input");
    ++nr_stdin;
  }
  else {
    for (;i<argc;++i) {
      if ((argv[i][0]=='-')&&(!argv[i][1])) ++nr_stdin,add_file(0,"standard input");
      else add_file(open(argv[i],O_RDONLY),argv[i]);
    }
  }

  if (nr_stdin>1) flag_follow=0;	/* hope this is ok */
  if (n_files==1) flag_header=0;
  if (flag_verbose) flag_header=1;
  if (flag_silent) flag_header=0;

#if 0
  printf("flag_follow:%d\nflag_silent:%d\nflag_verbose:%d\nflag_header:%d\n"
	 "number:%d %s\npid:%d\nsec:%d\nn_files:%d\n",
	 flag_follow,flag_silent,flag_verbose,flag_header,
	 number,(mode==LINES)?"LINES":"BYTES",pid,sec,n_files);
#endif

  if (flag_skip) do_skip();
  else do_tail();

  if (flag_follow) {
    while(1) {
      if (pid && check_pid()) return 0;
      check_files();
      waitsometime();
    }
  }
  return ret;
}
