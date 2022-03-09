#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "write12.h"


#define DEC(Char) (((Char) - ' ') & 077)

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

int read_line(char *buf, int size, int fd)
{
  char c;
  int count = 0;
  int rc;
  rc = read(fd, &c, 1);
  while (rc != 0 && c != '\n' && count < (size - 1)) {
    *buf++ = c;
    count++;
    rc = read(fd, &c, 1);
  }
  *buf = '\0';
  return count;
}

static inline write1_char(char c)
{
  return write(1, &c, 1);
}

static int read_stduu(const char *inname)
{
  char buf[2 * BUFSIZ];

  while (1) {
    int n;
    char *p;

    if (read_line(buf, sizeof(buf), 0) == 0) {
      __write2(inname);
      __write2(": Short file");
      exit(1);
      return 1;
    }
    p = buf;

    /* N is used to avoid writing out all the characters at the end of
       the file.  */

    n = DEC(*p);
    if (n <= 0)
      break;
    for (++p; n > 0; p += 4, n -= 3) {
      char ch;

      if (n >= 3) {
        ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
        write1_char(ch);
        ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
        write1_char(ch);
        ch = DEC(p[2]) << 6 | DEC(p[3]);
        write1_char(ch);
      } else {
        if (n >= 1) {
          ch = DEC(p[0]) << 2 | DEC(p[1]) >> 4;
          write1_char(ch);
        }
        if (n >= 2) {
          ch = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
          write1_char(ch);
        }
      }
    }
  }

  if (read_line(buf, sizeof(buf), 0) == 0 || strcmp(buf, "end")) {
    __write2(inname);
    __write2(": No `end' line\n");
    exit(1);
    return 1;
  }

  return 0;
}

static int isoctal(char c)
{
  return '0' <= c && c <= '7';
}

static int decode(const char *inname, const char *forced_outname)
{
  struct passwd *pw;
  register int n;
  register char *p;
  int mode = 0, n1;
  char buf[2 * BUFSIZ];
  char *outname;
  int do_base64 = 0;
  int fd;

  if (strcmp(inname,"stdin") && strcmp(inname,"-")) {
    fd = open(inname, O_RDONLY);

    if (fd < 0) {
      __write2("Couldn't open file ");
      __write2(inname);
      __write2("\n");
      exit(1);
    }

    close(0);
    dup(fd);
  } else {
    fd = 0;
  }

  /* Search for header line.  */

  while (1) {
    if (read_line(buf, sizeof(buf), 0) == 0) {
      __write2(inname);
      __write2(": No `begin' line\n");
      exit(1);
      return 1;
    }

    if (strncmp(buf, "begin ", 6) == 0) {
      char *mode_str = buf + 6;
      outname = buf + 10;
      if (isoctal(mode_str[0]) && isoctal(mode_str[1])
          && isoctal(mode_str[2])) {
        int i;
        for (i = 0; i < 3; i++) {
          mode = mode*8 + (mode_str[i] - '0');
        }
        break;
      }
    }
  }

  fd = open(outname,O_WRONLY|O_CREAT,mode);
  if (fd<0) {
    __write2("Error opening file ");
    __write2(outname);
    __write2(" for writing.\n");
    exit(1);
  }
  close(1);
  dup(fd);

  /* For each input line:  */
  return read_stduu(inname);
}

static void usage(void)
{
  __write2("usage: uudecode [ file ] ...\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int opt;
  int exit_status;
  const char *outname;
  int i;

  if (argc <= 1)
    exit_status =
        decode("stdin", outname) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  else {
    exit_status = EXIT_SUCCESS;
    for (i = 1; i <= argc; i++) {
      if (decode(argv[optind], outname) != 0) {
        exit_status = EXIT_FAILURE;
      }
    }
  }

  exit(exit_status);
}
