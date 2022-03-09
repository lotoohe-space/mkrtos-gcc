
/* split buf into n strings that are separated by c.  return n as *len.
 * Allocate plus more slots and leave the first ofs of them alone. */
char **split(char *buf,int c,int *len,int plus,int ofs) {
  int n=1;
  char **v=0;
  char **w;
  /* step 1: count tokens */
  char *s;
  for (s=buf; *s; s++) if (*s==c) n++;
  /* step 2: allocate space for pointers */
  v=(char **)malloc((n+plus)*sizeof(char*));
  if (!v) return 0;
  w=v+ofs;
  *w++=buf;
  for (s=buf; ; s++) {
    while (*s && *s!=c) s++;
    if (*s==0) break;
    if (*s==c) {
      *s=0;
      *w++=s+1;
    }
  }
  *len=w-v;
  return v;
}
