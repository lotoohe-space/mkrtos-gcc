#include <unistd.h>

int main() {
  write(1,"\e[H\e[J",6);
  return 0;
}
