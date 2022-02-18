dnl aclocal.m4 generated automatically by aclocal 1.4-p5

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl This macro sets HAVE_POSIX_GETGROUPS if the
dnl getgroups() function accepts a zero first argument.
AC_DEFUN(RC_FUNC_GETGROUPS, [
	AC_CACHE_CHECK(for POSIX getgroups, rc_cv_func_posix_getgroups, AC_TRY_RUN([
#include <sys/types.h>
#include <unistd.h>
int main(void) {
	return getgroups(0, (void *)0) == -1;
}
	], rc_cv_func_posix_getgroups=yes, rc_cv_func_posix_getgroups=no, rc_cv_func_posix_getgroups=yes))
	case "$rc_cv_func_posix_getgroups" in
	yes) AC_DEFINE(HAVE_POSIX_GETGROUPS) ;;
	esac
])


dnl We can't use AC_CHECK_FUNCS for sigsetjmp(), since it's a macro in
dnl some places.
AC_DEFUN(RC_FUNC_SIGSETJMP, [
	AC_CACHE_CHECK(for sigsetjmp, rc_cv_sigsetjmp,
		AC_TRY_LINK([
#include <setjmp.h>
		], [
sigjmp_buf e;
sigsetjmp(e, 1);
		], rc_cv_sigsetjmp=yes, rc_cv_sigsetjmp=no))
	case "$rc_cv_sigsetjmp" in
	yes)	AC_DEFINE(HAVE_SIGSETJMP) ;;
	esac
])

dnl Similarly, AC_CHECK_FUNCS doesn't find strerror() on NetBSD.
AC_DEFUN(RC_FUNC_STRERROR, [
	AC_CACHE_CHECK(for strerror, rc_cv_strerror,
		AC_TRY_LINK([
#include <string.h>
		], [
strerror(0);
		], rc_cv_strerror=yes, rc_cv_strerror=no))
	case "$rc_cv_strerror" in
	yes)	AC_DEFINE(HAVE_STRERROR) ;;
	esac
])

dnl HPUX needs _KERNEL defined to pick up RLIMIT_foo defines.  (Why?)
AC_DEFUN(RC_NEED_KERNEL, [
	AC_CACHE_CHECK(if _KERNEL is required for RLIMIT defines, rc_cv_kernel_rlimit,
		AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/resource.h>
		], [
int f;
f = RLIMIT_DATA;
		], rc_cv_kernel_rlimit=no, [ AC_TRY_COMPILE([
#include <sys/types.h>
#define _KERNEL
#include <sys/resource.h>
#undef _KERNEL
			], [
int f;
f = RLIMIT_DATA;
			], rc_cv_kernel_rlimit=yes, rc_cv_kernel_rlimit=no)]))
	case "$rc_cv_kernel_rlimit" in
	yes)	AC_DEFINE(RLIMIT_NEEDS_KERNEL) ;;
	esac
])

dnl Look for rlim_t in sys/types.h and sys/resource.h
AC_DEFUN(RC_TYPE_RLIM_T, [
	AC_CACHE_CHECK(for rlim_t, rc_cv_have_rlim_t,
		AC_EGREP_CPP(rlim_t, [
#include <sys/types.h>
#if RLIMIT_NEEDS_KERNEL
#define _KERNEL
#endif
#include <sys/resource.h>
		], rc_cv_have_rlim_t=yes, rc_cv_have_rlim_t=no))

	case "$rc_cv_have_rlim_t" in
	yes)	AC_DEFINE(HAVE_RLIM_T) ;;
	no)	AC_CACHE_CHECK(for native quad_t, rc_cv_have_quad_t,
			AC_TRY_COMPILE([
#include <sys/types.h>
			], [
typedef quad_t align_t;
align_t a;
a = (quad_t)0;
			], rc_cv_have_quad_t=yes, rc_cv_have_quad_t=no))

		case "$rc_cv_have_quad_t" in
		yes)	AC_DEFINE(HAVE_QUAD_T)
			AC_CACHE_CHECK(if rlimit values are quad_t, rc_cv_rlim_t_is_quad_t,
				AC_TRY_RUN([
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#if RLIMIT_NEEDS_KERNEL
#define _KERNEL
#endif
#include <sys/resource.h>
#if RLIMIT_NEEDS_KERNEL
#undef _KERNEL
#endif
main(){
	struct rlimit rl;
	exit(sizeof rl.rlim_cur != sizeof(quad_t));
}
				], rc_cv_rlim_t_is_quad_t=yes, rc_cv_rlim_t_is_quad_t=no, $ac_cv_type_quad_t))

			case "$rc_cv_rlim_t_is_quad_t" in
			yes)	AC_DEFINE(RLIM_T_IS_QUAD_T) ;;
			esac
			;;
		esac
		;;
	esac
])


dnl Check type of sig_atomic_t.
AC_DEFUN(RC_TYPE_SIG_ATOMIC_T, [
	AC_CACHE_CHECK(for sig_atomic_t, rc_cv_sig_atomic_t,
		AC_EGREP_HEADER(sig_atomic_t, signal.h,
			rc_cv_sig_atomic_t=yes, rc_cv_sig_atomic_t=no))
	case "$rc_cv_sig_atomic_t" in
	no)	AC_DEFINE(sig_atomic_t, int) ;;
	esac
])


dnl Check for sigaction and SA_INTERRUPT
AC_DEFUN(RC_FUNC_SIGACTION, [
	AC_CACHE_CHECK(for sigaction and SA_INTERRUPT, rc_cv_sa_int,
		AC_TRY_COMPILE([
#include <signal.h>
		], [
struct sigaction foo;
foo.sa_flags = SA_INTERRUPT;
sigaction(SIGINT, 0, 0);
		], rc_cv_sa_int=yes, rc_cv_sa_int=no
		)
	)
])


dnl Do we have SysV SIGCLD semantics?  In other words, if we set the
dnl action for SIGCLD to SIG_IGN does wait() always say ECHILD?  Linux,
dnl of course, is bizarre here.  It basically implements the SysV
dnl semantics, but if the parent calls wait() before the child calls
dnl exit(), wait() returns with the PID of the child as normal.  (Real
dnl SysV waits for all children to exit, then returns with ECHILD.)
dnl Anyway, this is why the `sleep(1)' is there.
AC_DEFUN(RC_SYS_V_SIGCLD, [
	AC_CACHE_CHECK(for SysV SIGCLD semantics, rc_cv_sysv_sigcld,
		AC_TRY_RUN([
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {
	int i;
	signal(SIGCLD, SIG_IGN);
	switch (fork()) {
	case -1:
		return 1;
	case 0:
		return 0;
	default:
		sleep(1);
		if (wait(&i) == -1 && errno == ECHILD) return 0;
		else return 1;
	}
}
		], rc_cv_sysv_sigcld=yes, rc_cv_sysv_sigcld=no, rc_cv_sysv_sigcld=yes))
	case "$rc_cv_sysv_sigcld" in
	yes)	AC_DEFINE(HAVE_SYSV_SIGCLD) ;;
	esac
])


dnl Do we have /dev/fd or /proc/self/fd?
AC_DEFUN(RC_SYS_DEV_FD, [
	AC_CACHE_CHECK(for /dev/fd, rc_cv_sys_dev_fd,
		if test -d /dev/fd && test -r /dev/fd/0; then
			rc_cv_sys_dev_fd=yes
		elif test -d /proc/self/fd && test -r /proc/self/fd/0; then
			rc_cv_sys_dev_fd=odd
		else
			rc_cv_sys_dev_fd=no
		fi
	)
])


dnl Can mknod make FIFOs?
AC_DEFUN(RC_SYS_MKNOD_FIFO, [
	AC_CACHE_CHECK(for mknod FIFOs, rc_cv_sys_fifo,
		AC_TRY_RUN([
#include <sys/types.h>
#include <sys/stat.h>

main() {
	exit(mknod("/tmp/rc$$.0", S_IFIFO | 0666, 0) != 0);
}
		], rc_cv_sys_fifo=yes, rc_cv_sys_fifo=no, rc_cv_sys_fifo=no))
	rm -f /tmp/rc$$.0
	case "$rc_cv_sys_fifo" in
	yes)	AC_DEFINE(HAVE_FIFO) ;;
	esac
])

dnl Where is tgetent()?
AC_DEFUN(RC_LIB_TGETENT, [
	AC_CHECK_LIB(termcap, tgetent,
		rc_lib_tgetent=-ltermcap,
		AC_CHECK_LIB(ncurses, tgetent,
			rc_lib_tgetent=-lncurses,
			AC_MSG_ERROR(tgetent not found)
		)
	)
])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN([AM_INIT_AUTOMAKE],
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN([AM_SANITY_CHECK],
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN([AM_MISSING_PROG],
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN([AM_CONFIG_HEADER],
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])


# serial 1

# @defmac AC_PROG_CC_STDC
# @maindex PROG_CC_STDC
# @ovindex CC
# If the C compiler in not in ANSI C mode by default, try to add an option
# to output variable @code{CC} to make it so.  This macro tries various
# options that select ANSI C on some system or another.  It considers the
# compiler to be in ANSI C mode if it handles function prototypes correctly.
#
# If you use this macro, you should check after calling it whether the C
# compiler has been set to accept ANSI C; if not, the shell variable
# @code{am_cv_prog_cc_stdc} is set to @samp{no}.  If you wrote your source
# code in ANSI C, you can make an un-ANSIfied copy of it by using the
# program @code{ansi2knr}, which comes with Ghostscript.
# @end defmac

AC_DEFUN([AM_PROG_CC_STDC],
[AC_REQUIRE([AC_PROG_CC])
AC_BEFORE([$0], [AC_C_INLINE])
AC_BEFORE([$0], [AC_C_CONST])
dnl Force this before AC_PROG_CPP.  Some cpp's, eg on HPUX, require
dnl a magic option to avoid problems with ANSI preprocessor commands
dnl like #elif.
dnl FIXME: can't do this because then AC_AIX won't work due to a
dnl circular dependency.
dnl AC_BEFORE([$0], [AC_PROG_CPP])
AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(am_cv_prog_cc_stdc,
[am_cv_prog_cc_stdc=no
ac_save_CC="$CC"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc -D__EXTENSIONS__
for ac_arg in "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" "-Xc -D__EXTENSIONS__"
do
  CC="$ac_save_CC $ac_arg"
  AC_TRY_COMPILE(
[#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Most of the following tests are stolen from RCS 5.7's src/conf.sh.  */
struct buf { int x; };
FILE * (*rcsopen) (struct buf *, struct stat *, int);
static char *e (p, i)
     char **p;
     int i;
{
  return p[i];
}
static char *f (char * (*g) (char **, int), char **p, ...)
{
  char *s;
  va_list v;
  va_start (v,p);
  s = g (p, va_arg (v,int));
  va_end (v);
  return s;
}
int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};
int pairnames (int, char **, FILE *(*)(struct buf *, struct stat *, int), int, int);
int argc;
char **argv;
], [
return f (e, argv, 0) != argv[0]  ||  f (e, argv, 1) != argv[1];
],
[am_cv_prog_cc_stdc="$ac_arg"; break])
done
CC="$ac_save_CC"
])
if test -z "$am_cv_prog_cc_stdc"; then
  AC_MSG_RESULT([none needed])
else
  AC_MSG_RESULT($am_cv_prog_cc_stdc)
fi
case "x$am_cv_prog_cc_stdc" in
  x|xno) ;;
  *) CC="$CC $am_cv_prog_cc_stdc" ;;
esac
])

# Define a conditional.

AC_DEFUN([AM_CONDITIONAL],
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

