unsigned int fmt_str(char *out,const char *in) {
  register char* s=out;
  register const char* t=in;
  for (;;) {
    if (!*t) break; if (out) *s=*t; ++s; ++t;
    if (!*t) break; if (out) *s=*t; ++s; ++t;
    if (!*t) break; if (out) *s=*t; ++s; ++t;
    if (!*t) break; if (out) *s=*t; ++s; ++t;
  }
  return s-out;
}

