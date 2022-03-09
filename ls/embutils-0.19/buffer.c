typedef struct buffer {
  char *x;
  unsigned int p;
  unsigned int n;
  int fd;
  int (*op)();
} buffer;

#define BUFFER_INIT(op,fd,buf,len) { (buf), 0, (len), (fd), (op) }
#define BUFFER_INSIZE 8192
#define BUFFER_OUTSIZE 8192

#ifndef DONT_NEED_BUFFER_1
char buffer_1_space[BUFFER_INSIZE];
static buffer it = BUFFER_INIT((void*)write,1,buffer_1_space,sizeof buffer_1_space);
buffer *buffer_1 = &it;
#endif

#ifdef WANT_BUFFER_2
char buffer_2_space[BUFFER_INSIZE];
static buffer it2 = BUFFER_INIT((void*)write,2,buffer_2_space,sizeof buffer_2_space);
buffer *buffer_2 = &it2;
#endif

int buffer_flush(buffer* b) {
  register int p;
  if (!(p=b->p)) return 0;
  b->p=0;
  return b->op(b->fd,b->x,p);
}

int buffer_putc(buffer *b,char c) {
  if (b->p>=b->n) if (buffer_flush(b)<0) return -1;
  b->x[b->p++]=c;
  return 0;
}

int buffer_puts(buffer *b,const char* s) {
  for (; *s; ++s)
    if (buffer_putc(b,*s)<0) return -1;
  return 0;
}

#ifndef DONT_NEED_PUT
int buffer_put(buffer *b,char* s,int len) {
  int i;
  for (i=0; i<len; ++i)
    if (buffer_putc(b,s[i])<0) return -1;
  return 0;
}
#endif

#ifndef DONT_NEED_PUTULONG
int buffer_putulong(buffer *b,unsigned long l) {
  char buf[50];
  return buffer_put(b,buf,fmt_ulong(buf,l));
}
#endif

static int buffer_putnlflush(buffer* b) {
  buffer_putc(b,'\n');
  buffer_flush(b);
}
