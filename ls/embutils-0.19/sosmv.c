#include <unistd.h>

int main(int argc,char* argv[]) {
  if (argc==3) {
    if (rename(argv[1],argv[2])) {
      write(2,"fail\n",5);
      return 1;
    }
  }
  return 0;
}
