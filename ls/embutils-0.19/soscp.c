#include <unistd.h>
#include <fcntl.h>

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

int main(int argc,char* argv[]) {
  int a,b;
  if (argc==3) {
    a=open(argv[1],O_RDONLY|O_LARGEFILE);
    b=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE,0666);
    if (a==-1 || b==-1) {
      write(2,"openfail\n",9);
      return 1;
    }
    {
      char buf[1024];
      int len,written;
      for (;;) {
	len=read(a,buf,1024);
	if (len<0) {
	  write(2,"readfail\n",9);
	  return 1;
	}
	if (!len) break;
	written=write(b,buf,len);
	if (written!=len) {
	  write(2,"writefail\n",10);
	  return 1;
	}
      }
    }
    close(a);
    close(b);
  }
  return 0;
}
