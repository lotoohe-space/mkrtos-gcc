/* ls [-1ACFLRSabcdfgilmnopqrstux] [file...]
   -1 single-column format
 ~ -C lists output in vertically (down the page) sorted columns
   -F append / for dir, * for executable, @ for symlink, = for sockets, | for fifo
   -L follow symlinks
   -R recurse
   -S sort by size
   -a also list files starting with "."
   -b display non-printable characters octal (\ddd)
   -c sort by ctime
   -d list directory, not it's contents
   -f unsorted (turn off -l, turn on -a)
   -g ignored
   -i prepend inode number
   -l long format
   -m stream format (separated by ", ", no new-lines)
   -n show numerical uid/gid
   -o owner only, don't display group
   -p append / after directories
   -r reverse sort
   -s prepend file size in blocks (after inode number, if -i is also given)
   -t sort by mtime
   -u sort by atime
   -x lists output in horizontally sorted columns
 */
#include "embfeatures.h"
#ifdef SUPPORT_64BIT_FILESIZE
#define _FILE_OFFSET_BITS 32
#endif
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <limits.h>

struct statlist {
  struct stat s;
  struct statlist* next;
};

static unsigned int ttywidth=80;

#ifdef SUPPORT_64BIT_FILESIZE
static unsigned int fmt_ulonglong(char *dest,unsigned long long i) {
  register unsigned long len,len2;
  register unsigned long long tmp;
  for (len=1, tmp=i; tmp>9; ++len) tmp/=10;
  if (dest)
    for (tmp=i, dest+=len, len2=len+1; --len2; tmp/=10)
      *--dest = (tmp%10)+'0';
  return len;
}

static unsigned int fmt_ulong(char* dest,unsigned long i) {
  return fmt_ulonglong(dest,i);
}
#else
#include "fmt_ulong.c"
#endif

#define DONT_NEED_PUT
#define DONT_NEED_PUTULONG
#define WANT_BUFFER_2
#include "buffer.c"

/* don't change the order here! */
enum filetype { REGULAR, SYMLINK, BROKENLINK, SUBDIR, SOCKET, FIFO, CHAR, BLOCK };

struct de {
  const char *name,*link;
  struct stat s,fs;
  enum filetype type;
  char qual;
  struct de *next;
} *root_files,*root_dirs,*last_files,*last_dirs;

unsigned int num_files,num_dirs;

static int _A,_F,_L,_R,_a,_b,_d,_i,_n,_o,_p,_r,_s;

#ifdef SUPPORT_HUMAN_READABLE_SIZE
static int _h,_H;
#endif

#ifdef SUPPORT_COLORS
static char* colors;
#endif

static time_t now;

enum {
  SINGLECOL,
  VERTICAL,
  HORIZONTAL,
  LONG,
  COMMAS
} display=VERTICAL;

int byname(struct de** a,struct de** b) {
  int res=strcmp((*a)->name,(*b)->name);
  return _r?-res:res;
}

int bysize(struct de** a,struct de** b) {
  int res;
  if ((*a)->s.st_size>(*b)->s.st_size)
    res=1;
  else if ((*a)->s.st_size<(*b)->s.st_size)
    res=-1;
  else return byname(a,b);
  return _r?res:-res;
}

int byctime(struct de** a,struct de** b) {
  int res=(*a)->s.st_ctime-(*b)->s.st_ctime;
  if (res==0) return byname(a,b);
  return _r?res:-res;
}

int bymtime(struct de** a,struct de** b) {
  int res=(*a)->s.st_mtime-(*b)->s.st_mtime;
  if (res==0) return byname(a,b);
  return _r?res:-res;
}

int byatime(struct de** a,struct de** b) {
  int res=(*a)->s.st_atime-(*b)->s.st_atime;
  if (res==0) return byname(a,b);
  return _r?res:-res;
}

int (*compare)(const void *a,const void *b)=(int (*)(const void *,const void *))byname;

static int res;

static inline void msg(const char *message) {
  buffer_puts(buffer_2,message);
}

static void warn(const char *message) {
  msg(message); buffer_putc(buffer_2,'\n');
  buffer_flush(buffer_2);
}

static void oops(const char *fn) {
  msg("ls: ");
  if (fn) { msg(fn); msg(": "); }
  switch (errno) {
  case EPERM:		warn("permission denied"); break;
  case EEXIST:		warn("file exists"); break;
  case ENAMETOOLONG:	warn("name too long"); break;
  case ENOENT:		warn("file not found"); break;
  case ENOMEM:		warn("out of virtual memory"); break;
  case EACCES:		warn("directory search permissions denied"); break;
  case ELOOP:		warn("too many symbolic links"); break;
  case EIO:		warn("I/O error"); break;
  default: warn("unknown error");
  }
  res=1;
}

static void nomem(void) {
  warn("ls: out of virtual memory");
  exit(1);
}

static unsigned int str_copy(char *out,const char *in) {
  register char* s=out;
  register const char* t=in;
  for (;;) {
    if (!(*s=*t)) break; ++s; ++t;
#if 0
    if (!(*s=*t)) break; ++s; ++t;
    if (!(*s=*t)) break; ++s; ++t;
    if (!(*s=*t)) break; ++s; ++t;
#endif
  }
  return s-out;
}

static void qualify(struct de* d) {
  if (S_ISDIR(d->s.st_mode)) d->type=SUBDIR;
  else if (S_ISLNK(d->s.st_mode)) {
    if (d->type!=BROKENLINK) d->type=SYMLINK;
  } else if (S_ISSOCK(d->s.st_mode)) d->type=SOCKET;
  else if (S_ISFIFO(d->s.st_mode)) d->type=FIFO;
  else if (S_ISCHR(d->s.st_mode)) d->type=CHAR;
  else if (S_ISBLK(d->s.st_mode)) d->type=BLOCK;
  else d->type=REGULAR;
  d->qual=0;
  if (_F || _p) {
    static char append[]={0,'@','@','/','=','|',0,0};
    char c=append[d->type];
    if (_F && c==0 && d->s.st_mode&0111) c='*';
    d->qual=c;
  }
}

static struct statlist* root;

static int circular(struct stat* s) {
  struct statlist* x=root;
  while (x) {
    if (x->s.st_dev==s->st_dev && x->s.st_ino==s->st_ino)
      return 1;
    x=x->next;
  }
  return 0;
}

static void dols(const char* fn,const char* expanded,int ignoreenoent) {
  struct de* nn;
  struct stat s;

  if (_L?stat(fn,&s):lstat(fn,&s)) {
    if (!ignoreenoent) {
      oops(fn);
      res=-1;
    }
    return;
  }
  if (S_ISDIR(s.st_mode)) {
    /* check if we already visited this directory */
    if (circular(&s)) return;
    /* add this directory to the list */
    {
      struct statlist* tmp=malloc(sizeof(struct statlist));
      if (!tmp) { res=-1; write(2,"ls: out of memory.\n",19); exit(1); }
      tmp->next=root;
      tmp->s=s;
      root=tmp;
    }
  }
  if (!(nn=(struct de*)malloc(sizeof *nn))) nomem();
  nn->type=REGULAR;
  nn->s=nn->fs=s;
  nn->name=strdup(fn);
  nn->link=0;
  if (S_ISLNK(s.st_mode)) {
    if (stat(fn,&nn->fs))
      nn->type=BROKENLINK;
    if (display==LONG) {
      char buf[PATH_MAX];
      int len;
      if ((len=readlink(fn,buf,PATH_MAX-1))>0) {
	buf[len]=0;
	nn->link=strdup(buf);
      } else
	nn->type=BROKENLINK;
    }
  }

  qualify(nn);
  nn->next=0;
  if (_d==0 && S_ISDIR(s.st_mode)) {
alsoaddtodirs:
    if (root_dirs)
      last_dirs=(last_dirs->next=nn);
    else
      root_dirs=last_dirs=nn;
    ++num_dirs;
  } else {
    if (root_files)
      last_files=(last_files->next=nn);
    else
      root_files=last_files=nn;
    ++num_files;
    if (_R && S_ISDIR(s.st_mode)) {
      struct de* tmp=nn;
      if (fn[0]=='.' && (!fn[1] || (fn[1]=='.' && !fn[2]))) return;
      if (!(nn=malloc(sizeof *nn))) nomem();
      if (!(nn->name=strdup(expanded))) nomem();
      if (nn->link=tmp->link)
	if (!(nn->link=strdup(tmp->link))) nomem();
      nn->type=tmp->type;
      nn->s=tmp->s;
      goto alsoaddtodirs;
    }
  }
}

static int __printname(struct de* d) {
  int res=strlen(d->name);
#ifdef SUPPORT_COLORS
  char *tmp=colors;
  char *normal=0;
  char *file=0;
  char *found=0;
// no=0:fi=0:di=32:ln=36:or=1;40:mi=1;40:pi=31:so=33:bd=44;37:cd=44;37:ex=35:*.jpg=1;32:*.jpeg=1;32:*.JPG=1;32:*.gif=1;32:*.png=1;32:*.jpeg=1;32:*.ppm=1;32:*.pgm=1;32:*.pbm=1;32:*.c=1;33:*.C=1;33:*.h=1;33:*.cc=1;33:*.awk=1;33:*.pl=1;33:*.gz=1;31:*.bz2=1;31:*.tar=1;31:*.zip=1;31:*.lha=1;31:*.lzh=1;31:*.arj=1;31:*.tgz=1;31:*.taz=1;31:*.ace=1;31:*.html=1;34:*.htm=1;34:*.doc=1;34:*.txt=1;34:*.o=1;36:*.a=1;36:*.wav=1;37:*.mp3=1;37:*.ogg=1;37:*.avi=1;35:*.mpg=1;35:*.mov=1;35:*.mpeg=1;35
  while (tmp) {
    switch (*tmp) {
      /* tags:
       *   "bd" -> block device
       *   "cd" -> char device
       *   "di" -> dir
       *   "do" -> door
       *   "ex" -> excutable
       *   "fi" -> file
       *   "ln" -> link
       *   "no" -> normal
       *   "or" -> orphan
       *   "pi" -> pipe
       *   "so" -> socket */
    case 'b': if (d->type==BLOCK) found=tmp+3; break;
    case 'c': if (d->type==CHAR) found=tmp+3; break;
    case 'd': if (tmp[1]=='i' && d->type==SUBDIR) found=tmp+3; break;
    case 'e': if (d->s.st_mode&0111) found=tmp+3; break;
    case 'f': if (d->type==REGULAR) file=tmp+3; break;
    case 'l': if (d->type==SYMLINK) found=tmp+3; break;
    case 'n': normal=tmp+3; break;
    case 'o': if (d->type==BROKENLINK) found=tmp+3; break;
    case 'p': if (d->type==FIFO) found=tmp+3; break;
    case 's': if (d->type==SOCKET) found=tmp+3; break;
    case '*':
      {
	char *x;
	tmp++;
	if ((x=strchr(tmp,'='))) {
	  *x=0;
	  if (!strcmp(tmp,d->name+strlen(d->name)-(x-tmp)))
	    found=x+1;
	  *x='=';
	}
      }
    }
    if (found) break;
    if ((tmp=strchr(tmp,':'))) ++tmp;
  }
  if (!found && d->type==REGULAR) found=file;
  if (found) {
    char *x=strchr(found,':');
    buffer_puts(buffer_1,"\e[");
    if (x) *x=0;
    buffer_puts(buffer_1,found);
    if (x) *x=':';
    buffer_putc(buffer_1,'m');
  }
#endif
  buffer_puts(buffer_1,d->name);
#ifdef SUPPORT_COLORS
  if (found)
    buffer_puts(buffer_1,"\e[0m");
#endif
  if (d->qual) {
    ++res;
    buffer_putc(buffer_1,d->qual);
  }
  return res;
}

static void printname(struct de* d) {
  __printname(d);
  buffer_putc(buffer_1,'\n');
}

#ifdef SUPPORT_SYMBOLIC_UID_GID
#define EMBEDDED
#include "openreadclose.c"
#include "split.c"
#define DONT_NEED_PASSWD
#define DONT_NEED_GECOS
#define DONT_NEED_DIR
#define DONT_NEED_SHELL
#include "mygetpwnam.c"
#define DONT_NEED_MEMBERS
#include "mygetgrnam.c"

struct cache {
  char *name;
  unsigned long i;
  struct cache *next;
};

static const char *group(gid_t g) {
  struct group *gr;
  static struct cache *root=0;
  struct cache *tmp;
  for (tmp=root; tmp&&tmp->i!=g; tmp=tmp->next);
  if (tmp) return tmp->name;
  gr=getgrgid(g);
  if (!(tmp=malloc(sizeof(struct cache)))) nomem();
  tmp->next=root;
  root=tmp;
  if (gr) {
    tmp->name=gr->gr_name;
    tmp->i=gr->gr_gid;
  } else {
    tmp->i=g;
    if (!(tmp->name=malloc(fmt_ulong(0,g)+1))) nomem();
    tmp->name[fmt_ulong(tmp->name,g)]=0;
  }
  return tmp->name;
}

static const char *user(uid_t u) {
  struct passwd *pw;
  static struct cache *root=0;
  struct cache *tmp;
  for (tmp=root; tmp&&tmp->i!=u; tmp=tmp->next);
  if (tmp) return tmp->name;
  pw=getpwuid(u);
  if (!(tmp=malloc(sizeof(struct cache)))) nomem();
  tmp->next=root;
  root=tmp;
  if (pw) {
    tmp->name=pw->pw_name;
    tmp->i=pw->pw_uid;
  } else {
    tmp->i=u;
    if (!(tmp->name=malloc(fmt_ulong(0,u)+1))) nomem();
    tmp->name[fmt_ulong(tmp->name,u)]=0;
  }
  return tmp->name;
}
#endif

static void fmt_num2(char *dest,int i) {
  dest[0]=i/10+'0';
  dest[1]=i%10+'0';
}

#ifdef SUPPORT_64BIT_FILESIZE
static void __printrjnum(unsigned long long n,unsigned width,char c) {
  char buf[60];
  int i,len;
  buf[len=fmt_ulonglong(buf,n)]=c;
  buf[++len]=0;
  for (i=0; i<((int)width-len); ++i) buffer_putc(buffer_1,' ');
  buffer_puts(buffer_1,buf);
}

static inline void printrjnum(unsigned long long n,unsigned width) {
  __printrjnum(n,width,' ');
}
#else
static void __printrjnum(unsigned long n,unsigned width,char c) {
  char buf[60];
  int i,len;
  buf[len=fmt_ulong(buf,n)]=c;
  buf[++len]=0;
  for (i=0; i<((int)width-len); ++i) buffer_putc(buffer_1,' ');
  buffer_puts(buffer_1,buf);
}

static inline void printrjnum(unsigned long n,unsigned width) {
  __printrjnum(n,width,' ');
}
#endif

void printlong(struct de* d) {
  static char append[]={'-','l','l','d','s','p','c','b'};
  char buf[30];
  char *c=buf;
#ifdef SUPPORT_COLORS
  enum filetype saved=d->type;
#endif

  if (_i) /* " 123456 " */
    printrjnum(d->s.st_ino,8);
  if (_s) /* "   4 " */
    printrjnum(d->s.st_blocks,5);
/* "-rw-r--r--    1 leitner  users        5818 Apr 12 17:28 ls.c"*/
  /* "-rw-r--r--" */

  *c=append[d->type];

  *++c=d->s.st_mode&S_IRUSR?'r':'-';
  *++c=d->s.st_mode&S_IWUSR?'w':'-';
  ++c; if (d->s.st_mode&S_IXUSR) *c=d->s.st_mode&S_ISUID?'s':'x'; else *c=d->s.st_mode&S_ISUID?'S':'-';
  *++c=d->s.st_mode&S_IRGRP?'r':'-';
  *++c=d->s.st_mode&S_IWGRP?'w':'-';
  ++c; if (d->s.st_mode&S_IXGRP) *c=d->s.st_mode&S_ISGID?'s':'x'; else *c=d->s.st_mode&S_ISGID?'S':'-';
  *++c=d->s.st_mode&S_IROTH?'r':'-';
  *++c=d->s.st_mode&S_IWOTH?'w':'-';
  ++c; if (d->s.st_mode&S_IXOTH) *c=d->s.st_mode&S_ISVTX?'t':'x'; else *c=d->s.st_mode&S_ISVTX?'T':'-';
  *++c=0;
  buffer_puts(buffer_1,buf);

  /* "    1 " */
  printrjnum(d->s.st_nlink,6);

  /* "leitner  " */
  memset(buf,' ',30); buf[9]=0;
#ifdef SUPPORT_SYMBOLIC_UID_GID
  if (_n) {
#endif
    fmt_ulong(buf,d->s.st_uid);
    buffer_puts(buffer_1,buf);
#ifdef SUPPORT_SYMBOLIC_UID_GID
  } else {
    const char *tmp=user(d->s.st_uid);
    int len=strlen(tmp);
    buffer_puts(buffer_1,tmp);
    do {
      buffer_putc(buffer_1,' ');
    } while (++len<9);
  }
#endif
  /* "users    " */
  memset(buf,' ',30); buf[9]=0;
#ifdef SUPPORT_SYMBOLIC_UID_GID
  if (_n) {
#endif
    fmt_ulong(buf,d->s.st_gid);
    buffer_puts(buffer_1,buf);
#ifdef SUPPORT_SYMBOLIC_UID_GID
  } else {
    const char *tmp=group(d->s.st_gid);
    int len=strlen(tmp);
    buffer_puts(buffer_1,tmp);
    do {
      buffer_putc(buffer_1,' ');
    } while (++len<9);
  }
#endif

  if (S_ISCHR(d->s.st_mode) || S_ISBLK(d->s.st_mode)) {
    /* "  1,   3 " */
    __printrjnum(major(d->s.st_rdev),4,',');
    printrjnum(minor(d->s.st_rdev),5);
  } else { /* "    5818 " */
#ifdef SUPPORT_HUMAN_READABLE_SIZE
    unsigned long a,b;
    char c=0;
    if (_H) {
      if (d->s.st_size>1000000000) {
	a=(d->s.st_size+500000000)/100000000;
	c='G';
      } else if (d->s.st_size>1000000) {
	a=(d->s.st_size+50000)/100000;
	c='M';
      } else if (d->s.st_size>1000) {
	a=(d->s.st_size+50)/100;
	c='k';
      }
      if (c) {
	b=a%10;
	a/=10;
      }
    } else if (_h) {
      if (d->s.st_size>=1024*1024*1024) {
	a=(d->s.st_size+(1024*1024*1024/20))/(1024*1024*1024/10);
	c='G';
      } else if (d->s.st_size>=1024*1024) {
	a=(d->s.st_size+(1024*1024/20))/(1024*1024/10);
	c='M';
      } else if (d->s.st_size>=1024) {
	a=(d->s.st_size+(1024/20))/(1024/10);
	c='k';
      }
      if (c) {
	b=a%10;
	a/=10;
      }
    }
    if (c) {
      __printrjnum(a,6,'.');
      __printrjnum(b,2,c);
      buffer_putc(buffer_1,' ');
    } else
#endif
    printrjnum(d->s.st_size,9);
  }

  /* "Apr 12 17:28 " */
  {
    static char *smonths[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    struct tm* T;
    time_t t;
    char buf[20];
    if ((void*)compare==(void*)byctime) t=d->s.st_ctime;
    else if ((void*)compare==(void*)byatime) t=d->s.st_atime;
    else t=d->s.st_mtime;
    T=localtime(&t);
    buffer_puts(buffer_1,smonths[T->tm_mon]);
    memset(buf,' ',sizeof(buf));
    fmt_num2(buf+1,T->tm_mday);
    if (buf[1]=='0') buf[1]=' ';
    if (t>now||now-t>60*60*24*365/2) {
      fmt_num2(buf+5,(T->tm_year+1900)/100);
      fmt_num2(buf+7,(T->tm_year+1900)%100);
    } else {
      fmt_num2(buf+4,T->tm_hour);
      buf[6]=':';
      fmt_num2(buf+7,T->tm_min);
    }
    buf[10]=0;
    buffer_puts(buffer_1,buf);
  }
  /* "ls.c" */
  if (d->type==SYMLINK || d->type==BROKENLINK) {
    d->qual=0;
#ifdef SUPPORT_COLORS
    d->type=SYMLINK;
#endif
  }
  __printname(d);
  if (d->type==SYMLINK) {
    const char *tmp=d->name;
    buffer_puts(buffer_1," -> ");
#ifdef SUPPORT_COLORS
    d->type=saved;
    d->s=d->fs;
#endif
    d->name=d->link;
    if (d->type!=BROKENLINK)
      qualify(d);
    if (!d->name) d->name="[readlink failed]";
    __printname(d);
    d->name=tmp;
  }
  buffer_putc(buffer_1,'\n');
}

static int displaytotal=0;

static void lsfiles() {
  unsigned long total=0;
  struct de **array=alloca(sizeof(struct de*)*num_files);
  {
    struct de* tmp=root_files;
    int i;
    for (i=0; i<num_files; ++i) {
      array[i]=tmp;
      total+=tmp->s.st_blocks;
      tmp=tmp->next;
    }
  }
  if (compare)
    qsort(array,num_files,sizeof(struct de*),compare);
  {
    unsigned int i,j,k,rows,ttycols,thiscol,cols=1;
    unsigned int *columns=alloca(sizeof(unsigned int)*num_files);
    if (displaytotal) {
      char buf[30];
      int len;
      buffer_puts(buffer_1,"total ");
      buf[len=fmt_ulong(buf,total)]='\n';
      buf[++len]=0;
      buffer_puts(buffer_1,buf);
    }
    columns[0]=1;
    switch (display) {
    case VERTICAL:
      if (!num_files) break;
      /* algorithm: try increasing rows until it fits */
      for (rows=1; rows<num_files; ++rows) {
	cols=ttycols=thiscol=0;
	/* see if it fits */
	for (k=i=j=0; j<num_files; ++j) {
	  int len;
	  if (i>=rows) {
	    i=0;
	    ttycols+=thiscol+2;
	    columns[cols]=thiscol;
	    thiscol=0;
	    ++cols;
	  }
	  if (thiscol<(len=strlen(array[k]->name)+(!!array[k]->qual))) thiscol=len;
	  ++i; ++k;
	}
	ttycols+=thiscol;
	if (ttycols<ttywidth) break;
      }
      {
	for (j=0; j<rows; ++j) {
	  for (i=0; i<=cols; ++i) {
	    if (i*rows+j<num_files) {
	      int k=__printname(array[i*rows+j]);
	      if (i<cols) {
		while (k<columns[i]+2) {
		  ++k;
		  buffer_putc(buffer_1,' ');
		}
	      }
	    }
	  }
	  buffer_putc(buffer_1,'\n');
	}
      }
      break;
    default:
      for (i=0; i<num_files; ++i) {
	switch (display) {
	  case SINGLECOL: printname(array[i]); break;
	  case LONG: printlong(array[i]); break;
	  default: warn("display mode not supported yet."); exit(1);
	}
      }
    }
  }
  displaytotal=1;
}

static void ls_dir(const char* name) {
  struct de* tmp;
  DIR* d;
  struct dirent *e;
  int savedir=open(".",O_RDONLY);
  if (savedir<0) oops(".");

  if (chdir(name)) {
    oops(name);
    close(savedir);
    return;
  }
  /* free old file list */
  while (num_files) {
    tmp=root_files;
    root_files=root_files->next;
    free((char*)tmp->link);
    free((char*)tmp->name);
    free(tmp);
    --num_files;
  }

  /* read directory */
  if (!(d=opendir("."))) {
    oops(name);
    return;
  }
  _d=1;
  while ((e=readdir(d))) {
    int len;
    char *expanded;
    if (e->d_name[0]=='.') {
      /* is it "." or ".."? */
      if (e->d_name[1]==0 || (e->d_name[1]=='.' && e->d_name[2]==0))
	if (_A || !_a) continue;
      if (!_A && !_a) continue;
    }
    expanded=alloca(strlen(name)+strlen(e->d_name)+3);
    len=str_copy(expanded,name);
    expanded[len]='/'; ++len;
    expanded[len+str_copy(expanded+len,e->d_name)]=0;
    dols(e->d_name,expanded,1);
  }
  closedir(d);
  if (fchdir(savedir))
    oops("fchdir");
  close(savedir);
  lsfiles();
}

int main(int argc,char *argv[]) {
  int i,args;
  args=0;
  time(&now);

#ifdef SUPPORT_COLORS
  colors=getenv("LS_COLORS");
#endif

  for (i=1; i<argc; ++i)
    if (argv[i][0]!='-') {
      ++args;
    } else {
      int j;
      for (j=1; argv[i][j]!=0; ++j) {
	char buf[2];
	switch (argv[i][j]) {
	case '1': display=SINGLECOL; break;
	case 'l': display=LONG; break;
	case 'C': display=VERTICAL; break;
	case 'x': display=HORIZONTAL; break;
	case 'm': display=COMMAS; break;

	case 'S': compare=(int (*)(const void *,const void *))bysize; break;
	case 'c': compare=(int (*)(const void *,const void *))byctime; break;
	case 't': compare=(int (*)(const void *,const void *))bymtime; break;
	case 'u': compare=(int (*)(const void *,const void *))byatime; break;
	case 'f': compare=0; break;

#ifdef SUPPORT_HUMAN_READABLE_SIZE
	case 'H': _H=1; break;
	case 'h': _h=1; break;
#endif
	case 'A': _A=1; break;
	case 'F': _F=1; break;
	case 'L': _L=1; break;
	case 'R': _R=1; break;
	case 'a': _a=1; break;
	case 'b': _b=1; break;
	case 'd': _d=1; break;
	case 'g': break;
	case 'i': _i=1; break;
	case 'n': _n=1; break;
	case 'o': _o=1; break;
	case 'p': _p=1; break;
	case 'r': _r=1; break;
	case 's': _s=1; break;
	default:
	  {
	    char buf[]="ls: invalid option: _\n";
	    buf[20]=argv[i][j];
	    write(2,buf,22);
	  }
	  exit(1);
	}
      }
    }

  {
    struct winsize tmp;
    if (ioctl(1,TIOCGWINSZ,&tmp)) {
      if (display==VERTICAL || display==HORIZONTAL)
	display=SINGLECOL;
#ifdef SUPPORT_COLORS
      colors=0;
#endif
    } else
      ttywidth=tmp.ws_col;
  }

  if (args<1)
    dols(".",".",0);
  else
    for (i=1; i<argc; ++i) {
      if (argv[i][0]!='-')
	dols(argv[i],argv[i],0);
    }
  {
    int need_prefix=0;
    int notfirst=0;
    if (num_dirs>1) need_prefix=displaytotal=1;
    if (num_files) {
      lsfiles();
      need_prefix=displaytotal=1;
      notfirst=1;
    }
    while (num_dirs) {
      struct de* tmp=root_dirs;
      if (need_prefix) {
	if (notfirst) buffer_putc(buffer_1,'\n');
	buffer_puts(buffer_1,root_dirs->name);
	buffer_puts(buffer_1,":\n");
      }
      notfirst=1;
      ls_dir(root_dirs->name);
      root_dirs=root_dirs->next;
      free((char*)tmp->name);
      free(tmp);
      need_prefix=1;
      --num_dirs;
    }
  }
  buffer_flush(buffer_1);
  return res;
}
