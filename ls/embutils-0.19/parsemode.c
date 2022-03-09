
void parsemode(char *m,mode_t *and,mode_t *or) {
  int mult,val;
  int add;
  char op;

  if (*m>='0' && *m<='7') {	/* octal number */
    int mode=0;
    do {
      mode=mode*8+*m++-'0';
    } while (*m>='0' && *m<='7');
    *and=0; *or=mode;
    return;
  }
  /* [ugoa]*{+|-|=}[rwxst]* */
  *and=(mode_t)-1; *or=0;
  while (*m) {
    for (mult=0; *m>='a' && *m<='z'; m++) {
      switch (*m) {
      case 'a': mult|=0111; break;
      case 'u': mult|=0100; break;
      case 'g': mult|=0010; break;
      case 'o': mult|=0001; break;
      default: error("[augo] expected");
      }
    }
    if (mult==0) mult=0111;
    switch (*m) {
    case '+': case '-': case '=': op=*m; break;
    default: error("{+|-|=} expected");
    }
    ++m;
    for (add=val=0; *m>='a'&&*m<='z'; m++) {
      switch (*m) {
      case 'r': val|=4; break;
      case 'w': val|=2; break;
      case 'x': val|=1; break;
      case 's':
	if (mult&0100) add|=04000;
	if (mult&0010) add|=02000;
	break;
      case 't': add|=01000; break;
      default: error("[rwxst] expected");
      }
    }
    switch (op) {
    case '+': *or |= (mult*val) | add; break;
    case '-': *and &= 0xffff-((mult*val) | add); break;
    case '=': *and &= 0xffff-mult*7;
	      *or &= 0xffff-mult*7;
	      *or |= (mult*val) | add; break;
    }
    if (*m==',')
      m++;
  }
}

