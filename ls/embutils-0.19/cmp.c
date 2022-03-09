#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "write12.h"

#define BUF_SIZE 2048

int option_s = 0, option_l = 0;

static void usage(char * argv0) {
  __write2(argv0);
  __write2(": usage: ");
  __write2(argv0);
  __write2(" [-ls] file1 file2\n");
  exit(0);
}

static void write1_uint_r(unsigned int i, int base, int level, int pad) {
  if (i==0) {
    int j;
    for (j=0;j<pad-(level?level:1);j++) {
      __write1(" ");
    }
    if (level==0)
      __write1("0");
  } else {
    char c[2];
    c[0] = i%base+'0'; c[1]=0;
    write1_uint_r(i/base,base,level+1,pad);
    __write1(c);
  }
}

/* only works if base <= 10 */
static void write1_uint(unsigned int i, int base, int pad) {
  write1_uint_r(i,base,0, pad);
}

static void print_l(off_t byte_num, unsigned char b1, unsigned char b2) {
  write1_uint(byte_num,10,6);
  __write1(" ");
  write1_uint(b1,8,3);
  __write1(" ");
  write1_uint(b2,8,3);
  __write1("\n");
}

static void print_normal(char * f1, char * f2, off_t byte_num, int line_num) {
  __write1(f1);
  __write1(" ");
  __write1(f2);
  __write1(" differ: char ");
  write1_uint(byte_num,10,0);
  __write1(", line ");
  write1_uint(line_num,10,0);
  __write1("\n");
}

int main(int argc, char * argv[]) {
  int i, fd1, fd2, rc1, rc2, different = 0;
  unsigned long count = 0;
  char * file1, * file2;
  unsigned char cb1[BUF_SIZE], cb2[BUF_SIZE];
  int line = 1;
  if (argc<3) {
    usage(argv[0]);
  }
  for (i=1;i<argc;i++) {
    if (argv[i][0]=='-') {
      if (argv[i][1]==0) break;
      if (argv[i][1]=='s' && argv[i][2]=='\0') {
        option_s = 1;
      } else if (argv[i][1]=='l' && argv[i][2]=='\0') {
        option_l = 1;
      }
    } else {
      break;
    }
  }
  if ((file1 = argv[i])==NULL || (file2 = argv[i+1])==NULL)
    usage(argv[0]);
  fd1 = strcmp(file1,"-")?open(file1,O_RDONLY):0;
  fd2 = strcmp(file2,"-")?open(file2,O_RDONLY):0;
  if (fd1 < 0 || fd2 < 0) {
    __write2("unable to open file ");
    __write2((fd1<0)?file1:file2);
    return 1;
  }
  while (1) {
    rc1 = read(fd1,&cb1,sizeof(cb1));
    rc2 = read(fd2,&cb2,sizeof(cb2));
    if (rc1==0 && rc2==0) {
      return different?1:0;
    }
    if ((rc1>0 && rc2==0) || (rc1==0 && rc2>0)) {
      if (!option_s) {
        __write2(argv[0]);
        __write2(": EOF on ");
        __write2((rc1>rc2)?file2:file1);
        __write2("\n");
      }
      return 1;
    }
    for (i=0;i<((rc1>rc2)?rc2:rc1);i++) {
      count++;
      if (cb1[i] != cb2[i]) {
        different = 1;
        if (!option_s) {
          if (option_l) {
            print_l(count,cb1[i],cb2[i]);
          } else {
            print_normal(file1,file2,count,line);
            return 1;
          }
        } else
          return 1;
      }
      if (cb1[i]=='\n')
        line++;
    }
  }
  return 0;
}
