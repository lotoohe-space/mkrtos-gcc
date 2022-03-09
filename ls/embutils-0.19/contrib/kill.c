/*
    kill - terminate or signal processes
    Copyright (C) 2001  Andreas Krennmair <a.krennmair@aon.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

char * signame_tbl[] = {
  "Unknown signal", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", /*  0 -  6 */
  "BUS", "FPE", "KILL", "USR1", "SEGV", "USR2", "PIPE",          /*  7 - 13 */
  "ALRM", "TERM", "STKFLT", "CHLD", "CONT", "STOP", "TSTP",      /* 14 - 20 */
  "TTIN", "TTOU", "URG", "XCPU", "XFSZ", "VTALRM", "PROF",       /* 21 - 27 */
  "WINCH", "POLL", "PWR", "SYS", NULL };                         /* 28 - 31 */

#define MAX_SIGNAL 31

#define PROGNAME "kill"


void error_msg(const char * str) {
  write(2,str,strlen(str));
}

void print_syserr(int e) {
  switch (e) {
    case EINVAL:
      error_msg("Invalid signal.\n");
      break;
    case ESRCH:
      error_msg("PID or process group does not exist.\n");
      break;
    case EPERM:
      error_msg("Permission denied.\n");
      break;
    default:
      error_msg("Unknown error.\n");
      /* FALL THROUGH */
  }
}

void usage(void) {
  error_msg(PROGNAME ": usage: " PROGNAME " [OPTION] PID...\n"
                 "    -s SIGNAL_NAME    send signal SIGNAL to PIDs\n"
                 "    -l [EXIT_STATUS]  print signal name(s)\n"
                 "    -SIGNAL_NUMBER    send signal SIGNAL_NUMBER to PIDs\n"
                 "    -SIGNAL_NAME      send signal SIGNAL_NAME to PIDs\n");
  exit(EXIT_FAILURE);
}

void invalid_pid(void) {
  error_msg(PROGNAME ": invalid PID.\n");
  exit(EXIT_FAILURE);
}

void invalid_signal(void) {
  error_msg(PROGNAME ": invalid signal.\n");
  exit(EXIT_FAILURE);
}

int lookup_signame(char * signame) {
  int i;
  for (i=1;i<MAX_SIGNAL+1;i++) {
    if (strcmp(signame,signame_tbl[i])==0)
      return i;
  }
  invalid_signal();
  /* NOT REACHED */
  return EXIT_FAILURE;
}

void kill_pids(int sig, int pos, char * argv[]) {
  int rc;
  char * endptr;
  pid_t pid;
  while (argv[pos]!=NULL) {
    pid = strtol(argv[pos],&endptr,10);
    if (endptr==argv[pos])
      invalid_pid();
    rc = kill(pid,sig);
    if (rc!=0) {
      error_msg(PROGNAME ": couldn't kill process ");
      error_msg(argv[pos]);
      error_msg(": ");
      print_syserr(errno);
    }
    pos++;
  }
}

int main(int argc, char * argv[]) {
  int sig;
  char * signame;

  if (argc==1)
    usage();
  else if (strcmp(argv[1],"-l")==0) {
    if (argv[2]!=NULL) {
      sig = strtol(argv[2],NULL,10);
      if (sig<1 || sig>31)
        invalid_signal();
      write(1,signame_tbl[sig],strlen(signame_tbl[sig]));
      exit(EXIT_SUCCESS);
    } else {
      int j;
      for (j = 1; j<MAX_SIGNAL+1; j++) {
        write(1,signame_tbl[j],strlen(signame_tbl[j]));
        write(1," ",1);
      }
      write(1,"\n",1);
      exit(EXIT_SUCCESS);
    } 
  } else if (strcmp(argv[1],"-s")==0) {
    signame = argv[2];
    if (signame==NULL)
      usage();
    if (argv[3]==NULL)
      usage();
    sig = lookup_signame(signame);
    kill_pids(sig,3,argv);
  } else if (argv[1][0]=='-') {
    if (argv[1][1]>='0' && argv[1][1]<='9') {
      sig = strtol(argv[1]+1,NULL,10);
      if (sig<2) {
        invalid_pid();
      }
      kill_pids(sig,2,argv);
    } else if ((argv[1][1]>='A' && argv[1][1]<='Z') ||
               (argv[1][1]>='a' && argv[1][1]<='z')) {
      signame = argv[1]+1;
      sig = lookup_signame(signame);
      kill_pids(sig,2,argv);
    } else {
      invalid_signal();
    }
  } else {
    sig = SIGTERM;
    kill_pids(sig,1,argv);
  }
  return EXIT_SUCCESS;
}
