#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include <sys/uio.h>

/*
 * if=FILE	read from FILE instead of stdin
 * of=FILE	write to  FILE instead of stdout
 *
 * ibs=BYTES	read  BYTES bytes at a time
 * obs=BYTES	write BYTES bytes at a time
 *  bs=BYTES	ibs=obs=BYTES
 *
 * count=BLOCKS	copy only BLOCKS input blocks
 *
 * seek=BLOCKS	skip BLOCKS obs-sized blocks at start of output
 * skip=BLOCKS	skip BLOCKS ibs-sized blocks at start of input
 *
 * missing:
 * conv=TYPE
 * cbs=BYTES	convert only BYTES bytes at a time
 */

void die(const char* s,const char* p) {
  struct iovec tmp[4] = { { (void*)s, strlen(s) }, {" ",1}, { (void*)p, strlen(p) }, {"\n", 1} };
  writev(2,tmp,4);
  exit(0);
}

#define PAGE_ALIGN(s)	(((s)+(PAGE_SIZE-1))&(~(PAGE_SIZE-1)))

void *get_block(size_t size) {
  return mmap(0,PAGE_ALIGN(size),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
}

void seek_skip(int fd, size_t count, size_t block) {
  char *buf=get_block(block);
  size_t dist=count*block;
  if (lseek(fd,SEEK_CUR,dist)==-1) {
    for (;dist && count;count--) {
      if ((dist=read(fd,buf,block))==-1) {
	die("error while","seek/skip");
      }
    }
  }
  munmap(buf,PAGE_ALIGN(block));
}

static inline unsigned int SOUL() { return (sizeof(unsigned long)==8)?64:32; }
size_t get_block_size(const char *s) {
  size_t ret=0;
  char *r=0;
  ret=strtoul(s, &r, 10);
  if (s==r) return 0;
  if (*r==0) ; /* no size type */
  else if  (*r=='c') ;
  else if ((*r=='w')&&(!(ret&(1<<(SOUL()-1)))))
    ret<<=1;
  else if ((*r=='b')&&(ret<(1<<(SOUL()- 9))))
    ret<<=9;
  else if ((*r=='k')&&(ret<(1<<(SOUL()-10))))
    ret<<=10;
  else if ((*r=='M')&&(ret<(1<<(SOUL()-20))))
    ret<<=20;
  else if ((*r=='G')&&(ret<(1<<(SOUL()-30))))
    ret<<=30;
  else die("unknown or too large block:",s);
  return ret;
}

int main(int argc, char *argv[]) {
  int in=0, out=1, i;
  void *ibuf, *obuf;
  size_t ibs=512, obs=512,obn=0;
  size_t count=(size_t)-1;
  size_t seek=0, skip=0;
  size_t inumber,onumber;
  {
    for (i=1;i<argc;i++) {
      if (!memcmp(argv[i],"if=",3)) {
	if ((in=open(argv[i]+3,O_RDONLY))<0) die("can't open:","input");
      }
      else if (!memcmp(argv[i],"of=",3)) {
	if ((out=open(argv[i]+3,O_RDWR|O_CREAT,0666))<0) die("can't open:","output");
      }
      else if (!memcmp(argv[i],"ibs=",4)) {
	ibs=get_block_size(argv[i]+4);
      }
      else if (!memcmp(argv[i],"obs=",4)) {
	obs=get_block_size(argv[i]+4);
      }
      else if (!memcmp(argv[i],"bs=",3)) {
	ibs=obs=get_block_size(argv[i]+3);
      }
      else if (!memcmp(argv[i],"count=",6)) {
	count=get_block_size(argv[i]+6);
      }
      else if (!memcmp(argv[i],"seek=",5)) {
	seek=get_block_size(argv[i]+5);
      }
      else if (!memcmp(argv[i],"skip=",5)) {
	skip=get_block_size(argv[i]+5);
      }
      else die("unknown operation:",argv[i]);
    }
  }

  if (seek) seek_skip(out,seek,obs);
  if (skip) seek_skip( in,skip,ibs);

  ibuf=get_block(ibs);
  obuf=get_block(obs);

  for (inumber=0;inumber<count;++inumber) {
    size_t n,m,b=0;
    char*tmp;
    if ((n=read(in,ibuf,ibs))==-1) die("error while","read input");
    if (!n) b=1;

    tmp=ibuf;

    if (obn) {
      m=n+obn;
      if (m>=obs) {
	register size_t d=(obs-obn);
	memcpy(obuf+obn,tmp,d);
	obn=0;
	if (write(out,obuf,obs)!=obs) die("error while","write output");
	++onumber;
	tmp+=d;
	n=m-obs;
      }
    }
    m=n;
    for (;m>=obs;m-=obs) {
      if (write(out,tmp,obs)!=obs) die("error while","write output");
      ++onumber;
      tmp+=obs;
    }
    if (m) {
      memcpy(obuf+obn,tmp,m);
      obn+=m;
    }
    if (b) break;
  }
  if (obn) write(out,obuf,obn);
  return 0;
}
