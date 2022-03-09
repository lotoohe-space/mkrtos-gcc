#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "write12.h"
#include <sys/mman.h>

#include "fmt_ulong.c"
#include "fmt_8long.c"

int aflag = 0;
unsigned int nnumber = 4;
char tformat = 0;

static inline char tohex(char c) {
  return c>=10?c-10+'a':c+'0';
}

unsigned int fmt_xlong(char *dest,unsigned long i) {
  register unsigned long len,tmp;
  for (len=1, tmp=i; tmp>15; ++len) tmp>>=4;
    if (dest)
    for (tmp=i, dest+=len; ; ) {
      *--dest = tohex(tmp&15);
      if (!(tmp>>=4)) break;
  }
  return len;
}


int arr_is_printable(char arr[], int size) {
  int i;
  for (i=0;i<size;i++) {
    if (!isprint(arr[i])) {
      return 0;
    }
  }
  return 1;
}

int get_length(unsigned int offs, unsigned int base) {
  int len;
  for (len = 1; offs>=base; ++len)
    offs /= base;
  return len;
}

void print_offset(char format, unsigned int offs) {
  char * str;
  unsigned int len, i;
  switch (format) {
  case 'd':
    len = fmt_ulong(NULL,offs);
    str = alloca(len+1);
    str[fmt_ulong(str,offs)] = 0;
    break;
  case 'o':
    len = fmt_8long(NULL,offs);
    str = alloca(len+1);
    str[fmt_8long(str,offs)] = 0;
    break;
  case 'x':
    len = fmt_xlong(NULL,offs);
    str = alloca(len+1);
    str[fmt_xlong(str,offs)] = 0;
    break;
  }
  for (i=0;i<(7-len);i++) {
    __write1(" ");
  }
  __write1(str);
  __write1(" ");
}

char * find_end(char * cp, int pos, int max) {
  while (pos < max && isprint(cp[pos])) {
    pos++;
  }
  return cp + pos;
}

void process_file(int fd) {
  char * buf;
  int cursize;
  char curchar;
  int i;
  off_t fsize = lseek(fd,0,SEEK_END);
  buf = mmap(0,fsize,PROT_READ,MAP_PRIVATE,fd,0);
  if (buf < 0) {
    return;
  }
  for (i=0;i<(fsize-nnumber);i++) {
    int offset = i;
    if (arr_is_printable(buf+i,nnumber)) {
      if (tformat) {
        print_offset(tformat,offset);
      }
      i+=write(1,buf+i,find_end(buf,i,fsize)-(buf+i));
      __write1("\n");
    }
  }
  munmap(buf,fsize);
}

void usage(char * argv0) {
  __write2(argv0);
  __write2(": usage: ");
  __write2(argv0);
  __write2(" [-a][-t format][-n number][file ...]\n");
  exit(1);
}

int main(int argc, char * argv[]) {
  int i;
  int enc_fn = 0;
  for (i=1;i<argc;i++) {
    if (argv[i][0]=='-') {
      switch (argv[i][1]) {
      case 'a': 
        break; /* we will scan the whole file, anyway */
      case 'n':
        if (argv[i+1]!=NULL) {
          nnumber = strtoul(argv[i+1],NULL,10);
          i++;
        } else {
          usage(argv[0]);
        }
        break;
      case 't':
        if (argv[i+1]!=NULL) {
          char c = argv[i+1][0];
          if (c == 'd' || c=='o' || c=='x') {
            tformat = c;
            i++;
          } else {
            usage(argv[0]);
          }
        } else {
          usage(argv[0]);
        }
        break;
      default:
        usage(argv[0]);
      }
    } else {
      enc_fn = 1;
      break;
    }
  }
  if (!enc_fn) {
    process_file(0);
  } else {
    for (;i<argc;i++) {
      int fd = open(argv[i],O_RDONLY);
      if (fd < 0) {
        __write2("unable to open file `");
        __write2(argv[i]);
        __write2("'\n");
        exit(1);
      } else {
        process_file(fd);
      }
    }
  }
  return 0;
}
