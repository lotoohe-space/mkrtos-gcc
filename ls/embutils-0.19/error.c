#ifndef __GNUC__
#define __attribute__(xyz)
#endif

void warn(const char *message) {
  write(2,message,strlen(message));
  write(2,"\n",1);
}

void error(const char* message) __attribute__((noreturn));

void error(const char *message) {
  warn(message);
  exit(1);
}

