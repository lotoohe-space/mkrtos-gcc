#ifndef _STRING_H__
#define _STRING_H__
#include "type.h"

int strncasecmp(const char *s1, const char *s2, register unsigned int n);

char * strdup(char *str)  ;
int 	strncmp(register const char *s1, register const char *s2, int n);
char* strncpy(char * __restrict s1, register const char * __restrict s2,
				int n);
int 	memcmp(const void *s1, const void *s2, int n);
void *memset(void *s, int c, int n);
char *strstr(const char *s1, const char *s2);
char* strcat(char * __restrict s1, register const char * __restrict s2);
char *strcpy(char * __restrict s1, const char * __restrict s2);
int 	strcmp(register const char *s1, register const char *s2);
void *memcpy(void * __restrict s1, const void * __restrict s2, unsigned int n);
int 	strlen(const char *s);
void *memmove(void *s1, const void *s2, int n);

char *strchr(register const char *s, int c);
#endif
