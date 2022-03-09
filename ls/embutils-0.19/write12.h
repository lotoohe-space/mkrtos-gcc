#ifdef __dietlibc__
#include <write12.h>
#else
#include <string.h>
static inline int __write1(const char* s) {
  write(1,s,strlen(s));
}
static inline int __write2(const char* s) {
  write(2,s,strlen(s));
}
#endif
