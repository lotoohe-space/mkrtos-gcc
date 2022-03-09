#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if (argc>=2) {
    int n=atoi(argv[1]);
    int fd=open("/dev/tty0",O_RDONLY);
    if (fd<0) fd=open("/dev/console",O_RDONLY);
    if (fd<0) return 1;
    if (ioctl(fd,VT_ACTIVATE,n) || ioctl(fd,VT_WAITACTIVE,n))
      return 1;
    close(fd);
    return 0;
  }
  return 1;
}
