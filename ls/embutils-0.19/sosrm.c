#include <unistd.h>

int main(int argc,char* argv[]) {
  int i;
  for (i=1; i<argc; ++i) {
    if (unlink(argv[i])) {
      write(2,"fail: ",6);
      write(2,argv[i],strlen(argv[i]));
      write(2,"\n",1);
    }
  }
  return 0;
}
