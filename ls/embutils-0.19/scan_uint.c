
unsigned int scan_uint(const char* src,unsigned int* dest) {
  register const char *tmp=src;
  register unsigned int l=0;
  register unsigned char c;
  while ((c=*tmp-'0')<10) {
    unsigned int n;
    /* division is very slow on most architectures */
    n=l<<3; if ((n>>3)!=l) break;
    if (n+(l<<1) < n) break;
    n+=l<<1;
    if (n+c < n) break;
    l=n+c;
    ++tmp;
  }
  *dest=l;
  return tmp-src;
}
