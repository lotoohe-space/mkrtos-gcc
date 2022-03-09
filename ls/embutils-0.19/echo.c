#include <stdio.h>
#include <string.h>
#include <unistd.h>

void  interpret_esc(unsigned char* src) {
  unsigned char *dst = src;
  do {
    if ( (*dst = *src) == '\\' ) {
      switch (*++src) {
      case 'a' : *dst = '\a'; break;
      case 'b' : *dst = '\b'; break;
      case 'f' : *dst = '\f'; break;
      case 'n' : *dst = '\n'; break;
      case 't' : *dst = '\t'; break;
      case 'v' : *dst = '\v'; break;
      case '\\': *dst = '\\'; break;
      default:
	*dst = 0;
	while ( (unsigned int)(*src - '0') < 8u )
	    *dst = *dst * 8 + (unsigned int)(*src++ - '0');
	src--;
	break;
      }
    }
    dst++;
  } while (*src++);
}

int main (int argc, char** argv) {
  static char  chars [2] = { '\n', ' ' };
  int          escape    = 0;
  int          blank     = 0;
  int          newline   = 1;

  while (*++argv) {
    write(1, chars+1, blank);
    if ( **argv == '-' ) {
      while ( *++*argv ) {
	switch ( **argv ) {
	case 'n': newline = 0; break;
	case 'e': escape  = 1; break;
	case 'E': escape  = 0; break;
	}
      }
    } else {
      if (escape)
	interpret_esc ( *argv );
      write(1, *argv, strlen(*argv));
      blank = 1;
    }
  }
  write(1,chars+0,newline);
  return 0;
}

