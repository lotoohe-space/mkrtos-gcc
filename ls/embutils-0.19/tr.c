#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

void pf(const char*msg) { write(2,msg,strlen(msg)); }
void die(const char*msg) { pf(msg); exit(1); }

unsigned int set1_len=0,set2_len=0;
unsigned char trans_tbl[256];

unsigned char buf1[4096],buf2[4096];
int len;

int complement_of_set1() {
  register int i;
  for (i=0;i<256;++i) trans_tbl[i]=1;
  for (i=0;i<set1_len;++i) trans_tbl[buf1[i]]=0;
  set1_len=0;
  for (i=0;i<256;++i) {
    if (trans_tbl[i]) {
      buf1[set1_len]=i;
      ++set1_len;
    }
  }
}

void translate_init() {
  register int i;
  for (i=0;i<256;++i) trans_tbl[i]=i;
  if (set1_len>set2_len) {
    for (;set2_len<set1_len;++set2_len) {
      buf2[set2_len]=buf2[set2_len-1];
    }
  }
  for (i=0;i<set1_len;++i) trans_tbl[buf1[i]]=buf2[i];
}

int translate() {
  register int i;
  for(i=0;i<len;++i) buf2[i]=trans_tbl[buf1[i]];
  return len;
}

void delete_init() {
  register int i;
  for (i=0;i<256;++i) trans_tbl[i]=0;
  for (i=0;i<set1_len;++i) trans_tbl[buf1[i]]=1;
}

int delete() {
  int i,j;
  unsigned char ch;
  for(j=i=0;i<len;++i) {
    ch=buf1[i];
    if (!trans_tbl[ch]) {
      buf2[j]=ch; ++j;
    }
  }
  return j;
}

unsigned char squeeze_pch,squeeze_init=0;
int squeeze() {
  int i,j;
  unsigned char ch;
  for(j=i=0;i<len;++i) {
    ch=buf1[i];
    if (trans_tbl[ch]) {
      if (!squeeze_init) {
	squeeze_pch=ch;
	squeeze_init=1;
	goto put_in;
      }
      else if (ch!=squeeze_pch) {
	squeeze_pch=ch;
	goto put_in;
      }
    }
    else {
      squeeze_init=0;
put_in:
      buf2[j]=ch; ++j;
    }
  }
  return j;
}

int get_next(unsigned char*arg) {
  int ret=*arg;
  if (ret=='\\') {
    if ((ret=arg[1])) {
      if (ret=='a') ret='\a';
      else if (ret=='b') ret='\b';
      else if (ret=='f') ret='\f';
      else if (ret=='n') ret='\n';
      else if (ret=='r') ret='\r';
      else if (ret=='t') ret='\t';
      else if (ret=='v') ret='\v';
      else if (ret=='\\') ret='\\';
      else if (ret!='-') {
	int k;
	unsigned int val=0;
	for (k=0;k<3;++k) {
	  if ((ret|7)!=0x37) goto p_err;
	  val=(val<<3)+(ret&7);
	  ret=arg[k+2];
	}
	if (val>=256) {
p_err:	  die("set parse error\n");
	}
	ret=val|0x200;
      }
      ret|=0x100;
    }
    else ret='\\';
  }
  return ret;
}
int parse_set(unsigned char set[256],char*arg) {
  int _dash=0,j=0,tmp=get_next(arg);
  unsigned char ch=tmp&0xff;
  arg+=1+(tmp>>8);
  while ((tmp=get_next(arg))) {
    arg+=1+(tmp>>8);
    if (_dash) {
      unsigned char max=tmp&0xff;
      for (;ch<max;++ch) set[j++]=ch;
      ch=max;
      _dash=0;
    }
    else if (tmp=='-') _dash=1;
    else if (tmp=='\\') set[j]='\\';
    else {
      set[j++]=ch;
      ch=tmp&0xff;
    }
  }
  set[j++]=(_dash)?'-':ch;
  return j;
}

void usage() {
  die("usage: tr [-c] [-d] [-s] [-t] set1 [set2]\n");
}

int flag_complement;

int (*action)();

int main(int argc,char*argv[]) {
  int func_delete=0,func_squeeze=0,flag_truncate=0;
  int i;
  if (argc>1) {
    for (i=1;i<argc;++i) {
      if (argv[i][0]=='-') {
	unsigned char ch=argv[i][1];
	if (ch=='c') flag_complement=1;
	else if (ch=='d') func_delete=1;
	else if (ch=='s') func_squeeze=1;
	else if (ch=='t') flag_truncate=1;
	else if (ch!=0) usage();
	else goto isset;
      }
      else {
isset:
	set1_len=parse_set(buf1,argv[i]);
	if (++i<argc) {
	  set2_len=parse_set(buf2,argv[i]);
	}
	break;
      }
    }
  }
  if (set1_len==0) usage();

  if (flag_complement) complement_of_set1();

  if (func_delete) {
    action=delete;
    delete_init();
  }
  else if (func_squeeze) {
    action=squeeze;
    delete_init();
  }
  else {
    if (set2_len==0) die("need set2 for translations\n");
    if ((flag_truncate)&&(set1_len>set2_len)) set1_len=set2_len;
    action=translate;
    translate_init();
  }

  while ((len=read(0,buf1,sizeof(buf1)))>0) {
    register int i=action();
    write(1,buf2,i);
  }

  return 0;
}

