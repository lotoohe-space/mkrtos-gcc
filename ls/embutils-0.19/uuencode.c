#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "write12.h"

const char uu_table[64] = {
  '`', '!', '"', '#', '$', '%', '&', '\'',
  '(', ')', '*', '+', ',', '-', '.', '/',
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', ':', ';', '<', '=', '>', '?',
  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
  'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
  'X', 'Y', 'Z', '[', '\\', ']', '^', '_'
};

#define ENC(x)    (uu_table[(x) & 077])

static inline write1_char(char c)
{
  return write(1, &c, 1);
}

static inline void write1_octal(unsigned int mode)
{
  char c;
  if (mode != 0) {
    c = (mode % 8) + '0';
    write1_octal(mode / 8);
    write1_char(c);
  }
}

static inline void encode(void)
{
  register int ch, n;
  register char *p;
  char buf[80];

  while (1) {
    n = 0;
    do {
      register int m = read(0, buf, 45 - n);
      if (m == 0)
        break;
      n += m;
    }
    while (n < 45);

    if (n == 0)
      break;

    if (write1_char(ENC(n)) < 1)
      break;
    for (p = buf; n > 2; n -= 3, p += 3) {
      ch = *p >> 2;
      ch = ENC(ch);
      if (write1_char(ch) < 1)
        break;
      ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
      ch = ENC(ch);
      if (write1_char(ch) < 1)
        break;
      ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
      ch = ENC(ch);
      if (write1_char(ch) < 1)
        break;
      ch = p[2] & 077;
      ch = ENC(ch);
      if (write1_char(ch) < 1)
        break;
    }

    if (n != 0)
      break;

    if (write1_char('\n') < 1)
      break;
  }

  while (n != 0) {
    char c1 = *p;
    char c2 = n == 1 ? 0 : p[1];

    ch = c1 >> 2;
    ch = ENC(ch);
    if (write1_char(ch) < 1)
      break;

    ch = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
    ch = ENC(ch);
    if (write1_char(ch) < 1)
      break;

    if (n == 1)
      ch = ENC('\0');
    else {
      ch = (c2 << 2) & 074;
      ch = ENC(ch);
    }
    if (write1_char(ch) < 1)
      break;
    ch = ENC('\0');
    if (write1_char(ch) < 1)
      break;
    write1_char('\n');
    break;
  }

  write1_char(ENC('\0'));
  write1_char('\n');
}

static inline void usage(void)
{
  __write2("usage: uuencode [file] decode_pathname\n");
  exit(1);
}

int main(int argc, char * argv[]) {
  int opt;
  struct stat sb;
  int mode;
  int fd;

  switch (argc) {
  case 3:

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
      __write2("error: file ");
      __write2(argv[1]);
      __write2(" doesn't exist.\n");
      return EXIT_FAILURE;
    }
    close(0);
    dup(fd);
    fstat(0, &sb);

    mode = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    break;

  case 2:
    mode = 0664 & ~umask(0664);
    break;

  /* case 0: */
  default:
    usage();
  }

  __write1("begin ");
  write1_octal(mode);
  __write1(" ");
  __write1(argv[argc-1]);
  __write1("\n");
  encode();
  __write1("end\n");
  exit(EXIT_SUCCESS);
}
