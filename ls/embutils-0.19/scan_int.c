
unsigned int scan_int(const char *src,int *dest) {
  register const char *tmp;
  register int l;
  register unsigned char c;
  int neg;
  tmp=src; l=0; neg=0;
  switch (*tmp) {
  case '-': neg=1;
  case '+': ++tmp;
  }
  while ((c=*tmp-'0')<10) {
    l=l*10+c;
    ++tmp;
  }
  *dest=(neg?-l:l);
  return tmp-src;
}
