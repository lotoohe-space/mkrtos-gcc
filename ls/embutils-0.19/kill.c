#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "fmt_ulong.c"
#include "fmt_str.c"

static const char usage[] =
"usage: kill [-signal] [ pid [ pid [...] ] ]\ninvalid argument\n";

#if defined(__hppa__)
static const char strsig[] =
/*0        1         2         3         4         5         6         7   */
/*123456789012345678901234567890123456789012345678901234567890123456789012 * 0-72 */
"HUP INT QUIT ILL TRAP ABRT EMT FPE KILL BUS SEGV SYS PIPE ALRM TERM USR1\n"
/*      8         9        10        11        12        13        14        15 */
/*45678901234567890123456789012345678901234567890123456789012345678901234567890 * 73-150 */
"USR2 CHLD PWR VTALRM PROF POLL WINCH STOP TSTP CONT TTIN TTOU URG LOST UNUSED\n"
/*       16        17        18           */
/*234567890123456789012345678901 2 345678 * 151-188 */
"UNUSED XCPU XFSZ UNUSED STKFLT\n\0IOTCLD";
#define STR_POS(a,s,m,h) h
#elif defined(__mips__)
static const char strsig[] =
/*0        1         2         3         4         5         6         7   */
/*123456789012345678901234567890123456789012345678901234567890123456789012 * 0-72 */
"HUP INT QUIT ILL TRAP ABRT EMT FPE KILL BUS SEGV SYS PIPE ALRM TERM USR1\n"
/*      8         9        10        11        12        13        14           */
/*456789012345678901234567890123456789012345678901234567890123456789012345678 9 * 73-149 */
"USR2 CHLD PWR WINCH URG POLL STOP TSTP CONT TTIN TTOU VTALRM PROF XCPU XFSZ\n\0"
/*15      16  */
/*12345678901 * 150-161 */
"IOTCLDUNUSED";
#define STR_POS(a,s,m,h) m
#elif defined(__sparc__) || defined(__alpha__)
static const char strsig[] =
/*0        1         2         3         4         5         6         7   */
/*12345678901234567890123456789012345678901234567890123456789012345678901 * 0-71 */
"HUP INT QUIT ILL TRAP ABRT EMT FPE KILL BUS SEGV SYS PIPE ALRM TERM URG\n"
/*       8         9        10        11        12        13        14            */
/*3456789012345678901234567890123456789012345678901234567890123456789012345678 9  * 72-149 */
"STOP TSTP CONT CHLD TTIN TTOU POLL XCPU XFSZ VTALRM PROF WINCH PWR USR1 USR2\n\0"
/*15      16      */
/*123456789012345 * 150-165 */
"IOTCLDUNUSEDINFO";
#define STR_POS(a,s,m,h) s
#else
static const char strsig[] =
/*0        1         2         3         4         5         6         7       */
/*1234567890123456789012345678901234567890123456789012345678901234567890123456 * 0-76 */
"HUP INT QUIT ILL TRAP ABRT BUS FPE KILL USR1 SEGV USR2 PIPE ALRM TERM STKFLT\n"
/*  8         9        10        11        12        13        14        15     */
/*89012345678901234567890123456789012345678901234567890123456789012345678901 2  * 77-152*/
"CHLD CONT STOP TSTP TTIN TTOU URG XCPU XFSZ VTALRM PROF WINCH POLL PWR SYS\n\0"
/*     16     */
/*45678901234 * 153-164 */
"IOTCLDUNUSED";
#define STR_POS(a,s,m,h) a
#endif

static const struct {
  unsigned char signal;
  unsigned char len;	/* length of string (in strsig) */
  unsigned char name;	/* start in strsig */
} signals[] = {
  { SIGHUP,	3, STR_POS(  0,  0,  0,  0) },
  { SIGINT,	3, STR_POS(  4,  4,  4,  4) },
  { SIGQUIT,	4, STR_POS(  8,  8,  8,  8) },
  { SIGILL,	3, STR_POS( 13, 13, 13, 13) },
  { SIGTRAP,	4, STR_POS( 17, 17, 17, 17) },
  { SIGABRT,	4, STR_POS( 22, 22, 22, 22) },
  { SIGIOT,	3, STR_POS(153,150,150,183) },
#if defined(SIGEMT)
  { SIGEMT,	3, STR_POS(  0, 27, 27, 27) },
#endif
  { SIGBUS,	3, STR_POS( 27, 40, 40, 40) },
  { SIGFPE,	3, STR_POS( 31, 31, 31, 31) },
  { SIGKILL,	4, STR_POS( 35, 35, 35, 35) },
  { SIGUSR1,	4, STR_POS( 40,139, 68, 68) },
  { SIGSEGV,	4, STR_POS( 45, 44, 44, 44) },
  { SIGUSR2,	4, STR_POS( 50,144, 73, 73) },
  { SIGPIPE,	4, STR_POS( 55, 53, 53, 53) },
  { SIGALRM,	4, STR_POS( 60, 58, 58, 58) },
  { SIGTERM,	4, STR_POS( 65, 63, 63, 63) },
#if defined(SIGSTKFLT)
  { SIGSTKFLT,	6, STR_POS( 70,  0,  0,175) },
#endif
  { SIGCHLD,	4, STR_POS( 77, 87, 78, 78) },
  { SIGCLD,	3, STR_POS(156,153,153,186) },
  { SIGCONT,	4, STR_POS( 82, 82,112,120) },
  { SIGSTOP,	4, STR_POS( 87, 72,102,108) },
  { SIGTSTP,	4, STR_POS( 92, 77,107,115) },
  { SIGTTIN,	4, STR_POS( 97, 92,117,125) },
  { SIGTTOU,	4, STR_POS(102, 97,122,130) },
  { SIGURG,	3, STR_POS(107, 68, 93,135) },
  { SIGXCPU,	4, STR_POS(111,107,139,158) },
  { SIGXFSZ,	4, STR_POS(116,112,144,163) },
  { SIGVTALRM,	6, STR_POS(121,117,127, 87) },
  { SIGPROF,	4, STR_POS(128,124,134, 94) },
  { SIGWINCH,	5, STR_POS(133,129, 87,104) },
  { SIGPOLL,	4, STR_POS(139,102, 97, 99) },
  { SIGIO,	2, STR_POS(153,150,150,183) },
  { SIGPWR,	3, STR_POS(144,135, 83, 83) },
  { SIGSYS,	3, STR_POS(148, 49, 49, 49) },
  { SIGUNUSED,	6, STR_POS(159,156,156,144) },
#if defined(SIGINFO)
  { SIGINFO,	4, STR_POS(  0,162,  0,  0) },
#endif
#if defined(SIGLOST)
  { SIGLOST,	4, STR_POS(  0,  0,  0,139) },
#endif
  { 0, 0, 0 },
};

char buf[128];
int main(int argc, char**argv)
{
  int sig=SIGTERM;
  register int i,pid;
  if ((argc>1)&&(argv[1][0]=='-')) {
    register char *p=(argv[1]+1);
    if (isdigit(*p)) {
      sig=atoi(p);
    }
    else if (*p=='l') {
      write(1,strsig,151);
      return 0;
    }
    else {
      for (i=0;signals[i].len;i++) {
	if (!memcmp(p,strsig+signals[i].name,signals[i].len)) {
	  sig=signals[i].signal;
	  break;
	}
      }
    }
    --argc;
    ++argv;
  }
  if ((argc==1)||(sig==0)) {
    write(1,usage,44);
  }
  for (i=1;i<argc;i++) {
    pid=atoi(argv[i]);
    if (kill(pid,sig)) {
      char *p=buf;
      memcpy(p,usage+7,5);
      p+=5;
      if (pid<0) {
	*p='-';
	++p;
	pid=-pid;
      }
      p+=fmt_ulong(p,pid);
      memcpy(p,usage+5,2);
      p+=2;

      if (errno==EPERM) p+=fmt_str(p,"operation not permitted\n");
      else if (errno==ESRCH) p+=fmt_str(p,"no such process\n");
      else p+=fmt_str(p,usage+45);
      write(1,buf,p-buf);
    }
  }
  return 0;
}
