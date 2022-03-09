unsigned int fmt_8long(char *dest,unsigned long i) {
  register unsigned long len,tmp,len2;
  for (len=1, tmp=i; tmp>7; ++len) tmp/=8;
  if (dest)
    for (tmp=i, dest+=len, len2=len+1; --len2; tmp/=8)
      *--dest = (tmp%8)+'0';
  return len;
}

