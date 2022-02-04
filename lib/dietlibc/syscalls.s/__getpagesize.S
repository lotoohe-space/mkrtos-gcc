#include "syscalls.h"

#ifndef __getpagesize__
#define __getpagesize__
#ifdef __NR_getpagesize
syscall(getpagesize,getpagesize)
#endif
#endif