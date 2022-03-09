unsigned int fmt_ulong(char *dest,unsigned long int i);
unsigned int fmt_long(char *dest,long int i) {
  if (i<0) {
    if (dest) *dest++='-';
    return fmt_ulong(dest,-i)+1;
  } else
    return fmt_ulong(dest,i);
}
